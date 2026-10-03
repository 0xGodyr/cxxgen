#include "cxxgen.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

void test_parse_clang_gcc_error() {
    using namespace cxxgen::terminal;

    std::string line = "src/main.cpp:42:10: error: use of undeclared identifier 'x'";
    DiagnosticMessage diag = DiagnosticsFormatter::parse_line(line);

    assert(diag.is_parsed);
    assert(!diag.is_msvc);
    assert(diag.file == "src/main.cpp");
    assert(diag.line == 42);
    assert(diag.column == 10);
    assert(diag.severity == DiagnosticSeverity::Error);
    assert(diag.message == "use of undeclared identifier 'x'");
    assert(diag.code.empty());

    std::cout << "[PASS] test_parse_clang_gcc_error\n";
}

void test_parse_clang_gcc_warning_with_flag() {
    using namespace cxxgen::terminal;

    std::string line = "src/utils.cpp:18:9: warning: unused variable 'counter' [-Wunused-variable]";
    DiagnosticMessage diag = DiagnosticsFormatter::parse_line(line);

    assert(diag.is_parsed);
    assert(!diag.is_msvc);
    assert(diag.file == "src/utils.cpp");
    assert(diag.line == 18);
    assert(diag.column == 9);
    assert(diag.severity == DiagnosticSeverity::Warning);
    assert(diag.message == "unused variable 'counter'");
    assert(diag.code == "-Wunused-variable");

    std::cout << "[PASS] test_parse_clang_gcc_warning_with_flag\n";
}

void test_parse_clang_gcc_fatal_error() {
    using namespace cxxgen::terminal;

    std::string line = "C:/projects/engine/header.h:5:1: fatal error: 'missing.h' file not found";
    DiagnosticMessage diag = DiagnosticsFormatter::parse_line(line);

    assert(diag.is_parsed);
    assert(!diag.is_msvc);
    assert(diag.file == "C:/projects/engine/header.h");
    assert(diag.line == 5);
    assert(diag.column == 1);
    assert(diag.severity == DiagnosticSeverity::FatalError);
    assert(diag.message == "'missing.h' file not found");

    std::cout << "[PASS] test_parse_clang_gcc_fatal_error\n";
}

void test_parse_clang_gcc_no_column() {
    using namespace cxxgen::terminal;

    std::string line = "src/main.cpp:99: error: expected ';' before '}' token";
    DiagnosticMessage diag = DiagnosticsFormatter::parse_line(line);

    assert(diag.is_parsed);
    assert(!diag.is_msvc);
    assert(diag.file == "src/main.cpp");
    assert(diag.line == 99);
    assert(diag.column == 0);
    assert(diag.severity == DiagnosticSeverity::Error);
    assert(diag.message == "expected ';' before '}' token");

    std::cout << "[PASS] test_parse_clang_gcc_no_column\n";
}

void test_parse_msvc_error_with_column() {
    using namespace cxxgen::terminal;

    std::string line = "src\\renderer.cpp(55,14): error C2065: 'pipeline': undeclared identifier";
    DiagnosticMessage diag = DiagnosticsFormatter::parse_line(line);

    assert(diag.is_parsed);
    assert(diag.is_msvc);
    assert(diag.file == "src\\renderer.cpp");
    assert(diag.line == 55);
    assert(diag.column == 14);
    assert(diag.severity == DiagnosticSeverity::Error);
    assert(diag.code == "C2065");
    assert(diag.message == "'pipeline': undeclared identifier");

    std::cout << "[PASS] test_parse_msvc_error_with_column\n";
}

void test_parse_msvc_warning_no_column() {
    using namespace cxxgen::terminal;

    std::string line = "C:\\projects\\app\\src\\app.cpp(102): warning C4100: 'argc': unreferenced formal parameter";
    DiagnosticMessage diag = DiagnosticsFormatter::parse_line(line);

    assert(diag.is_parsed);
    assert(diag.is_msvc);
    assert(diag.file == "C:\\projects\\app\\src\\app.cpp");
    assert(diag.line == 102);
    assert(diag.column == 0);
    assert(diag.severity == DiagnosticSeverity::Warning);
    assert(diag.code == "C4100");
    assert(diag.message == "'argc': unreferenced formal parameter");

    std::cout << "[PASS] test_parse_msvc_warning_no_column\n";
}

void test_parse_msvc_fatal_error() {
    using namespace cxxgen::terminal;

    std::string line = "main.cpp(1): fatal error C1083: Cannot open include file: 'foo.h': No such file or directory";
    DiagnosticMessage diag = DiagnosticsFormatter::parse_line(line);

    assert(diag.is_parsed);
    assert(diag.is_msvc);
    assert(diag.file == "main.cpp");
    assert(diag.line == 1);
    assert(diag.column == 0);
    assert(diag.severity == DiagnosticSeverity::FatalError);
    assert(diag.code == "C1083");
    assert(diag.message == "Cannot open include file: 'foo.h': No such file or directory");

    std::cout << "[PASS] test_parse_msvc_fatal_error\n";
}

