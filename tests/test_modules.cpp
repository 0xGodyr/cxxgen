#include "cxxgen.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace cxxgen;
using namespace cxxgen::modules;

// ============================================================================
// Story 12.1: Module Interface Dependency Scanner (P1689R5 & clang-scan-deps)
// ============================================================================

void test_scanner_strip_comments_and_strings() {
    std::cout << "Testing ModuleScanner comment and string literal stripping...\n";

    std::string code = "// export module fake_comment;\n"
                       "/* multi-line\n"
                       "   import fake_block;\n"
                       "*/\n"
                       "const char* str = \"export module fake_str;\";\n"
                       "const char* raw = R\"(import fake_raw;)\";\n"
                       "export module real_module;\n"
                       "import real_dep;\n";

    std::string stripped = ModuleScanner::strip_comments_and_strings(code);
    assert(stripped.find("fake_comment") == std::string::npos);
    assert(stripped.find("fake_block") == std::string::npos);
    assert(stripped.find("fake_str") == std::string::npos);
    assert(stripped.find("fake_raw") == std::string::npos);
    assert(stripped.find("real_module") != std::string::npos);
    assert(stripped.find("real_dep") != std::string::npos);

    std::cout << "[PASS] ModuleScanner stripping verified.\n";
}

void test_scanner_builtin_syntax() {
    std::cout << "Testing ModuleScanner built-in regex syntax detection...\n";

    // 1. Primary module interface
    {
        std::string src = "export module my_lib;\nexport void foo() {}\n";
        auto rule = ModuleScanner::scan_source_text(src, "my_lib.cppm");
        assert(rule.provides.size() == 1);
        assert(rule.provides[0].logical_name == "my_lib");
        assert(rule.provides[0].is_interface == true);
        assert(rule.required_modules.empty());
    }

    // 2. Module partition interface
    {
        std::string src = "export module my_lib:detail;\nexport void helper() {}\n";
        auto rule = ModuleScanner::scan_source_text(src, "detail.cppm");
        assert(rule.provides.size() == 1);
        assert(rule.provides[0].logical_name == "my_lib:detail");
        assert(rule.provides[0].is_interface == true);
    }

    // 3. Module implementation unit
    {
        std::string src = "module my_lib;\nvoid foo() {}\n";
        auto rule = ModuleScanner::scan_source_text(src, "my_lib.cpp");
        assert(rule.provides.size() == 1);
        assert(rule.provides[0].logical_name == "my_lib");
        assert(rule.provides[0].is_interface == false);
    }

    // 4. Imports: standard, export import, partition import, header unit
    {
        std::string src = "export module app;\n"
                          "export import dep_a;\n"
                          "import dep_b;\n"
                          "import :detail;\n"
                          "import <vector>;\n"
                          "import \"config.h\";\n";
        auto rule = ModuleScanner::scan_source_text(src, "app.cppm");
        assert(rule.provides.size() == 1);
        assert(rule.provides[0].logical_name == "app");
        assert(rule.required_modules.size() >= 5);

        bool has_dep_a = false, has_dep_b = false, has_detail = false, has_vector = false, has_config = false;
        for (const auto &req : rule.required_modules) {
            if (req.logical_name == "dep_a")
                has_dep_a = true;
            if (req.logical_name == "dep_b")
                has_dep_b = true;
            if (req.logical_name == ":detail")
                has_detail = true;
            if (req.logical_name == "<vector>")
                has_vector = true;
            if (req.logical_name == "\"config.h\"")
                has_config = true;
        }
        assert(has_dep_a && has_dep_b && has_detail && has_vector && has_config);
    }

    std::cout << "[PASS] ModuleScanner built-in regex syntax detection verified.\n";
}

