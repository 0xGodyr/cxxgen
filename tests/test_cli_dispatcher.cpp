#include "cxxgen.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

void test_parse_defaults() {
    char *argv[] = {(char *)"cxxgen"};
    int argc = 1;
    cxxgen::CliArgs args = cxxgen::parse_cli_args(argc, argv);
    assert(args.command == "build");
    assert(args.target_or_step.empty());
    assert(!args.verbose);
    assert(!args.show_help);
    assert(!args.show_version);
    assert(!args.force);
    assert(args.init_type == "exe");
    assert(args.passthrough_args.empty());
    std::cout << "[PASS] test_parse_defaults\n";
}

void test_parse_commands() {
    {
        char *argv[] = {(char *)"cxxgen", (char *)"run", (char *)"my_app"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(3, argv);
        assert(args.command == "run");
        assert(args.target_or_step == "my_app");
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"test"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(2, argv);
        assert(args.command == "test");
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"clean"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(2, argv);
        assert(args.command == "clean");
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"generate", (char *)"ninja"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(3, argv);
        assert(args.command == "generate");
        assert(args.generator_format == "ninja");
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"init", (char *)"--type=lib", (char *)"--force"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(4, argv);
        assert(args.command == "init");
        assert(args.init_type == "lib");
        assert(args.force);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"watch"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(2, argv);
        assert(args.command == "watch");
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"toolchain", (char *)"detect"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(3, argv);
        assert(args.command == "toolchain");
        assert(args.toolchain_action == "detect");
    }
    std::cout << "[PASS] test_parse_commands\n";
}

void test_parse_flags() {
    {
        char *argv[] = {(char *)"cxxgen", (char *)"-O=ReleaseFast", (char *)"--compiler=clang", (char *)"-j8",
                        (char *)"-v"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(5, argv);
        assert(args.optimize.has_value() && *args.optimize == cxxgen::Optimization::ReleaseFast);
        assert(args.compiler == cxxgen::CompilerFamily::Clang);
        assert(args.jobs == 8);
        assert(args.verbose);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"-C", (char *)"subproject", (char *)"--jobs", (char *)"4"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(5, argv);
        assert(args.work_dir == "subproject");
        assert(args.jobs == 4);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"--optimize", (char *)"ReleaseSmall", (char *)"--compiler",
                        (char *)"gcc"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(5, argv);
        assert(args.optimize.has_value() && *args.optimize == cxxgen::Optimization::ReleaseSmall);
        assert(args.compiler == cxxgen::CompilerFamily::Gcc);
    }
    std::cout << "[PASS] test_parse_flags\n";
}

void test_passthrough_args() {
    char *argv[] = {(char *)"cxxgen", (char *)"run",  (char *)"my_server", (char *)"--",
                    (char *)"--port", (char *)"8080", (char *)"-v",        (char *)"config.json"};
    cxxgen::CliArgs args = cxxgen::parse_cli_args(8, argv);
    assert(args.command == "run");
    assert(args.target_or_step == "my_server");
    assert(!args.verbose); // The -v was AFTER --, so it must not set verbose flag
    assert(args.passthrough_args.size() == 4);
    assert(args.passthrough_args[0] == "--port");
    assert(args.passthrough_args[1] == "8080");
    assert(args.passthrough_args[2] == "-v");
    assert(args.passthrough_args[3] == "config.json");
    std::cout << "[PASS] test_passthrough_args\n";
}

void test_help_and_version() {
    {
        char *argv[] = {(char *)"cxxgen", (char *)"--help"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(2, argv);
        assert(args.show_help);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"-h"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(2, argv);
        assert(args.show_help);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"help"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(2, argv);
        assert(args.show_help);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"--version"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(2, argv);
        assert(args.show_version);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"-V"};
        cxxgen::CliArgs args = cxxgen::parse_cli_args(2, argv);
        assert(args.show_version);
    }

    std::ostringstream ss_help;
    cxxgen::print_help(ss_help);
    std::string help_str = ss_help.str();
    assert(help_str.find("cxxgen") != std::string::npos);
    assert(help_str.find("build") != std::string::npos);
    assert(help_str.find("run") != std::string::npos);
    assert(help_str.find("test") != std::string::npos);
    assert(help_str.find("generate") != std::string::npos);
    assert(help_str.find("init") != std::string::npos);
    assert(help_str.find("watch") != std::string::npos);
    assert(help_str.find("toolchain") != std::string::npos);
    assert(help_str.find("--force") != std::string::npos);
    assert(help_str.find("--work-dir") != std::string::npos);

    std::ostringstream ss_ver;
    cxxgen::print_version(ss_ver);
    std::string ver_str = ss_ver.str();
    assert(ver_str.find("cxxgen version") != std::string::npos);

    std::cout << "[PASS] test_help_and_version\n";
}

void test_run_cli_help_and_clean() {
    {
        char *argv[] = {(char *)"cxxgen", (char *)"--help"};
        int res = cxxgen::run_cli(2, argv, [](cxxgen::Builder &b) { (void)b; });
        assert(res == 0);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"--version"};
        int res = cxxgen::run_cli(2, argv, [](cxxgen::Builder &b) { (void)b; });
        assert(res == 0);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"clean"};
        int res = cxxgen::run_cli(2, argv, [](cxxgen::Builder &b) { (void)b; });
        assert(res == 0);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"toolchain"};
        int res = cxxgen::run_cli(2, argv, [](cxxgen::Builder &b) { (void)b; });
        assert(res == 0);
    }
    std::cout << "[PASS] test_run_cli_help_and_clean\n";
}

int main() {
    std::cout << "Running test_cli_dispatcher...\n";
    test_parse_defaults();
    test_parse_commands();
    test_parse_flags();
    test_passthrough_args();
    test_help_and_version();
    test_run_cli_help_and_clean();
    std::cout << "All CLI dispatcher tests passed successfully!\n";
    return 0;
}
