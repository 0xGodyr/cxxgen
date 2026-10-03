#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

void setup_shared_lib_fixtures(const fs::path &fixture_dir) {
    fs::create_directories(fixture_dir / "math_lib/include");
    fs::create_directories(fixture_dir / "math_lib/src");
    fs::create_directories(fixture_dir / "app/src");
    fs::create_directories(fixture_dir / "custom_lib/src");
    fs::create_directories(fixture_dir / "custom_app/src");

    // 1. math_dyn.hpp
    std::ofstream hpp(fixture_dir / "math_lib/include/math_dyn.hpp");
    hpp << "#pragma once\n"
        << "#if defined(_WIN32)\n"
        << "  #if defined(MATH_DYNAMIC_EXPORTS)\n"
        << "    #define MATH_API __declspec(dllexport)\n"
        << "  #else\n"
        << "    #define MATH_API __declspec(dllimport)\n"
        << "  #endif\n"
        << "#else\n"
        << "  #define MATH_API __attribute__((visibility(\"default\")))\n"
        << "#endif\n\n"
        << "MATH_API int math_dyn_add(int a, int b);\n"
        << "MATH_API int math_dyn_multiplier(int x);\n";
    hpp.close();

    // 2. math_dyn.cpp
    std::ofstream cpp(fixture_dir / "math_lib/src/math_dyn.cpp");
    cpp << "#include \"math_dyn.hpp\"\n"
        << "int math_dyn_add(int a, int b) {\n"
        << "    return a + b;\n"
        << "}\n"
        << "int math_dyn_multiplier(int x) {\n"
        << "    return x * 10;\n"
        << "}\n";
    cpp.close();

    // 3. app/src/main.cpp
    std::ofstream main(fixture_dir / "app/src/main.cpp");
    main << "#include \"math_dyn.hpp\"\n"
         << "#include <iostream>\n"
         << "int main() {\n"
         << "    int val = math_dyn_multiplier(math_dyn_add(3, 4));\n"
         << "    std::cout << \"Shared Lib Output: \" << val << std::endl;\n"
         << "    return 0;\n"
         << "}\n";
    main.close();

    // 4. custom_math.cpp (for custom output_dir / output_name test)
    std::ofstream custom_cpp(fixture_dir / "custom_lib/src/custom_math.cpp");
    custom_cpp << "#if defined(_WIN32)\n"
               << "  #if defined(CUSTOM_MATH_EXPORTS)\n"
               << "    #define CUSTOM_API __declspec(dllexport)\n"
               << "  #else\n"
               << "    #define CUSTOM_API __declspec(dllimport)\n"
               << "  #endif\n"
               << "#else\n"
               << "  #define CUSTOM_API __attribute__((visibility(\"default\")))\n"
               << "#endif\n\n"
               << "CUSTOM_API int custom_calc(int v) {\n"
               << "    return v * 42;\n"
               << "}\n";
    custom_cpp.close();

    // 5. custom_app/src/main.cpp
    std::ofstream custom_app_main(fixture_dir / "custom_app/src/main.cpp");
    custom_app_main << "#if defined(_WIN32)\n"
                    << "  #define CUSTOM_API __declspec(dllimport)\n"
                    << "#else\n"
                    << "  #define CUSTOM_API\n"
                    << "#endif\n"
                    << "CUSTOM_API int custom_calc(int v);\n"
                    << "#include <iostream>\n"
                    << "int main() {\n"
                    << "    int val = custom_calc(2);\n"
                    << "    std::cout << \"Custom DLL Output: \" << val << std::endl;\n"
                    << "    return 0;\n"
                    << "}\n";
    custom_app_main.close();
}

void cleanup_shared_lib_fixtures(const fs::path &fixture_dir) {
    std::error_code ec;
    fs::remove_all(fixture_dir, ec);
    fs::remove_all("bin", ec);
    fs::remove_all("lib", ec);
    fs::remove_all("custom_output", ec);
    fs::remove_all(".cxxgen", ec);
}

