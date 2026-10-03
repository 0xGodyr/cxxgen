#include "cxxgen.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

void test_cli_argument_parsing() {
    using namespace cxxgen;

    // Test default command
    {
        char *argv[] = {(char *)"build_runner"};
        auto args = parse_cli_args(1, argv);
        assert(args.command == "build");
        assert(!args.show_help);
        assert(!args.show_version);
    }

    // Test help flags
    {
        char *argv[] = {(char *)"build_runner", (char *)"--help"};
        auto args = parse_cli_args(2, argv);
        assert(args.show_help);
    }
    {
        char *argv[] = {(char *)"build_runner", (char *)"-h"};
        auto args = parse_cli_args(2, argv);
        assert(args.show_help);
    }

    // Test version flags
    {
        char *argv[] = {(char *)"build_runner", (char *)"--version"};
        auto args = parse_cli_args(2, argv);
        assert(args.show_version);
    }

    // Test optimization flags
    {
        char *argv[] = {(char *)"build_runner", (char *)"-O", (char *)"ReleaseFast"};
        auto args = parse_cli_args(3, argv);
        assert(args.optimize.has_value());
        assert(*args.optimize == Optimization::ReleaseFast);
    }
    {
        char *argv[] = {(char *)"build_runner", (char *)"--optimize=ReleaseSmall"};
        auto args = parse_cli_args(2, argv);
        assert(args.optimize.has_value());
        assert(*args.optimize == Optimization::ReleaseSmall);
    }

    // Test compiler flag
    {
        char *argv[] = {(char *)"build_runner", (char *)"--compiler=clang"};
        auto args = parse_cli_args(2, argv);
        assert(args.compiler.has_value());
        assert(*args.compiler == CompilerFamily::Clang);
    }

    // Test jobs flag
    {
        char *argv[] = {(char *)"build_runner", (char *)"-j", (char *)"8"};
        auto args = parse_cli_args(3, argv);
        assert(args.jobs == 8);
    }
    {
        char *argv[] = {(char *)"build_runner", (char *)"--jobs=16"};
        auto args = parse_cli_args(2, argv);
        assert(args.jobs == 16);
    }

    // Test command and passthrough args
    {
        char *argv[] = {(char *)"build_runner", (char *)"run",       (char *)"my_app",
                        (char *)"--",           (char *)"--verbose", (char *)"-f"};
        auto args = parse_cli_args(6, argv);
        assert(args.command == "run");
        assert(args.target_or_step == "my_app");
        assert(args.passthrough_args.size() == 2);
        assert(args.passthrough_args[0] == "--verbose");
        assert(args.passthrough_args[1] == "-f");
    }

    // Test generate command
    {
        char *argv[] = {(char *)"build_runner", (char *)"generate", (char *)"cmake"};
        auto args = parse_cli_args(3, argv);
        assert(args.command == "generate");
        assert(args.generator_format == "cmake");
    }

    std::cout << "[PASS] CLI argument parsing tests passed!\n";
}

void sample_build_fn(cxxgen::Builder &b) {
    auto &exe = b.add_executable({.name = "demo_app", .sources = {"test_demo.cpp"}});
    b.install_artifact(exe);
    b.add_test({.name = "unit_tests", .sources = {"test_demo.cpp"}});
}

void test_run_cli_dispatch() {
    using namespace cxxgen;

    {
        std::ofstream of("test_demo.cpp");
        of << "int main() { return 0; }\n";
    }

    // Test help dispatch
    {
        char *argv[] = {(char *)"build_runner", (char *)"--help"};
        int code = run_cli(2, argv, sample_build_fn);
        assert(code == 0);
    }

    // Test version dispatch
    {
        char *argv[] = {(char *)"build_runner", (char *)"--version"};
        int code = run_cli(2, argv, sample_build_fn);
        assert(code == 0);
    }

    // Test build dispatch
    {
        char *argv[] = {(char *)"build_runner", (char *)"build", (char *)"-O", (char *)"ReleaseFast"};
        int code = run_cli(4, argv, sample_build_fn);
        assert(code == 0);
    }

    // Test run dispatch
    {
        char *argv[] = {(char *)"build_runner", (char *)"run"};
        int code = run_cli(2, argv, sample_build_fn);
        assert(code == 0);
    }

    // Test test dispatch
    {
        char *argv[] = {(char *)"build_runner", (char *)"test"};
        int code = run_cli(2, argv, sample_build_fn);
        assert(code == 0);
    }

    // Test generate dispatch
    {
        char *argv[] = {(char *)"build_runner", (char *)"generate", (char *)"cmake"};
        int code = run_cli(3, argv, sample_build_fn);
        assert(code == 0);
    }

    // Test clean dispatch
    {
        char *argv[] = {(char *)"build_runner", (char *)"clean"};
        int code = run_cli(2, argv, sample_build_fn);
        assert(code == 0);
    }

    std::filesystem::remove("test_demo.cpp");
    std::filesystem::remove("CMakeLists.txt");

    std::cout << "[PASS] run_cli dispatch tests passed!\n";
}

int main() {
    std::cout << "Running Story 1.3 CLI Entrypoint Test Suite...\n";
    test_cli_argument_parsing();
    test_run_cli_dispatch();
    std::cout << "All Story 1.3 Tests Passed Successfully!\n";
    return 0;
}
