#if __has_include("cxxgen.hpp")
#include "cxxgen.hpp"
#else
#include "../../include/cxxgen.hpp"
#endif

void build(cxxgen::Builder &b) {
    // Standard target options (e.g. host OS / target arch)
    auto target = b.standard_target_options();

    // Standard optimization options (Debug, Release, RelWithDebInfo, MinSizeRel)
    auto optimize = b.standard_optimize_options();

    // Define executable target
    auto &exe = b.add_executable({
        .name = "hello_world",
        .sources = {"src/main.cpp"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });

    // Mark as primary install artifact (built when running `cxxgen build`)
    b.install_artifact(exe);
}

CXXGEN_MAIN(build)
