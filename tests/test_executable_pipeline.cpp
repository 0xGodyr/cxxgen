#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

void setup_test_fixtures(const fs::path &fixture_dir) {
    fs::create_directories(fixture_dir / "src");
    fs::create_directories(fixture_dir / "include");

    // 1. calculator.hpp
    std::ofstream hpp(fixture_dir / "include/calculator.hpp");
    hpp << "#pragma once\n"
        << "int add(int a, int b);\n"
        << "int multiply(int a, int b);\n";
    hpp.close();

    // 2. calculator.cpp
    std::ofstream cpp(fixture_dir / "src/calculator.cpp");
    cpp << "#include \"calculator.hpp\"\n"
        << "int add(int a, int b) { return a + b; }\n"
        << "int multiply(int a, int b) { return a * b; }\n";
    cpp.close();

    // 3. main.cpp
    std::ofstream main(fixture_dir / "src/main.cpp");
    main << "#include \"calculator.hpp\"\n"
         << "#include <iostream>\n"
         << "int main() {\n"
         << "    std::cout << \"Calc Result: \" << add(40, 2) << \" and mult: \" << multiply(6, 7) << std::endl;\n"
         << "    return 0;\n"
         << "}\n";
    main.close();

    // 4. bad_syntax.cpp
    std::ofstream bad(fixture_dir / "src/bad_syntax.cpp");
    bad << "this is invalid C++ syntax that must fail compilation;\n";
    bad.close();
}

void cleanup_test_fixtures(const fs::path &fixture_dir) {
    std::error_code ec;
    fs::remove_all(fixture_dir, ec);
    fs::remove_all("bin", ec);
    fs::remove_all("custom_dist", ec);
    fs::remove_all(".cxxgen", ec);
}

void test_multi_file_executable(const cxxgen::IToolchain &toolchain) {
    using namespace cxxgen;
    fs::path fixture_dir = "tests/fixtures/test_exec_pipeline";

    Builder b;
    auto &exe = b.add_executable({
        .name = "calc_app",
        .sources = {(fixture_dir / "src/main.cpp").string(), (fixture_dir / "src/calculator.cpp").string()},
        .include_dirs = {(fixture_dir / "include").string()},
        .cpp_version = CppVersion::Cpp20,
        .optimize = Optimization::Debug,
    });
    b.install_artifact(exe);

    // 1. Check artifact path computation
    fs::path expected_path = b.get_artifact_path(exe, toolchain);
#if defined(_WIN32)
    assert(expected_path == fs::path("bin/calc_app.exe"));
#else
    assert(expected_path == fs::path("bin/calc_app"));
#endif

    // 2. Build target
    BuildResult res = b.build_target(exe, toolchain);
    if (!res.success) {
        std::cerr << "Build failed: " << res.error_message << "\n";
    }
    assert(res.success);
    assert(res.exit_code == 0);
    assert(res.object_files.size() == 2);
    assert(fs::exists(res.artifact_path));

    // 3. Execute the built binary and verify output
    ProcessResult run_res = ProcessRunner::run({res.artifact_path});
    assert(run_res.success());
    assert(run_res.exit_code == 0);
    assert(run_res.stdout_output.find("Calc Result: 42") != std::string::npos);
    assert(run_res.stdout_output.find("and mult: 42") != std::string::npos);

    std::cout << "[PASS] Multi-file C++ executable compilation & execution passed with " << toolchain.name() << "!\n";
}

