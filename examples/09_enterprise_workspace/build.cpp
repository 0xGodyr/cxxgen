#if __has_include("cxxgen.hpp")
#include "cxxgen.hpp"
#else
#include "../../include/cxxgen.hpp"
#endif

void build(cxxgen::Builder &b) {
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    // 1. Shared core engine library
    auto &engine = b.add_static_library({
        .name = "engine",
        .sources = {"engine/src/engine.cpp"},
        .include_dirs = {"engine/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    b.install_artifact(engine);

    // 2. Command-line client application
    auto &cli = b.add_executable({
        .name = "cli_client",
        .sources = {"cli_client/src/main.cpp"},
        .include_dirs = {"engine/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    cli.link(engine);
    b.install_artifact(cli);

    // 3. Background service daemon
    auto &daemon = b.add_executable({
        .name = "service_daemon",
        .sources = {"service_daemon/src/main.cpp"},
        .include_dirs = {"engine/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    daemon.link(engine);
    b.install_artifact(daemon);

    // 4. Automated unit test suite
    auto &tests = b.add_test({
        .name = "test_engine",
        .sources = {"tests/test_engine.cpp"},
        .include_dirs = {"engine/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    tests.link(engine);
}

CXXGEN_MAIN(build)
