#include "cxxgen.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

void test_basic_execution() {
    using namespace cxxgen;
#if defined(_WIN32)
    ProcessResult res = ProcessRunner::run({"cmd.exe", "/c", "echo", "Hello from cxxgen ProcessRunner"});
#else
    ProcessResult res = ProcessRunner::run({"echo", "Hello from cxxgen ProcessRunner"});
#endif
    assert(res.success());
    assert(res.exit_code == 0);
    assert(!res.timed_out);
    assert(res.stdout_output.find("Hello from cxxgen ProcessRunner") != std::string::npos);
    assert(res.duration.count() >= 0);

    std::cout << "[PASS] Basic process execution test passed! Duration: " << res.duration.count() << "ms\n";
}

void test_exit_code_propagation() {
    using namespace cxxgen;
#if defined(_WIN32)
    ProcessResult res42 = ProcessRunner::run({"cmd.exe", "/c", "exit", "42"});
    ProcessResult res1 = ProcessRunner::run({"cmd.exe", "/c", "exit", "1"});
#else
    ProcessResult res42 = ProcessRunner::run({"sh", "-c", "exit 42"});
    ProcessResult res1 = ProcessRunner::run({"sh", "-c", "exit 1"});
#endif
    assert(!res42.success());
    assert(res42.exit_code == 42);

    assert(!res1.success());
    assert(res1.exit_code == 1);

    std::cout << "[PASS] Exit code propagation tests passed!\n";
}

void test_stderr_capture() {
    using namespace cxxgen;
#if defined(_WIN32)
    ProcessResult res = ProcessRunner::execute("cmd.exe /c echo diagnostic error message 1>&2");
#else
    ProcessResult res = ProcessRunner::execute("sh -c 'echo diagnostic error message >&2'");
#endif
    assert(res.success());
    assert(res.stderr_output.find("diagnostic error message") != std::string::npos);
    assert(res.combined_output().find("diagnostic error message") != std::string::npos);

    std::cout << "[PASS] Stderr capture test passed!\n";
}

void test_invalid_command() {
    using namespace cxxgen;
    ProcessResult res = ProcessRunner::run({"this_binary_definitely_does_not_exist_xyz_999"});
    assert(!res.success());
    assert(res.exit_code != 0);
    assert(!res.stderr_output.empty());

    std::cout << "[PASS] Invalid command handling test passed!\n";
}

void test_large_output_deadlock_prevention() {
    using namespace cxxgen;
    // Produces ~120KB of output, far exceeding standard 4KB/64KB OS pipe buffers.
    // In naive implementations, this would deadlock if stdout is not continuously drained.
#if defined(_WIN32)
    std::string script =
        "for /L %i in (1,1,1500) do @echo Line %i: The quick brown fox jumps over the lazy dog 1234567890";
    ProcessResult res = ProcessRunner::execute("cmd.exe /c " + script);
#else
    std::string script =
        "for i in $(seq 1 1500); do echo \"Line $i: The quick brown fox jumps over the lazy dog 1234567890\"; done";
    ProcessResult res = ProcessRunner::run({"sh", "-c", script});
#endif
    assert(res.success());
    assert(res.exit_code == 0);
    assert(res.stdout_output.size() > 80000);
    assert(res.stdout_output.find("Line 1:") != std::string::npos);
    assert(res.stdout_output.find("Line 1500:") != std::string::npos);

    std::cout << "[PASS] Large output deadlock prevention test passed! Read " << res.stdout_output.size()
              << " bytes across 1500 lines.\n";
}

void test_concurrent_execution() {
    using namespace cxxgen;
    constexpr int kNumThreads = 8;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int t = 0; t < kNumThreads; ++t) {
        threads.emplace_back([t, &success_count]() {
            std::string marker = "Thread_" + std::to_string(t) + "_Marker";
#if defined(_WIN32)
            ProcessResult res = ProcessRunner::run({"cmd.exe", "/c", "echo", marker});
#else
            ProcessResult res = ProcessRunner::run({"echo", marker});
#endif
            if (res.success() && res.stdout_output.find(marker) != std::string::npos) {
                success_count.fetch_add(1);
            }
        });
    }

    for (auto &th : threads) {
        th.join();
    }

    assert(success_count.load() == kNumThreads);
    std::cout << "[PASS] Concurrent child processes test passed (" << kNumThreads << " parallel processes)!\n";
}