void test_format_diagnostic_visual_contrast() {
    using namespace cxxgen::terminal;

    // Test error formatting with color: red severity, cyan location
    DiagnosticMessage err_diag = DiagnosticsFormatter::parse_line("src/main.cpp:42:10: error: undefined symbol");
    std::string err_formatted = DiagnosticsFormatter::format_diagnostic(err_diag, true);
    assert(err_formatted.find("\033[") != std::string::npos);
    assert(err_formatted.find("31m") != std::string::npos); // Red
    assert(err_formatted.find("36m") != std::string::npos); // Cyan file
    assert(err_formatted.find("undefined symbol") != std::string::npos);

    // Test warning formatting with color: yellow severity
    DiagnosticMessage warn_diag =
        DiagnosticsFormatter::parse_line("src/main.cpp(15): warning C4100: 'x': unreferenced");
    std::string warn_formatted = DiagnosticsFormatter::format_diagnostic(warn_diag, true);
    assert(warn_formatted.find("33m") != std::string::npos); // Yellow
    assert(warn_formatted.find("36m") != std::string::npos); // Cyan file

    // Test uncolored formatting preserves clean text
    std::string err_uncolored = DiagnosticsFormatter::format_diagnostic(err_diag, false);
    assert(err_uncolored.find("\033") == std::string::npos);
    assert(err_uncolored == "src/main.cpp:42:10: error: undefined symbol");

    std::cout << "[PASS] test_format_diagnostic_visual_contrast\n";
}

void test_format_compiler_output_multiline() {
    using namespace cxxgen::terminal;

    std::string raw = "src/main.cpp:42:10: error: use of undeclared identifier 'x'\n"
                      "    int a = x + 1;\n"
                      "            ^\n"
                      "1 error generated.\n";

    std::string colored = DiagnosticsFormatter::format_compiler_output(raw, true);
    assert(colored.find("31m") != std::string::npos);                                             // Red for error
    assert(colored.find("36m") != std::string::npos);                                             // Cyan for location
    assert(colored.find("92m") != std::string::npos || colored.find("32m") != std::string::npos); // Green for caret

    std::string uncolored = DiagnosticsFormatter::format_compiler_output(raw, false);
    assert(uncolored.find("\033") == std::string::npos);
    assert(uncolored.find("src/main.cpp:42:10: error: use of undeclared identifier 'x'") != std::string::npos);
    assert(uncolored.find("    int a = x + 1;") != std::string::npos);
    assert(uncolored.find("            ^") != std::string::npos);

    std::cout << "[PASS] test_format_compiler_output_multiline\n";
}

void test_diagnostics_print_formatted_thread_safety() {
    using namespace cxxgen::terminal;

    std::ostringstream ss;
    const size_t thread_count = 6;
    const size_t msgs_per_thread = 15;

    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    for (size_t t = 0; t < thread_count; ++t) {
        threads.emplace_back([&, t]() {
            for (size_t m = 0; m < msgs_per_thread; ++m) {
                std::string err = "file" + std::to_string(t) + ".cpp:" + std::to_string(m + 1) +
                                  ":5: error: syntax error in thread " + std::to_string(t) + "\n";
                DiagnosticsFormatter::print_formatted(err, ss, false);
            }
        });
    }

    for (auto &th : threads) {
        th.join();
    }

    std::string output = ss.str();
    size_t line_count = 0;
    for (char c : output) {
        if (c == '\n') {
            ++line_count;
        }
    }
    assert(line_count == thread_count * msgs_per_thread);

    std::cout << "[PASS] test_diagnostics_print_formatted_thread_safety\n";
}

void test_diagnostic_demangler_empty_args_and_malformed() {
    using namespace cxxgen::terminal;

    // Test that empty template arguments e.g. std::vector<> or list<> do not crash DiagnosticDemangler
    std::string s1 = DiagnosticDemangler::simplify_type("std::vector<>");
    assert(!s1.empty());

    std::string s2 = DiagnosticDemangler::simplify_type("std::deque<>");
    assert(!s2.empty());

    std::string s3 = DiagnosticDemangler::simplify_type("std::list<>");
    assert(!s3.empty());

    std::string s4 = DiagnosticDemangler::simplify_type("std::unique_ptr<>");
    assert(!s4.empty());

    std::string s5 = DiagnosticDemangler::simplify_type("std::set<>");
    assert(!s5.empty());

    std::string s6 = DiagnosticDemangler::simplify_type("std::unordered_set<>");
    assert(!s6.empty());

    std::string s7 = DiagnosticDemangler::simplify_type("std::map<>");
    assert(!s7.empty());

    std::string s8 = DiagnosticDemangler::simplify_type("std::unordered_map<>");
    assert(!s8.empty());

    // Also test demangle_text with these constructs
    std::string demangled = DiagnosticDemangler::demangle_text("error with 'std::vector<>' and 'std::set<>'");
    assert(!demangled.empty());

    std::cout << "[PASS] test_diagnostic_demangler_empty_args_and_malformed\n";
}

int main() {
    std::cout << "Running Story 9.2 Compiler Diagnostics tests...\n";
    test_parse_clang_gcc_error();
    test_parse_clang_gcc_warning_with_flag();
    test_parse_clang_gcc_fatal_error();
    test_parse_clang_gcc_no_column();
    test_parse_msvc_error_with_column();
    test_parse_msvc_warning_no_column();
    test_parse_msvc_fatal_error();
    test_format_diagnostic_visual_contrast();
    test_format_compiler_output_multiline();
    test_diagnostics_print_formatted_thread_safety();
    test_diagnostic_demangler_empty_args_and_malformed();
    std::cout << "All Story 9.2 tests passed successfully!\n";
    return 0;
}
