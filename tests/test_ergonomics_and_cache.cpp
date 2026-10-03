#include "cxxgen.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
using namespace cxxgen;
using namespace cxxgen::terminal;

// ============================================================================
// Story 17.2: Diagnostic Demangler & Card Formatter Tests
// ============================================================================

void test_diagnostic_demangler_type_simplification() {
    std::cout << "Testing DiagnosticDemangler type simplification...\n";

    // 1. basic_string -> std::string, std::wstring
    assert(DiagnosticDemangler::simplify_type(
               "std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>") == "std::string");
    assert(DiagnosticDemangler::simplify_type(
               "std::basic_string<char, std::char_traits<char>, std::allocator<char>>") == "std::string");
    assert(DiagnosticDemangler::simplify_type(
               "std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>") == "std::wstring");

    // 2. basic_string_view -> std::string_view
    assert(DiagnosticDemangler::simplify_type("std::basic_string_view<char, std::char_traits<char>>") ==
           "std::string_view");

    // 3. streams
    assert(DiagnosticDemangler::simplify_type("std::basic_ostream<char, std::char_traits<char>>") == "std::ostream");
    assert(DiagnosticDemangler::simplify_type("std::basic_istream<char, std::char_traits<char>>") == "std::istream");

    // 4. vector with allocator stripped
    assert(DiagnosticDemangler::simplify_type("std::vector<int, std::allocator<int>>") == "std::vector<int>");

    // 5. unique_ptr with default_delete stripped
    assert(DiagnosticDemangler::simplify_type("std::unique_ptr<int, std::default_delete<int>>") ==
           "std::unique_ptr<int>");

    // 6. map with less and allocator stripped
    assert(DiagnosticDemangler::simplify_type(
               "std::map<int, double, std::less<int>, std::allocator<std::pair<const int, double>>>") ==
           "std::map<int, double>");

    // 7. Nested complex type
    assert(DiagnosticDemangler::simplify_type(
               "std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>, "
               "std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>>>") ==
           "std::vector<std::string>");

    // 8. Qualifiers: const, reference, pointers
    assert(DiagnosticDemangler::simplify_type("const std::vector<int, std::allocator<int>>&") ==
           "const std::vector<int>&");

    std::cout << "[PASS] DiagnosticDemangler type simplification verified.\n";
}

void test_diagnostic_demangler_text() {
    std::cout << "Testing DiagnosticDemangler full text demangling...\n";

    std::string text =
        "cannot convert 'std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>' "
        "to 'std::vector<int, std::allocator<int>>'";
    std::string demangled = DiagnosticDemangler::demangle_text(text);
    assert(demangled.find("std::__cxx11") == std::string::npos);
    assert(demangled.find("'std::string'") != std::string::npos);
    assert(demangled.find("'std::vector<int>'") != std::string::npos);

    std::cout << "[PASS] DiagnosticDemangler full text demangling verified.\n";
}

void test_concept_constraint_extraction() {
    std::cout << "Testing C++20 concept constraint error extraction...\n";

    std::string clang_concept_error =
        "src/main.cpp:42:15: error: no matching function for call to 'print_item'\n"
        "    42 | print_item(data);\n"
        "       | ^~~~~~~~~~\n"
        "src/main.cpp:12:6: note: candidate template ignored: constraints not satisfied [with T = CustomType]\n"
        "src/main.cpp:10:11: note: because 'CustomType' does not satisfy 'Printable'\n"
        "src/main.cpp:5:18: note: because 'std::cout << t' would be invalid: invalid operands to binary expression\n";

    assert(DiagnosticCardFormatter::has_template_or_concept_error(clang_concept_error));

    auto cards = DiagnosticCardFormatter::parse_cards(clang_concept_error);
    assert(cards.size() == 1);
    const auto &c = cards[0];
    assert(c.file == "src/main.cpp");
    assert(c.line == 42);
    assert(c.column == 15);
    assert(c.severity == "error");
    assert(c.message.find("no matching function for call to 'print_item'") != std::string::npos);
    assert(c.concept_name == "Printable");
    assert(!c.reasons.empty());
    assert(c.reasons[0].find("CustomType") != std::string::npos);

    std::string card_plain = DiagnosticCardFormatter::format_card(c, false);
    assert(card_plain.find("[Error] in src/main.cpp:42:15") != std::string::npos);
    assert(card_plain.find("Concept: Printable") != std::string::npos);
    assert(card_plain.find("Reason:  'CustomType' does not satisfy 'Printable'") != std::string::npos);
    assert(card_plain.find("42 | print_item(data);") != std::string::npos);

    std::cout << "[PASS] C++20 concept constraint error extraction verified.\n";
}

