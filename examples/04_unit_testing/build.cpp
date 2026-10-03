#if __has_include("cxxgen.hpp")
#include "cxxgen.hpp"
#else
#include "../../include/cxxgen.hpp"
#endif

void build(cxxgen::Builder &b) {
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    // 1. Library under test
    auto &calc_lib = b.add_static_library({
        .name = "calculator",
        .sources = {"src/calculator.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    b.install_artifact(calc_lib);

    // 2. Basic test suite
    auto &test_basic = b.add_test({
        .name = "test_basic",
        .sources = {"tests/test_basic.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    test_basic.link(calc_lib);

    // 3. Advanced test suite
    auto &test_adv = b.add_test({
        .name = "test_advanced",
        .sources = {"tests/test_advanced.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    test_adv.link(calc_lib);
}

CXXGEN_MAIN(build)
