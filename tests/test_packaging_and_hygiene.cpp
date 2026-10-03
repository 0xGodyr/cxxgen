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

void test_sha256_file_hashing() {
    std::cout << "Testing Sha256::hash_file...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_sha256";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path sample_file = test_dir / "sample.txt";
    std::string sample_data = "Hello, C++ Builder Enterprise Packaging Engine!";
    std::ofstream out(sample_file, std::ios::binary);
    out << sample_data;
    out.close();

    std::string mem_hash = cxxgen::hash::Sha256::hash(sample_data);
    std::string file_hash = cxxgen::hash::Sha256::hash_file(sample_file);

    assert(!file_hash.empty());
    assert(file_hash == mem_hash);
    assert(file_hash.size() == 64);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] Sha256::hash_file verified.\n";
}

void test_lockfile_serialization_and_loading() {
    std::cout << "Testing DependencyLockfile serialization, loading, and formatting...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_lockfile";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    cxxgen::DependencyLockfile lf = cxxgen::DependencyLockfile::load(test_dir);
    assert(!lf.exists_on_disk());
    assert(lf.entries().empty());

    // 1. Add entries
    cxxgen::DependencyLockEntry git_entry;
    git_entry.name = "fmt";
    git_entry.type = "git";
    git_entry.url = "https://github.com/fmtlib/fmt.git";
    git_entry.tag = "10.2.1";
    git_entry.commit = "e69e5f977dffe65e1d4e6d7bf766df5337164e9f";
    lf.set(git_entry);

    cxxgen::DependencyLockEntry url_entry;
    url_entry.name = "stb";
    url_entry.type = "url";
    url_entry.url = "https://github.com/nothings/stb/archive/master.zip";
    url_entry.sha256 = "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef";
    lf.set(url_entry);

    assert(lf.entries().size() == 2);
    assert(lf.find("fmt") != nullptr);
    assert(lf.find("stb") != nullptr);
    assert(lf.find("nonexistent") == nullptr);

    // 2. Save lockfile to disk
    lf.save(test_dir);
    assert(fs::exists(test_dir / "cxxgen.lock"));

    // 3. Reload lockfile from disk and verify fields
    cxxgen::DependencyLockfile reloaded = cxxgen::DependencyLockfile::load(test_dir);
    assert(reloaded.exists_on_disk());
    assert(reloaded.entries().size() == 2);

    const auto *fmt_reloaded = reloaded.find("fmt");
    assert(fmt_reloaded != nullptr);
    assert(fmt_reloaded->type == "git");
    assert(fmt_reloaded->url == "https://github.com/fmtlib/fmt.git");
    assert(fmt_reloaded->tag == "10.2.1");
    assert(fmt_reloaded->commit == "e69e5f977dffe65e1d4e6d7bf766df5337164e9f");

    const auto *stb_reloaded = reloaded.find("stb");
    assert(stb_reloaded != nullptr);
    assert(stb_reloaded->type == "url");
    assert(stb_reloaded->url == "https://github.com/nothings/stb/archive/master.zip");
    assert(stb_reloaded->sha256 == "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef");

    // 4. Remove entry
    reloaded.remove("stb");
    assert(reloaded.find("stb") == nullptr);
    assert(reloaded.entries().size() == 1);
    reloaded.save(test_dir);

    cxxgen::DependencyLockfile reloaded2 = cxxgen::DependencyLockfile::load(test_dir);
    assert(reloaded2.entries().size() == 1);
    assert(reloaded2.find("fmt") != nullptr);
    assert(reloaded2.find("stb") == nullptr);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] DependencyLockfile serialization and reloading verified.\n";
}

