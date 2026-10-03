#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void test_find_llvm_tools() {
    auto profdata = cxxgen::find_llvm_profdata();
    assert(profdata.has_value());
    assert(fs::exists(*profdata));
    std::cout << "[PASS] test_find_llvm_tools: found profdata " << *profdata << "\n";

    auto cov = cxxgen::find_llvm_cov();
    assert(cov.has_value());
    assert(fs::exists(*cov));
    std::cout << "[PASS] test_find_llvm_tools: found llvm-cov " << *cov << "\n";
}

void test_coverage_flags_injection() {
    using namespace cxxgen;

    ClangToolchain clang("clang++", "llvm-ar");
    CompileParams cparams;
    cparams.coverage = true;
    cparams.source_file = "sample.cpp";
    cparams.object_file = "sample.o";

    auto ccmd = clang.build_compile_command(cparams);
    std::string flat_ccmd = util::flatten_command(ccmd);
    assert(flat_ccmd.find("-fprofile-instr-generate") != std::string::npos);
    assert(flat_ccmd.find("-fcoverage-mapping") != std::string::npos);

    LinkParams lparams;
    lparams.coverage = true;
    lparams.output_binary = "sample.exe";
    lparams.object_files = {"sample.o"};

    auto lcmd = clang.build_link_command(lparams);
    std::string flat_lcmd = util::flatten_command(lcmd);
    assert(flat_lcmd.find("-fprofile-instr-generate") != std::string::npos);
    assert(flat_lcmd.find("-fcoverage-mapping") != std::string::npos);

    GccToolchain gcc("g++", "ar");
    auto gcc_ccmd = gcc.build_compile_command(cparams);
    std::string flat_gcc_ccmd = util::flatten_command(gcc_ccmd);
    assert(flat_gcc_ccmd.find("--coverage") != std::string::npos);

    auto gcc_lcmd = gcc.build_link_command(lparams);
    std::string flat_gcc_lcmd = util::flatten_command(gcc_lcmd);
    assert(flat_gcc_lcmd.find("--coverage") != std::string::npos);

    std::cout << "[PASS] test_coverage_flags_injection\n";
}

void test_builder_coverage_config() {
    using namespace cxxgen;
    Builder b;
    assert(!b.coverage());

    b.set_coverage(true);
    assert(b.coverage());

    auto &t1 = b.add_executable({.name = "app", .sources = {"src/main.cpp"}});
    assert(t1.has_coverage());

    auto &t2 = b.add_test({.name = "unit_tests", .sources = {"tests/test.cpp"}});
    assert(t2.has_coverage());

    t2.set_coverage(false);
    assert(!t2.has_coverage());

    std::cout << "[PASS] test_builder_coverage_config\n";
}

void test_cli_coverage_parsing() {
    using namespace cxxgen;

    const char *argv1[] = {"cxxgen", "test", "--coverage"};
    CliArgs args1 = parse_cli_args(3, const_cast<char **>(argv1));
    assert(args1.command == "test");
    assert(args1.coverage);
    assert(args1.coverage_export.empty());

    const char *argv2[] = {"cxxgen", "test", "--coverage", "--export=html"};
    CliArgs args2 = parse_cli_args(4, const_cast<char **>(argv2));
    assert(args2.command == "test");
    assert(args2.coverage);
    assert(args2.coverage_export == "html");

    const char *argv3[] = {"cxxgen", "test", "--coverage", "--export=lcov"};
    CliArgs args3 = parse_cli_args(4, const_cast<char **>(argv3));
    assert(args3.command == "test");
    assert(args3.coverage);
    assert(args3.coverage_export == "lcov");

    std::cout << "[PASS] test_cli_coverage_parsing\n";
}

void test_real_coverage_pipeline() {
    using namespace cxxgen;

    fs::path test_dir = fs::current_path() / ".cxxgen_test_coverage_pipeline";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path src_file = test_dir / "calc.cpp";
    {
        std::ofstream out(src_file);
        out << "int compute(int a, int b) {\n"
            << "    if (a > 0) return a + b;\n"
            << "    return b - a;\n"
            << "}\n"
            << "int main() {\n"
            << "    return (compute(10, 20) == 30) ? 0 : 1;\n"
            << "}\n";
    }

    fs::path bin_file = test_dir / "calc.exe";
    std::vector<std::string> compile_cmd = {
        "clang++",         "-std=c++20", "-fprofile-instr-generate", "-fcoverage-mapping",
        src_file.string(), "-o",         bin_file.string()};
    auto pres = ProcessRunner::run(compile_cmd);
    assert(pres.success());
    assert(fs::exists(bin_file));

    // Execute instrumented binary with LLVM_PROFILE_FILE
    fs::path profraw = test_dir / "calc.profraw";
    fs::path profdata = test_dir / "calc.profdata";
    {
        ProcessOptions popts;
        popts.command = {bin_file.string()};
        popts.extra_env["LLVM_PROFILE_FILE"] = fs::absolute(profraw, ec).string();
        auto r_res = ProcessRunner::run(popts);
        assert(r_res.success());
    }
    assert(fs::exists(profraw));

    CoverageConfig ccfg;
    ccfg.binary_path = bin_file;
    ccfg.profraw_file = profraw;
    ccfg.profdata_file = profdata;
    ccfg.export_format = "html";
    ccfg.html_dir = test_dir / "html_report";
    ccfg.verbose = false;

    auto res = CoverageReporter::generate_report(ccfg);
    assert(res.success);
    assert(res.line_coverage_percent > 0.0);
    assert(res.function_coverage_percent > 0.0);
    assert(!res.report_output.empty());
    assert(fs::exists(profdata));
    assert(fs::exists(test_dir / "html_report"));

    // Test lcov export
    CoverageConfig ccfg_lcov;
    ccfg_lcov.binary_path = bin_file;
    ccfg_lcov.profraw_file = profraw;
    ccfg_lcov.profdata_file = profdata;
    ccfg_lcov.export_format = "lcov";
    ccfg_lcov.export_file = test_dir / "coverage.lcov";

    auto res_lcov = CoverageReporter::generate_report(ccfg_lcov);
    assert(res_lcov.success);
    assert(fs::exists(test_dir / "coverage.lcov"));

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_real_coverage_pipeline\n";
}

int main() {
    std::cout << "=== Running Coverage Tests ===\n";
    test_find_llvm_tools();
    test_coverage_flags_injection();
    test_builder_coverage_config();
    test_cli_coverage_parsing();
    test_real_coverage_pipeline();
    std::cout << "All Coverage tests passed successfully!\n";
    return 0;
}