void test_p1689_serialization_and_deserialization() {
    std::cout << "Testing P1689R5 JSON round-trip...\n";

    P1689Data original;
    original.version = 1;
    original.revision = 0;

    P1689Rule r1;
    r1.primary_output = "src/math.obj";
    r1.outputs = {"build/math.ifc"};
    r1.provides.push_back({.logical_name = "math", .source_path = "src/math.ixx", .is_interface = true});
    r1.required_modules.push_back({.logical_name = "std", .source_path = ""});
    r1.required_modules.push_back({.logical_name = "math:core", .source_path = ""});

    P1689Rule r2;
    r2.primary_output = "src/main.obj";
    r2.required_modules.push_back({.logical_name = "math", .source_path = ""});
    r2.required_modules.push_back({.logical_name = "std", .source_path = ""});

    original.rules.push_back(r1);
    original.rules.push_back(r2);

    std::string json_str = original.to_json();
    assert(!json_str.empty());
    assert(json_str.find("\"version\": 1") != std::string::npos);
    assert(json_str.find("\"logical-name\": \"math\"") != std::string::npos);
    assert(json_str.find("\"is-interface\": true") != std::string::npos);

    P1689Data parsed = P1689Data::from_json(json_str);
    assert(parsed.version == 1);
    assert(parsed.revision == 0);
    assert(parsed.rules.size() == 2);

    assert(parsed.rules[0].primary_output == "src/math.obj");
    assert(parsed.rules[0].outputs.size() == 1 && parsed.rules[0].outputs[0] == "build/math.ifc");
    assert(parsed.rules[0].provides.size() == 1);
    assert(parsed.rules[0].provides[0].logical_name == "math");
    assert(parsed.rules[0].provides[0].is_interface == true);
    assert(parsed.rules[0].required_modules.size() == 2);
    assert(parsed.rules[0].required_modules[0].logical_name == "std");
    assert(parsed.rules[0].required_modules[1].logical_name == "math:core");

    assert(parsed.rules[1].primary_output == "src/main.obj");
    assert(parsed.rules[1].required_modules.size() == 2);

    std::cout << "[PASS] P1689R5 JSON round-trip verified.\n";
}

void test_module_dependency_graph_topological_sort() {
    std::cout << "Testing ModuleDependencyGraph topological sorting and cycle detection...\n";

    fs::path temp_dir = fs::temp_directory_path() / "cxxgen_test_dag";
    fs::create_directories(temp_dir);

    fs::path f_core = temp_dir / "core.ixx";
    fs::path f_math = temp_dir / "math.ixx";
    fs::path f_app = temp_dir / "app.cpp";

    {
        std::ofstream(f_core) << "export module my_project.core;\nexport int core_val = 1;\n";
        std::ofstream(f_math)
            << "export module my_project.math;\nimport my_project.core;\nexport int get_sum() { return core_val; }\n";
        std::ofstream(f_app) << "import my_project.math;\nint main() { return get_sum(); }\n";
    }

    TargetConfig cfg;
    cfg.name = "test_target";
    cfg.sources = {f_app.string(), f_math.string(), f_core.string()};
    Target target(cfg);

    auto graph = ModuleScanner::scan_target(target);
    assert(graph.has_modules);
    assert(!graph.has_cycle);
    assert(graph.sorted_interfaces.size() == 2);
    // core must come before math
    assert(graph.sorted_interfaces[0] == f_core.string());
    assert(graph.sorted_interfaces[1] == f_math.string());
    assert(graph.non_interface_sources.size() == 1);
    assert(graph.non_interface_sources[0] == f_app.string());

    // Cycle detection
    fs::path f_cyc_a = temp_dir / "cycle_a.ixx";
    fs::path f_cyc_b = temp_dir / "cycle_b.ixx";
    {
        std::ofstream(f_cyc_a) << "export module cycle.a;\nimport cycle.b;\n";
        std::ofstream(f_cyc_b) << "export module cycle.b;\nimport cycle.a;\n";
    }

    TargetConfig cyc_cfg;
    cyc_cfg.name = "cycle_target";
    cyc_cfg.sources = {f_cyc_a.string(), f_cyc_b.string()};
    Target cyc_target(cyc_cfg);

    auto cyc_graph = ModuleScanner::scan_target(cyc_target);
    assert(cyc_graph.has_cycle);
    assert(!cyc_graph.cycle_path.empty());

    fs::remove_all(temp_dir);
    std::cout << "[PASS] ModuleDependencyGraph topological sorting and cycle detection verified.\n";
}

// ============================================================================
// Story 12.2: Two-Phase Module DAG & BMI Compiler (.ifc / .pcm / .gcm)
// ============================================================================

