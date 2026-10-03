#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void test_self_hosting_build() {
    std::cout << "[TEST] Running test_self_hosting_build...\n";

    // 1. Compile build.cpp with host compiler
    auto tc = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
    assert(tc != nullptr);

    fs::path runner_bin = fs::current_path() / "build_runner_self_host.exe";
    std::error_code ec;
    fs::remove(runner_bin, ec);

    cxxgen::CompileParams cp{
        .source_file = "build.cpp",
        .object_file = "build_runner_self_host.obj",
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .optimize = cxxgen::Optimization::ReleaseFast,
        .include_dirs = {"include"},
    };

    cxxgen::JitRunnerConfig jcfg;
    jcfg.working_dir = fs::current_path();
    jcfg.build_script = fs::current_path() / "build.cpp";
    jcfg.cxxgen_header = fs::current_path() / "include" / "cxxgen.hpp";
    jcfg.toolchain = tc;
    jcfg.force_rebuild = true;

    auto jres = cxxgen::JitRunner::prepare_runner(jcfg);
    assert(jres.success);
    assert(fs::exists(jres.runner_binary));

    // 2. Execute runner to build bin/cxxgen
    int code = cxxgen::JitRunner::execute_runner(jres.runner_binary, {"build"}, fs::current_path());
    assert(code == 0);

#if defined(_WIN32)
    fs::path cxxgen_bin = fs::current_path() / "bin" / "cxxgen.exe";
#else
    fs::path cxxgen_bin = fs::current_path() / "bin" / "cxxgen";
#endif
    assert(fs::exists(cxxgen_bin));
    std::cout << "[PASS] Self-hosting build produced: " << cxxgen_bin.string() << "\n";
}

void test_cli_binary_execution() {
    std::cout << "[TEST] Running test_cli_binary_execution...\n";
#if defined(_WIN32)
    fs::path cxxgen_bin = fs::current_path() / "bin" / "cxxgen.exe";
#else
    fs::path cxxgen_bin = fs::current_path() / "bin" / "cxxgen";
#endif
    assert(fs::exists(cxxgen_bin));

    // Test --version
    {
        cxxgen::ProcessOptions popts;
        popts.command = {cxxgen_bin.string(), "--version"};
        auto res = cxxgen::ProcessRunner::run(popts);
        assert(res.success());
        assert(res.combined_output().find("cxxgen version") != std::string::npos);
    }

    // Test --help
    {
        cxxgen::ProcessOptions popts;
        popts.command = {cxxgen_bin.string(), "--help"};
        auto res = cxxgen::ProcessRunner::run(popts);
        assert(res.success());
        assert(res.combined_output().find("Usage: cxxgen") != std::string::npos);
    }

    // Test toolchain detect
    {
        cxxgen::ProcessOptions popts;
        popts.command = {cxxgen_bin.string(), "toolchain", "detect"};
        auto res = cxxgen::ProcessRunner::run(popts);
        assert(res.success());
        assert(res.combined_output().find("Toolchain Discovery") != std::string::npos);
    }

    std::cout << "[PASS] CLI binary execution tests passed!\n";
}

void test_downstream_project_building() {
    std::cout << "[TEST] Running test_downstream_project_building...\n";

#if defined(_WIN32)
    fs::path cxxgen_bin = fs::current_path() / "bin" / "cxxgen.exe";
#else
    fs::path cxxgen_bin = fs::current_path() / "bin" / "cxxgen";
#endif
    assert(fs::exists(cxxgen_bin));

    fs::path downstream_dir = fs::current_path() / ".cxxgen_test_downstream_project";
    std::error_code ec;
    fs::remove_all(downstream_dir, ec);
    fs::create_directories(downstream_dir / "src", ec);
    fs::create_directories(downstream_dir / "include", ec);

    // Copy cxxgen.hpp into downstream include/
    fs::copy_file(fs::current_path() / "include" / "cxxgen.hpp", downstream_dir / "include" / "cxxgen.hpp",
                  fs::copy_options::overwrite_existing, ec);

    // Create src/main.cpp
    std::ofstream src(downstream_dir / "src" / "main.cpp");
    src << "#include <iostream>\n"
        << "int main() {\n"
        << "    std::cout << \"[DOGFOOD_SUCCESS] Hello from downstream project!\\n\";\n"
        << "    return 0;\n"
        << "}\n";
    src.close();

    // Create build.cpp
    std::ofstream bld(downstream_dir / "build.cpp");
    bld << "#include \"include/cxxgen.hpp\"\n"
        << "void build(cxxgen::Builder &b) {\n"
        << "    auto exe = b.add_executable({\n"
        << "        .name = \"downstream_app\",\n"
        << "        .sources = {\"src/main.cpp\"},\n"
        << "        .include_dirs = {\"include\"},\n"
        << "    });\n"
        << "    b.install_artifact(exe);\n"
        << "}\n"
        << "CXXGEN_MAIN(build)\n";
    bld.close();

    // Run cxxgen build using the self-hosted binary
    {
        cxxgen::ProcessOptions popts;
        popts.command = {cxxgen_bin.string(), "build"};
        popts.working_directory = downstream_dir.string();
        auto res = cxxgen::ProcessRunner::run(popts);
        if (!res.success()) {
            std::cerr << "Downstream build failed:\n" << res.combined_output() << "\n";
        }
        assert(res.success());
    }

#if defined(_WIN32)
    fs::path app_bin = downstream_dir / "bin" / "downstream_app.exe";
#else
    fs::path app_bin = downstream_dir / "bin" / "downstream_app";
#endif
    assert(fs::exists(app_bin));

    // Execute downstream application and check output
    {
        cxxgen::ProcessOptions popts;
        popts.command = {app_bin.string()};
        auto res = cxxgen::ProcessRunner::run(popts);
        assert(res.success());
        assert(res.combined_output().find("[DOGFOOD_SUCCESS]") != std::string::npos);
    }

    // Clean up
    fs::remove_all(downstream_dir, ec);
    std::cout << "[PASS] Downstream project built and verified successfully!\n";
}

int main() {
    std::cout << "Running Story 10.1 Self-Hosting Test Suite...\n";
    test_self_hosting_build();
    test_cli_binary_execution();
    test_downstream_project_building();
    std::cout << "All Story 10.1 Self-Hosting Tests Passed Successfully!\n";
    return 0;
}
