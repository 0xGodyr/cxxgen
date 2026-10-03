#include "cxxgen.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

static void clean_dir(const fs::path &p) {
    std::error_code ec;
    if (!fs::exists(p, ec))
        return;
    for (auto &entry : fs::recursive_directory_iterator(p, fs::directory_options::skip_permission_denied, ec)) {
        fs::permissions(entry.path(), fs::perms::owner_write | fs::perms::group_write, fs::perm_options::add, ec);
    }
    fs::remove_all(p, ec);
    if (fs::exists(p, ec)) {
#if defined(_WIN32)
        std::string cmd = "rmdir /s /q \"" + p.string() + "\" >nul 2>nul";
        (void)std::system(cmd.c_str());
#endif
    }
}

void test_local_path_dependency() {
    std::cout << "[TEST] Running test_local_path_dependency...\n";
    fs::path test_root = fs::current_path() / ".test_pkg_local";
    clean_dir(test_root);

    fs::path pkg_dir = test_root / "simple_math";
    fs::create_directories(pkg_dir / "include");
    fs::create_directories(pkg_dir / "src");

    {
        std::ofstream h(pkg_dir / "include" / "simple_math.hpp");
        h << "#pragma once\nint add_numbers(int a, int b);\n";
    }
    {
        std::ofstream s(pkg_dir / "src" / "simple_math.cpp");
        s << "#include \"simple_math.hpp\"\nint add_numbers(int a, int b) { return a + b; }\n";
    }

    fs::path app_src_dir = test_root / "app_src";
    fs::create_directories(app_src_dir);
    {
        std::ofstream app_s(app_src_dir / "main.cpp");
        app_s << "#include <iostream>\n"
              << "#include \"simple_math.hpp\"\n"
              << "int main() {\n"
              << "    std::cout << \"SUM=\" << add_numbers(19, 23) << \"\\n\";\n"
              << "    return 0;\n"
              << "}\n";
    }

    cxxgen::Builder b;
    auto &dep = b.dependency("simple_math", {.path = pkg_dir});
    assert(dep.name() == "simple_math");
    assert(fs::equivalent(dep.path(), pkg_dir));
    assert(dep.is_cached());

    // Add static library using dependency helpers
    auto &math_lib = dep.add_static_library({
        .name = "simple_math_lib",
        .sources = {"src/simple_math.cpp"},
        .include_dirs = {"include"},
    });
    assert(dep.has_target("simple_math_lib"));
    assert(dep.artifact("simple_math_lib").name() == "simple_math_lib");

    auto &exe = b.add_executable({
        .name = "local_dep_app",
        .sources = {(app_src_dir / "main.cpp").string()},
        .output_dir = (test_root / "bin").string(),
    });
    exe.link(math_lib);

    auto toolchain = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(toolchain != nullptr);
    cxxgen::ExecutionContext ctx{.toolchain = *toolchain, .verbose = false};

    auto res = b.execute_all(ctx);
    if (!res.success()) {
        std::cerr << "Build failed: " << res.error_message << "\n";
    }
    assert(res.success());

    fs::path exe_path = cxxgen::CompilationPipeline::get_artifact_path(exe, *toolchain);
    auto run_res = cxxgen::ProcessRunner::run({exe_path.string()});
    assert(run_res.success());
    assert(run_res.stdout_output.find("SUM=42") != std::string::npos);

    clean_dir(test_root);
    std::cout << "[PASS] test_local_path_dependency passed!\n";
}