void test_lockfile_enforcement_and_tamper_detection() {
    std::cout << "Testing --locked mode enforcement and tamper detection...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_locked_enforce";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    // Create a mock local dependency
    fs::path dep_dir = test_dir / "mock_pkg";
    fs::create_directories(dep_dir / "include", ec);
    std::ofstream hdr(dep_dir / "include" / "mock.h");
    hdr << "#pragma once\ninline int mock_val() { return 42; }\n";
    hdr.close();

    // Prepare a lockfile with pinned package
    cxxgen::DependencyLockfile lf = cxxgen::DependencyLockfile::load(test_dir);
    cxxgen::DependencyLockEntry entry;
    entry.name = "mock_pkg";
    entry.type = "url";
    entry.url = (dep_dir / "include" / "mock.h").string();
    std::string mock_sha = cxxgen::hash::Sha256::hash_file(dep_dir / "include" / "mock.h");
    entry.sha256 = mock_sha;
    lf.set(entry);
    lf.save(test_dir);

    // 1. Locked mode with valid hash: should SUCCEED
    {
        cxxgen::Builder b;
        b.set_working_directory(test_dir);
        b.set_locked(true);

        bool threw = false;
        try {
            auto &dep = b.dependency("mock_pkg", {
                                                     .url = (dep_dir / "include" / "mock.h").string(),
                                                     .sha256 = mock_sha,
                                                     .cache_dir = test_dir / "cache",
                                                 });
            assert(dep.name() == "mock_pkg");
        } catch (const std::exception &e) {
            threw = true;
        }
        assert(!threw);
    }

    // 2. Locked mode with missing package in lockfile: should FAIL
    {
        cxxgen::Builder b;
        b.set_working_directory(test_dir);
        b.set_locked(true);

        bool threw = false;
        std::string err_msg;
        try {
            b.dependency("unlocked_pkg", {
                                             .url = (dep_dir / "include" / "mock.h").string(),
                                             .cache_dir = test_dir / "cache",
                                         });
        } catch (const std::exception &e) {
            threw = true;
            err_msg = e.what();
        }
        assert(threw);
        assert(err_msg.find("not recorded in cxxgen.lock") != std::string::npos);
    }

    // 3. Locked mode with SHA-256 tamper/mismatch: should FAIL
    {
        cxxgen::Builder b;
        b.set_working_directory(test_dir);
        b.set_locked(true);

        bool threw = false;
        std::string err_msg;
        try {
            b.dependency("mock_pkg", {
                                         .url = (dep_dir / "include" / "mock.h").string(),
                                         .sha256 = "0000000000000000000000000000000000000000000000000000000000000000",
                                         .cache_dir = test_dir / "cache",
                                     });
        } catch (const std::exception &e) {
            threw = true;
            err_msg = e.what();
        }
        assert(threw);
        assert(err_msg.find("SHA-256 mismatch") != std::string::npos);
    }

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] Locked mode enforcement and tamper detection verified.\n";
}

void test_archive_dependency_fetching_and_sha256_validation() {
    std::cout << "Testing archive dependency fetching, SHA-256 validation, and extraction...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_archive_fetch";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    // 1. Create a dummy library directory structure
    fs::path raw_lib = test_dir / "mathlib_source" / "mathlib-1.0";
    fs::create_directories(raw_lib / "include", ec);
    std::ofstream h(raw_lib / "include" / "my_math.h");
    h << "#pragma once\ninline int add_numbers(int a, int b) { return a + b; }\n";
    h.close();

    // 2. Compress into zip archive using tar
    fs::path zip_path = test_dir / "mathlib-1.0.zip";
    auto zip_res = cxxgen::ProcessRunner::run(
        {"tar", "-a", "-c", "-f", zip_path.string(), "-C", (test_dir / "mathlib_source").string(), "mathlib-1.0"});
    assert(zip_res.success());
    assert(fs::exists(zip_path));

    std::string archive_sha = cxxgen::hash::Sha256::hash_file(zip_path);
    assert(!archive_sha.empty());

    // 3. Fetch using Builder::dependency with SHA-256 verification
    {
        cxxgen::Builder b;
        b.set_working_directory(test_dir);

        auto &dep = b.dependency("mathlib", {
                                                .url = zip_path.string(),
                                                .sha256 = archive_sha,
                                                .cache_dir = test_dir / "pkg_cache",
                                            });

        assert(dep.name() == "mathlib");
        assert(fs::exists(dep.path() / "include" / "my_math.h"));

        // cxxgen.lock should have been automatically created and populated!
        assert(fs::exists(test_dir / "cxxgen.lock"));
        cxxgen::DependencyLockfile lf = cxxgen::DependencyLockfile::load(test_dir);
        const auto *entry = lf.find("mathlib");
        assert(entry != nullptr);
        assert(entry->type == "url");
        assert(entry->sha256 == archive_sha);
    }

    // 4. Test intentional SHA-256 mismatch failure
    {
        cxxgen::Builder b;
        b.set_working_directory(test_dir);

        bool threw = false;
        std::string err;
        try {
            b.dependency("bad_mathlib",
                         {
                             .url = zip_path.string(),
                             .sha256 = "badbadbadbadbadbadbadbadbadbadbadbadbadbadbadbadbadbadbadbadbadb",
                             .cache_dir = test_dir / "pkg_cache",
                             .force_fetch = true,
                         });
        } catch (const std::exception &e) {
            threw = true;
            err = e.what();
        }
        assert(threw);
        assert(err.find("SHA-256 integrity check failed") != std::string::npos);
    }

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] Archive dependency fetching and SHA-256 validation verified.\n";
}