void test_toolchain_module_flags() {
    std::cout << "Testing toolchain BMI compiler command flags...\n";

    CompileParams params;
    params.source_file = "src/math.ixx";
    params.object_file = "build/math.obj";
    params.is_module_interface = true;
    params.module_interface_output = "build/math.ifc";
    params.module_references = {{"core", "build/core.ifc"}};

    // 1. MSVC
    MsvcToolchain msvc("cl.exe");
    auto msvc_cmd = msvc.build_compile_command(params);
    std::string msvc_joined = util::join(msvc_cmd, " ");
    assert(msvc_joined.find("/interface") != std::string::npos);
    assert(msvc_joined.find("/ifcOutput") != std::string::npos);
    assert(msvc_joined.find("math.ifc") != std::string::npos);
    assert(msvc_joined.find("/reference core=") != std::string::npos);
    assert(msvc_joined.find("core.ifc") != std::string::npos);

    // 2. Clang
    ClangToolchain clang("clang++.exe");
    params.module_interface_output = "build/math.pcm";
    params.module_references = {{"core", "build/core.pcm"}};
    auto clang_cmd = clang.build_compile_command(params);
    std::string clang_joined = util::join(clang_cmd, " ");
    assert(clang_joined.find("--precompile") != std::string::npos);
    assert(clang_joined.find("math.pcm") != std::string::npos);
    assert(clang_joined.find("-fmodule-file=core=") != std::string::npos);
    assert(clang_joined.find("core.pcm") != std::string::npos);

    // 3. GCC
    GccToolchain gcc("g++.exe");
    auto gcc_cmd = gcc.build_compile_command(params);
    std::string gcc_joined = util::join(gcc_cmd, " ");
    assert(gcc_joined.find("-fmodules-ts") != std::string::npos);

    std::cout << "[PASS] Toolchain BMI compiler flags verified.\n";
}

void test_bmi_cache() {
    std::cout << "Testing BmiCache key generation, store, restore, and clean...\n";

    fs::path temp_cache = fs::temp_directory_path() / "cxxgen_test_bmi_cache";
    BmiCache::instance().set_cache_dir(temp_cache);
    BmiCache::instance().clean();

    fs::path dummy_src = temp_cache / "dummy.ixx";
    fs::create_directories(temp_cache);
    std::ofstream(dummy_src) << "export module test_bmi_cache;\nexport int v = 42;\n";

    std::vector<std::string> flags = {"/std:c++latest", "/interface"};
    std::string key1 = BmiCache::compute_key(dummy_src, "msvc_19.44", flags);
    std::string key2 = BmiCache::compute_key(dummy_src, "msvc_19.44", flags);
    assert(key1 == key2);

    // Modifying flag changes key
    flags.push_back("/O2");
    std::string key3 = BmiCache::compute_key(dummy_src, "msvc_19.44", flags);
    assert(key1 != key3);

    // Create fake BMI and OBJ
    fs::path in_bmi = temp_cache / "test.ifc";
    fs::path in_obj = temp_cache / "test.obj";
    std::ofstream(in_bmi) << "IFC_BINARY_DATA";
    std::ofstream(in_obj) << "OBJ_BINARY_DATA";

    // Store in cache
    BmiCache::instance().store(key1, in_bmi, in_obj);

    // Restore to target location
    fs::path out_bmi = temp_cache / "restored.ifc";
    fs::path out_obj = temp_cache / "restored.obj";
    bool restored = BmiCache::instance().try_restore(key1, out_bmi, out_obj);
    assert(restored);
    assert(fs::exists(out_bmi) && fs::exists(out_obj));
    assert(util::read_file_text(out_bmi) == "IFC_BINARY_DATA");
    assert(util::read_file_text(out_obj) == "OBJ_BINARY_DATA");

    // Miss on different key
    bool missed = BmiCache::instance().try_restore("non_existent_key", out_bmi, out_obj);
    assert(!missed);

    assert(BmiCache::instance().hits() == 1);
    assert(BmiCache::instance().misses() == 1);

    BmiCache::instance().clean();
    assert(BmiCache::instance().hits() == 0);
    assert(BmiCache::instance().misses() == 0);
    assert(!fs::exists(out_bmi.parent_path() / key1));

    fs::remove_all(temp_cache);
    std::cout << "[PASS] BmiCache key generation, store, restore, and clean verified.\n";
}