void test_child_build_cpp_importing() {
    std::cout << "[TEST] Running test_child_build_cpp_importing...\n";
    fs::path test_root = fs::current_path() / ".test_pkg_child_build";
    clean_dir(test_root);

    fs::path pkg_dir = test_root / "string_util";
    fs::create_directories(pkg_dir / "include");
    fs::create_directories(pkg_dir / "src");

    {
        std::ofstream h(pkg_dir / "include" / "string_util.hpp");
        h << "#pragma once\n#include <string>\nstd::string shout(const std::string &s);\n";
    }
    {
        std::ofstream s(pkg_dir / "src" / "string_util.cpp");
        s << "#include \"string_util.hpp\"\n"
          << "#include <algorithm>\n"
          << "std::string shout(const std::string &s) {\n"
          << "    std::string res = s;\n"
          << "    for (char &c : res) c = (char)toupper(c);\n"
          << "    return res + \"!\";\n"
          << "}\n";
    }
    {
        // Child build.cpp
        std::ofstream bcpp(pkg_dir / "build.cpp");
        bcpp << "#include \"cxxgen.hpp\"\n"
             << "void build(cxxgen::Builder &b) {\n"
             << "    b.add_static_library({\n"
             << "        .name = \"str_util_lib\",\n"
             << "        .sources = {\"src/string_util.cpp\"},\n"
             << "        .include_dirs = {\"include\"}\n"
             << "    });\n"
             << "}\n"
             << "CXXGEN_MAIN(build)\n";
    }

    fs::path app_src_dir = test_root / "app_src";
    fs::create_directories(app_src_dir);
    {
        std::ofstream app_s(app_src_dir / "main.cpp");
        app_s << "#include <iostream>\n"
              << "#include \"string_util.hpp\"\n"
              << "int main() {\n"
              << "    std::cout << shout(\"cxxgen rocks\") << \"\\n\";\n"
              << "    return 0;\n"
              << "}\n";
    }

    cxxgen::Builder b;
    auto &dep = b.dependency("string_util", {.path = pkg_dir});
    assert(dep.has_target("str_util_lib"));
    assert(dep.artifact("str_util_lib").name() == "str_util_lib");

    auto &exe = b.add_executable({
        .name = "child_dep_app",
        .sources = {(app_src_dir / "main.cpp").string()},
        .output_dir = (test_root / "bin").string(),
    });
    exe.link(dep.artifact("str_util_lib"));

    auto toolchain = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(toolchain != nullptr);
    cxxgen::ExecutionContext ctx{.toolchain = *toolchain, .verbose = false};

    auto res = b.execute_all(ctx);
    if (!res.success()) {
        std::cerr << "Build failed: " << res.error_message << "\n";
    }
    assert(res.success());

    fs::path exe_path = cxxgen::CompilationPipeline::get_artifact_path(exe, *toolchain);
    auto run_res = cxxgen::ProcessRunner::run({exe_path.string()});
    assert(run_res.success());
    assert(run_res.stdout_output.find("CXXGEN ROCKS!") != std::string::npos);

    clean_dir(test_root);
    std::cout << "[PASS] test_child_build_cpp_importing passed!\n";
}