void test_shared_library_pipeline(const cxxgen::IToolchain &toolchain) {
    using namespace cxxgen;
    fs::path fixture_dir = "tests/fixtures/test_shared_lib_pipeline";

    Builder b;
    auto &math_dyn = b.add_shared_library({
        .name = "math_dynamic",
        .sources = {(fixture_dir / "math_lib/src/math_dyn.cpp").string()},
        .include_dirs = {(fixture_dir / "math_lib/include").string()},
        .defines = {"MATH_DYN_ENABLED=1"},
        .cpp_version = CppVersion::Cpp20,
    });

    auto &app = b.add_executable({
        .name = "dyn_app",
        .sources = {(fixture_dir / "app/src/main.cpp").string()},
        .cpp_version = CppVersion::Cpp20,
    });
    app.link(math_dyn);

    // 1. Verify shared library artifact path and import library path
    fs::path dll_path = b.get_artifact_path(math_dyn, toolchain);
#if defined(_WIN32)
    assert(dll_path.extension() == ".dll");
    fs::path implib_path = b.get_import_library_path(math_dyn, toolchain);
    if (toolchain.family() == CompilerFamily::Msvc) {
        assert(implib_path.extension() == ".lib");
    } else if (toolchain.family() == CompilerFamily::Clang) {
        assert(implib_path.extension() == ".lib" ||
               implib_path.filename().string().find(".dll.a") != std::string::npos);
    } else {
        assert(implib_path.filename().string().find(".dll.a") != std::string::npos);
    }
#elif defined(__APPLE__)
    assert(dll_path.extension() == ".dylib");
#else
    assert(dll_path.extension() == ".so");
#endif

    // 2. Build shared library
    BuildResult lib_res = b.build_target(math_dyn, toolchain);
    if (!lib_res.success) {
        std::cerr << "Shared library build failed: " << lib_res.error_message << "\n";
    }
    assert(lib_res.success);
    assert(fs::exists(lib_res.artifact_path));

#if defined(_WIN32)
    assert(fs::exists(implib_path));
#endif

    // 3. Build executable (linking shared library)
    BuildResult app_res = b.build_target(app, toolchain);
    if (!app_res.success) {
        std::cerr << "Executable link failed: " << app_res.error_message << "\n";
    }
    assert(app_res.success);
    assert(fs::exists(app_res.artifact_path));

    // 4. Run executable and verify dynamic link runtime execution
    ProcessResult run_res = ProcessRunner::run({app_res.artifact_path});
    if (!run_res.success()) {
        std::cerr << "Executable execution failed with exit code: " << run_res.exit_code << "\n"
                  << "Output: " << run_res.combined_output() << "\n";
    }
    assert(run_res.success());
    assert(run_res.stdout_output.find("Shared Lib Output: 70") != std::string::npos);

    std::cout << "[PASS] Basic shared library pipeline tests passed with " << toolchain.name()
              << "! Output DLL: " << lib_res.artifact_path << "\n";

    // 5. Test custom output directory and custom output name
    auto &custom_dyn = b.add_shared_library({
        .name = "custom_math",
        .sources = {(fixture_dir / "custom_lib/src/custom_math.cpp").string()},
        .cpp_version = CppVersion::Cpp20,
        .output_dir = "custom_output/dlls",
        .output_name = "super_calc",
    });

    auto &custom_app = b.add_executable({
        .name = "custom_app",
        .sources = {(fixture_dir / "custom_app/src/main.cpp").string()},
        .cpp_version = CppVersion::Cpp20,
        .output_dir = "custom_output/bin",
    });
    custom_app.link(custom_dyn);

    fs::path custom_dll = b.get_artifact_path(custom_dyn, toolchain);
    assert(custom_dll.string().find("super_calc") != std::string::npos);
    assert(custom_dll.parent_path() == fs::path("custom_output/dlls"));

    // Build custom_app (which also tests auto-building custom_dyn and copying DLL alongside)
    BuildResult custom_app_res = b.build_target(custom_app, toolchain);
    if (!custom_app_res.success) {
        std::cerr << "Custom app build failed: " << custom_app_res.error_message << "\n";
    }
    assert(custom_app_res.success);
    assert(fs::exists(custom_dll));
    assert(fs::exists(custom_app_res.artifact_path));

#if defined(_WIN32)
    // Verify runtime DLL was copied alongside custom_app in custom_output/bin
    fs::path copied_dll = fs::path("custom_output/bin") / custom_dll.filename();
    assert(fs::exists(copied_dll));
#endif

    ProcessResult custom_run_res = ProcessRunner::run({custom_app_res.artifact_path});
    if (!custom_run_res.success()) {
        std::cerr << "Custom app execution failed with code: " << custom_run_res.exit_code << "\n"
                  << "Output: " << custom_run_res.combined_output() << "\n";
    }
    assert(custom_run_res.success());
    assert(custom_run_res.stdout_output.find("Custom DLL Output: 84") != std::string::npos);

    std::cout << "[PASS] Custom output shared library pipeline tests passed with " << toolchain.name() << "!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Running Shared Library Pipeline Test Suite (Story 3.3)\n";
    std::cout << "========================================\n";

    fs::path fixture_dir = "tests/fixtures/test_shared_lib_pipeline";
    cleanup_shared_lib_fixtures(fixture_dir);
    setup_shared_lib_fixtures(fixture_dir);

    // 1. Default toolchain (Clang or GCC)
    auto toolchain = cxxgen::find_toolchain();
    assert(toolchain != nullptr);
    std::cout << "[INFO] Using Primary Toolchain: " << toolchain->name() << "\n";
    test_shared_library_pipeline(*toolchain);

    // 2. Also test with MSVC if available
    auto msvc = cxxgen::find_msvc_toolchain();
    if (msvc.has_value()) {
        std::cout << "\n[INFO] Testing with MSVC Toolchain: " << msvc->name() << "\n";
        test_shared_library_pipeline(*msvc);
    }

    cleanup_shared_lib_fixtures(fixture_dir);

    std::cout << "========================================\n";
    std::cout << "All Story 3.3 Shared Library Tests Passed Successfully!\n";
    std::cout << "========================================\n";
    return 0;
}
