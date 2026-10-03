#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ----------------------------------------------------------------------------
// Target 1: Console Executable linking Static Lib
// ----------------------------------------------------------------------------
void test_target1_executable_linking_static_lib() {
    std::cout << "[TARGET 1] Testing Console Executable linking Static Lib...\n";

    fs::path fixture = fs::current_path() / ".cxxgen_test_matrix_target1";
    std::error_code ec;
    fs::remove_all(fixture, ec);
    fs::create_directories(fixture / "src", ec);
    fs::create_directories(fixture / "include", ec);

    // Math library header and source
    std::ofstream hdr(fixture / "include" / "math_lib.hpp");
    hdr << "#pragma once\nint add(int a, int b);\nint multiply(int a, int b);\n";
    hdr.close();

    std::ofstream src_lib(fixture / "src" / "math_lib.cpp");
    src_lib << "#include \"math_lib.hpp\"\nint add(int a, int b) { return a + b; }\nint multiply(int a, int b) { "
               "return a * b; }\n";
    src_lib.close();

    // App source
    std::ofstream src_app(fixture / "src" / "calc_main.cpp");
    src_app << "#include \"math_lib.hpp\"\n#include <iostream>\nint main() {\n"
            << "    std::cout << \"CALC_RESULT: \" << add(10, multiply(4, 5)) << std::endl;\n"
            << "    return 0;\n}\n";
    src_app.close();

    auto tc = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(tc != nullptr);

    cxxgen::Builder b;
    auto &lib = b.add_static_library({
        .name = "static_math",
        .sources = {(fixture / "src" / "math_lib.cpp").string()},
        .include_dirs = {(fixture / "include").string()},
    });

    auto &app = b.add_executable({
        .name = "calc_app",
        .sources = {(fixture / "src" / "calc_main.cpp").string()},
        .output_dir = (fixture / "bin").string(),
    });
    app.link(lib);

    auto bld_res = b.build_target(app, *tc);
    if (!bld_res.success) {
        std::cerr << "Target 1 build failed:\n" << bld_res.error_message << "\n";
    }
    assert(bld_res.success);

    fs::path exe_path = bld_res.artifact_path;
    assert(fs::exists(exe_path));

    cxxgen::ProcessOptions popts;
    popts.command = {exe_path.string()};
    auto run_res = cxxgen::ProcessRunner::run(popts);
    assert(run_res.success());
    assert(run_res.combined_output().find("CALC_RESULT: 30") != std::string::npos);

    fs::remove_all(fixture, ec);
    std::cout << "[PASS] Target 1: Console Executable linking Static Lib verified!\n";
}

// ----------------------------------------------------------------------------
// Target 2: Shared Library / DLL with exported functions and classes
// ----------------------------------------------------------------------------
void test_target2_shared_library_dll() {
    std::cout << "[TARGET 2] Testing Shared Library / DLL with exported functions...\n";

    fs::path fixture = fs::current_path() / ".cxxgen_test_matrix_target2";
    std::error_code ec;
    fs::remove_all(fixture, ec);
    fs::create_directories(fixture / "src", ec);
    fs::create_directories(fixture / "include", ec);

    // Shared library header with export macro
    std::ofstream hdr(fixture / "include" / "greeter.hpp");
    hdr << "#pragma once\n"
        << "#if defined(_WIN32)\n"
        << "  #if defined(GREETER_LIB_EXPORTS)\n"
        << "    #define GREETER_API __declspec(dllexport)\n"
        << "  #else\n"
        << "    #define GREETER_API __declspec(dllimport)\n"
        << "  #endif\n"
        << "#else\n"
        << "  #define GREETER_API __attribute__((visibility(\"default\")))\n"
        << "#endif\n"
        << "extern \"C\" GREETER_API const char* get_greeting();\n";
    hdr.close();

    std::ofstream src_lib(fixture / "src" / "greeter.cpp");
    src_lib << "#include \"greeter.hpp\"\n"
            << "const char* get_greeting() { return \"Hello from cxxgen Shared Library!\"; }\n";
    src_lib.close();

    std::ofstream src_app(fixture / "src" / "greeter_main.cpp");
    src_app << "#include \"greeter.hpp\"\n#include <iostream>\nint main() {\n"
            << "    std::cout << \"GREETING: \" << get_greeting() << std::endl;\n"
            << "    return 0;\n}\n";
    src_app.close();

    auto tc = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(tc != nullptr);

    cxxgen::Builder b;
    auto &shlib = b.add_shared_library({
        .name = "greeter_lib",
        .sources = {(fixture / "src" / "greeter.cpp").string()},
        .include_dirs = {(fixture / "include").string()},
        .output_dir = (fixture / "bin").string(),
    });

    auto &app = b.add_executable({
        .name = "greeter_app",
        .sources = {(fixture / "src" / "greeter_main.cpp").string()},
        .output_dir = (fixture / "bin").string(),
    });
    app.link(shlib);

    auto bld_res = b.build_target(app, *tc);
    if (!bld_res.success) {
        std::cerr << "Target 2 build failed:\n" << bld_res.error_message << "\n";
    }
    assert(bld_res.success);

    fs::path exe_path = bld_res.artifact_path;
    assert(fs::exists(exe_path));

    cxxgen::ProcessOptions popts;
    popts.command = {exe_path.string()};
    popts.working_directory = (fixture / "bin").string();
    auto run_res = cxxgen::ProcessRunner::run(popts);
    assert(run_res.success());
    assert(run_res.combined_output().find("GREETING: Hello from cxxgen Shared Library!") != std::string::npos);

    fs::remove_all(fixture, ec);
    std::cout << "[PASS] Target 2: Shared Library / DLL verified!\n";
}

