#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

void worker_thread(int id) {
    std::cout << "  [Thread " << id << "] Running concurrent work with thread support...\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "    External Dependency Discovery Demo  \n";
    std::cout << "========================================\n";

#if defined(HAVE_THREADS)
    std::cout << "[System Package] Threading package linked successfully!\n";
    std::vector<std::thread> workers;
    for (int i = 1; i <= 3; ++i) {
        workers.emplace_back(worker_thread, i);
    }
    for (auto &t : workers) {
        t.join();
    }
#else
    std::cout << "[System Package] Running single-threaded.\n";
#endif

#if defined(HAVE_CUSTOM_DEF)
    std::cout << "[Custom Define] Macro injected from dependency package!\n";
#endif

    std::cout << "\nDependencies resolved and linked cleanly by cxxgen!\n";
    return 0;
}
