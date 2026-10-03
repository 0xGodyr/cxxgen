#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

void setup_interface_fixtures(const fs::path &fixture_dir) {
    fs::create_directories(fixture_dir / "math_interface/include");
    fs::create_directories(fixture_dir / "core_interface/include");
    fs::create_directories(fixture_dir / "backend_lib/src");
    fs::create_directories(fixture_dir / "app/src");

    // 1. math_interface/include/fast_math.hpp (Header-only library 1)
    std::ofstream hpp1(fixture_dir / "math_interface/include/fast_math.hpp");
    hpp1 << "#pragma once\n"
         << "#ifndef MATH_INTERFACE_VAL\n"
         << "#error \"MATH_INTERFACE_VAL define was not propagated!\"\n"
         << "#endif\n\n"
         << "inline int fast_add(int a, int b) {\n"
         << "    return a + b + MATH_INTERFACE_VAL;\n"
         << "}\n";
    hpp1.close();

    // 2. core_interface/include/core_types.hpp (Header-only library 2, chained into math_interface)
    std::ofstream hpp2(fixture_dir / "core_interface/include/core_types.hpp");
    hpp2 << "#pragma once\n"
         << "#ifndef CORE_INTERFACE_ACTIVE\n"
         << "#error \"CORE_INTERFACE_ACTIVE define was not propagated!\"\n"
         << "#endif\n\n"
         << "struct Vec2D { int x; int y; };\n";
    hpp2.close();

    // 3. backend_lib/src/backend.cpp (Static library linked through an interface library)
    std::ofstream bcpp(fixture_dir / "backend_lib/src/backend.cpp");
    bcpp << "int backend_secret_multiplier() {\n"
         << "    return 10;\n"
         << "}\n";
    bcpp.close();

    // 4. app/src/main.cpp (Consuming executable)
    std::ofstream main(fixture_dir / "app/src/main.cpp");
    main << "#include \"fast_math.hpp\"\n"
         << "#include \"core_types.hpp\"\n"
         << "#include <iostream>\n\n"
         << "extern int backend_secret_multiplier();\n\n"
         << "int main() {\n"
         << "    Vec2D vec{3, 4};\n"
         << "    int sum = fast_add(vec.x, vec.y);\n"
         << "    int total = sum * backend_secret_multiplier();\n"
         << "    std::cout << \"InterfaceApp Total: \" << total << std::endl;\n"
         << "    return 0;\n"
         << "}\n";
    main.close();
}

void cleanup_interface_fixtures(const fs::path &fixture_dir) {
    std::error_code ec;
    fs::remove_all(fixture_dir, ec);
    fs::remove_all("bin", ec);
    fs::remove_all("lib", ec);
    fs::remove_all(".cxxgen", ec);
}

void test_interface_library_pipeline(const cxxgen::IToolchain &toolchain) {
    using namespace cxxgen;
    fs::path fixture_dir = "tests/fixtures/test_interface_pipeline";

    Builder b;

    // 1. Core interface library (nested)
    auto &core_iface = b.add_interface_library({
        .name = "core_interface",
        .include_dirs = {(fixture_dir / "core_interface/include").string()},
        .defines = {"CORE_INTERFACE_ACTIVE=1"},
    });

    // 2. Static backend library
    auto &backend_lib = b.add_static_library({
        .name = "backend_lib",
        .sources = {(fixture_dir / "backend_lib/src/backend.cpp").string()},
        .cpp_version = CppVersion::Cpp20,
    });

    // 3. Math interface library linking core_iface AND backend_lib
    auto &math_iface = b.add_interface_library({
        .name = "math_interface",
        .include_dirs = {(fixture_dir / "math_interface/include").string()},
        .defines = {"MATH_INTERFACE_VAL=100"},
    });
    math_iface.link(core_iface);
    math_iface.link(backend_lib);

    // 4. Consuming executable linking ONLY math_iface
    auto &app = b.add_executable({
        .name = "interface_app",
        .sources = {(fixture_dir / "app/src/main.cpp").string()},
        .cpp_version = CppVersion::Cpp20,
    });
    app.link(math_iface);

    // Acceptance Criterion 1: Verify get_artifact_path returns empty path for interface targets
    fs::path math_iface_path = b.get_artifact_path(math_iface, toolchain);
    assert(math_iface_path.empty());

    // Acceptance Criterion 1: Verify no compilation or link commands are emitted for interface targets
    BuildResult iface_res = b.build_target(math_iface, toolchain);
    assert(iface_res.success);
    assert(iface_res.exit_code == 0);
    assert(iface_res.artifact_path.empty());
    assert(iface_res.object_files.empty());

    // Acceptance Criterion 2: Verify transitive includes and defines propagate to consuming target
    // Build consuming executable
    BuildResult app_res = b.build_target(app, toolchain);
    if (!app_res.success) {
        std::cerr << "Consuming executable failed to build: " << app_res.error_message << "\n";
    }
    assert(app_res.success);
    assert(fs::exists(app_res.artifact_path));

    // Run consuming executable
    // (vec.x=3 + vec.y=4 + MATH_INTERFACE_VAL=100) = 107 * backend_multiplier=10 = 1070
    ProcessResult run_res = ProcessRunner::run({app_res.artifact_path});
    if (!run_res.success()) {
        std::cerr << "Run failed with code: " << run_res.exit_code << "\n"
                  << "Output: " << run_res.combined_output() << "\n";
    }
    assert(run_res.success());
    assert(run_res.stdout_output.find("InterfaceApp Total: 1070") != std::string::npos);

    std::cout << "[PASS] Interface library pipeline tests passed with " << toolchain.name() << "!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Running Interface Library Pipeline Test Suite (Story 3.5)\n";
    std::cout << "========================================\n";

    fs::path fixture_dir = "tests/fixtures/test_interface_pipeline";
    cleanup_interface_fixtures(fixture_dir);
    setup_interface_fixtures(fixture_dir);

    // 1. Default toolchain (Clang or GCC)
    auto toolchain = cxxgen::find_toolchain();
    assert(toolchain != nullptr);
    std::cout << "[INFO] Using Primary Toolchain: " << toolchain->name() << "\n";
    test_interface_library_pipeline(*toolchain);

    // 2. Also test with MSVC if available
    auto msvc = cxxgen::find_msvc_toolchain();
    if (msvc.has_value()) {
        std::cout << "\n[INFO] Testing with MSVC Toolchain: " << msvc->name() << "\n";
        test_interface_library_pipeline(*msvc);
    }

    cleanup_interface_fixtures(fixture_dir);

    std::cout << "========================================\n";
    std::cout << "All Story 3.5 Interface Library Tests Passed Successfully!\n";
    std::cout << "========================================\n";
    return 0;
}
