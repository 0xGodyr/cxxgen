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

void test_embed_file_generation() {
    std::cout << "Testing EmbedFileStep generation and format...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_embed_gen";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path bin_file = test_dir / "sample.bin";
    std::vector<uint8_t> test_bytes = {0x00, 0x01, 0x02, 0x03, 0x10, 0x20, 0x30, 0x40, 0xAA,
                                       0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x42, 0x7F, 0x80, 0x90};
    std::ofstream bin_out(bin_file, std::ios::binary);
    bin_out.write(reinterpret_cast<const char *>(test_bytes.data()), test_bytes.size());
    bin_out.close();

    fs::path header_dest = test_dir / "include" / "sample.h";
    cxxgen::EmbedFileOptions opts;
    opts.variable_name = "my_sample_asset";
    opts.as_constexpr = true;
    opts.null_terminate = false;
    opts.namespace_name = "test_res";

    std::string err;
    bool ok = cxxgen::EmbedFileStep::embed(bin_file, header_dest, opts, err);
    assert(ok);
    assert(err.empty());
    assert(fs::exists(header_dest));

    std::ifstream h_in(header_dest);
    std::string content((std::istreambuf_iterator<char>(h_in)), std::istreambuf_iterator<char>());
    h_in.close();

    // Verify key C++ constructs
    assert(content.find("#pragma once") != std::string::npos);
    assert(content.find("#include <array>") != std::string::npos);
    assert(content.find("namespace test_res") != std::string::npos);
    assert(content.find("inline constexpr size_t my_sample_asset_size = 18;") != std::string::npos);
    assert(content.find("inline constexpr std::array<uint8_t, 18> my_sample_asset = {") != std::string::npos);
    assert(content.find("0x00, 0x01, 0x02, 0x03") != std::string::npos);
    assert(content.find("0x42") != std::string::npos);
    assert(content.find("my_sample_asset_string_view()") != std::string::npos);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] EmbedFileStep generation verified.\n";
}

void test_embed_file_null_terminated_and_string_view() {
    std::cout << "Testing EmbedFileStep null-termination and string_view...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_embed_string";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path json_file = test_dir / "config.json";
    std::string json_data = "{\"name\":\"cxxgen\",\"version\":1}";
    std::ofstream out(json_file);
    out << json_data;
    out.close();

    fs::path header_dest = test_dir / "include" / "config_json.h";
    cxxgen::EmbedFileOptions opts;
    opts.as_constexpr = true;
    opts.null_terminate = true;
    opts.namespace_name = "";

    std::string err;
    bool ok = cxxgen::EmbedFileStep::embed(json_file, header_dest, opts, err);
    assert(ok);
    assert(fs::exists(header_dest));

    std::ifstream h_in(header_dest);
    std::string content((std::istreambuf_iterator<char>(h_in)), std::istreambuf_iterator<char>());
    h_in.close();

    // Size should be data length, array should have +1 byte for null terminator
    size_t expected_size = json_data.size();
    size_t expected_array_len = expected_size + 1;
    assert(content.find("config_json_size = " + std::to_string(expected_size) + ";") != std::string::npos);
    assert(content.find("std::array<uint8_t, " + std::to_string(expected_array_len) + "> config_json = {") !=
           std::string::npos);
    assert(content.find("0x00\n};") != std::string::npos);
    assert(content.find("config_json_c_str()") != std::string::npos);
    assert(content.find("config_json_string_view()") != std::string::npos);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] EmbedFileStep null-termination verified.\n";
}

void test_embed_file_incremental_change_detection() {
    std::cout << "Testing EmbedFileStep change detection and is_up_to_date...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_embed_incremental";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path src_file = test_dir / "font.ttf";
    std::ofstream s_out(src_file, std::ios::binary);
    s_out.put('A').put('B').put('C');
    s_out.close();

    fs::path dst_file = test_dir / "font.h";
    cxxgen::EmbedFileStep step(src_file, dst_file);

    // 1. Output doesn't exist yet -> not up to date
    assert(!step.is_up_to_date());

    cxxgen::ClangToolchain dummy_tc;
    cxxgen::ExecutionContext ctx{.toolchain = dummy_tc};
    auto res1 = step.execute(ctx);
    assert(res1.success());
    assert(fs::exists(dst_file));

    // 3. Immediately after execution -> up to date
    assert(step.is_up_to_date());

    // Second execution returns UpToDate status
    auto res2 = step.execute(ctx);
    assert(res2.status == cxxgen::StepStatus::UpToDate);

    // 4. Modify source file (with timestamp sleep for filesystem resolution)
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::ofstream s_mod(src_file, std::ios::binary | std::ios::app);
    s_mod.put('D').put('E');
    s_mod.close();
    fs::last_write_time(src_file, fs::file_time_type::clock::now());

    // 5. Source is now newer than output -> not up to date
    assert(!step.is_up_to_date());

    // 6. Execute step again -> regenerates output
    auto res3 = step.execute(ctx);
    assert(res3.success());
    assert(step.is_up_to_date());

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] EmbedFileStep incremental change detection verified.\n";
}

