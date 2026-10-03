#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

void setup_static_lib_fixtures(const fs::path &fixture_dir) {
    fs::create_directories(fixture_dir / "core_lib/include");
    fs::create_directories(fixture_dir / "core_lib/src");
    fs::create_directories(fixture_dir / "app/src");

    // 1. math_core.hpp
    std::ofstream hpp(fixture_dir / "core_lib/include/math_core.hpp");
    hpp << "#pragma once\n"
        << "#define CORE_MATH_MAGIC 100\n"
        << "int core_math_compute(int x);\n";
    hpp.close();

    // 2. math_core.cpp
    std::ofstream cpp(fixture_dir / "core_lib/src/math_core.cpp");
    cpp << "#include \"math_core.hpp\"\n"
        << "int core_math_compute(int x) {\n"
        << "    return x + CORE_MATH_MAGIC;\n"
        << "}\n";
    cpp.close();

    // 3. app/src/main.cpp (tests transitive include: math_core.hpp is included without explicit include_dirs in app)
    std::ofstream main(fixture_dir / "app/src/main.cpp");
    main << "#include \"math_core.hpp\"\n"
         << "#include <iostream>\n"
         << "int main() {\n"
         << "    int val = core_math_compute(50);\n"
         << "    std::cout << \"Static Link Success: \" << val << std::endl;\n"
         << "    return 0;\n"
         << "}\n";
    main.close();
}

void cleanup_static_lib_fixtures(const fs::path &fixture_dir) {
    std::error_code ec;
    fs::remove_all(fixture_dir, ec);
    fs::remove_all("bin", ec);
    fs::remove_all("lib", ec);
    fs::remove_all(".cxxgen", ec);
}

void test_static_library_pipeline(const cxxgen::IToolchain &toolchain) {
    using namespace cxxgen;
    fs::path fixture_dir = "tests/fixtures/test_static_lib_pipeline";

    Builder b;
    auto &core_lib = b.add_static_library({
        .name = "core_math",
        .sources = {(fixture_dir / "core_lib/src/math_core.cpp").string()},
        .include_dirs = {(fixture_dir / "core_lib/include").string()},
        .defines = {"CORE_MATH_ENABLED=1"},
        .cpp_version = CppVersion::Cpp20,
    });

    auto &app = b.add_executable({
        .name = "math_app",
        .sources = {(fixture_dir / "app/src/main.cpp").string()},
        .cpp_version = CppVersion::Cpp20,
    });
    // app links core_lib: should inherit transitive include_dirs and defines!
    app.link(core_lib);

    // 1. Verify static library artifact path formatting
    fs::path lib_path = b.get_artifact_path(core_lib, toolchain);
    if (toolchain.family() == CompilerFamily::Msvc) {
        assert(lib_path.extension() == ".lib");
    } else {
        assert(lib_path.extension() == ".a");
    }

    // 2. Build static library
    BuildResult lib_res = b.build_target(core_lib, toolchain);
    if (!lib_res.success) {
        std::cerr << "Static library build failed: " << lib_res.error_message << "\n";
    }
    assert(lib_res.success);
    assert(fs::exists(lib_res.artifact_path));

    // 3. Build executable (linking static library)
    BuildResult app_res = b.build_target(app, toolchain);
    if (!app_res.success) {
        std::cerr << "Executable link failed: " << app_res.error_message << "\n";
    }
    assert(app_res.success);
    assert(fs::exists(app_res.artifact_path));

    // 4. Run executable and verify static symbol resolution
    ProcessResult run_res = ProcessRunner::run({app_res.artifact_path});
    assert(run_res.success());
    assert(run_res.stdout_output.find("Static Link Success: 150") != std::string::npos);

    std::cout << "[PASS] Static library pipeline tests passed with " << toolchain.name()
              << "! Output archive: " << lib_res.artifact_path << "\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Running Static Library Pipeline Test Suite (Story 3.2)\n";
    std::cout << "========================================\n";

    fs::path fixture_dir = "tests/fixtures/test_static_lib_pipeline";
    setup_static_lib_fixtures(fixture_dir);

    // 1. Default toolchain (Clang or GCC)
    auto toolchain = cxxgen::find_toolchain();
    assert(toolchain != nullptr);
    std::cout << "[INFO] Using Primary Toolchain: " << toolchain->name() << "\n";
    test_static_library_pipeline(*toolchain);

    // 2. Also test with MSVC if available
    auto msvc = cxxgen::find_msvc_toolchain();
    if (msvc.has_value()) {
        std::cout << "\n[INFO] Testing with MSVC Toolchain: " << msvc->name() << "\n";
        test_static_library_pipeline(*msvc);
    }

    cleanup_static_lib_fixtures(fixture_dir);

    std::cout << "========================================\n";
    std::cout << "All Story 3.2 Static Library Tests Passed Successfully!\n";
    std::cout << "========================================\n";
    return 0;
}
