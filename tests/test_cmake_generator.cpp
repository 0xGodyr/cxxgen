#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void test_cmake_generator_unit() {
    std::cout << "Testing CMakeGenerator unit output generation...\n";

    cxxgen::Builder b;
    b.set_project_name("TestSandbox");

    // 1. Interface library
    auto &iface = b.add_interface_library({
        .name = "common_iface",
        .include_dirs = {"include/common"},
        .defines = {"COMMON_ENABLED=1"},
    });

    // 2. Static library
    auto &core_lib = b.add_static_library({
        .name = "core_lib",
        .sources = {"core/src/core.cpp"},
        .include_dirs = {"core/include"},
        .defines = {"CORE_BUILD=1"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    core_lib.link(iface);

    // 3. Shared library
    auto &math_dll = b.add_shared_library({
        .name = "math_dll",
        .sources = {"math/src/math.cpp"},
        .include_dirs = {"math/include"},
        .defines = {"MATH_BUILD=1"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    math_dll.link(core_lib);

    // 4. Plugin
    b.add_plugin({
        .name = "audio_plugin",
        .sources = {"audio/src/plugin.cpp"},
        .include_dirs = {"audio/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    // 5. Executable
    auto &app = b.add_executable({
        .name = "main_app",
        .sources = {"app/src/main.cpp"},
        .include_dirs = {"app/include"},
        .defines = {"APP_MODE=1"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .output_name = "custom_main_app",
    });
    app.link(math_dll);

    // 6. Test target
    auto &test_exe = b.add_test({
        .name = "core_test",
        .sources = {"tests/core_test.cpp"},
        .include_dirs = {"tests/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    test_exe.link(core_lib);

    // 7. Custom command step
    b.add_custom_command({
        .name = "gen_version",
        .description = "Generate version header",
        .command = {"python", "scripts/gen_version.py"},
        .inputs = {"scripts/version.in"},
        .outputs = {"app/include/version.h"},
    });

    cxxgen::CMakeGenerator gen(b);
    std::string cmake_content = gen.generate();

    // Verify key CMake constructs
    assert(cmake_content.find("cmake_minimum_required(VERSION 3.20)") != std::string::npos);
    assert(cmake_content.find("project(TestSandbox LANGUAGES CXX C)") != std::string::npos);
    assert(cmake_content.find("enable_testing()") != std::string::npos);

    // Verify targets
    assert(cmake_content.find("add_library(common_iface INTERFACE)") != std::string::npos);
    assert(cmake_content.find("add_library(core_lib STATIC") != std::string::npos);
    assert(cmake_content.find("add_library(math_dll SHARED") != std::string::npos);
    assert(cmake_content.find("add_library(audio_plugin MODULE") != std::string::npos);
    assert(cmake_content.find("add_executable(main_app") != std::string::npos);
    assert(cmake_content.find("add_executable(core_test") != std::string::npos);
    assert(cmake_content.find("add_test(NAME core_test COMMAND core_test)") != std::string::npos);

    // Verify includes, defines, and links
    assert(cmake_content.find("target_include_directories(common_iface INTERFACE") != std::string::npos);
    assert(cmake_content.find("target_compile_definitions(core_lib PUBLIC") != std::string::npos);
    assert(cmake_content.find("target_link_libraries(main_app PRIVATE") != std::string::npos);
    assert(cmake_content.find("math_dll") != std::string::npos);
    assert(cmake_content.find("target_compile_features(main_app PRIVATE cxx_std_20)") != std::string::npos);

    // Verify custom command
    assert(cmake_content.find("add_custom_command(") != std::string::npos);
    assert(cmake_content.find("app/include/version.h") != std::string::npos);
    assert(cmake_content.find("scripts/version.in") != std::string::npos);

    // Verify target properties
    assert(cmake_content.find("OUTPUT_NAME \"custom_main_app\"") != std::string::npos);

    std::cout << "[PASS] CMakeGenerator unit generation verified!\n";
}

void test_cmake_end_to_end_build() {
    std::cout << "Testing CMakeGenerator end-to-end execution with cmake & build...\n";

    fs::path fixture_dir = "test_cmake_fixture";
    std::error_code ec;
    fs::remove_all(fixture_dir, ec);

    fs::create_directories(fixture_dir / "lib" / "include", ec);
    fs::create_directories(fixture_dir / "lib" / "src", ec);
    fs::create_directories(fixture_dir / "app" / "src", ec);

    // 1. Create source files
    fs::path math_h = fixture_dir / "lib" / "include" / "math_lib.h";
    fs::path math_cpp = fixture_dir / "lib" / "src" / "math_lib.cpp";
    fs::path main_cpp = fixture_dir / "app" / "src" / "main.cpp";

    {
        std::ofstream f(math_h);
        f << "#pragma once\nint compute_answer();\n";
    }
    {
        std::ofstream f(math_cpp);
        f << "#include \"math_lib.h\"\nint compute_answer() { return 42; }\n";
    }
    {
        std::ofstream f(main_cpp);
        f << "#include \"math_lib.h\"\n#include <iostream>\nint main() {\n"
          << "    std::cout << \"Answer: \" << compute_answer() << \"\\n\";\n"
          << "    return (compute_answer() == 42) ? 0 : 1;\n"
          << "}\n";
    }

    // 2. Configure Builder
    cxxgen::Builder b;
    b.set_project_name("E2E_CMakeProject");

    auto math_lib = b.add_static_library({
        .name = "math_lib",
        .sources = {math_cpp.string()},
        .include_dirs = {(fixture_dir / "lib" / "include").string()},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    auto app = b.add_executable({
        .name = "e2e_app",
        .sources = {main_cpp.string()},
        .include_dirs = {(fixture_dir / "lib" / "include").string()},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    app.link(math_lib);

    // 3. Generate CMakeLists.txt
    cxxgen::CMakeGenerator gen(b);
    fs::path cmake_file = fixture_dir / "CMakeLists.txt";
    assert(gen.generate_file(cmake_file));
    assert(fs::exists(cmake_file));

    // Fix relative paths in CMakeLists.txt so they are relative to fixture_dir root
    // Notice that math_cpp is "test_cmake_fixture/lib/src/math_lib.cpp"
    // Inside fixture_dir, CMake will look for "test_cmake_fixture/..." unless relative to fixture_dir.
    // Let's create a builder with paths relative to fixture_dir:
    cxxgen::Builder b_rel;
    b_rel.set_project_name("E2E_CMakeProject");

    auto &math_lib_rel = b_rel.add_static_library({
        .name = "math_lib",
        .sources = {"lib/src/math_lib.cpp"},
        .include_dirs = {"lib/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    auto &app_rel = b_rel.add_executable({
        .name = "e2e_app",
        .sources = {"app/src/main.cpp"},
        .include_dirs = {"lib/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    app_rel.link(math_lib_rel);

    cxxgen::CMakeGenerator gen_rel(b_rel);
    assert(gen_rel.generate_file(cmake_file));

    // 4. Run cmake configure: cmake -B build -G Ninja
    cxxgen::ProcessOptions cmake_cfg_opts;
    cmake_cfg_opts.command = {"cmake", "-B", "build", "-G", "Ninja"};
    cmake_cfg_opts.working_directory = fixture_dir;
    auto cfg_res = cxxgen::ProcessRunner::run(cmake_cfg_opts);
    if (!cfg_res.success()) {
        std::cerr << "CMake configure output:\n" << cfg_res.combined_output() << "\n";
    }
    assert(cfg_res.success());

    // 5. Run cmake build: cmake --build build
    cxxgen::ProcessOptions cmake_bld_opts;
    cmake_bld_opts.command = {"cmake", "--build", "build"};
    cmake_bld_opts.working_directory = fixture_dir;
    auto bld_res = cxxgen::ProcessRunner::run(cmake_bld_opts);
    if (!bld_res.success()) {
        std::cerr << "CMake build output:\n" << bld_res.combined_output() << "\n";
    }
    assert(bld_res.success());

    // 6. Run the built binary
#if defined(_WIN32)
    fs::path exe_path = fixture_dir / "build" / "e2e_app.exe";
#else
    fs::path exe_path = fixture_dir / "build" / "e2e_app";
#endif
    assert(fs::exists(exe_path));

    cxxgen::ProcessOptions run_opts;
    run_opts.command = {exe_path.string()};
    auto run_res = cxxgen::ProcessRunner::run(run_opts);
    assert(run_res.success());
    assert(run_res.exit_code == 0);
    assert(cxxgen::util::trim(run_res.stdout_output) == "Answer: 42");

    // Cleanup
    fs::remove_all(fixture_dir, ec);

    std::cout << "[PASS] End-to-end CMake configure, build, and execution verified!\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "Running Story 6.1: CMake Generator Tests\n";
    std::cout << "===========================================\n";

    test_cmake_generator_unit();
    test_cmake_end_to_end_build();

    std::cout << "\nAll Story 6.1 Tests Passed Successfully!\n";
    return 0;
}
