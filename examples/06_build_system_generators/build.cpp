#if __has_include("cxxgen.hpp")
#include "cxxgen.hpp"
#else
#include "../../include/cxxgen.hpp"
#endif

void build(cxxgen::Builder &b) {
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    // 1. Math static library
    auto &math_lib = b.add_static_library({
        .name = "core_math",
        .sources = {"src/core_math.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });

    // 2. Main application linking math_lib
    auto &app = b.add_executable({
        .name = "generator_demo",
        .sources = {"src/main.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    app.link(math_lib);

    b.install_artifact(app);
}

CXXGEN_MAIN(build)
