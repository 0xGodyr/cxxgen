#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

static void clean_dir(const fs::path &p) {
    std::error_code ec;
    fs::remove_all(p, ec);
}

void test_flag_parsers() {
    std::cout << "[TEST] Running test_flag_parsers...\n";

    // 1. split_arguments
    {
        std::string raw = "-I\"C:\\Program Files\\Vulkan\\Include\" -DENABLE_VULKAN=1 -O2 'arg with spaces'";
        auto tokens = cxxgen::split_arguments(raw);
        assert(tokens.size() == 4);
        assert(tokens[0] == "-IC:\\Program Files\\Vulkan\\Include");
        assert(tokens[1] == "-DENABLE_VULKAN=1");
        assert(tokens[2] == "-O2");
        assert(tokens[3] == "arg with spaces");
    }

    // 2. parse_pkg_config_cflags
    {
        std::string output = "-I/usr/include/SDL2 -D_REENTRANT -pthread -Wall";
        std::vector<std::string> incs, defs, flags;
        cxxgen::parse_pkg_config_cflags(output, incs, defs, flags);

        assert(incs.size() == 1);
        assert(incs[0] == "/usr/include/SDL2");
        assert(defs.size() == 1);
        assert(defs[0] == "_REENTRANT");
        assert(flags.size() == 2);
        assert(flags[0] == "-pthread");
        assert(flags[1] == "-Wall");
    }

    // 3. parse_pkg_config_libs
    {
        std::string output = "-L/usr/lib/x86_64-linux-gnu -lSDL2 -Wl,-rpath,/usr/lib";
        std::vector<std::string> lib_dirs, libs, link_flags;
        cxxgen::parse_pkg_config_libs(output, lib_dirs, libs, link_flags);

        assert(lib_dirs.size() == 1);
        assert(lib_dirs[0] == "/usr/lib/x86_64-linux-gnu");
        assert(libs.size() == 1);
        assert(libs[0] == "SDL2");
        assert(link_flags.size() == 1);
        assert(link_flags[0] == "-Wl,-rpath,/usr/lib");
    }

    std::cout << "[PASS] test_flag_parsers passed!\n";
}

void test_standard_and_env_packages() {
    std::cout << "[TEST] Running test_standard_and_env_packages...\n";
    cxxgen::Builder b;

#if defined(_WIN32)
    // On Windows, ws2_32 and opengl should always resolve
    auto ws2 = b.find_system_package("ws2_32");
    assert(ws2.is_found());
    assert(std::find(ws2.link_libraries.begin(), ws2.link_libraries.end(), "ws2_32") != ws2.link_libraries.end());

    auto gl = b.find_system_package("opengl");
    assert(gl.is_found());
    assert(std::find(gl.link_libraries.begin(), gl.link_libraries.end(), "opengl32") != gl.link_libraries.end());
#else
    // On POSIX, math library or pthread should always resolve
    auto m = b.find_system_package("m");
    assert(m.is_found());
    assert(std::find(m.link_libraries.begin(), m.link_libraries.end(), "m") != m.link_libraries.end());
#endif

    // Test VULKAN_SDK environment fallback
    fs::path temp_sdk = fs::current_path() / ".test_mock_vulkan_sdk";
    clean_dir(temp_sdk);
    fs::create_directories(temp_sdk / "Include" / "vulkan");
#if defined(_WIN32)
    fs::create_directories(temp_sdk / "Lib");
    {
        std::ofstream f(temp_sdk / "Lib" / "vulkan-1.lib");
        f << "dummy lib";
    }
#else
    fs::create_directories(temp_sdk / "lib");
    {
        std::ofstream f(temp_sdk / "lib" / "libvulkan.so");
        f << "dummy lib";
    }
#endif
    {
        std::ofstream f(temp_sdk / "Include" / "vulkan" / "vulkan.h");
        f << "#pragma once\n";
    }

#if defined(_WIN32)
    _putenv_s("VULKAN_SDK", temp_sdk.string().c_str());
#else
    setenv("VULKAN_SDK", temp_sdk.string().c_str(), 1);
#endif

    auto vlk = b.find_system_package("vulkan");
    assert(vlk.is_found());
    assert(!vlk.include_dirs.empty());
    assert(vlk.include_dirs[0].find(".test_mock_vulkan_sdk") != std::string::npos);

    clean_dir(temp_sdk);
    std::cout << "[PASS] test_standard_and_env_packages passed!\n";
}

void test_hints_discovery() {
    std::cout << "[TEST] Running test_hints_discovery...\n";
    fs::path test_root = fs::current_path() / ".test_pkg_hints";
    clean_dir(test_root);

    fs::path custom_inc = test_root / "my_sdk" / "include";
    fs::path custom_lib = test_root / "my_sdk" / "lib";
    fs::create_directories(custom_inc);
    fs::create_directories(custom_lib);

    {
        std::ofstream f(custom_inc / "custom_crypto.h");
        f << "#pragma once\nint crypto_hash_test();\n";
    }
    {
#if defined(_WIN32)
        std::ofstream f(custom_lib / "custom_crypto.lib");
#else
        std::ofstream f(custom_lib / "libcustom_crypto.a");
#endif
        f << "dummy archive";
    }

    cxxgen::Builder b;
    auto pkg = b.find_system_package("custom_crypto", {
                                                          .include_hints = {custom_inc.string()},
                                                          .lib_hints = {custom_lib.string()},
                                                      });

    assert(pkg.is_found());
    assert(std::find(pkg.include_dirs.begin(), pkg.include_dirs.end(), custom_inc.string()) != pkg.include_dirs.end());
    assert(std::find(pkg.lib_dirs.begin(), pkg.lib_dirs.end(), custom_lib.string()) != pkg.lib_dirs.end());
    assert(std::find(pkg.link_libraries.begin(), pkg.link_libraries.end(), "custom_crypto") !=
           pkg.link_libraries.end());

    clean_dir(test_root);
    std::cout << "[PASS] test_hints_discovery passed!\n";
}

