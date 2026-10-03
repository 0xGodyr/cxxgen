#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void test_scaffold_exe() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_scaffold_exe";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path current_cxxgen_header = fs::absolute("include/cxxgen.hpp");
    assert(fs::exists(current_cxxgen_header));

    cxxgen::ProjectScaffolderConfig cfg{
        .target_dir = test_dir,
        .project_name = "test_app",
        .type = "exe",
        .force = false,
        .verbose = false,
        .header_source = current_cxxgen_header,
    };

    auto res = cxxgen::ProjectScaffolder::scaffold(cfg);
    assert(res.success);
    assert(fs::exists(test_dir / ".gitignore"));
    assert(fs::exists(test_dir / ".clang-format"));
    assert(fs::exists(test_dir / ".clang-tidy"));
    assert(fs::exists(test_dir / ".clangd"));
    assert(fs::exists(test_dir / "build.cpp"));
    assert(fs::exists(test_dir / "src" / "main.cpp"));
    assert(fs::exists(test_dir / "src" / "mathlib" / "math.hpp"));
    assert(fs::exists(test_dir / "src" / "mathlib" / "math.cpp"));
    assert(fs::exists(test_dir / "tests" / "test_math.cpp"));
    assert(!fs::exists(test_dir / "include" / "cxxgen.hpp"));
    assert(fs::exists(test_dir / "include" / "cxxgen" / "cxxgen.hpp"));

    // Verify file contents
    {
        std::ifstream f(test_dir / "src" / "main.cpp");
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        assert(content.find("[[maybe_unused]] int argc") != std::string::npos);
        assert(content.find("[[maybe_unused]] char **argv") != std::string::npos);
        assert(content.find("(void)argc;") == std::string::npos);
    }
    {
        std::ifstream f(test_dir / "tests" / "test_math.cpp");
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        assert(content.find("<gtest/gtest.h>") != std::string::npos);
        assert(content.find("<gmock/gmock.h>") != std::string::npos);
        assert(content.find("MOCK_METHOD") != std::string::npos);
    }
    {
        std::ifstream f(test_dir / "build.cpp");
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        assert(content.find("googletest") != std::string::npos);
        assert(content.find("gmock_main") != std::string::npos);
        assert(content.find("test_math") != std::string::npos);
    }

    // Verify the scaffolded project immediately builds, tests, and runs!
    cxxgen::JitRunnerConfig jcfg{
        .working_dir = test_dir,
        .build_script = test_dir / "build.cpp",
        .cxxgen_header = test_dir / "include" / "cxxgen" / "cxxgen.hpp",
        .verbose = false,
        .force_rebuild = false,
    };
    auto jres = cxxgen::JitRunner::prepare_runner(jcfg);
    assert(jres.success);
    assert(fs::exists(jres.runner_binary));

    // Execute the runner with "build" command
    int build_code = cxxgen::JitRunner::execute_runner(jres.runner_binary, {"build"}, test_dir);
    assert(build_code == 0);

#if defined(_WIN32)
    fs::path app_bin = test_dir / "bin" / "test_app.exe";
#else
    fs::path app_bin = test_dir / "bin" / "test_app";
#endif
    assert(fs::exists(app_bin));

    // Execute the runner with "test" command to verify out-of-the-box unit tests!
    int test_code = cxxgen::JitRunner::execute_runner(jres.runner_binary, {"test"}, test_dir);
    assert(test_code == 0);

    // Execute the runner with "run" command
    int run_code = cxxgen::JitRunner::execute_runner(jres.runner_binary, {"run"}, test_dir);
    assert(run_code == 0);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_scaffold_exe\n";
}

void test_scaffold_lib() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_scaffold_lib";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path current_cxxgen_header = fs::absolute("include/cxxgen.hpp");
    assert(fs::exists(current_cxxgen_header));

    cxxgen::ProjectScaffolderConfig cfg{
        .target_dir = test_dir,
        .project_name = "math_lib",
        .type = "lib",
        .force = false,
        .verbose = false,
        .header_source = current_cxxgen_header,
    };

    auto res = cxxgen::ProjectScaffolder::scaffold(cfg);
    assert(res.success);
    assert(fs::exists(test_dir / ".gitignore"));
    assert(fs::exists(test_dir / "build.cpp"));
    assert(fs::exists(test_dir / "include" / "math_lib" / "math_lib.hpp"));
    assert(fs::exists(test_dir / "src" / "math_lib.cpp"));

    // Verify build script
    cxxgen::JitRunnerConfig jcfg{
        .working_dir = test_dir,
        .build_script = test_dir / "build.cpp",
        .cxxgen_header = test_dir / "include" / "cxxgen" / "cxxgen.hpp",
        .verbose = false,
    };
    auto jres = cxxgen::JitRunner::prepare_runner(jcfg);
    assert(jres.success);

    int build_code = cxxgen::JitRunner::execute_runner(jres.runner_binary, {"build"}, test_dir);
    assert(build_code == 0);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_scaffold_lib\n";
}

