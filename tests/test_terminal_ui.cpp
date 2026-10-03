#include "cxxgen.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

void test_terminal_ansi_colors() {
    using namespace cxxgen::terminal;

    std::string text = "Hello World";

    // Test with forced color
    std::string red_text = colorize(text, Color::Red, Style::None, true);
    assert(red_text.find("\033[31m") != std::string::npos);
    assert(red_text.find("\033[0m") != std::string::npos);
    assert(strip_ansi(red_text) == text);

    std::string bold_cyan = colorize(text, Color::Cyan, Style::Bold, true);
    assert(bold_cyan.find("1;36m") != std::string::npos ||
           (bold_cyan.find("1") != std::string::npos && bold_cyan.find("36") != std::string::npos));
    assert(strip_ansi(bold_cyan) == text);

    std::string bold_text = bold(text, true);
    assert(bold_text.find("\033[1m") != std::string::npos);
    assert(strip_ansi(bold_text) == text);

    std::string green_text = green(text, true);
    assert(green_text.find("\033[32m") != std::string::npos);
    assert(strip_ansi(green_text) == text);

    std::string yellow_text = yellow(text, true);
    assert(yellow_text.find("\033[33m") != std::string::npos);
    assert(strip_ansi(yellow_text) == text);

    std::string blue_text = blue(text, true);
    assert(blue_text.find("\033[34m") != std::string::npos);
    assert(strip_ansi(blue_text) == text);

    std::string gray_text = gray(text, true);
    assert(gray_text.find("\033[90m") != std::string::npos);
    assert(strip_ansi(gray_text) == text);

    std::cout << "[PASS] test_terminal_ansi_colors\n";
}

void test_terminal_no_color_override() {
    using namespace cxxgen::terminal;

    set_color_override(false);
    assert(!color_enabled());

    std::string sample = "Sample Text";
    assert(red(sample) == sample);
    assert(bold(sample) == sample);
    assert(green(sample) == sample);
    assert(yellow(sample) == sample);
    assert(cyan(sample) == sample);

    // Reset override
    set_color_override(std::nullopt);
    std::cout << "[PASS] test_terminal_no_color_override\n";
}

void test_terminal_no_color_env() {
    using namespace cxxgen::terminal;

    // Reset override
    set_color_override(std::nullopt);

    // Set NO_COLOR environment variable
#if defined(_WIN32)
    _putenv("NO_COLOR=1");
#else
    setenv("NO_COLOR", "1", 1);
#endif

    assert(!is_color_supported());

    // Clear NO_COLOR
#if defined(_WIN32)
    _putenv("NO_COLOR=");
#else
    unsetenv("NO_COLOR");
#endif

    std::cout << "[PASS] test_terminal_no_color_env\n";
}

void test_format_step_status_uncolored() {
    using namespace cxxgen::terminal;

    // Acceptance criteria pattern: [ 4/12 ] [ 33%] Compiling src/renderer.cpp (0.42s)
    std::string formatted = format_step_status(4, 12, "Compiling", "src/renderer.cpp", 0.42, false, false);
    assert(formatted == "[  4/12 ] [ 33%] Compiling src/renderer.cpp (0.42s)");

    std::string done_step = format_step_status(12, 12, "Linking", "bin/app.exe", 1.20, false, false);
    assert(done_step == "[ 12/12 ] [100%] Linking bin/app.exe (1.20s)");

    std::string single_digit = format_step_status(1, 8, "Compiling", "src/main.cpp", 0.05, false, false);
    assert(single_digit == "[ 1/8 ] [ 12%] Compiling src/main.cpp (0.05s)");

    std::cout << "[PASS] test_format_step_status_uncolored\n";
}

void test_format_step_status_colored() {
    using namespace cxxgen::terminal;

    std::string colored = format_step_status(4, 12, "Compiling", "src/renderer.cpp", 0.42, false, true);
    assert(colored.find("\033[") != std::string::npos);
    assert(strip_ansi(colored) == "[  4/12 ] [ 33%] Compiling src/renderer.cpp (0.42s)");

    std::cout << "[PASS] test_format_step_status_colored\n";
}

