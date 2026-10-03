#include "cxxgen.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void test_sha256_vectors() {
    // FIPS 180-4 Standard Test Vectors
    std::string h_empty = cxxgen::hash::Sha256::hash("");
    assert(h_empty == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    std::string h_abc = cxxgen::hash::Sha256::hash("abc");
    assert(h_abc == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    std::cout << "[PASS] test_sha256_vectors\n";
}

void test_jit_find_script() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_jit_find";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "sub1" / "sub2", ec);

    // No build.cpp yet within test_dir boundary
    assert(!cxxgen::JitRunner::find_build_script(test_dir / "sub1" / "sub2", test_dir));

    // Write build.cpp in test_dir
    std::ofstream of(test_dir / "build.cpp");
    of << "// dummy build script\n";
    of.close();

    auto found = cxxgen::JitRunner::find_build_script(test_dir / "sub1" / "sub2", test_dir);
    assert(found.has_value());
    assert(fs::equivalent(*found, test_dir / "build.cpp"));

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_jit_find_script\n";
}

void test_jit_compile_and_cache() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_jit_project";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    // Locate current cxxgen.hpp header path
    fs::path current_cxxgen_header = fs::absolute("include/cxxgen.hpp");
    assert(fs::exists(current_cxxgen_header));

    // Create a minimal build.cpp that uses cxxgen
    fs::path build_script = test_dir / "build.cpp";
    {
        std::ofstream of(build_script);
        of << "#include \"cxxgen.hpp\"\n"
           << "void build(cxxgen::Builder &b) {\n"
           << "    (void)b;\n"
           << "}\n"
           << "CXXGEN_MAIN(build)\n";
    }

    cxxgen::JitRunnerConfig config{
        .working_dir = test_dir,
        .build_script = build_script,
        .cxxgen_header = current_cxxgen_header,
        .verbose = false,
        .force_rebuild = false,
    };

    // 1. First run: cache miss, compiles build.cpp
    std::cout << "Compiling JIT runner for the first time (cache miss)...\n";
    cxxgen::JitResult res1 = cxxgen::JitRunner::prepare_runner(config);
    if (!res1.success) {
        std::cerr << "Compile failed: " << res1.error_message << "\n";
    }
    assert(res1.success);
    assert(!res1.cache_hit);
    assert(fs::exists(res1.runner_binary));
    assert(fs::exists(test_dir / ".cxxgen" / "cache" / "build_runner.hash"));

    // 2. Second run: cache hit! Must be fast (< 15ms)
    std::cout << "Checking JIT runner cache hit...\n";
    auto start_cache_check = std::chrono::steady_clock::now();
    cxxgen::JitResult res2 = cxxgen::JitRunner::prepare_runner(config);
    auto cache_hit_dur =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_cache_check);

    assert(res2.success);
    assert(res2.cache_hit);
    assert(res2.hash == res1.hash);
    assert(res2.runner_binary == res1.runner_binary);
    std::cout << "Cache hit took: " << cache_hit_dur.count() << " ms\n";
    assert(cache_hit_dur.count() < 100); // well within bounds, typically < 15ms

    // 3. Execute runner binary
    int exit_code = cxxgen::JitRunner::execute_runner(res2.runner_binary, {"--version"}, test_dir);
    assert(exit_code == 0);

    // 4. Modify build.cpp: should trigger recompilation (cache miss)
    std::cout << "Modifying build.cpp to trigger automatic recompilation...\n";
    {
        std::ofstream of(build_script, std::ios::app);
        of << "\n// modified by test\n";
    }

    cxxgen::JitResult res3 = cxxgen::JitRunner::prepare_runner(config);
    assert(res3.success);
    assert(!res3.cache_hit);
    assert(res3.hash != res1.hash);

    // 5. Run again after modification: cache hit on new hash
    cxxgen::JitResult res4 = cxxgen::JitRunner::prepare_runner(config);
    assert(res4.success);
    assert(res4.cache_hit);
    assert(res4.hash == res3.hash);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_jit_compile_and_cache\n";
}

int main() {
    std::cout << "Running test_jit_runner...\n";
    test_sha256_vectors();
    test_jit_find_script();
    test_jit_compile_and_cache();
    std::cout << "All JIT runner tests passed successfully!\n";
    return 0;
}
