#if __has_include("cxxgen.hpp")
#include "cxxgen.hpp"
#else
#include "../../include/cxxgen.hpp"
#endif

void build(cxxgen::Builder &b) {
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    // 1. Static library containing core business logic
    auto &lib = b.add_static_library({
        .name = "greeting",
        .sources = {"src/greeting.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });

    // 2. Executable application linking the library
    auto &exe = b.add_executable({
        .name = "hello_world",
        .sources = {"src/main.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    exe.link(lib);
    b.install_artifact(exe);

    // 3. Test target for automated test verification
    auto &test_target = b.add_test({
        .name = "hello_test",
        .sources = {"tests/test.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    test_target.link(lib);
}

CXXGEN_MAIN(build)