void test_cmake_config_generation_and_find_package() {
    std::cout << "Testing CMake <Package>Config.cmake generation and find_package integration...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_cmake_config";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    // Setup mock project
    cxxgen::Builder b;
    b.set_project_name("AwesomeEngine");

    auto &lib = b.add_static_library({
        .name = "awesome_core",
        .sources = {"src/core.cpp"},
        .include_dirs = {"include"},
        .defines = {"AWESOME_ENGINE_STATIC=1"},
    });

    // Export CMake config
    cxxgen::CMakeConfigOptions opt;
    opt.package_name = "AwesomeEngine";
    opt.version = "3.2.1";
    opt.namespace_name = "Awesome";
    opt.output_dir = test_dir / "lib" / "cmake" / "AwesomeEngine";

    std::string err;
    bool exported = b.export_cmake_config(opt, &err);
    assert(exported);

    fs::path cfg_file = opt.output_dir / "AwesomeEngineConfig.cmake";
    fs::path ver_file = opt.output_dir / "AwesomeEngineConfigVersion.cmake";
    fs::path tgt_file = opt.output_dir / "AwesomeEngineTargets.cmake";

    assert(fs::exists(cfg_file));
    assert(fs::exists(ver_file));
    assert(fs::exists(tgt_file));

    // Verify ConfigVersion file contents
    std::ifstream v_in(ver_file);
    std::string v_content((std::istreambuf_iterator<char>(v_in)), std::istreambuf_iterator<char>());
    assert(v_content.find("PACKAGE_VERSION \"3.2.1\"") != std::string::npos);
    assert(v_content.find("PACKAGE_FIND_VERSION_MAJOR EQUAL 3") != std::string::npos);

    // Verify Targets file contents
    std::ifstream t_in(tgt_file);
    std::string t_content((std::istreambuf_iterator<char>(t_in)), std::istreambuf_iterator<char>());
    assert(t_content.find("Awesome::awesome_core") != std::string::npos);
    assert(t_content.find("INTERFACE_INCLUDE_DIRECTORIES") != std::string::npos);
    assert(t_content.find("AWESOME_ENGINE_STATIC=1") != std::string::npos);

    // Now test real CMake find_package execution via a sample CMake project!
    fs::path cmake_consumer_dir = test_dir / "cmake_consumer";
    fs::create_directories(cmake_consumer_dir, ec);

    std::ofstream cm(cmake_consumer_dir / "CMakeLists.txt");
    cm << "cmake_minimum_required(VERSION 3.15)\n"
       << "project(ConsumerApp LANGUAGES CXX)\n"
       << "list(APPEND CMAKE_PREFIX_PATH \"" << cxxgen::util::normalize_path(test_dir.string(), false) << "\")\n"
       << "find_package(AwesomeEngine 3.2 REQUIRED)\n"
       << "if(TARGET Awesome::awesome_core)\n"
       << "  message(STATUS \"AwesomeEngine target located successfully!\")\n"
       << "else()\n"
       << "  message(FATAL_ERROR \"Target not found\")\n"
       << "endif()\n";
    cm.close();

    auto cm_res = cxxgen::ProcessRunner::run(
        {"cmake", "-S", cmake_consumer_dir.string(), "-B", (cmake_consumer_dir / "build").string()});
    assert(cm_res.success());
    assert(cm_res.combined_output().find("AwesomeEngine target located successfully!") != std::string::npos);

    for (int retry = 0; retry < 5 && fs::exists(test_dir, ec); ++retry) {
        fs::remove_all(test_dir, ec);
        if (ec) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
    std::cout << "[PASS] CMake Config exporter and find_package verified.\n";
}

void test_package_engine_distribution_bundle() {
    std::cout << "Testing PackageEngine distribution bundle creation (cxxgen package)...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_pkg_engine";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    // Setup sample source files
    fs::create_directories(test_dir / "src", ec);
    fs::create_directories(test_dir / "include", ec);

    std::ofstream h(test_dir / "include" / "mylib.h");
    h << "#pragma once\nint mylib_value();\n";
    h.close();

    std::ofstream s_lib(test_dir / "src" / "mylib.cpp");
    s_lib << "#include \"../include/mylib.h\"\nint mylib_value() { return 100; }\n";
    s_lib.close();

    std::ofstream s_main(test_dir / "src" / "main.cpp");
    s_main << "#include \"../include/mylib.h\"\n#include <iostream>\nint main() { std::cout << mylib_value() << "
              "std::endl; return 0; }\n";
    s_main.close();

    std::ofstream readme(test_dir / "README.md");
    readme << "# Sample Package Distribution\n";
    readme.close();

    auto tc = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(tc != nullptr);

    cxxgen::Builder b;
    b.set_working_directory(test_dir);
    b.set_project_name("calculator_sdk");

    auto &lib = b.add_static_library({
        .name = "calc_lib",
        .sources = {(test_dir / "src" / "mylib.cpp").string()},
        .include_dirs = {(test_dir / "include").string()},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .output_dir = (test_dir / "lib").string(),
    });

    auto &app = b.add_executable({
        .name = "calc_cli",
        .sources = {(test_dir / "src" / "main.cpp").string()},
        .include_dirs = {(test_dir / "include").string()},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .output_dir = (test_dir / "bin").string(),
    });
    app.link(lib);
    b.install_artifact(app);
    b.install_artifact(lib);

    // Build artifacts
    auto res_lib = b.build_target(lib, *tc);
    assert(res_lib.success);
    auto res_app = b.build_target(app, *tc);
    assert(res_app.success);

    // Configure package
    cxxgen::PackageConfig pcfg;
    pcfg.package_name = "calculator_sdk";
    pcfg.version = "1.5.0";
    pcfg.output_dir = test_dir / "dist";
    pcfg.include_dirs = {(test_dir / "include").string()};
    pcfg.extra_files = {(test_dir / "README.md").string()};
    pcfg.format = "zip";

    auto pres = cxxgen::PackageEngine::package(b, *tc, pcfg, false);
    assert(pres.success);
    assert(fs::exists(pres.archive_path));
    assert(pres.archive_size > 0);
    assert(pres.package_name == "calculator_sdk");
    assert(pres.version == "1.5.0");

    // Extract generated package to verify layout
    fs::path extract_dir = test_dir / "extracted";
    fs::create_directories(extract_dir, ec);

    auto ext_res = cxxgen::ProcessRunner::run({"tar", "-xf", pres.archive_path.string(), "-C", extract_dir.string()});
    assert(ext_res.success());

    // Verify staging layout in archive
    fs::path base_pkg = extract_dir / "calculator_sdk-1.5.0";
    assert(fs::exists(base_pkg));
    assert(fs::exists(base_pkg / "bin"));
    assert(fs::exists(base_pkg / "lib"));
    assert(fs::exists(base_pkg / "include" / "mylib.h"));
    assert(fs::exists(base_pkg / "README.md"));
    assert(fs::exists(base_pkg / "lib" / "cmake" / "calculator_sdk" / "calculator_sdkConfig.cmake"));
    assert(fs::exists(base_pkg / "lib" / "cmake" / "calculator_sdk" / "calculator_sdkConfigVersion.cmake"));
    assert(fs::exists(base_pkg / "lib" / "cmake" / "calculator_sdk" / "calculator_sdkTargets.cmake"));

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] PackageEngine distribution bundle verified.\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "Running Epic 14: Packaging, Hygiene & CMake Config\n";
    std::cout << "====================================================\n";

    test_sha256_file_hashing();
    test_lockfile_serialization_and_loading();
    test_lockfile_enforcement_and_tamper_detection();
    test_archive_dependency_fetching_and_sha256_validation();
    test_cmake_config_generation_and_find_package();
    test_package_engine_distribution_bundle();

    std::cout << "\nAll Epic 14 Tests Passed Successfully!\n";
    return 0;
}