void test_end_to_end_linking() {
    std::cout << "[TEST] Running test_end_to_end_linking...\n";
    fs::path test_root = fs::current_path() / ".test_pkg_e2e";
    clean_dir(test_root);

    fs::path src_dir = test_root / "src";
    fs::create_directories(src_dir);

    {
        std::ofstream f(src_dir / "main.cpp");
#if defined(_WIN32)
        f << "#include <iostream>\n"
          << "#include <winsock2.h>\n"
          << "int main() {\n"
          << "    WSADATA wsaData;\n"
          << "    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);\n"
          << "    if (res == 0) {\n"
          << "        std::cout << \"WSAStartup SUCCESS\\n\";\n"
          << "        WSACleanup();\n"
          << "    }\n"
          << "    return 0;\n"
          << "}\n";
#else
        f << "#include <iostream>\n"
          << "#include <cmath>\n"
          << "int main() {\n"
          << "    double val = std::sqrt(1764.0);\n"
          << "    std::cout << \"SQRT=\" << (int)val << \"\\n\";\n"
          << "    return 0;\n"
          << "}\n";
#endif
    }

    // Method 1: Direct exe.link(pkg)
    {
        cxxgen::Builder b;
#if defined(_WIN32)
        auto ws2_pkg = b.find_system_package("ws2_32");
#else
        auto ws2_pkg = b.find_system_package("m");
#endif
        assert(ws2_pkg.is_found());

        auto &exe = b.add_executable({
            .name = "e2e_sys_app",
            .sources = {(src_dir / "main.cpp").string()},
            .output_dir = (test_root / "bin").string(),
        });
        exe.link(ws2_pkg);

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
#if defined(_WIN32)
        assert(run_res.stdout_output.find("WSAStartup SUCCESS") != std::string::npos);
#else
        assert(run_res.stdout_output.find("SQRT=42") != std::string::npos);
#endif
    }

    // Method 2: Via b.add_system_package(...) interface target
    {
        cxxgen::Builder b;
#if defined(_WIN32)
        auto &sys_iface = b.add_system_package("ws2_32");
#else
        auto &sys_iface = b.add_system_package("m");
#endif
        assert(sys_iface.type() == cxxgen::TargetType::Interface);

        auto &exe2 = b.add_executable({
            .name = "e2e_sys_app2",
            .sources = {(src_dir / "main.cpp").string()},
            .output_dir = (test_root / "bin2").string(),
        });
        exe2.link(sys_iface);

        auto toolchain = cxxgen::find_toolchain(cxxgen::CompilerFamily::Auto);
        assert(toolchain != nullptr);
        cxxgen::ExecutionContext ctx{.toolchain = *toolchain, .verbose = false};

        auto res = b.execute_all(ctx);
        if (!res.success()) {
            std::cerr << "Method 2 failed: " << res.error_message << "\n";
        }
        assert(res.success());

        fs::path exe_path = cxxgen::CompilationPipeline::get_artifact_path(exe2, *toolchain);
        auto run_res = cxxgen::ProcessRunner::run({exe_path.string()});
        assert(run_res.success());
    }

    clean_dir(test_root);
    std::cout << "[PASS] test_end_to_end_linking passed!\n";
}

void test_missing_package_and_instructions() {
    std::cout << "[TEST] Running test_missing_package_and_instructions...\n";
    cxxgen::Builder b;

    // 1. Not required: returns pkg with found = false and clear instructions
    auto missing = b.find_system_package("non_existent_library_abc123", {.required = false});
    assert(!missing.is_found());
    assert(!static_cast<bool>(missing));
    assert(!missing.install_instructions.empty());
    assert(missing.error_message.find("not found") != std::string::npos);
#if defined(_WIN32)
    assert(missing.install_instructions.find("vcpkg install") != std::string::npos);
    assert(missing.install_instructions.find("scoop install") != std::string::npos);
#elif defined(__APPLE__)
    assert(missing.install_instructions.find("brew install") != std::string::npos);
#else
    assert(missing.install_instructions.find("apt-get install") != std::string::npos);
#endif

    // 2. Required (default): throws std::runtime_error with actionable instructions
    bool caught_exception = false;
    try {
        b.find_system_package("non_existent_library_abc123");
    } catch (const std::exception &e) {
        caught_exception = true;
        std::string msg = e.what();
        assert(msg.find("not found on this system") != std::string::npos);
        assert(msg.find("Actionable Installation Instructions") != std::string::npos);
    }
    assert(caught_exception);

    // 3. Custom install_hint
    bool caught_custom_hint = false;
    try {
        b.find_system_package("custom_missing_pkg",
                              {
                                  .install_hint = "Download SDK from https://example.com/sdk and set CUSTOM_SDK_PATH",
                              });
    } catch (const std::exception &e) {
        caught_custom_hint = true;
        std::string msg = e.what();
        assert(msg.find("https://example.com/sdk") != std::string::npos);
    }
    assert(caught_custom_hint);

    std::cout << "[PASS] test_missing_package_and_instructions passed!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Running Story 8.2 System Package Test Suite\n";
    std::cout << "========================================\n";

    test_flag_parsers();
    test_standard_and_env_packages();
    test_hints_discovery();
    test_end_to_end_linking();
    test_missing_package_and_instructions();

    std::cout << "========================================\n";
    std::cout << "All Story 8.2 Tests Passed Successfully!\n";
    std::cout << "========================================\n";
    return 0;
}
