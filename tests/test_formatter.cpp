#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void test_find_clang_format() {
    auto fmt = cxxgen::find_clang_format();
    assert(fmt.has_value());
    assert(fs::exists(*fmt));
    std::cout << "[PASS] test_find_clang_format: found " << *fmt << "\n";
}

void test_format_file_and_check() {
    auto fmt_opt = cxxgen::find_clang_format();
    assert(fmt_opt.has_value());
    std::string fmt = *fmt_opt;

    fs::path test_dir = fs::current_path() / ".cxxgen_test_fmt_single";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path unformatted_file = test_dir / "messy.cpp";
    {
        std::ofstream out(unformatted_file);
        out << "int   foo( int  x,int y ){return   x+y;}\n";
    }

    // 1. Run check mode: must detect mismatch
    cxxgen::FormatConfig check_cfg;
    check_cfg.paths = {unformatted_file};
    check_cfg.check = true;
    auto res_check = cxxgen::CodeFormatter::format_all(fmt, check_cfg);
    assert(res_check.success);
    assert(res_check.mismatched_files.size() == 1);

    // 2. Run format mode: must format file in place
    cxxgen::FormatConfig fmt_cfg;
    fmt_cfg.paths = {unformatted_file};
    fmt_cfg.check = false;
    auto res_fmt = cxxgen::CodeFormatter::format_all(fmt, fmt_cfg);
    assert(res_fmt.success);
    assert(res_fmt.files_formatted == 1);

    // 3. Run check mode again: must now be clean!
    auto res_check2 = cxxgen::CodeFormatter::format_all(fmt, check_cfg);
    assert(res_check2.success);
    assert(res_check2.mismatched_files.empty());

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_format_file_and_check\n";
}

void test_format_recursive_directory() {
    auto fmt_opt = cxxgen::find_clang_format();
    assert(fmt_opt.has_value());
    std::string fmt = *fmt_opt;

    fs::path test_dir = fs::current_path() / ".cxxgen_test_fmt_rec";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "src", ec);
    fs::create_directories(test_dir / "include", ec);
    fs::create_directories(test_dir / "bin", ec); // ignored dir

    {
        std::ofstream out(test_dir / "src" / "a.cpp");
        out << "void  a( ){int x=1;}\n";
    }
    {
        std::ofstream out(test_dir / "include" / "b.hpp");
        out << "struct  B{  int val;};\n";
    }
    {
        // Inside ignored directory
        std::ofstream out(test_dir / "bin" / "ignored.cpp");
        out << "void  ignored( ){}\n";
    }

    cxxgen::FormatConfig cfg;
    cfg.paths = {test_dir};
    cfg.check = false;
    auto res = cxxgen::CodeFormatter::format_all(fmt, cfg);
    assert(res.success);
    // Should scan a.cpp and b.hpp (2 files), and ignore bin/ignored.cpp
    assert(res.files_scanned == 2);
    assert(res.files_formatted == 2);

    // Now check mode should report all matching
    cfg.check = true;
    auto res_check = cxxgen::CodeFormatter::format_all(fmt, cfg);
    assert(res_check.success);
    assert(res_check.mismatched_files.empty());

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_format_recursive_directory\n";
}

int main() {
    std::cout << "Running test_formatter...\n";
    test_find_clang_format();
    test_format_file_and_check();
    test_format_recursive_directory();
    std::cout << "All formatter tests passed successfully!\n";
    return 0;
}