void test_environment_variables() {
    using namespace cxxgen;
    ProcessOptions opts;
#if defined(_WIN32)
    opts.command = {"cmd.exe", "/c", "echo", "%CXXGEN_TEST_ENV_VAR%"};
#else
    opts.command = {"sh", "-c", "echo $CXXGEN_TEST_ENV_VAR"};
#endif
    opts.extra_env = {{"CXXGEN_TEST_ENV_VAR", "ProcessRunner_Env_Success_Val"}};

    ProcessResult res = ProcessRunner::run(opts);
    assert(res.success());
    assert(res.stdout_output.find("ProcessRunner_Env_Success_Val") != std::string::npos);

    std::cout << "[PASS] Environment variable injection test passed!\n";
}

void test_working_directory() {
    using namespace cxxgen;
    std::filesystem::path target_dir = std::filesystem::current_path() / "tests";
    ProcessOptions opts;
#if defined(_WIN32)
    opts.command = {"cmd.exe", "/c", "cd"};
#else
    opts.command = {"pwd"};
#endif
    opts.working_directory = target_dir;

    ProcessResult res = ProcessRunner::run(opts);
    assert(res.success());
    assert(res.stdout_output.find("tests") != std::string::npos);

    std::cout << "[PASS] Working directory test passed!\n";
}

void test_streaming_callbacks() {
    using namespace cxxgen;
    size_t callback_bytes = 0;
    ProcessOptions opts;
#if defined(_WIN32)
    opts.command = {"cmd.exe", "/c", "echo", "Stream callback test payload"};
#else
    opts.command = {"echo", "Stream callback test payload"};
#endif
    opts.on_stdout = [&](std::string_view chunk) { callback_bytes += chunk.size(); };

    ProcessResult res = ProcessRunner::run(opts);
    assert(res.success());
    assert(callback_bytes > 0);
    assert(res.stdout_output.find("Stream callback test payload") != std::string::npos);

    std::cout << "[PASS] Streaming callbacks test passed! Received " << callback_bytes << " bytes via callback.\n";
}

void test_real_compiler_diagnostics() {
    using namespace cxxgen;
    // Check if Clang is available
    auto clang_opt = find_clang_toolchain();
    if (clang_opt.has_value()) {
        // 1. Valid execution
        ProcessResult res_ver = ProcessRunner::run({clang_opt->compiler_path(), "--version"});
        assert(res_ver.success());
        assert(res_ver.stdout_output.find("clang") != std::string::npos);

        // 2. Real error capture: invalid flag
        ProcessResult res_err = ProcessRunner::run({clang_opt->compiler_path(), "-invalid-unrecognized-flag-xyz"});
        assert(!res_err.success());
        assert(res_err.exit_code != 0);
        assert(res_err.combined_output().find("unknown argument") != std::string::npos ||
               res_err.combined_output().find("unrecognized") != std::string::npos);
        std::cout << "[PASS] Real compiler diagnostic error capture passed with " << clang_opt->name() << "!\n";
    }

    // Check if GCC is available
    auto gcc_opt = find_gcc_toolchain();
    if (gcc_opt.has_value()) {
        ProcessResult res_ver = ProcessRunner::run({gcc_opt->compiler_path(), "--version"});
        assert(res_ver.success());
        assert(res_ver.stdout_output.find("g++") != std::string::npos ||
               res_ver.stdout_output.find("GCC") != std::string::npos);
        std::cout << "[PASS] Real compiler verification passed with " << gcc_opt->name() << "!\n";
    }
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Running ProcessRunner Test Suite (Story 2.5)\n";
    std::cout << "========================================\n";

    test_basic_execution();
    test_exit_code_propagation();
    test_stderr_capture();
    test_invalid_command();
    test_large_output_deadlock_prevention();
    test_concurrent_execution();
    test_environment_variables();
    test_working_directory();
    test_streaming_callbacks();
    test_real_compiler_diagnostics();

    std::cout << "========================================\n";
    std::cout << "All ProcessRunner Tests Passed Successfully!\n";
    std::cout << "========================================\n";
    return 0;
}