// ----------------------------------------------------------------------------
// Target 3: Dynamic Plugin loaded via LoadLibrary / dlopen
// ----------------------------------------------------------------------------
void test_target3_dynamic_plugin() {
    std::cout << "[TARGET 3] Testing Dynamic Plugin loaded via LoadLibrary / dlopen...\n";

    fs::path fixture = fs::current_path() / ".cxxgen_test_matrix_target3";
    std::error_code ec;
    fs::remove_all(fixture, ec);
    fs::create_directories(fixture / "src", ec);

    std::ofstream src_plugin(fixture / "src" / "audio_dsp.cpp");
    src_plugin << "#if defined(_WIN32)\n"
               << "  #define PLUGIN_EXPORT extern \"C\" __declspec(dllexport)\n"
               << "#else\n"
               << "  #define PLUGIN_EXPORT extern \"C\" __attribute__((visibility(\"default\")))\n"
               << "#endif\n"
               << "PLUGIN_EXPORT int process_audio(int input) {\n"
               << "    return input * 2;\n"
               << "}\n";
    src_plugin.close();

    auto tc = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(tc != nullptr);

    cxxgen::Builder b;
    auto &plugin = b.add_plugin({
        .name = "audio_dsp_plugin",
        .sources = {(fixture / "src" / "audio_dsp.cpp").string()},
        .output_dir = (fixture / "bin" / "plugins").string(),
    });

    auto bld_res = b.build_target(plugin, *tc);
    assert(bld_res.success);
    fs::path plugin_path = bld_res.artifact_path;
    assert(fs::exists(plugin_path));

    // Dynamically load
#if defined(_WIN32)
    HMODULE mod = LoadLibraryW(plugin_path.wstring().c_str());
    assert(mod != NULL);
    typedef int (*ProcessFn)(int);
    union {
        FARPROC proc;
        ProcessFn fn;
    } caster;
    caster.proc = GetProcAddress(mod, "process_audio");
    ProcessFn fn = caster.fn;
    assert(fn != NULL);
    assert(fn(42) == 84);
    FreeLibrary(mod);
#else
    void *mod = dlopen(plugin_path.c_str(), RTLD_NOW);
    assert(mod != nullptr);
    typedef int (*ProcessFn)(int);
    ProcessFn fn = (ProcessFn)dlsym(mod, "process_audio");
    assert(fn != nullptr);
    assert(fn(42) == 84);
    dlclose(mod);
#endif

    fs::remove_all(fixture, ec);
    std::cout << "[PASS] Target 3: Dynamic Plugin verified!\n";
}

// ----------------------------------------------------------------------------
// Target 4: Unit test suite execution (cxxgen test)
// ----------------------------------------------------------------------------
void test_target4_unit_test_suite() {
    std::cout << "[TARGET 4] Testing Unit test suite execution (cxxgen test)...\n";

    fs::path fixture = fs::current_path() / ".cxxgen_test_matrix_target4";
    std::error_code ec;
    fs::remove_all(fixture, ec);
    fs::create_directories(fixture / "src", ec);

    std::ofstream t1(fixture / "src" / "test_suite1.cpp");
    t1 << "#include <iostream>\nint main() { std::cout << \"TEST_SUITE_1_PASS\\n\"; return 0; }\n";
    t1.close();

    std::ofstream t2(fixture / "src" / "test_suite2.cpp");
    t2 << "#include <iostream>\nint main() { std::cout << \"TEST_SUITE_2_PASS\\n\"; return 0; }\n";
    t2.close();

    auto tc = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(tc != nullptr);

    cxxgen::Builder b;
    b.add_test({
        .name = "suite1",
        .sources = {(fixture / "src" / "test_suite1.cpp").string()},
        .output_dir = (fixture / "bin").string(),
    });
    b.add_test({
        .name = "suite2",
        .sources = {(fixture / "src" / "test_suite2.cpp").string()},
        .output_dir = (fixture / "bin").string(),
    });

    cxxgen::ExecutionContext ctx{
        .toolchain = *tc,
        .verbose = false,
    };

    // Execute tests
    for (const auto &t : b.targets()) {
        if (t->type() == cxxgen::TargetType::Test) {
            cxxgen::TargetBuildStep bstep(b, *t);
            auto bres = bstep.execute(ctx);
            assert(bres.success());

            cxxgen::RunArtifactStep rstep(*t);
            auto rres = rstep.execute(ctx);
            assert(rres.success());
            assert(rres.exit_code == 0);
        }
    }

    fs::remove_all(fixture, ec);
    std::cout << "[PASS] Target 4: Unit test suite execution verified!\n";
}

