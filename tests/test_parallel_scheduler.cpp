#include "cxxgen.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

void test_thread_pool_basic() {
    std::cout << "Testing ThreadPool basic enqueue and wait_all...\n";
    cxxgen::ThreadPool pool(4);
    assert(pool.thread_count() == 4);

    std::atomic<int> counter{0};
    constexpr int total_tasks = 20;

    for (int i = 0; i < total_tasks; ++i) {
        pool.enqueue([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            counter.fetch_add(1);
        });
    }

    pool.wait_all();
    assert(counter.load() == total_tasks);
    std::cout << "[PASS] ThreadPool basic execution verified!\n";
}

void test_thread_pool_cancel_pending() {
    std::cout << "Testing ThreadPool cancel_pending...\n";
    cxxgen::ThreadPool pool(2);

    std::atomic<int> executed{0};
    // Block the workers
    pool.enqueue([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        executed.fetch_add(1);
    });
    pool.enqueue([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        executed.fetch_add(1);
    });

    // Enqueue pending tasks that should be cancelled
    for (int i = 0; i < 10; ++i) {
        pool.enqueue([&]() { executed.fetch_add(1); });
    }

    // Cancel pending before workers finish
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    pool.cancel_pending();
    pool.wait_all();

    // Only the 2 in-flight tasks should have executed
    assert(executed.load() == 2);
    std::cout << "[PASS] ThreadPool cancel_pending verified!\n";
}

void test_parallel_scheduler_concurrency_and_in_degree(const cxxgen::IToolchain &tc) {
    std::cout << "Testing ParallelScheduler concurrent execution and in-degree tracking...\n";
    cxxgen::Builder b;

    std::atomic<int> concurrent_active{0};
    std::atomic<int> max_concurrency{0};
    std::atomic<bool> a_done{false};
    std::atomic<bool> b_done{false};
    std::atomic<bool> c_started_prematurely{false};

    auto step_a = b.add_step("task_A", "Task A", [&](cxxgen::ExecutionContext &) {
        int cur = ++concurrent_active;
        int old_max = max_concurrency.load();
        while (cur > old_max && !max_concurrency.compare_exchange_weak(old_max, cur)) {}
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        a_done = true;
        --concurrent_active;
        return cxxgen::StepResult::ok();
    });

    auto step_b = b.add_step("task_B", "Task B", [&](cxxgen::ExecutionContext &) {
        int cur = ++concurrent_active;
        int old_max = max_concurrency.load();
        while (cur > old_max && !max_concurrency.compare_exchange_weak(old_max, cur)) {}
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        b_done = true;
        --concurrent_active;
        return cxxgen::StepResult::ok();
    });

    // Step C depends on both A and B
    auto step_c = b.add_step("task_C", "Task C", [&](cxxgen::ExecutionContext &) {
        if (!a_done.load() || !b_done.load()) {
            c_started_prematurely = true;
        }
        return cxxgen::StepResult::ok();
    });
    step_c->depend_on(step_a);
    step_c->depend_on(step_b);

    cxxgen::ExecutionContext ctx{
        .toolchain = tc,
        .thread_count = 4,
    };

    auto start = std::chrono::steady_clock::now();
    auto res = b.execute_step("task_C", ctx);
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);

    assert(res.success());
    assert(!c_started_prematurely.load());
    assert(a_done.load() && b_done.load());
    assert(max_concurrency.load() >= 2);
    // Since task A and B ran concurrently (each taking 60ms), total time should be << 120ms
    std::cout << "  Execution duration with 4 threads: " << dur.count()
              << " ms (max concurrency: " << max_concurrency.load() << ")\n";
    assert(dur.count() < 120);

    std::cout << "[PASS] ParallelScheduler concurrent execution and in-degree tracking verified!\n";
}

void test_parallel_scheduler_abort_on_failure(const cxxgen::IToolchain &tc) {
    std::cout << "Testing ParallelScheduler abort on failure...\n";
    cxxgen::Builder b;

    std::atomic<bool> dependent_executed{false};

    // Failing task
    auto failing_step = b.add_step("failing_task", "Fails quickly", [&](cxxgen::ExecutionContext &) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return cxxgen::StepResult::fail("Critical compile error", 101);
    });

    // Dependent task - must never execute
    auto dep_step = b.add_step("dependent_task", "Should not run", [&](cxxgen::ExecutionContext &) {
        dependent_executed = true;
        return cxxgen::StepResult::ok();
    });
    dep_step->depend_on(failing_step);

    // Independent task with delay
    auto delayed_step = b.add_step("delayed_task", "Long independent task", [&](cxxgen::ExecutionContext &) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return cxxgen::StepResult::ok();
    });

    auto root = b.add_step("root", "Root", [&](cxxgen::ExecutionContext &) { return cxxgen::StepResult::ok(); });
    root->depend_on(dep_step);
    root->depend_on(delayed_step);

    cxxgen::ExecutionContext ctx{
        .toolchain = tc,
        .thread_count = 4,
    };

    auto res = b.execute_step("root", ctx);
    assert(!res.success());
    assert(res.exit_code == 101);
    assert(res.error_message == "Critical compile error");
    assert(!dependent_executed.load());

    std::cout << "[PASS] Abort on failure correctly prevented dependent execution!\n";
}

