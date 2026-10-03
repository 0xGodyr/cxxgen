#include "cxxgen.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>

namespace fs = std::filesystem;

void test_watcher_snapshot_and_diff() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_watcher_snapshot";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "src", ec);
    fs::create_directories(test_dir / "include", ec);
    fs::create_directories(test_dir / ".cxxgen", ec);

    cxxgen::WatcherConfig cfg{
        .watch_directories = {test_dir / "src", test_dir / "include"},
        .debounce = std::chrono::milliseconds(150),
    };

    cxxgen::FileWatcher watcher(cfg);

    // Initial snapshot should be empty
    auto changes0 = watcher.scan_changes();
    assert(changes0.empty());

    // 1. Add src/hello.cpp -> Created
    fs::path file1 = test_dir / "src" / "hello.cpp";
    {
        std::ofstream of(file1);
        of << "int main() {}\n";
    }

    // Also add a file inside .cxxgen which MUST be ignored
    {
        std::ofstream of(test_dir / ".cxxgen" / "ignored.cpp");
        of << "// ignored\n";
    }

    auto changes1 = watcher.scan_changes();
    assert(changes1.size() == 1);
    assert(fs::equivalent(changes1[0].path, file1));
    assert(changes1[0].event == cxxgen::WatchEvent::Created);

    // 2. Modify src/hello.cpp -> Modified
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    {
        std::ofstream of(file1, std::ios::app);
        of << "// modified content\n";
    }

    auto changes2 = watcher.scan_changes();
    assert(changes2.size() == 1);
    assert(fs::equivalent(changes2[0].path, file1));
    assert(changes2[0].event == cxxgen::WatchEvent::Modified);

    // 3. Add include/hello.hpp -> Created
    fs::path file2 = test_dir / "include" / "hello.hpp";
    {
        std::ofstream of(file2);
        of << "#pragma once\n";
    }

    auto changes3 = watcher.scan_changes();
    assert(changes3.size() == 1);
    assert(fs::equivalent(changes3[0].path, file2));
    assert(changes3[0].event == cxxgen::WatchEvent::Created);

    // 4. Delete src/hello.cpp -> Deleted
    fs::remove(file1, ec);

    auto changes4 = watcher.scan_changes();
    assert(changes4.size() == 1);
    assert(changes4[0].event == cxxgen::WatchEvent::Deleted);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_watcher_snapshot_and_diff\n";
}

void test_watcher_debounce_and_callback() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_watcher_debounce";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "src", ec);

    cxxgen::WatcherConfig cfg{
        .watch_directories = {test_dir / "src"},
        .debounce = std::chrono::milliseconds(150),
        .poll_interval = std::chrono::milliseconds(25),
    };

    cxxgen::FileWatcher watcher(cfg);

    std::mutex mtx;
    std::condition_variable cv;
    std::vector<cxxgen::FileChangeEvent> received_events;
    std::atomic<int> callback_count{0};

    watcher.start([&](const std::vector<cxxgen::FileChangeEvent> &events) {
        std::lock_guard<std::mutex> lock(mtx);
        received_events = events;
        callback_count++;
        cv.notify_all();
    });

    assert(watcher.is_running());

    // Rapidly write 3 times to src/rapid.cpp (within 50ms)
    fs::path f = test_dir / "src" / "rapid.cpp";
    for (int i = 0; i < 3; ++i) {
        std::ofstream of(f, std::ios::app);
        of << "// write " << i << "\n";
        of.close();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

    // Wait for the 150ms debounce window to trigger the callback
    {
        std::unique_lock<std::mutex> lock(mtx);
        bool signaled = cv.wait_for(lock, std::chrono::seconds(3), [&]() { return callback_count.load() > 0; });
        assert(signaled);
    }

    // Debounce must coalesce the multiple rapid writes into 1 callback notification
    assert(callback_count.load() == 1);
    assert(!received_events.empty());

    watcher.stop();
    assert(!watcher.is_running());

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_watcher_debounce_and_callback\n";
}

void test_watcher_automatic_rebuild() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_watcher_rebuild";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "src", ec);
    fs::create_directories(test_dir / "include", ec);

    fs::path src_file = test_dir / "src" / "main.cpp";
    {
        std::ofstream of(src_file);
        of << "#include <iostream>\n"
           << "int main() { std::cout << \"v1\\n\"; return 0; }\n";
    }

    cxxgen::Builder b;
    auto &exe = b.add_executable({
        .name = "watch_app",
        .sources = {src_file.string()},
        .output_dir = (test_dir / "bin").string(),
    });
    b.install_artifact(exe);

    auto toolchain = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(toolchain != nullptr);
    cxxgen::ExecutionContext ctx{
        .toolchain = *toolchain,
        .verbose = false,
    };

    // Initial build
    cxxgen::StepResult r0 = b.execute_all(ctx);
    assert(r0.success());

    cxxgen::WatcherConfig cfg{
        .watch_directories = {test_dir / "src"},
        .debounce = std::chrono::milliseconds(150),
        .poll_interval = std::chrono::milliseconds(25),
    };

    cxxgen::FileWatcher watcher(cfg);

    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<int> rebuild_count{0};

    watcher.start([&](const std::vector<cxxgen::FileChangeEvent> &events) {
        (void)events;
        cxxgen::StepResult res = b.execute_all(ctx);
        if (res.success()) {
            rebuild_count++;
            cv.notify_all();
        }
    });

    // Modify source file to trigger automatic rebuild
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    {
        std::ofstream of(src_file);
        of << "#include <iostream>\n"
           << "int main() { std::cout << \"v2\\n\"; return 0; }\n";
    }

    // Wait for watcher to trigger rebuild
    {
        std::unique_lock<std::mutex> lock(mtx);
        bool signaled = cv.wait_for(lock, std::chrono::seconds(4), [&]() { return rebuild_count.load() > 0; });
        assert(signaled);
    }

    assert(rebuild_count.load() >= 1);
    watcher.stop();

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_watcher_automatic_rebuild\n";
}

void test_watcher_stop_from_callback() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_watcher_self_stop";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "src", ec);

    cxxgen::WatcherConfig cfg{
        .watch_directories = {test_dir / "src"},
        .debounce = std::chrono::milliseconds(50),
        .poll_interval = std::chrono::milliseconds(20),
    };

    cxxgen::FileWatcher watcher(cfg);
    std::atomic<bool> stopped{false};
    std::mutex mtx;
    std::condition_variable cv;

    watcher.start([&](const std::vector<cxxgen::FileChangeEvent> &) {
        // Stop watcher directly from within callback (worker thread)
        watcher.stop();
        stopped.store(true);
        cv.notify_all();
    });

    // Write a file to trigger the callback
    fs::path f = test_dir / "src" / "trigger.cpp";
    {
        std::ofstream of(f);
        of << "// trigger\n";
    }

    {
        std::unique_lock<std::mutex> lock(mtx);
        bool signaled = cv.wait_for(lock, std::chrono::seconds(3), [&]() { return stopped.load(); });
        assert(signaled);
    }

    assert(!watcher.is_running());
    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_watcher_stop_from_callback\n";
}

int main() {
    std::cout << "Running test_file_watcher...\n";
    test_watcher_snapshot_and_diff();
    test_watcher_debounce_and_callback();
    test_watcher_automatic_rebuild();
    test_watcher_stop_from_callback();
    std::cout << "All file watcher tests passed successfully!\n";
    return 0;
}
