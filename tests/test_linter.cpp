#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void test_find_clang_tidy() {
    auto tidy = cxxgen::find_clang_tidy();
    assert(tidy.has_value());
    assert(fs::exists(*tidy));
    std::cout << "[PASS] test_find_clang_tidy: found " << *tidy << "\n";
}

void test_is_lintable() {
    using namespace cxxgen;
    assert(CodeLinter::is_lintable("src/main.cpp"));
    assert(CodeLinter::is_lintable("core/engine.cxx"));
    assert(CodeLinter::is_lintable("util.cc"));
    assert(CodeLinter::is_lintable("c_api.c"));
    assert(!CodeLinter::is_lintable("header.hpp"));
    assert(!CodeLinter::is_lintable("readme.txt"));
    assert(!CodeLinter::is_lintable("script.py"));
    std::cout << "[PASS] test_is_lintable\n";
}

void test_lint_all_clean_file() {
    auto tidy_opt = cxxgen::find_clang_tidy();
    assert(tidy_opt.has_value());
    std::string tidy = *tidy_opt;

    fs::path test_dir = fs::current_path() / ".cxxgen_test_lint_clean";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path clean_file = test_dir / "clean.cpp";
    {
        std::ofstream out(clean_file);
        out << "int add(int a, int b) {\n"
            << "    return a + b;\n"
            << "}\n";
    }

    cxxgen::LintConfig cfg;
    cfg.paths = {clean_file};
    cfg.build_path = test_dir;
    cfg.fix = false;

    auto res = cxxgen::CodeLinter::lint_all(tidy, cfg);
    assert(res.success);
    assert(res.files_scanned == 1);
    assert(res.errors_count == 0);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_lint_all_clean_file\n";
}

void test_lint_recursive_directory_filtering() {
    auto tidy_opt = cxxgen::find_clang_tidy();
    assert(tidy_opt.has_value());
    std::string tidy = *tidy_opt;

    fs::path test_dir = fs::current_path() / ".cxxgen_test_lint_rec";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "src", ec);
    fs::create_directories(test_dir / "bin", ec); // ignored dir

    fs::path f1 = test_dir / "src" / "one.cpp";
    fs::path f2 = test_dir / "src" / "two.cpp";
    fs::path f_ignored = test_dir / "bin" / "ignored.cpp";

    {
        std::ofstream out(f1);
        out << "int f1() { return 1; }\n";
    }
    {
        std::ofstream out(f2);
        out << "int f2() { return 2; }\n";
    }
    {
        std::ofstream out(f_ignored);
        out << "int ignored() { return 0; }\n";
    }

    cxxgen::LintConfig cfg;
    cfg.paths = {test_dir};
    cfg.build_path = test_dir;

    auto res = cxxgen::CodeLinter::lint_all(tidy, cfg);
    assert(res.success);
    // Should scan one.cpp and two.cpp (2 files), and ignore bin/ignored.cpp
    assert(res.files_scanned == 2);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_lint_recursive_directory_filtering\n";
}

void test_lint_error_detection() {
    auto tidy_opt = cxxgen::find_clang_tidy();
    assert(tidy_opt.has_value());
    std::string tidy = *tidy_opt;

    fs::path test_dir = fs::current_path() / ".cxxgen_test_lint_err";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path err_file = test_dir / "syntax_error.cpp";
    {
        std::ofstream out(err_file);
        out << "int broken_func( {\n"
            << "    return 42;\n"
            << "}\n";
    }

    cxxgen::LintConfig cfg;
    cfg.paths = {err_file};
    cfg.build_path = test_dir;

    auto res = cxxgen::CodeLinter::lint_all(tidy, cfg);
    assert(!res.success);
    assert(res.files_scanned == 1);
    assert(res.errors_count > 0);
    assert(!res.report.empty());

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_lint_error_detection\n";
}

void test_lint_empty_directory() {
    auto tidy_opt = cxxgen::find_clang_tidy();
    assert(tidy_opt.has_value());
    std::string tidy = *tidy_opt;

    fs::path test_dir = fs::current_path() / ".cxxgen_test_lint_empty";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    cxxgen::LintConfig cfg;
    cfg.paths = {test_dir};
    cfg.build_path = test_dir;

    auto res = cxxgen::CodeLinter::lint_all(tidy, cfg);
    assert(res.success);
    assert(res.files_scanned == 0);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_lint_empty_directory\n";
}

void test_cli_lint_parsing() {
    using namespace cxxgen;

    const char *argv1[] = {"cxxgen", "lint"};
    CliArgs args1 = parse_cli_args(2, const_cast<char **>(argv1));
    assert(args1.command == "lint");
    assert(args1.lint_paths.empty());
    assert(!args1.fix);

    const char *argv2[] = {"cxxgen", "lint", "src/main.cpp", "core/", "--fix"};
    CliArgs args2 = parse_cli_args(5, const_cast<char **>(argv2));
    assert(args2.command == "lint");
    assert(args2.lint_paths.size() == 2);
    assert(args2.lint_paths[0] == "src/main.cpp");
    assert(args2.lint_paths[1] == "core/");
    assert(args2.fix);

    std::cout << "[PASS] test_cli_lint_parsing\n";
}

int main() {
    std::cout << "=== Running Linter Tests ===\n";
    test_find_clang_tidy();
    test_is_lintable();
    test_lint_all_clean_file();
    test_lint_recursive_directory_filtering();
    test_lint_error_detection();
    test_lint_empty_directory();
    test_cli_lint_parsing();
    std::cout << "All Linter tests passed successfully!\n";
    return 0;
}
