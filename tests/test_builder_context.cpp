#include "cxxgen.hpp"

#include <cassert>
#include <iostream>

void test_builder_target_creation_and_linking() {
    using namespace cxxgen;
    Builder b;

    auto &core_lib = b.add_shared_library({
        .name = "core_engine",
        .sources = {"src/engine.cpp"},
        .include_dirs = {"include"},
        .defines = {"BUILD_DLL"},
    });
    assert(core_lib.name() == "core_engine");
    assert(core_lib.type() == TargetType::SharedLibrary);

    auto &audio_plugin = b.add_plugin({
        .name = "audio_plugin",
        .sources = {"plugins/audio.cpp"},
    });
    audio_plugin.link(core_lib);
    audio_plugin.link_system_lib("openal32");
    assert(audio_plugin.type() == TargetType::Plugin);
    assert(audio_plugin.linked_targets().size() == 1);
    assert(audio_plugin.linked_targets()[0] == "core_engine");
    assert(audio_plugin.linked_system_libs().size() == 1);
    assert(audio_plugin.linked_system_libs()[0] == "openal32");

    auto &exe = b.add_executable({
        .name = "game_app",
        .sources = {"src/main.cpp"},
    });
    exe.link(core_lib);
    exe.add_source("src/game.cpp");
    exe.add_include_dir("game/include");
    exe.add_define("GAME_RELEASE=1");

    assert(exe.config().sources.size() == 2);
    assert(exe.config().include_dirs.size() == 1);
    assert(exe.config().defines.size() == 1);
    assert(exe.linked_targets().size() == 1);
    assert(exe.linked_targets()[0] == "core_engine");

    // Reference stability check: core_lib is still valid
    assert(core_lib.name() == "core_engine");

    // Static library & interface library & test
    auto &math_lib = b.add_static_library({.name = "math_lib"});
    assert(math_lib.type() == TargetType::StaticLibrary);

    auto &header_lib = b.add_interface_library({.name = "glm_headers"});
    assert(header_lib.type() == TargetType::Interface);

    auto &tests = b.add_test({.name = "engine_tests"});
    assert(tests.type() == TargetType::Test);

    assert(b.targets().size() == 6);
    assert(b.find_target("game_app") != nullptr);
    assert(b.find_target("game_app")->name() == "game_app");
    assert(b.find_target("non_existent") == nullptr);

    std::cout << "[PASS] Target creation and linking tests passed!\n";
}

void test_install_artifacts_and_steps() {
    using namespace cxxgen;
    Builder b;

    auto &exe = b.add_executable({.name = "app"});
    auto &lib = b.add_shared_library({.name = "shared"});

    b.install_artifact(exe);
    b.install_artifact(lib);
    // Duplicate install should be idempotent
    b.install_artifact(exe);

    assert(b.installed_artifacts().size() == 2);
    assert(b.installed_artifacts()[0] == "app");
    assert(b.installed_artifacts()[1] == "shared");

    // Check install step exists and contains dependencies
    Step *install_step = b.find_step("install");
    assert(install_step != nullptr);
    assert(install_step->dependencies().size() == 2);
    assert(install_step->dependencies()[0] == "app");
    assert(install_step->dependencies()[1] == "shared");

    // Custom steps and dependencies
    auto &run_step = b.step("run", "Run the application");
    run_step.depend_on(exe);
    assert(run_step.dependencies().size() == 1);
    assert(run_step.dependencies()[0] == "app");

    std::cout << "[PASS] Install artifacts and step tests passed!\n";
}

void test_target_collision_and_validation() {
    using namespace cxxgen;
    Builder b;

    b.add_executable({.name = "unique_app"});

    bool collision_caught = false;
    try {
        b.add_static_library({.name = "unique_app"});
    } catch (const std::invalid_argument &e) {
        collision_caught = true;
        std::string msg = e.what();
        assert(msg.find("unique_app") != std::string::npos);
    }
    assert(collision_caught);

    bool empty_name_caught = false;
    try {
        b.add_executable({.name = ""});
    } catch (const std::invalid_argument &) {
        empty_name_caught = true;
    }
    assert(empty_name_caught);

    std::cout << "[PASS] Target collision and validation tests passed!\n";
}

void test_standard_options() {
    using namespace cxxgen;
    Builder b;

    auto target_opts = b.standard_target_options();
    assert(target_opts.arch == "native");
    assert(target_opts.os == "host");

    auto opt_opts = b.standard_optimize_options();
    assert(opt_opts.mode == Optimization::Debug);
    assert(opt_opts.lto == false);

    b.set_standard_optimize_options({.mode = Optimization::ReleaseFast, .lto = true});
    assert(b.standard_optimize_options().mode == Optimization::ReleaseFast);
    assert(b.standard_optimize_options().lto == true);

    std::cout << "[PASS] Standard options tests passed!\n";
}

int main() {
    std::cout << "Running Story 1.2 Builder Context Test Suite...\n";
    test_builder_target_creation_and_linking();
    test_install_artifacts_and_steps();
    test_target_collision_and_validation();
    test_standard_options();
    std::cout << "All Story 1.2 Tests Passed Successfully!\n";
    return 0;
}