void test_scaffold_plugin() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_scaffold_plugin";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path current_cxxgen_header = fs::absolute("include/cxxgen.hpp");
    assert(fs::exists(current_cxxgen_header));

    cxxgen::ProjectScaffolderConfig cfg{
        .target_dir = test_dir,
        .project_name = "audio_plugin",
        .type = "plugin",
        .force = false,
        .verbose = false,
        .header_source = current_cxxgen_header,
    };

    auto res = cxxgen::ProjectScaffolder::scaffold(cfg);
    assert(res.success);
    assert(fs::exists(test_dir / ".gitignore"));
    assert(fs::exists(test_dir / "build.cpp"));
    assert(fs::exists(test_dir / "include" / "plugin_api.hpp"));
    assert(fs::exists(test_dir / "src" / "plugin.cpp"));

    cxxgen::JitRunnerConfig jcfg{
        .working_dir = test_dir,
        .build_script = test_dir / "build.cpp",
        .cxxgen_header = test_dir / "include" / "cxxgen" / "cxxgen.hpp",
        .verbose = false,
    };
    auto jres = cxxgen::JitRunner::prepare_runner(jcfg);
    assert(jres.success);

    int build_code = cxxgen::JitRunner::execute_runner(jres.runner_binary, {"build"}, test_dir);
    assert(build_code == 0);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_scaffold_plugin\n";
}

void test_overwrite_protection() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_scaffold_overwrite";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path current_cxxgen_header = fs::absolute("include/cxxgen.hpp");

    cxxgen::ProjectScaffolderConfig cfg{
        .target_dir = test_dir,
        .project_name = "guard_app",
        .type = "exe",
        .force = false,
        .verbose = false,
        .header_source = current_cxxgen_header,
    };

    // First scaffold: succeeds
    auto res1 = cxxgen::ProjectScaffolder::scaffold(cfg);
    assert(res1.success);

    // Second scaffold without force: MUST refuse to overwrite
    auto res2 = cxxgen::ProjectScaffolder::scaffold(cfg);
    assert(!res2.success);
    assert(res2.error_message.find("file already exists") != std::string::npos);
    assert(res2.error_message.find("--force") != std::string::npos);

    // Third scaffold with force: MUST succeed
    cfg.force = true;
    auto res3 = cxxgen::ProjectScaffolder::scaffold(cfg);
    assert(res3.success);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_overwrite_protection\n";
}

void test_run_cli_init() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_run_cli_init";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    std::string dir_str = test_dir.string();
    char *argv[] = {(char *)"cxxgen", (char *)"init", (char *)"--type=exe", (char *)"-C", (char *)dir_str.c_str()};

    int ret = cxxgen::run_cli(5, argv, [](cxxgen::Builder &b) { (void)b; });
    assert(ret == 0);
    assert(fs::exists(test_dir / "build.cpp"));
    assert(fs::exists(test_dir / "src" / "main.cpp"));
    assert(fs::exists(test_dir / "src" / "mathlib" / "math.hpp"));
    assert(fs::exists(test_dir / "src" / "mathlib" / "math.cpp"));
    assert(fs::exists(test_dir / "tests" / "test_math.cpp"));

    for (int retry = 0; retry < 5 && fs::exists(test_dir, ec); ++retry) {
        fs::remove_all(test_dir, ec);
        if (ec) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
    std::cout << "[PASS] test_run_cli_init\n";
}

int main() {
    std::cout << "Running test_scaffolder...\n";
    test_scaffold_exe();
    test_scaffold_lib();
    test_scaffold_plugin();
    test_overwrite_protection();
    test_run_cli_init();
    std::cout << "All scaffolder tests passed successfully!\n";
    return 0;
}