void test_git_clone_and_commit_caching() {
    std::cout << "[TEST] Running test_git_clone_and_commit_caching...\n";

    // Verify git is installed before running git clone test
    auto git_check = cxxgen::ProcessRunner::run({"git", "--version"});
    if (!git_check.success()) {
        std::cout << "[SKIP] git not available in environment; skipping git clone test\n";
        return;
    }

    fs::path test_root = fs::current_path() / ".test_git_repo_fixture";
    clean_dir(test_root);

    fs::path repo_dir = test_root / "remote_origin";
    fs::create_directories(repo_dir / "include");
    fs::create_directories(repo_dir / "src");

    // Initialize fixture git repo
    cxxgen::ProcessRunner::run({"git", "-C", repo_dir.string(), "init"});
    cxxgen::ProcessRunner::run({"git", "-C", repo_dir.string(), "config", "user.name", "CxxgenTest"});
    cxxgen::ProcessRunner::run({"git", "-C", repo_dir.string(), "config", "user.email", "test@cxxgen.dev"});

    {
        std::ofstream h(repo_dir / "include" / "git_calc.hpp");
        h << "#pragma once\nint multiply(int a, int b);\n";
    }
    {
        std::ofstream s(repo_dir / "src" / "git_calc.cpp");
        s << "#include \"git_calc.hpp\"\nint multiply(int a, int b) { return a * b; }\n";
    }
    {
        std::ofstream bcpp(repo_dir / "build.cpp");
        bcpp << "#include \"cxxgen.hpp\"\n"
             << "void build(cxxgen::Builder &b) {\n"
             << "    b.add_static_library({\n"
             << "        .name = \"git_calc_lib\",\n"
             << "        .sources = {\"src/git_calc.cpp\"},\n"
             << "        .include_dirs = {\"include\"}\n"
             << "    });\n"
             << "}\n"
             << "CXXGEN_MAIN(build)\n";
    }

    cxxgen::ProcessRunner::run({"git", "-C", repo_dir.string(), "add", "."});
    cxxgen::ProcessRunner::run({"git", "-C", repo_dir.string(), "commit", "-m", "Initial commit"});

    auto rev = cxxgen::ProcessRunner::run({"git", "-C", repo_dir.string(), "rev-parse", "HEAD"});
    assert(rev.success());
    std::string commit_hash = rev.stdout_output;
    while (!commit_hash.empty() &&
           (commit_hash.back() == '\r' || commit_hash.back() == '\n' || commit_hash.back() == ' ')) {
        commit_hash.pop_back();
    }
    assert(commit_hash.size() == 40);

    fs::path cache_dir = test_root / "pkg_cache";

    // 1. First fetch: should clone and cache
    {
        cxxgen::Builder b;
        auto &dep = b.dependency("git_calc", {
                                                 .git = repo_dir.string(),
                                                 .commit = commit_hash,
                                                 .cache_dir = cache_dir,
                                             });

        assert(dep.name() == "git_calc");
        assert(!dep.is_cached()); // First fetch was not cached
        assert(dep.commit_hash() == commit_hash);
        assert(dep.has_target("git_calc_lib"));
        assert(fs::exists(dep.path() / ".cxxgen_pkg_ready"));
    }

    // 2. Second fetch: must hit the cache permanently!
    {
        auto t0 = std::chrono::steady_clock::now();
        cxxgen::Builder b;
        auto &dep = b.dependency("git_calc", {
                                                 .git = repo_dir.string(),
                                                 .commit = commit_hash,
                                                 .cache_dir = cache_dir,
                                             });
        auto t1 = std::chrono::steady_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        assert(dep.is_cached()); // Cache hit!
        assert(dep.commit_hash() == commit_hash);
        assert(dep.has_target("git_calc_lib"));
        std::cout << "  [INFO] Cache hit took: " << duration_ms << " ms\n";
    }

    // 3. Delete the original remote repo! Cached repo must still resolve and build!
    clean_dir(repo_dir);
    assert(!fs::exists(repo_dir));

    {
        fs::path app_src_dir = test_root / "app_src";
        fs::create_directories(app_src_dir);
        {
            std::ofstream app_s(app_src_dir / "main.cpp");
            app_s << "#include <iostream>\n"
                  << "#include \"git_calc.hpp\"\n"
                  << "int main() {\n"
                  << "    std::cout << \"PRODUCT=\" << multiply(6, 7) << \"\\n\";\n"
                  << "    return 0;\n"
                  << "}\n";
        }

        cxxgen::Builder b;
        auto &dep = b.dependency("git_calc", {
                                                 .git = repo_dir.string(),
                                                 .commit = commit_hash,
                                                 .cache_dir = cache_dir,
                                             });
        assert(dep.is_cached());
        assert(dep.has_target("git_calc_lib"));

        auto &exe = b.add_executable({
            .name = "git_dep_app",
            .sources = {(app_src_dir / "main.cpp").string()},
            .output_dir = (test_root / "bin").string(),
        });
        exe.link(dep.artifact("git_calc_lib"));

        auto toolchain = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
        assert(toolchain != nullptr);
        cxxgen::ExecutionContext ctx{.toolchain = *toolchain, .verbose = false};

        auto res = b.execute_all(ctx);
        if (!res.success()) {
            std::cerr << "Build failed: " << res.error_message << "\n";
        }
        assert(res.success());

        fs::path exe_path = cxxgen::CompilationPipeline::get_artifact_path(exe, *toolchain);
        auto run_res = cxxgen::ProcessRunner::run({exe_path.string()});
        assert(run_res.success());
        assert(run_res.stdout_output.find("PRODUCT=42") != std::string::npos);
    }

    clean_dir(test_root);
    std::cout << "[PASS] test_git_clone_and_commit_caching passed!\n";
}

void test_error_handling() {
    std::cout << "[TEST] Running test_error_handling...\n";
    cxxgen::Builder b;

    // Missing git and path
    bool caught_no_source = false;
    try {
        b.dependency("bad_dep", {});
    } catch (const std::exception &e) {
        caught_no_source = true;
        std::string msg = e.what();
        assert(msg.find("neither 'git' URL nor 'path'") != std::string::npos);
    }
    assert(caught_no_source);

    // Non-existent path
    bool caught_bad_path = false;
    try {
        b.dependency("missing_path", {.path = "non_existent_path_xyz_123"});
    } catch (const std::exception &e) {
        caught_bad_path = true;
        std::string msg = e.what();
        assert(msg.find("does not exist") != std::string::npos);
    }
    assert(caught_bad_path);

    // Non-existent target lookup
    fs::path temp_dir = fs::current_path() / ".test_err_dir";
    clean_dir(temp_dir);
    fs::create_directories(temp_dir);
    auto &dep = b.dependency("dummy_dep", {.path = temp_dir});
    bool caught_bad_target = false;
    try {
        (void)dep.target("imaginary_target");
    } catch (const std::exception &e) {
        caught_bad_target = true;
        std::string msg = e.what();
        assert(msg.find("imaginary_target") != std::string::npos);
    }
    assert(caught_bad_target);

    clean_dir(temp_dir);
    std::cout << "[PASS] test_error_handling passed!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Running Story 8.1 Git Dependency Test Suite\n";
    std::cout << "========================================\n";

    test_local_path_dependency();
    test_child_build_cpp_importing();
    test_git_clone_and_commit_caching();
    test_error_handling();

    std::cout << "========================================\n";
    std::cout << "All Story 8.1 Tests Passed Successfully!\n";
    std::cout << "========================================\n";
    return 0;
}