void test_embed_file_compilation_and_execution() {
    std::cout << "Testing end-to-end binary embedding compilation and execution...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_embed_e2e";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "include", ec);
    fs::create_directories(test_dir / "src", ec);
    fs::create_directories(test_dir / "assets", ec);

    // 1. Create binary asset
    fs::path asset_path = test_dir / "assets" / "greeting.txt";
    std::string text = "Hello from embedded binary asset!";
    std::ofstream a_out(asset_path, std::ios::binary);
    a_out.write(text.data(), text.size());
    a_out.close();

    // 2. Embed into header
    fs::path header_path = test_dir / "include" / "greeting_asset.h";
    cxxgen::EmbedFileOptions opts;
    opts.variable_name = "greeting_data";
    opts.as_constexpr = true;
    opts.null_terminate = true;
    opts.namespace_name = "embedded";

    std::string err;
    bool ok = cxxgen::EmbedFileStep::embed(asset_path, header_path, opts, err);
    assert(ok);

    // 3. Write program using the embedded header
    fs::path main_cpp = test_dir / "src" / "main.cpp";
    std::ofstream m_out(main_cpp);
    m_out << "#include \"greeting_asset.h\"\n"
          << "#include <cassert>\n"
          << "#include <iostream>\n"
          << "#include <string_view>\n\n"
          << "int main() {\n"
          << "    assert(embedded::greeting_data_size == 33);\n"
          << "    assert(embedded::greeting_data[0] == 'H');\n"
          << "    assert(embedded::greeting_data_string_view() == \"Hello from embedded binary asset!\");\n"
          << "    std::cout << embedded::greeting_data_c_str() << std::endl;\n"
          << "    return 0;\n"
          << "}\n";
    m_out.close();

    // 4. Compile and run executable
    fs::path exe_path = test_dir / "app.exe";
    cxxgen::ProcessOptions popts;
    popts.command = {"clang++",         "-std=c++20", "-I" + (test_dir / "include").string(),
                     main_cpp.string(), "-o",         exe_path.string()};
    popts.working_directory = test_dir;
    auto bld_res = cxxgen::ProcessRunner::run(popts);
    if (!bld_res.success()) {
        std::cerr << "Embed compilation failed:\n" << bld_res.combined_output() << "\n";
    }
    assert(bld_res.success());

    cxxgen::ProcessOptions run_opts;
    run_opts.command = {exe_path.string()};
    run_opts.working_directory = test_dir;
    auto run_res = cxxgen::ProcessRunner::run(run_opts);
    assert(run_res.success());
    assert(cxxgen::util::trim(run_res.stdout_output) == "Hello from embedded binary asset!");

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] End-to-end binary embedding compilation and execution verified.\n";
}

void test_check_headers_discovery() {
    std::cout << "Testing HeaderChecker header discovery and deduplication...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_chk_discovery";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "include" / "core", ec);
    fs::create_directories(test_dir / "src", ec);
    fs::create_directories(test_dir / ".git", ec);
    fs::create_directories(test_dir / "build", ec);

    // Valid headers
    std::ofstream(test_dir / "include" / "core" / "engine.h") << "#pragma once\n";
    std::ofstream(test_dir / "include" / "core" / "math.hpp") << "#pragma once\n";
    std::ofstream(test_dir / "src" / "internal.hxx") << "#pragma once\n";
    std::ofstream(test_dir / "src" / "detail.inl") << "#pragma once\n";

    // Non-header and ignored files
    std::ofstream(test_dir / "src" / "main.cpp") << "int main() {}\n";
    std::ofstream(test_dir / ".git" / "git_header.h") << "#pragma once\n";
    std::ofstream(test_dir / "build" / "build_header.hpp") << "#pragma once\n";

    cxxgen::HeaderCheckConfig cfg;
    cfg.work_dir = test_dir;
    auto discovered = cxxgen::HeaderChecker::discover_headers(cfg);

    assert(discovered.size() == 4);

    auto has_file = [&](std::string_view name) {
        for (const auto &d : discovered) {
            if (d.filename() == name)
                return true;
        }
        return false;
    };

    assert(has_file("engine.h"));
    assert(has_file("math.hpp"));
    assert(has_file("internal.hxx"));
    assert(has_file("detail.inl"));
    assert(!has_file("git_header.h"));
    assert(!has_file("build_header.hpp"));
    assert(!has_file("main.cpp"));

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] HeaderChecker discovery verified.\n";
}