void test_compile_bmi_step_model() {
    std::cout << "Testing CompileBMIStep DAG integration and step type...\n";

    fs::path temp_dir = fs::temp_directory_path() / "cxxgen_test_bmi_step";
    fs::create_directories(temp_dir);

    fs::path src = temp_dir / "mod.ixx";
    fs::path ifc = temp_dir / "mod.ifc";
    fs::path obj = temp_dir / "mod.obj";
    std::ofstream(src) << "export module mod;\n";

    MsvcToolchain tc("cl.exe");
    TargetConfig tcfg;
    tcfg.name = "dummy";
    Target dummy_target(tcfg);

    auto step = std::make_shared<CompileBMIStep>("mod", src, ifc, obj, dummy_target);
    assert(step->step_type() == StepType::CompileBMI);
    assert(step->name() == "bmi:mod");

    // Initially not up to date because outputs do not exist
    assert(!step->is_up_to_date());

    // Create fake outputs with newer timestamp
    std::ofstream(ifc) << "IFC";
    std::ofstream(obj) << "OBJ";
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    fs::last_write_time(ifc, fs::file_time_type::clock::now());
    fs::last_write_time(obj, fs::file_time_type::clock::now());

    assert(step->is_up_to_date());

    fs::remove_all(temp_dir);
    std::cout << "[PASS] CompileBMIStep DAG integration and step type verified.\n";
}

// ============================================================================
// Story 12.3: Standard Library Module Support (import std;)
// ============================================================================

void test_std_module_target_and_builder_api() {
    std::cout << "Testing Target and Builder enable_std_module API...\n";

    Builder b;
    assert(!b.is_std_module_enabled());
    b.enable_std_module();
    assert(b.is_std_module_enabled());

    TargetConfig cfg;
    assert(!cfg.enable_std_module);
    cfg.name = "my_target";
    auto &target = b.add_executable(cfg);
    assert(target.has_std_module()); // Inherited from builder

    TargetConfig cfg2;
    cfg2.name = "target_without_std";
    Builder b2;
    auto &target2 = b2.add_executable(cfg2);
    assert(!target2.has_std_module());
    target2.enable_std_module(true);
    assert(target2.has_std_module());

    std::cout << "[PASS] Target and Builder enable_std_module API verified.\n";
}

void test_std_module_helper_discovery() {
    std::cout << "Testing StdModuleHelper discovery...\n";

    auto msvc_opt = find_msvc_toolchain();
    if (msvc_opt) {
        fs::path std_src = StdModuleHelper::find_std_module_source(*msvc_opt);
        std::cout << "Discovered MSVC std module source: " << std_src.string() << "\n";
        assert(!std_src.empty());
        assert(fs::exists(std_src));
        assert(std_src.filename() == "std.ixx");
    }

    auto clang_opt = find_clang_toolchain();
    if (clang_opt) {
        fs::path std_src = StdModuleHelper::find_std_module_source(*clang_opt);
        if (!std_src.empty()) {
            std::cout << "Discovered Clang std module source: " << std_src.string() << "\n";
            assert(fs::exists(std_src));
        }
    }

    std::cout << "[PASS] StdModuleHelper discovery verified.\n";
}