// ----------------------------------------------------------------------------
// Target 5: CMake export verification (cmake -B build && cmake --build build)
// ----------------------------------------------------------------------------
void test_target5_cmake_export() {
    std::cout << "[TARGET 5] Testing CMake export verification...\n";

    fs::path fixture = fs::current_path() / ".cxxgen_test_matrix_target5";
    std::error_code ec;
    fs::remove_all(fixture, ec);
    fs::create_directories(fixture / "src", ec);

    std::ofstream src(fixture / "src" / "main.cpp");
    src << "#include <iostream>\nint main() { std::cout << \"CMAKE_EXPORT_SUCCESS\\n\"; return 0; }\n";
    src.close();

    cxxgen::Builder b;
    b.set_project_name("E2ECMakeProj");
    b.add_executable({
        .name = "cmake_app",
        .sources = {"src/main.cpp"},
    });

    cxxgen::CMakeGenerator gen(b);
    assert(gen.generate_file(fixture / "CMakeLists.txt"));
    assert(fs::exists(fixture / "CMakeLists.txt"));

    // Check if cmake is installed
    cxxgen::ProcessOptions chk;
    chk.command = {"cmake", "--version"};
    auto chk_res = cxxgen::ProcessRunner::run(chk);
    if (chk_res.success()) {
        // Run cmake configure
        cxxgen::ProcessOptions cfg;
        cfg.command = {"cmake", "-B", (fixture / "build").string(), "-S", fixture.string()};
        auto cfg_res = cxxgen::ProcessRunner::run(cfg);
        if (!cfg_res.success()) {
            std::cerr << "cmake -B failed:\n" << cfg_res.combined_output() << "\n";
        }
        assert(cfg_res.success());

        // Run cmake build
        cxxgen::ProcessOptions bld;
        bld.command = {"cmake", "--build", (fixture / "build").string()};
        auto bld_res = cxxgen::ProcessRunner::run(bld);
        if (!bld_res.success()) {
            std::cerr << "cmake --build failed:\n" << bld_res.combined_output() << "\n";
        }
        assert(bld_res.success());
        std::cout << "[PASS] Target 5: CMake export and compilation verified!\n";
    } else {
        std::cout << "[SKIP] CMake not found in PATH, verified CMakeLists.txt generation only.\n";
    }

    fs::remove_all(fixture, ec);
}

// ----------------------------------------------------------------------------
// Target 6: MSVC export verification (MSBuild solution compile on Windows)
// ----------------------------------------------------------------------------
void test_target6_msvc_export() {
    std::cout << "[TARGET 6] Testing MSVC export verification...\n";

    fs::path fixture = fs::current_path() / ".cxxgen_test_matrix_target6";
    std::error_code ec;
    fs::remove_all(fixture, ec);
    fs::create_directories(fixture / "src", ec);

    std::ofstream src(fixture / "src" / "main.cpp");
    src << "#include <iostream>\nint main() { std::cout << \"MSBUILD_EXPORT_SUCCESS\\n\"; return 0; }\n";
    src.close();

    cxxgen::Builder b;
    b.set_project_name("E2EMsvcProj");
    b.add_executable({
        .name = "msvc_app",
        .sources = {"src/main.cpp"},
    });

    cxxgen::MsvcGenerator gen(b);
    assert(gen.generate_all(fixture));
    assert(fs::exists(fixture / "E2EMsvcProj.sln"));
    assert(fs::exists(fixture / "msvc_app.vcxproj"));
    assert(fs::exists(fixture / "msvc_app.vcxproj.filters"));

#if defined(_WIN32)
    fs::path msbuild =
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe";
    if (fs::exists(msbuild)) {
        cxxgen::ProcessOptions bld;
        bld.command = {
            msbuild.string(), "E2EMsvcProj.sln", "/p:Configuration=Release", "/p:Platform=x64", "/m",
        };
        bld.working_directory = fixture.string();
        auto bld_res = cxxgen::ProcessRunner::run(bld);
        if (!bld_res.success()) {
            std::cerr << "MSBuild failed:\n" << bld_res.combined_output() << "\n";
        }
        assert(bld_res.success());
        std::cout << "[PASS] Target 6: MSVC export and MSBuild compile verified!\n";
    } else {
        std::cout << "[SKIP] MSBuild executable not found, verified project file generation.\n";
    }
#else
    std::cout << "[SKIP] Non-Windows OS, skipped MSBuild execution.\n";
#endif

    fs::remove_all(fixture, ec);
}

