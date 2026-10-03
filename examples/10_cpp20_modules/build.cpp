#if __has_include("cxxgen.hpp")
#include "cxxgen.hpp"
#else
#include "../../include/cxxgen.hpp"
#endif

void build(cxxgen::Builder &b) {
    // Target architecture & optimization settings
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    // Define executable target consuming C++20 modules
    // cxxgen automatically scans source files with P1689 scanner, detects:
    // 1. src/math.ixx defines module 'math'
    // 2. src/greeter.ixx defines module 'greeter' and imports 'math'
    // 3. src/main.cpp imports 'math' and 'greeter'
    // It automatically orders BMI compilation: math -> greeter -> main!
    auto &demo = b.add_executable({
        .name = "modules_demo",
        .sources =
            {
                "src/math.ixx",
                "src/greeter.ixx",
                "src/main.cpp",
            },
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });

    // Mark as primary install artifact
    b.install_artifact(demo);
}

CXXGEN_MAIN(build)