void test_format_summary() {
    using namespace cxxgen::terminal;

    // Acceptance criteria pattern: Build completed successfully in 1.34s (8 compiled, 4 cached)
    std::string success_summary = format_summary(true, 1.34, 8, 4, 0, false);
    assert(success_summary == "Build completed successfully in 1.34s (8 compiled, 4 cached)");

    std::string fail_summary = format_summary(false, 0.52, 2, 4, 1, false);
    assert(fail_summary == "Build failed in 0.52s (2 compiled, 1 failed, 4 cached)");

    // Test colored versions strip to identical strings
    std::string success_colored = format_summary(true, 1.34, 8, 4, 0, true);
    assert(strip_ansi(success_colored) == "Build completed successfully in 1.34s (8 compiled, 4 cached)");

    std::string fail_colored = format_summary(false, 0.52, 2, 4, 1, true);
    assert(strip_ansi(fail_colored) == "Build failed in 0.52s (2 compiled, 1 failed, 4 cached)");

    std::cout << "[PASS] test_format_summary\n";
}

void test_progress_reporter_basic() {
    using namespace cxxgen::terminal;

    std::ostringstream ss;
    ProgressReporter reporter(5, ss, false);

    assert(reporter.total_steps() == 5);
    assert(reporter.current_step() == 0);
    assert(reporter.compiled_count() == 0);
    assert(reporter.cached_count() == 0);
    assert(reporter.failed_count() == 0);

    reporter.report_step("Compiling", "src/a.cpp", 0.10, false);
    reporter.report_step("Compiling", "src/b.cpp", 0.12, false);
    reporter.report_step("Compiling", "src/c.cpp", 0.01, true); // cached
    reporter.report_step("Archiving", "lib/libcore.a", 0.05, false);
    reporter.report_step("Linking", "bin/app", 0.20, false);

    assert(reporter.current_step() == 5);
    assert(reporter.compiled_count() == 4);
    assert(reporter.cached_count() == 1);
    assert(reporter.failed_count() == 0);

    std::string output = ss.str();
    assert(output.find("[ 1/5 ] [ 20%] Compiling src/a.cpp (0.10s)") != std::string::npos);
    assert(output.find("[ 5/5 ] [100%] Linking bin/app (0.20s)") != std::string::npos);

    reporter.print_summary();
    std::string full_output = ss.str();
    assert(full_output.find("Build completed successfully in") != std::string::npos);
    assert(full_output.find("(4 compiled, 1 cached)") != std::string::npos);

    std::cout << "[PASS] test_progress_reporter_basic\n";
}

void test_progress_reporter_concurrency() {
    using namespace cxxgen::terminal;

    std::ostringstream ss;
    const size_t thread_count = 8;
    const size_t steps_per_thread = 20;
    const size_t total_steps = thread_count * steps_per_thread;

    ProgressReporter reporter(total_steps, ss, false);

    std::vector<std::thread> workers;
    workers.reserve(thread_count);

    for (size_t t = 0; t < thread_count; ++t) {
        workers.emplace_back([&, t]() {
            for (size_t s = 0; s < steps_per_thread; ++s) {
                std::string target = "src/t" + std::to_string(t) + "_file" + std::to_string(s) + ".cpp";
                bool is_cached = (s % 4 == 0);
                reporter.report_step("Compiling", target, 0.05, is_cached);
            }
        });
    }

    for (auto &w : workers) {
        w.join();
    }

    assert(reporter.current_step() == total_steps);
    assert(reporter.cached_count() == thread_count * (steps_per_thread / 4));
    assert(reporter.compiled_count() == total_steps - reporter.cached_count());

    // Verify stream output has exactly total_steps lines
    std::string output = ss.str();
    size_t line_count = 0;
    for (char c : output) {
        if (c == '\n') {
            ++line_count;
        }
    }
    assert(line_count == total_steps);

    std::cout << "[PASS] test_progress_reporter_concurrency\n";
}

void test_cli_no_color_parsing() {
    char arg0[] = "cxxgen";
    char arg1[] = "build";
    char arg2[] = "--no-color";
    char *argv[] = {arg0, arg1, arg2};

    cxxgen::CliArgs args = cxxgen::parse_cli_args(3, argv);
    assert(args.command == "build");
    assert(args.no_color == true);

    std::cout << "[PASS] test_cli_no_color_parsing\n";
}

int main() {
    std::cout << "Running Story 9.1 Terminal UI tests...\n";
    test_terminal_ansi_colors();
    test_terminal_no_color_override();
    test_terminal_no_color_env();
    test_format_step_status_uncolored();
    test_format_step_status_colored();
    test_format_summary();
    test_progress_reporter_basic();
    test_progress_reporter_concurrency();
    test_cli_no_color_parsing();
    std::cout << "All Story 9.1 tests passed successfully!\n";
    return 0;
}