void test_parallel_source_compilation(const cxxgen::IToolchain &tc) {
    std::cout << "Testing parallel multi-source target compilation...\n";
    fs::path test_dir = "build/parallel_src_test";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    // Create 4 source files
    for (int i = 1; i <= 4; ++i) {
        std::ofstream src(test_dir / ("part" + std::to_string(i) + ".cpp"));
        src << "int part" << i << "_val() { return " << i << "; }\n";
        src.close();
    }

    std::ofstream main_src(test_dir / "main.cpp");
    main_src << "int part1_val(); int part2_val(); int part3_val(); int part4_val();\n";
    main_src << "int main() { return (part1_val() + part2_val() + part3_val() + part4_val() == 10) ? 0 : 1; }\n";
    main_src.close();

    cxxgen::Builder b;
    auto &exe = b.add_executable({
        .name = "parallel_app",
        .sources =
            {
                (test_dir / "main.cpp").string(),
                (test_dir / "part1.cpp").string(),
                (test_dir / "part2.cpp").string(),
                (test_dir / "part3.cpp").string(),
                (test_dir / "part4.cpp").string(),
            },
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    auto res = b.build_target(exe, tc, false, 4);
    if (!res.success) {
        std::cerr << "Build failed: " << res.error_message << "\n";
    }
    assert(res.success);
    assert(res.object_files.size() == 5);
    assert(fs::exists(res.artifact_path));

    // Run the compiled executable
    cxxgen::ProcessOptions popts;
    popts.command = {res.artifact_path};
    auto pres = cxxgen::ProcessRunner::run(popts);
    assert(pres.success());
    assert(pres.exit_code == 0);

    fs::remove(res.artifact_path, ec);
    fs::remove_all(test_dir, ec);
    fs::remove("build", ec);

    std::cout << "[PASS] Parallel multi-source target compilation verified!\n";
}

void test_parallel_source_compilation_failure(const cxxgen::IToolchain &tc) {
    std::cout << "Testing parallel multi-source compilation failure handling...\n";
    fs::path test_dir = "build/parallel_fail_test";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    // Create 3 good source files and 1 broken source file
    std::ofstream good1(test_dir / "good1.cpp");
    good1 << "int g1() { return 1; }\n";
    good1.close();
    std::ofstream good2(test_dir / "good2.cpp");
    good2 << "int g2() { return 2; }\n";
    good2.close();
    std::ofstream bad(test_dir / "bad.cpp");
    bad << "this is a syntax error intentionally here;\n";
    bad.close();

    cxxgen::Builder b;
    auto &lib = b.add_static_library({
        .name = "fail_lib",
        .sources =
            {
                (test_dir / "good1.cpp").string(),
                (test_dir / "bad.cpp").string(),
                (test_dir / "good2.cpp").string(),
            },
    });

    auto res = b.build_target(lib, tc, false, 4);
    assert(!res.success);
    assert(res.exit_code != 0);
    assert(res.error_message.find("bad.cpp") != std::string::npos);

    fs::remove_all(test_dir, ec);
    fs::remove("build", ec);

    std::cout << "[PASS] Parallel source compilation failure correctly detected and reported!\n";
}

void test_parallel_scheduler_cycle_deadlock_prevention(const cxxgen::IToolchain &tc) {
    std::cout << "Testing ParallelScheduler cycle / unresolvable dependency deadlock prevention...\n";
    // 1. Initial cycle: Step A depends on Step B, Step B depends on Step A
    cxxgen::Builder b1;
    auto step_a = b1.add_step("step_a", "Step A", [](cxxgen::ExecutionContext &) { return cxxgen::StepResult::ok(); });
    auto step_b = b1.add_step("step_b", "Step B", [](cxxgen::ExecutionContext &) { return cxxgen::StepResult::ok(); });
    step_a->depend_on(step_b);
    step_b->depend_on(step_a);

    cxxgen::ExecutionContext ctx1{
        .toolchain = tc,
        .thread_count = 2,
    };
    auto res1 = cxxgen::ParallelScheduler::execute({step_a, step_b}, ctx1);
    assert(!res1.success());
    assert(res1.error_message.find("Cycle or unresolvable dependency") != std::string::npos);

    // 2. Mid-execution cycle: Step Root succeeds, then Step C and Step D depend on each other
    cxxgen::Builder b2;
    auto step_root =
        b2.add_step("step_root", "Step Root", [](cxxgen::ExecutionContext &) { return cxxgen::StepResult::ok(); });
    auto step_c = b2.add_step("step_c", "Step C", [](cxxgen::ExecutionContext &) { return cxxgen::StepResult::ok(); });
    auto step_d = b2.add_step("step_d", "Step D", [](cxxgen::ExecutionContext &) { return cxxgen::StepResult::ok(); });
    step_c->depend_on(step_root);
    step_d->depend_on(step_root);
    step_c->depend_on(step_d);
    step_d->depend_on(step_c);

    cxxgen::ExecutionContext ctx2{
        .toolchain = tc,
        .thread_count = 2,
    };
    auto res2 = cxxgen::ParallelScheduler::execute({step_root, step_c, step_d}, ctx2);
    assert(!res2.success());
    assert(res2.error_message.find("Cycle or unresolvable dependency") != std::string::npos);

    std::cout << "[PASS] ParallelScheduler cycle deadlock prevention verified!\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "Running Story 4.2: Parallel Scheduler Tests\n";
    std::cout << "===========================================\n";

    test_thread_pool_basic();
    test_thread_pool_cancel_pending();

    auto toolchain = cxxgen::find_toolchain();
    assert(toolchain != nullptr);
    std::cout << "[INFO] Using Toolchain: " << toolchain->name() << "\n";

    test_parallel_scheduler_concurrency_and_in_degree(*toolchain);
    test_parallel_scheduler_abort_on_failure(*toolchain);
    test_parallel_source_compilation(*toolchain);
    test_parallel_source_compilation_failure(*toolchain);
    test_parallel_scheduler_cycle_deadlock_prevention(*toolchain);

    std::cout << "\nAll Story 4.2 Tests Passed Successfully!\n";
    return 0;
}