// ----------------------------------------------------------------------------
// Target 7: Ninja export verification (ninja)
// ----------------------------------------------------------------------------
void test_target7_ninja_export() {
    std::cout << "[TARGET 7] Testing Ninja export verification...\n";

    fs::path fixture = fs::current_path() / ".cxxgen_test_matrix_target7";
    std::error_code ec;
    fs::remove_all(fixture, ec);
    fs::create_directories(fixture / "src", ec);

    std::ofstream src(fixture / "src" / "main.cpp");
    src << "#include <iostream>\nint main() { std::cout << \"NINJA_EXPORT_SUCCESS\\n\"; return 0; }\n";
    src.close();

    cxxgen::Builder b;
    b.add_executable({
        .name = "ninja_app",
        .sources = {"src/main.cpp"},
    });

    cxxgen::NinjaGenerator gen(b);
    assert(gen.generate_file(fixture / "build.ninja"));
    assert(fs::exists(fixture / "build.ninja"));

    // Check if ninja is installed
    cxxgen::ProcessOptions chk;
    chk.command = {"ninja", "--version"};
    auto chk_res = cxxgen::ProcessRunner::run(chk);
    if (chk_res.success()) {
        cxxgen::ProcessOptions bld;
        bld.command = {"ninja"};
        bld.working_directory = fixture.string();
        auto bld_res = cxxgen::ProcessRunner::run(bld);
        if (!bld_res.success()) {
            std::cerr << "ninja failed:\n" << bld_res.combined_output() << "\n";
        }
        assert(bld_res.success());
        std::cout << "[PASS] Target 7: Ninja export and build verified!\n";
    } else {
        std::cout << "[SKIP] Ninja not found in PATH, verified build.ninja generation only.\n";
    }

    fs::remove_all(fixture, ec);
}

// ----------------------------------------------------------------------------
// Target 8: compile_commands.json schema validation
// ----------------------------------------------------------------------------
void test_target8_compile_commands_validation() {
    std::cout << "[TARGET 8] Testing compile_commands.json schema validation...\n";

    fs::path fixture = fs::current_path() / ".cxxgen_test_matrix_target8";
    std::error_code ec;
    fs::remove_all(fixture, ec);
    fs::create_directories(fixture / "src", ec);

    std::ofstream src1(fixture / "src" / "alpha.cpp");
    src1 << "int alpha() { return 1; }\n";
    src1.close();

    std::ofstream src2(fixture / "src" / "beta.cpp");
    src2 << "int beta() { return 2; }\n";
    src2.close();

    cxxgen::Builder b;
    b.add_executable({
        .name = "schema_app",
        .sources = {(fixture / "src" / "alpha.cpp").string(), (fixture / "src" / "beta.cpp").string()},
    });

    cxxgen::CompileCommandsGenerator gen(b);
    fs::path cc_path = fixture / "compile_commands.json";
    assert(gen.generate_file(cc_path));
    assert(fs::exists(cc_path));

    // Validate JSON structure
    std::ifstream f(cc_path);
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();

    assert(content.front() == '[');
    assert(content.find("\"directory\":") != std::string::npos);
    assert(content.find("\"command\":") != std::string::npos);
    assert(content.find("\"file\":") != std::string::npos);
    assert(content.find("\"output\":") != std::string::npos);
    assert(content.find("alpha.cpp") != std::string::npos);
    assert(content.find("beta.cpp") != std::string::npos);

    fs::remove_all(fixture, ec);
    std::cout << "[PASS] Target 8: compile_commands.json schema validation verified!\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "Running Story 10.2: Multi-Target Integration Test Suite\n";
    std::cout << "========================================================\n";

    test_target1_executable_linking_static_lib();
    test_target2_shared_library_dll();
    test_target3_dynamic_plugin();
    test_target4_unit_test_suite();
    test_target5_cmake_export();
    test_target6_msvc_export();
    test_target7_ninja_export();
    test_target8_compile_commands_validation();

    std::cout << "\nAll 8 Target Matrix Tests Passed Successfully!\n";
    return 0;
}