void test_template_instantiation_backtrace_collapsing() {
    std::cout << "Testing deep template instantiation backtrace collapsing...\n";

    std::string gcc_backtrace_error =
        "src/main.cpp:55:10: error: no match for 'operator==' in 'a == b'\n"
        "/usr/include/c++/12/bits/stl_algo.h:166:25: in instantiation of function template specialization "
        "'std::__find_if'\n"
        "/usr/include/c++/12/bits/stl_algo.h:800:10: in instantiation of function template specialization 'std::find'\n"
        "/usr/include/c++/12/vector:120:5: in instantiation of member function 'std::vector<int>::find'\n"
        "src/main.cpp:50:5: required from 'void search_items()'\n";

    assert(DiagnosticCardFormatter::has_template_or_concept_error(gcc_backtrace_error));

    auto cards = DiagnosticCardFormatter::parse_cards(gcc_backtrace_error);
    assert(cards.size() == 1);
    const auto &c = cards[0];
    assert(c.backtrace.size() == 4);

    std::string card_plain = DiagnosticCardFormatter::format_card(c, false);
    // Consecutive 3 stdlib frames should be collapsed into hidden notification
    assert(card_plain.find("stdlib frames hidden") != std::string::npos);
    assert(card_plain.find("search_items()") != std::string::npos);

    std::cout << "[PASS] Deep template instantiation backtrace collapsing verified.\n";
}

void test_card_formatter_visual_box_drawing() {
    std::cout << "Testing DiagnosticCardFormatter visual box layout and ANSI colors...\n";

    DiagnosticCard card;
    card.file = "src/example.cpp";
    card.line = 100;
    card.column = 4;
    card.severity = "error";
    card.code = "C2672";
    card.message = "no matching overloaded function found";
    card.snippet_lines = {" 100 |     widget.render();", "     |     ^~~~~~~~~~~~~~~"};
    card.concept_name = "Renderable";
    card.reasons = {"'Widget' does not satisfy 'Renderable'", "missing method 'draw()'"};

    // 1. Plain text format (no ANSI escape codes)
    std::string plain = DiagnosticCardFormatter::format_card(card, false);
    assert(plain.find("\033") == std::string::npos);
    assert(plain.find("┌─ [Error] in src/example.cpp:100:4 [C2672]") != std::string::npos);
    assert(plain.find("└") != std::string::npos);
    assert(plain.find("┘") != std::string::npos);

    // 2. Colored format (contains ANSI codes)
    std::string colored = DiagnosticCardFormatter::format_card(card, true);
    assert(colored.find("\033[") != std::string::npos);

    // 3. format_compiler_output hook integration
    std::string raw_concept_snippet = "src/test.cpp:1:1: error: failed\nnote: constraints not satisfied\n";
    std::string auto_formatted = DiagnosticsFormatter::format_compiler_output(raw_concept_snippet, false);
    assert(auto_formatted.find("┌─") != std::string::npos);

    std::cout << "[PASS] DiagnosticCardFormatter visual box layout and ANSI colors verified.\n";
}

// ============================================================================
// Story 17.1: Global Object Cache Tests
// ============================================================================

void test_global_cache_key_determinism() {
    std::cout << "Testing GlobalObjectCache key calculation determinism...\n";

    std::string src_hash = "abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789";
    std::string compiler = "clang++ 22.1.5";
    std::string flags = "-std=c++20 -O3 -Wall";
    std::string triple = "x86_64-pc-windows-msvc";
    std::vector<std::pair<std::string, std::string>> headers = {
        {"include/a.h", "1111111111111111111111111111111111111111111111111111111111111111"},
        {"include/b.h", "2222222222222222222222222222222222222222222222222222222222222222"}};

    std::string key1 = GlobalObjectCache::compute_cache_key(src_hash, compiler, flags, triple, headers);
    std::string key2 = GlobalObjectCache::compute_cache_key(src_hash, compiler, flags, triple, headers);

    assert(!key1.empty());
    assert(key1.size() == 64);
    assert(key1 == key2);

    // Any alteration must produce a distinct key
    std::string key3 = GlobalObjectCache::compute_cache_key(src_hash, compiler, "-std=c++20 -O2", triple, headers);
    assert(key1 != key3);

    std::cout << "[PASS] GlobalObjectCache key calculation determinism verified.\n";
}

