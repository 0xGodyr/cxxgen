#if __has_include("cxxgen.hpp")
#include "cxxgen.hpp"
#else
#include "../../include/cxxgen.hpp"
#endif

void build(cxxgen::Builder &b) {
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    // 1. ROT13 dynamic plugin
    auto &plug_rot13 = b.add_plugin({
        .name = "plugin_rot13",
        .sources = {"plugins/plugin_rot13/src/plugin_rot13.cpp"},
        .include_dirs = {"include"},
        .defines = {"BUILDING_PLUGIN"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
        .output_dir = "bin/plugins",
    });
    b.install_artifact(plug_rot13);

    // 2. Reverse dynamic plugin
    auto &plug_rev = b.add_plugin({
        .name = "plugin_reverse",
        .sources = {"plugins/plugin_reverse/src/plugin_reverse.cpp"},
        .include_dirs = {"include"},
        .defines = {"BUILDING_PLUGIN"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
        .output_dir = "bin/plugins",
    });
    b.install_artifact(plug_rev);

    // 3. Host application that dynamically discovers and loads plugins
    auto &host = b.add_executable({
        .name = "plugin_host",
        .sources = {"host/src/main.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    b.install_artifact(host);
}

CXXGEN_MAIN(build)