void test_custom_output_name_and_dir(const cxxgen::IToolchain &toolchain) {
    using namespace cxxgen;
    fs::path fixture_dir = "tests/fixtures/test_exec_pipeline";

    Builder b;
    auto &exe = b.add_executable({
        .name = "original_name",
        .sources = {(fixture_dir / "src/main.cpp").string(), (fixture_dir / "src/calculator.cpp").string()},
        .include_dirs = {(fixture_dir / "include").string()},
        .output_dir = "custom_dist/tools",
        .output_name = "renamed_calculator",
    });

    // 1. Verify artifact path
    fs::path artifact_path = exe.artifact_path(toolchain);
#if defined(_WIN32)
    assert(artifact_path == fs::path("custom_dist/tools/renamed_calculator.exe"));
#else
    assert(artifact_path == fs::path("custom_dist/tools/renamed_calculator"));
#endif

    // 2. Build target
    BuildResult res = b.build_target(exe, toolchain);
    assert(res.success);
    assert(fs::exists(res.artifact_path));

    // 3. Run and verify output
    ProcessResult run_res = ProcessRunner::run({res.artifact_path});
    assert(run_res.success());
    assert(run_res.stdout_output.find("Calc Result: 42") != std::string::npos);

    std::cout << "[PASS] Custom output name and output directory override test passed!\n";
}

void test_subsystem_selection(const cxxgen::IToolchain &toolchain) {
    using namespace cxxgen;

    // Test Console subsystem link params
    LinkParams console_params{
        .output_binary = "bin/test_console.exe",
        .subsystem = WindowsSubsystem::Console,
    };
    auto console_cmd = toolchain.build_link_command(console_params);
    std::string flat_console = util::flatten_command(console_cmd);

    // Test Windows GUI subsystem link params
    LinkParams gui_params{
        .output_binary = "bin/test_gui.exe",
        .subsystem = WindowsSubsystem::Windows,
    };
    auto gui_cmd = toolchain.build_link_command(gui_params);
    std::string flat_gui = util::flatten_command(gui_cmd);

    if (toolchain.family() == CompilerFamily::Msvc) {
        assert(flat_console.find("/SUBSYSTEM:CONSOLE") != std::string::npos);
        assert(flat_gui.find("/SUBSYSTEM:WINDOWS") != std::string::npos);
    } else {
        assert(flat_gui.find("-mwindows") != std::string::npos ||
               flat_gui.find("-Wl,-subsystem:windows") != std::string::npos);
    }

    std::cout << "[PASS] Subsystem selection (Console vs Windows) test passed!\n";
}

void test_compilation_failure_reporting(const cxxgen::IToolchain &toolchain) {
    using namespace cxxgen;
    fs::path fixture_dir = "tests/fixtures/test_exec_pipeline";

    Builder b;
    auto &bad_exe = b.add_executable({
        .name = "bad_exe",
        .sources = {(fixture_dir / "src/bad_syntax.cpp").string()},
    });

    BuildResult res = b.build_target(bad_exe, toolchain);
    assert(!res.success);
    assert(res.exit_code != 0);
    assert(!res.error_message.empty());
    assert(res.error_message.find("Compilation failed") != std::string::npos);

    std::cout << "[PASS] Compilation failure diagnostic capture test passed!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Running Executable Pipeline Test Suite (Story 3.1)\n";
    std::cout << "========================================\n";

    fs::path fixture_dir = "tests/fixtures/test_exec_pipeline";
    setup_test_fixtures(fixture_dir);

    // Discover toolchain
    auto toolchain = cxxgen::find_toolchain();
    assert(toolchain != nullptr);
    std::cout << "[INFO] Using Toolchain: " << toolchain->name() << "\n";

    test_multi_file_executable(*toolchain);
    test_custom_output_name_and_dir(*toolchain);
    test_subsystem_selection(*toolchain);
    test_compilation_failure_reporting(*toolchain);

    // Also test with MSVC if installed
    auto msvc = cxxgen::find_msvc_toolchain();
    if (msvc.has_value()) {
        std::cout << "\n[INFO] Testing with MSVC Toolchain: " << msvc->name() << "\n";
        test_multi_file_executable(*msvc);
        test_custom_output_name_and_dir(*msvc);
        test_subsystem_selection(*msvc);
        test_compilation_failure_reporting(*msvc);
    }

    cleanup_test_fixtures(fixture_dir);

    std::cout << "========================================\n";
    std::cout << "All Story 3.1 Executable Pipeline Tests Passed Successfully!\n";
    std::cout << "========================================\n";
    return 0;
}