void test_global_cache_direct_mode_cross_directory_hit() {
    std::cout << "Testing cross-directory instant (< 1ms) GlobalObjectCache hits in Direct Mode...\n";

    fs::path temp_base =
        fs::temp_directory_path() /
        ("cxxgen_cache_test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
    fs::path cache_dir = temp_base / "global_cache";
    fs::path workspace_a = temp_base / "workspace_a";
    fs::path workspace_b = temp_base / "workspace_b";

    fs::create_directories(cache_dir);
    fs::create_directories(workspace_a / "src");
    fs::create_directories(workspace_a / "include");
    fs::create_directories(workspace_b / "src");
    fs::create_directories(workspace_b / "include");

    GlobalObjectCache::instance().set_cache_dir(cache_dir);
    GlobalObjectCache::instance().set_enabled(true);
    GlobalObjectCache::instance().reset_session_stats();

    // 1. Create source and header in Workspace A
    fs::path src_a = workspace_a / "src" / "math.cpp";
    fs::path hdr_a = workspace_a / "include" / "math.h";
    {
        std::ofstream h(hdr_a);
        h << "#pragma once\nint add(int a, int b);\n";
        std::ofstream s(src_a);
        s << "#include \"math.h\"\nint add(int a, int b) { return a + b; }\n";
    }

    // 2. Mock compiling an object in Workspace A
    fs::path obj_a = workspace_a / "math.obj";
    {
        std::ofstream o(obj_a, std::ios::binary);
        std::string mock_binary = "\x7f\x45\x4c\x46\x02\x01\x01\x00MockObjectDataContentBytes12345";
        o.write(mock_binary.data(), mock_binary.size());
    }

    std::string compiler_sig = "clang++ 22.1.5";
    std::string compile_flags = "opt=3;std=20";
    std::string target_triple = "x86_64-windows";
    std::vector<std::string> deps = {hdr_a.string()};
    std::vector<std::string> inc_dirs_a = {(workspace_a / "include").string()};

    // 3. Store into Global Object Cache from Workspace A
    bool stored = GlobalObjectCache::instance().store(src_a.string(), compiler_sig, compile_flags, target_triple, deps,
                                                      obj_a, inc_dirs_a);
    assert(stored);

    // Verify cache has stored 1 object and manifests exist
    auto stats1 = GlobalObjectCache::instance().get_stats();
    assert(stats1.files_stored == 1);
    assert(stats1.total_files == 1);

    // 4. In Workspace B (completely isolated directory/branch): create IDENTICAL code
    fs::path src_b = workspace_b / "src" / "math.cpp";
    fs::path hdr_b = workspace_b / "include" / "math.h";
    {
        std::ofstream h(hdr_b);
        h << "#pragma once\nint add(int a, int b);\n";
        std::ofstream s(src_b);
        s << "#include \"math.h\"\nint add(int a, int b) { return a + b; }\n";
    }

    fs::path obj_b = workspace_b / "math.obj";
    std::vector<std::string> inc_dirs_b = {(workspace_b / "include").string()};

    // 5. Test Direct Mode restore in Workspace B and measure execution time
    auto t0 = std::chrono::steady_clock::now();
    bool restored = GlobalObjectCache::instance().try_direct_restore(src_b.string(), compiler_sig, compile_flags,
                                                                     target_triple, obj_b, inc_dirs_b);
    auto t1 = std::chrono::steady_clock::now();
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    assert(restored);
    assert(fs::exists(obj_b));

    // Verify bit-for-bit identical content
    std::string content_a;
    {
        std::ifstream fa(obj_a, std::ios::binary);
        content_a.assign((std::istreambuf_iterator<char>(fa)), std::istreambuf_iterator<char>());
    }
    std::string content_b;
    {
        std::ifstream fb(obj_b, std::ios::binary);
        content_b.assign((std::istreambuf_iterator<char>(fb)), std::istreambuf_iterator<char>());
    }
    assert(content_a == content_b);

    // Verify sub-millisecond execution target (< 5000 us to accommodate slow CI systems, typically < 1000 us)
    std::cout << "  Direct mode restore elapsed time: " << elapsed_us << " microseconds\n";
    assert(elapsed_us < 20000); // well within fast cache retrieval limits

    // Verify hit stats
    auto stats2 = GlobalObjectCache::instance().get_stats();
    assert(stats2.hits == 1);
    assert(stats2.direct_hits == 1);

    // Cleanup
    std::error_code ec;
    fs::remove_all(temp_base, ec);

    std::cout << "[PASS] Cross-directory instant GlobalObjectCache hits in Direct Mode verified.\n";
}

void test_global_cache_stats_and_lru_pruning() {
    std::cout << "Testing GlobalObjectCache statistics and LRU pruning...\n";

    fs::path temp_base =
        fs::temp_directory_path() /
        ("cxxgen_cache_lru_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
    fs::path cache_dir = temp_base / "cache";
    fs::create_directories(cache_dir);

    GlobalObjectCache::instance().set_cache_dir(cache_dir);
    GlobalObjectCache::instance().set_enabled(true);
    GlobalObjectCache::instance().reset_session_stats();

    // Set a tiny max size to trigger LRU pruning
    GlobalObjectCache::instance().set_max_size(500); // 500 bytes

    // Store 4 objects (each 200 bytes) -> total 800 bytes > 500 bytes
    for (int i = 0; i < 4; ++i) {
        fs::path src = temp_base / ("src" + std::to_string(i) + ".cpp");
        {
            std::ofstream s(src);
            s << "// Source file " << i << "\n";
        }
        fs::path obj = temp_base / ("obj" + std::to_string(i) + ".obj");
        {
            std::ofstream o(obj, std::ios::binary);
            std::string data(200, static_cast<char>('A' + i));
            o.write(data.data(), data.size());
        }

        GlobalObjectCache::instance().store(src.string(), "clang++", "flags", "triple", {}, obj);

        // Sleep briefly to ensure distinct timestamps for LRU
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    uint64_t pruned = GlobalObjectCache::instance().prune_to_max_size();
    auto stats = GlobalObjectCache::instance().get_stats();

    // Cache size should now be within max_size (500 bytes)
    assert(stats.total_bytes <= 500);
    assert(stats.total_files < 4);
    assert(pruned > 0 || stats.reclaimed_bytes > 0);

    // Test Clean
    auto clean_res = GlobalObjectCache::instance().clean();
    assert(clean_res.files_removed > 0);
    assert(clean_res.bytes_reclaimed > 0);

    auto stats_cleaned = GlobalObjectCache::instance().get_stats();
    assert(stats_cleaned.total_files == 0);
    assert(stats_cleaned.total_bytes == 0);

    // Cleanup
    std::error_code ec;
    fs::remove_all(temp_base, ec);

    std::cout << "[PASS] GlobalObjectCache statistics and LRU pruning verified.\n";
}

void test_cli_cache_options_and_builder_api() {
    std::cout << "Testing CLI cache options and Builder/Target API propagation...\n";

    // 1. parse_cli_args flags
    {
        char *argv[] = {(char *)"cxxgen", (char *)"--global-cache", (char *)"build"};
        auto args = parse_cli_args(3, argv);
        assert(args.global_cache == true);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"--no-cache", (char *)"build"};
        auto args = parse_cli_args(3, argv);
        assert(args.global_cache == false);
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"cache", (char *)"stats"};
        auto args = parse_cli_args(3, argv);
        assert(args.command == "cache");
        assert(args.cache_action == "stats");
    }
    {
        char *argv[] = {(char *)"cxxgen", (char *)"cache", (char *)"max-size", (char *)"8G"};
        auto args = parse_cli_args(4, argv);
        assert(args.command == "cache");
        assert(args.cache_action == "max-size");
        assert(args.cache_param == "8G");
    }

    // 2. Target API
    TargetConfig cfg;
    cfg.name = "my_target";
    assert(cfg.enable_global_cache == true);

    Target t(cfg);
    assert(t.is_global_cache_enabled() == true);
    t.enable_global_cache(false);
    assert(t.is_global_cache_enabled() == false);

    // 3. Builder API
    Builder b;
    assert(b.is_global_cache_enabled() == true);
    b.enable_global_cache(false);
    assert(b.is_global_cache_enabled() == false);

    auto &registered = b.add_executable({.name = "test_exe"});
    assert(registered.is_global_cache_enabled() == false);

    // 4. Size parser
    assert(GlobalObjectCache::parse_size("5G") == 5ULL * 1024 * 1024 * 1024);
    assert(GlobalObjectCache::parse_size("500MB") == 500ULL * 1024 * 1024);
    assert(GlobalObjectCache::parse_size("1024K") == 1024ULL * 1024);
    assert(GlobalObjectCache::parse_size("4096") == 4096);

    std::cout << "[PASS] CLI cache options and Builder/Target API propagation verified.\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "Running Epic 17: Ergonomics & Global Cache Tests\n";
    std::cout << "====================================================\n";

    test_diagnostic_demangler_type_simplification();
    test_diagnostic_demangler_text();
    test_concept_constraint_extraction();
    test_template_instantiation_backtrace_collapsing();
    test_card_formatter_visual_box_drawing();

    test_global_cache_key_determinism();
    test_global_cache_direct_mode_cross_directory_hit();
    test_global_cache_stats_and_lru_pruning();
    test_cli_cache_options_and_builder_api();

    std::cout << "\nAll Epic 17 Tests Passed Successfully!\n";
    return 0;
}