void test_check_headers_clean_project() {
    std::cout << "Testing HeaderChecker with clean self-contained headers...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_chk_clean";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "include", ec);

    // 1. Math header - fully self contained
    std::ofstream(test_dir / "include" / "math_helper.hpp")
        << "#pragma once\n"
        << "#include <cmath>\n"
        << "inline double square_root(double val) { return std::sqrt(val); }\n";

    // 2. String header - fully self contained
    std::ofstream(test_dir / "include" / "string_helper.hpp")
        << "#pragma once\n"
        << "#include <string>\n"
        << "#include <string_view>\n"
        << "inline std::string wrap_string(std::string_view s) { return \"[\" + std::string(s) + \"]\"; }\n";

    auto tc = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(tc != nullptr);

    cxxgen::HeaderCheckConfig cfg;
    cfg.work_dir = test_dir;
    cfg.include_dirs = {test_dir / "include"};

    auto res = cxxgen::HeaderChecker::check_all(*tc, cfg);
    assert(res.success);
    assert(res.headers_scanned == 2);
    assert(res.headers_passed == 2);
    assert(res.headers_failed == 0);
    assert(res.failures.empty());
    assert(res.report.find("are self-contained") != std::string::npos);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] HeaderChecker clean project check verified.\n";
}

void test_check_headers_broken_detection() {
    std::cout << "Testing HeaderChecker detection of missing includes...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_chk_broken";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "include", ec);

    // 1. Self-contained header
    std::ofstream(test_dir / "include" / "valid.hpp") << "#pragma once\n"
                                                      << "#include <cstdint>\n"
                                                      << "inline uint32_t get_zero() { return 0; }\n";

    // 2. Broken header: uses std::vector without including <vector>
    fs::path broken_h = test_dir / "include" / "broken.hpp";
    std::ofstream(broken_h) << "#pragma once\n"
                            << "inline std::vector<int> get_numbers() { return {1, 2, 3}; }\n";

    auto tc = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(tc != nullptr);

    cxxgen::HeaderCheckConfig cfg;
    cfg.work_dir = test_dir;
    cfg.include_dirs = {test_dir / "include"};

    // Initial check should FAIL
    auto res1 = cxxgen::HeaderChecker::check_all(*tc, cfg);
    assert(!res1.success);
    assert(res1.headers_scanned == 2);
    assert(res1.headers_passed == 1);
    assert(res1.headers_failed == 1);
    assert(res1.failures.size() == 1);
    assert(res1.failures[0].header_path.filename() == "broken.hpp");
    assert(res1.failures[0].error_message.find("vector") != std::string::npos);

    // Fix the broken header by adding #include <vector>
    std::ofstream fix_out(broken_h);
    fix_out << "#pragma once\n"
            << "#include <vector>\n"
            << "inline std::vector<int> get_numbers() { return {1, 2, 3}; }\n";
    fix_out.close();

    // Re-check should now PASS
    auto res2 = cxxgen::HeaderChecker::check_all(*tc, cfg);
    assert(res2.success);
    assert(res2.headers_scanned == 2);
    assert(res2.headers_passed == 2);
    assert(res2.headers_failed == 0);
    assert(res2.failures.empty());

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] HeaderChecker broken header detection and fix verified.\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "Running Epic 13: Embed & Header Linter Tests\n";
    std::cout << "===========================================\n";

    test_embed_file_generation();
    test_embed_file_null_terminated_and_string_view();
    test_embed_file_incremental_change_detection();
    test_embed_file_compilation_and_execution();
    test_check_headers_discovery();
    test_check_headers_clean_project();
    test_check_headers_broken_detection();

    std::cout << "\nAll Epic 13 Tests Passed Successfully!\n";
    return 0;
}
