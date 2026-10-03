#include "cxxgen.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

void test_profiler_record_and_json() {
    cxxgen::BuildProfiler profiler;

    auto t0 = std::chrono::steady_clock::now();
    auto t1 = t0 + std::chrono::milliseconds(50);
    auto t2 = t1 + std::chrono::milliseconds(20);

    profiler.record("compile: src/math.cpp", "compile", t0, t1, 1, "Executed", 2048, "Target: math");
    profiler.record("compile: src/app.cpp", "compile", t1, t2, 2, "UpToDate", 4096, "Target: app");
    profiler.record("link: app", "link", t1, t2, 1, "Executed", 0, "Artifact: app.exe");

    auto evts = profiler.events();
    assert(evts.size() == 3);

    assert(evts[0].name == "compile: src/math.cpp");
    assert(evts[0].cat == "compile");
    assert(evts[0].ph == "X");
    assert(evts[0].dur >= 3000); // at least ~3-5ms in microseconds
    assert(evts[0].tid == 1);
    assert(evts[0].status == "Executed");
    assert(evts[0].file_size == 2048);

    assert(evts[1].name == "compile: src/app.cpp");
    assert(evts[1].status == "UpToDate");
    assert(evts[1].file_size == 4096);

    std::string json = profiler.to_json();
    assert(json.find("\"traceEvents\"") != std::string::npos);
    assert(json.find("\"displayTimeUnit\": \"ms\"") != std::string::npos);
    assert(json.find("\"name\": \"compile: src/math.cpp\"") != std::string::npos);
    assert(json.find("\"cat\": \"compile\"") != std::string::npos);
    assert(json.find("\"ph\": \"X\"") != std::string::npos);
    assert(json.find("\"dur\":") != std::string::npos);
    assert(json.find("\"ts\":") != std::string::npos);
    assert(json.find("\"status\": \"Executed\"") != std::string::npos);
    assert(json.find("\"file_size\": 2048") != std::string::npos);

    std::cout << "[PASS] test_profiler_record_and_json\n";
}

void test_profiler_trace_file_write() {
    cxxgen::BuildProfiler profiler;
    auto t0 = std::chrono::steady_clock::now();
    auto t1 = t0 + std::chrono::milliseconds(10);

    profiler.record("compile: test.cpp", "compile", t0, t1, 1, "Executed", 512);

    std::filesystem::path trace_path = ".cxxgen/test_trace.json";
    bool ok = profiler.write_trace_file(trace_path);
    assert(ok);
    assert(std::filesystem::exists(trace_path));
    assert(std::filesystem::file_size(trace_path) > 50);

    // Read back and verify valid JSON structure
    std::ifstream in(trace_path);
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    assert(content.find("\"traceEvents\"") != std::string::npos);
    assert(content.find("compile: test.cpp") != std::string::npos);

    std::error_code ec;
    std::filesystem::remove(trace_path, ec);

    std::cout << "[PASS] test_profiler_trace_file_write\n";
}

void test_profiler_summary_formatting() {
    cxxgen::BuildProfiler profiler;

    auto t0 = std::chrono::steady_clock::now();
    auto t1 = t0 + std::chrono::milliseconds(100);
    auto t2 = t1 + std::chrono::milliseconds(20);

    profiler.record("compile: slow.cpp", "compile", t0, t1, 1, "Executed", 10240, "Slow TU");
    profiler.record("compile: fast.cpp", "compile", t1, t2, 2, "Executed", 1024, "Fast TU");

    std::ostringstream oss;
    profiler.print_summary(oss, false);

    std::string out = oss.str();
    assert(out.find("cxxgen build --profile - Performance Summary") != std::string::npos);
    assert(out.find("Top Slowest Compilation Steps") != std::string::npos);
    assert(out.find("slow.cpp") != std::string::npos);
    assert(out.find("fast.cpp") != std::string::npos);
    assert(out.find("build_trace.json") != std::string::npos);

    // Verify slow.cpp is listed before fast.cpp (descending duration sort)
    size_t pos_slow = out.find("slow.cpp");
    size_t pos_fast = out.find("fast.cpp");
    assert(pos_slow < pos_fast);

    std::cout << "[PASS] test_profiler_summary_formatting\n";
}

void test_profiler_with_parallel_scheduler() {
    cxxgen::Builder b;

    auto step1 = std::make_shared<cxxgen::CustomStep>("custom_codegen", "Run codegen", [](cxxgen::ExecutionContext &) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return cxxgen::StepResult::ok();
    });

    auto step2 =
        std::make_shared<cxxgen::CustomStep>("custom_process", "Run data processing", [](cxxgen::ExecutionContext &) {
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
            return cxxgen::StepResult::ok();
        });
    step2->depend_on(step1);

    b.add_step(step1);
    b.add_step(step2);

    auto profiler = std::make_shared<cxxgen::BuildProfiler>();
    auto dummy_toolchain = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(dummy_toolchain);

    cxxgen::ExecutionContext ctx{
        .toolchain = *dummy_toolchain,
        .verbose = false,
        .thread_count = 2,
        .profiler = profiler,
    };

    auto res = b.execute_all(ctx);
    assert(res.success());

    auto evts = profiler->events();
    assert(evts.size() >= 2);

    bool found_step1 = false;
    bool found_step2 = false;
    for (const auto &e : evts) {
        if (e.name == "custom_codegen")
            found_step1 = true;
        if (e.name == "custom_process")
            found_step2 = true;
    }
    assert(found_step1);
    assert(found_step2);

    std::cout << "[PASS] test_profiler_with_parallel_scheduler\n";
}

void test_profiler_cli_parsing() {
    {
        const char *argv[] = {"cxxgen", "build", "--profile"};
        auto args = cxxgen::parse_cli_args(3, const_cast<char **>(argv));
        assert(args.command == "build");
        assert(args.profile == true);
        assert(args.profile_output == "build_trace.json");
    }
    {
        const char *argv[] = {"cxxgen", "build", "--profile=custom_trace.json"};
        auto args = cxxgen::parse_cli_args(3, const_cast<char **>(argv));
        assert(args.command == "build");
        assert(args.profile == true);
        assert(args.profile_output == "custom_trace.json");
    }
    {
        const char *argv[] = {"cxxgen", "build", "--profile", "my_trace.json"};
        auto args = cxxgen::parse_cli_args(4, const_cast<char **>(argv));
        assert(args.command == "build");
        assert(args.profile == true);
        assert(args.profile_output == "my_trace.json");
    }
    std::cout << "[PASS] test_profiler_cli_parsing\n";
}

int main() {
    std::cout << "Running test_build_profiler...\n";
    test_profiler_record_and_json();
    test_profiler_trace_file_write();
    test_profiler_summary_formatting();
    test_profiler_with_parallel_scheduler();
    test_profiler_cli_parsing();
    std::cout << "All build profiler tests passed successfully!\n";
    return 0;
}
