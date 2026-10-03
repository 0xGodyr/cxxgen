#if __has_include("cxxgen.hpp")
#include "cxxgen.hpp"
#else
#include "../../include/cxxgen.hpp"
#endif

void build(cxxgen::Builder &b) {
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    // Define executable target with Precompiled Header acceleration
    auto &app = b.add_executable({
        .name = "analytics_demo",
        .sources =
            {
                "src/data_processor.cpp",
                "src/report_generator.cpp",
                "src/main.cpp",
            },
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
        .pch_header = "include/pch.hpp",
    });

    b.install_artifact(app);
}

CXXGEN_MAIN(build)
