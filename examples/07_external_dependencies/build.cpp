#if __has_include("cxxgen.hpp")
#include "cxxgen.hpp"
#else
#include "../../include/cxxgen.hpp"
#endif

void build(cxxgen::Builder &b) {
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    // 1. Discover standard system package (e.g. threads)
    auto threads_pkg =
        b.find_system_package("threads", {
                                             .required = false,
                                             .install_hint = "Threading is part of standard C++ runtime or libpthread.",
                                         });

    // 2. Discover optional third-party system package (e.g. zlib or vulkan)
    auto zlib_pkg = b.find_system_package(
        "zlib", {
                    .required = false,
                    .install_hint = "Install via scoop (scoop install zlib) or apt (apt install zlib1g-dev).",
                });

    // 3. Define consumer application
    auto &app = b.add_executable({
        .name = "dependency_demo",
        .sources = {"src/main.cpp"},
        .defines = {"HAVE_CUSTOM_DEF"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });

    // If threads package found, link it into app
    if (threads_pkg) {
        app.link(threads_pkg);
        app.config().defines.push_back("HAVE_THREADS");
    }

    // If zlib is found, link it into app
    if (zlib_pkg) {
        app.link(zlib_pkg);
        app.config().defines.push_back("HAVE_ZLIB");
    }

    b.install_artifact(app);
}

CXXGEN_MAIN(build)