void test_live_multi_module_compilation_and_execution() {
    std::cout << "Testing live multi-module compilation and execution with detected toolchain...\n";

    auto msvc_opt = find_msvc_toolchain();
    if (!msvc_opt) {
        std::cout << "[SKIP] MSVC not available for live module execution test.\n";
        return;
    }

    fs::path proj_dir = fs::temp_directory_path() / "cxxgen_live_module_test";
    fs::create_directories(proj_dir);

    fs::path math_ixx = proj_dir / "math.ixx";
    fs::path main_cpp = proj_dir / "main.cpp";

    {
        std::ofstream out_math(math_ixx);
        out_math << "export module math;\n"
                 << "export int add_numbers(int a, int b) {\n"
                 << "    return a + b;\n"
                 << "}\n";

        std::ofstream out_main(main_cpp);
        out_main << "import math;\n"
                 << "int main() {\n"
                 << "    return (add_numbers(20, 22) == 42) ? 0 : 1;\n"
                 << "}\n";
    }

    Builder b;

    TargetConfig cfg;
    cfg.name = "module_app";
    cfg.sources = {math_ixx.string(), main_cpp.string()};
    cfg.cpp_version = CppVersion::Cpp20;
    cfg.output_dir = (proj_dir / "bin").string();

    auto &target = b.add_executable(cfg);
    b.install_artifact(target);

    // Build the target
    BuildResult res_build = b.build_target(target, *msvc_opt);
    if (!res_build.success) {
        std::cerr << "Build failed: " << res_build.error_message << "\n";
    }
    assert(res_build.success);

    fs::path exe_path = proj_dir / "bin" / "module_app.exe";
    assert(fs::exists(exe_path));

    // Run the executable
    ProcessOptions opts;
    opts.command = {exe_path.string()};
    auto res = ProcessRunner::run(opts);
    assert(res.success());
    assert(res.exit_code == 0);

    fs::remove_all(proj_dir);
    std::cout << "[PASS] Live multi-module compilation and execution verified.\n";
}

void test_live_import_std_compilation_and_execution() {
    std::cout << "Testing live 'import std;' compilation and execution...\n";

    auto msvc_opt = find_msvc_toolchain();
    if (!msvc_opt) {
        std::cout << "[SKIP] MSVC not available for live 'import std;' test.\n";
        return;
    }

    fs::path std_src = StdModuleHelper::find_std_module_source(*msvc_opt);
    if (std_src.empty() || !fs::exists(std_src)) {
        std::cout << "[SKIP] MSVC std.ixx not found.\n";
        return;
    }

    fs::path proj_dir = fs::temp_directory_path() / "cxxgen_live_import_std_test";
    fs::create_directories(proj_dir);

    fs::path main_cpp = proj_dir / "main.cpp";
    {
        std::ofstream out_main(main_cpp);
        out_main << "import std;\n"
                 << "int main() {\n"
                 << "    std::vector<int> v = {10, 20, 12};\n"
                 << "    int sum = 0;\n"
                 << "    for (int x : v) sum += x;\n"
                 << "    return (sum == 42) ? 0 : 1;\n"
                 << "}\n";
    }

    Builder b;
    b.enable_std_module();

    TargetConfig cfg;
    cfg.name = "std_mod_app";
    cfg.sources = {main_cpp.string()};
    cfg.cpp_version = CppVersion::Cpp23;
    cfg.output_dir = (proj_dir / "bin").string();

    auto &target = b.add_executable(cfg);
    b.install_artifact(target);

    // Build the target
    BuildResult res_build = b.build_target(target, *msvc_opt);
    if (!res_build.success) {
        std::cerr << "Build failed: " << res_build.error_message << "\n";
    }
    assert(res_build.success);

    fs::path exe_path = proj_dir / "bin" / "std_mod_app.exe";
    assert(fs::exists(exe_path));

    // Run the executable
    ProcessOptions opts;
    opts.command = {exe_path.string()};
    auto res = ProcessRunner::run(opts);
    assert(res.success());
    assert(res.exit_code == 0);

    fs::remove_all(proj_dir);
    std::cout << "[PASS] Live 'import std;' compilation and execution verified.\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "Running Epic 12: C++20 / C++23 Modules Test Suite\n";
    std::cout << "====================================================\n";

    test_scanner_strip_comments_and_strings();
    test_scanner_builtin_syntax();
    test_p1689_serialization_and_deserialization();
    test_module_dependency_graph_topological_sort();
    test_toolchain_module_flags();
    test_bmi_cache();
    test_compile_bmi_step_model();
    test_std_module_target_and_builder_api();
    test_std_module_helper_discovery();
    test_live_multi_module_compilation_and_execution();
    test_live_import_std_compilation_and_execution();

    std::cout << "\n[ALL TESTS PASSED] Epic 12 Modules Architecture successfully verified.\n";
    return 0;
}
