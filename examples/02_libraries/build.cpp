#if __has_include("cxxgen.hpp")
#include "cxxgen.hpp"
#else
#include "../../include/cxxgen.hpp"
#endif

void build(cxxgen::Builder &b) {
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    // 1. Static library (archived to lib/libmath_static.a or lib/math_static.lib)
    auto &math_lib = b.add_static_library({
        .name = "math_static",
        .sources = {"math_static/src/math_static.cpp"},
        .include_dirs = {"math_static/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });

    // 2. Shared dynamic library (linked to bin/string_shared.dll or lib/libstring_shared.so)
    auto &str_lib = b.add_shared_library({
        .name = "string_shared",
        .sources = {"string_shared/src/string_shared.cpp"},
        .include_dirs = {"string_shared/include"},
        .defines = {"BUILDING_STRING_SHARED"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });

    // 3. Executable linking both static and shared libraries
    auto &app = b.add_executable({
        .name = "library_consumer",
        .sources = {"app/src/main.cpp"},
        .include_dirs = {"math_static/include", "string_shared/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });

    // Wire dependency graph and linking
    app.link(math_lib);
    app.link(str_lib);

    // Install artifacts
    b.install_artifact(app);
}

CXXGEN_MAIN(build)
