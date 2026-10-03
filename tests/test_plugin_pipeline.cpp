#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

void setup_plugin_fixtures(const fs::path &fixture_dir) {
    fs::create_directories(fixture_dir / "plugin_src");
    fs::create_directories(fixture_dir / "host_src");

    // 1. sample_plugin.cpp
    std::ofstream plug(fixture_dir / "plugin_src/sample_plugin.cpp");
    plug << "#if defined(_WIN32)\n"
         << "  #define PLUGIN_API __declspec(dllexport)\n"
         << "#else\n"
         << "  #define PLUGIN_API __attribute__((visibility(\"default\")))\n"
         << "#endif\n\n"
         << "extern \"C\" {\n"
         << "PLUGIN_API int plugin_init() {\n"
         << "    return 42;\n"
         << "}\n\n"
         << "PLUGIN_API int plugin_compute(int a, int b) {\n"
         << "    return (a + b) * 2;\n"
         << "}\n\n"
         << "PLUGIN_API const char *plugin_name() {\n"
         << "    return \"SampleFilterPlugin\";\n"
         << "}\n"
         << "}\n";
    plug.close();

    // 2. host_main.cpp (executable that loads a plugin dynamically from path given in argv[1])
    std::ofstream host(fixture_dir / "host_src/host_main.cpp");
    host << "#include <iostream>\n"
         << "#include <string>\n"
         << "#if defined(_WIN32)\n"
         << "#define WIN32_LEAN_AND_MEAN\n"
         << "#include <windows.h>\n"
         << "#else\n"
         << "#include <dlfcn.h>\n"
         << "#endif\n\n"
         << "int main(int argc, char* argv[]) {\n"
         << "    if (argc < 2) {\n"
         << "        std::cerr << \"Usage: host <plugin_path>\\n\";\n"
         << "        return 1;\n"
         << "    }\n"
         << "    std::string plugin_path = argv[1];\n"
         << "#if defined(_WIN32)\n"
         << "    HMODULE mod = LoadLibraryA(plugin_path.c_str());\n"
         << "    if (!mod) {\n"
         << "        std::cerr << \"Failed to LoadLibraryA: \" << plugin_path << \" error: \" << GetLastError() << "
            "\"\\n\";\n"
         << "        return 2;\n"
         << "    }\n"
         << "#if defined(__GNUC__)\n"
         << "#pragma GCC diagnostic push\n"
         << "#pragma GCC diagnostic ignored \"-Wcast-function-type\"\n"
         << "#endif\n"
         << "    auto init_fn = reinterpret_cast<int(*)()>(GetProcAddress(mod, \"plugin_init\"));\n"
         << "    auto comp_fn = reinterpret_cast<int(*)(int, int)>(GetProcAddress(mod, \"plugin_compute\"));\n"
         << "    auto name_fn = reinterpret_cast<const char*(*)()>(GetProcAddress(mod, \"plugin_name\"));\n"
         << "#if defined(__GNUC__)\n"
         << "#pragma GCC diagnostic pop\n"
         << "#endif\n"
         << "    if (!init_fn || !comp_fn || !name_fn) {\n"
         << "        std::cerr << \"Failed to get symbols from plugin\\n\";\n"
         << "        FreeLibrary(mod);\n"
         << "        return 3;\n"
         << "    }\n"
         << "    std::cout << \"PluginLoaded: \" << name_fn() << \" InitVal: \" << init_fn()\n"
         << "              << \" Calc: \" << comp_fn(10, 20) << \"\\n\";\n"
         << "    FreeLibrary(mod);\n"
         << "#else\n"
         << "    void *mod = dlopen(plugin_path.c_str(), RTLD_NOW);\n"
         << "    if (!mod) {\n"
         << "        std::cerr << \"Failed to dlopen: \" << dlerror() << \"\\n\";\n"
         << "        return 2;\n"
         << "    }\n"
         << "    auto init_fn = reinterpret_cast<int(*)()>(dlsym(mod, \"plugin_init\"));\n"
         << "    auto comp_fn = reinterpret_cast<int(*)(int, int)>(dlsym(mod, \"plugin_compute\"));\n"
         << "    auto name_fn = reinterpret_cast<const char*(*)()>(dlsym(mod, \"plugin_name\"));\n"
         << "    if (!init_fn || !comp_fn || !name_fn) {\n"
         << "        std::cerr << \"Failed to get symbols from plugin: \" << dlerror() << \"\\n\";\n"
         << "        dlclose(mod);\n"
         << "        return 3;\n"
         << "    }\n"
         << "    std::cout << \"PluginLoaded: \" << name_fn() << \" InitVal: \" << init_fn()\n"
         << "              << \" Calc: \" << comp_fn(10, 20) << \"\\n\";\n"
         << "    dlclose(mod);\n"
         << "#endif\n"
         << "    return 0;\n"
         << "}\n";
    host.close();
}

void cleanup_plugin_fixtures(const fs::path &fixture_dir) {
    std::error_code ec;
    fs::remove_all(fixture_dir, ec);
    fs::remove_all("bin", ec);
    fs::remove_all("lib", ec);
    fs::remove_all("custom_plugin_out", ec);
    fs::remove_all(".cxxgen", ec);
}

void test_plugin_pipeline(const cxxgen::IToolchain &toolchain) {
    using namespace cxxgen;
    fs::path fixture_dir = "tests/fixtures/test_plugin_pipeline";

    Builder b;

    // 1. Define standard plugin target
    auto &plugin_target = b.add_plugin({
        .name = "filter_plugin",
        .sources = {(fixture_dir / "plugin_src/sample_plugin.cpp").string()},
        .cpp_version = CppVersion::Cpp20,
    });

    // Verify artifact path defaults to bin/plugins/<name>.<ext>
    fs::path plugin_path = b.get_artifact_path(plugin_target, toolchain);
#if defined(_WIN32)
    assert(plugin_path.extension() == ".dll");
#elif defined(__APPLE__)
    assert(plugin_path.extension() == ".dylib");
#else
    assert(plugin_path.extension() == ".so");
#endif
    assert(plugin_path.parent_path() == fs::path("bin/plugins"));

    // 2. Build plugin module
    BuildResult plug_res = b.build_target(plugin_target, toolchain);
    if (!plug_res.success) {
        std::cerr << "Plugin build failed: " << plug_res.error_message << "\n";
    }
    assert(plug_res.success);
    assert(fs::exists(plug_res.artifact_path));

    // 3. Direct dynamic loading test via OS API
#if defined(_WIN32)
    HMODULE handle = LoadLibraryA(plug_res.artifact_path.c_str());
    if (!handle) {
        std::cerr << "Direct LoadLibraryA failed on " << plug_res.artifact_path << " error: " << GetLastError() << "\n";
    }
    assert(handle != nullptr);

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    auto init_fn = reinterpret_cast<int (*)()>(GetProcAddress(handle, "plugin_init"));
    assert(init_fn != nullptr);
    assert(init_fn() == 42);

    auto comp_fn = reinterpret_cast<int (*)(int, int)>(GetProcAddress(handle, "plugin_compute"));
    assert(comp_fn != nullptr);
    assert(comp_fn(10, 20) == 60);

    auto name_fn = reinterpret_cast<const char *(*)()>(GetProcAddress(handle, "plugin_name"));
    assert(name_fn != nullptr);
    assert(std::string(name_fn()) == "SampleFilterPlugin");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    FreeLibrary(handle);
#else
    void *handle = dlopen(plug_res.artifact_path.c_str(), RTLD_NOW);
    assert(handle != nullptr);
    auto init_fn = reinterpret_cast<int (*)()>(dlsym(handle, "plugin_init"));
    assert(init_fn != nullptr);
    assert(init_fn() == 42);
    auto comp_fn = reinterpret_cast<int (*)(int, int)>(dlsym(handle, "plugin_compute"));
    assert(comp_fn != nullptr);
    assert(comp_fn(10, 20) == 60);
    auto name_fn = reinterpret_cast<const char *(*)()>(dlsym(handle, "plugin_name"));
    assert(name_fn != nullptr);
    assert(std::string(name_fn()) == "SampleFilterPlugin");
    dlclose(handle);
#endif

    std::cout << "[PASS] Direct dynamic load test passed for " << toolchain.name() << "\n";

    // 4. Build host executable and load plugin via process invocation
    auto &host_app = b.add_executable({
        .name = "plugin_host",
        .sources = {(fixture_dir / "host_src/host_main.cpp").string()},
        .cpp_version = CppVersion::Cpp20,
    });

    BuildResult host_res = b.build_target(host_app, toolchain);
    if (!host_res.success) {
        std::cerr << "Host executable build failed: " << host_res.error_message << "\n";
    }
    assert(host_res.success);
    assert(fs::exists(host_res.artifact_path));

    ProcessResult run_res = ProcessRunner::run({host_res.artifact_path, plug_res.artifact_path});
    if (!run_res.success()) {
        std::cerr << "Host runner failed: " << run_res.combined_output() << "\n";
    }
    assert(run_res.success());
    assert(run_res.stdout_output.find("PluginLoaded: SampleFilterPlugin InitVal: 42 Calc: 60") != std::string::npos);

    std::cout << "[PASS] Host process execution with dynamic plugin passed for " << toolchain.name() << "\n";

    // 5. Test custom output directory and custom output name
    auto &custom_plugin = b.add_plugin({
        .name = "dsp_plugin",
        .sources = {(fixture_dir / "plugin_src/sample_plugin.cpp").string()},
        .cpp_version = CppVersion::Cpp20,
        .output_dir = "custom_plugin_out",
        .output_name = "audio_dsp",
    });

    fs::path custom_plug_path = b.get_artifact_path(custom_plugin, toolchain);
    assert(custom_plug_path.parent_path() == fs::path("custom_plugin_out"));
    assert(custom_plug_path.string().find("audio_dsp") != std::string::npos);

    BuildResult custom_plug_res = b.build_target(custom_plugin, toolchain);
    assert(custom_plug_res.success);
    assert(fs::exists(custom_plug_res.artifact_path));

    ProcessResult custom_run_res = ProcessRunner::run({host_res.artifact_path, custom_plug_res.artifact_path});
    assert(custom_run_res.success());
    assert(custom_run_res.stdout_output.find("PluginLoaded: SampleFilterPlugin InitVal: 42 Calc: 60") !=
           std::string::npos);

    std::cout << "[PASS] Custom output plugin pipeline tests passed for " << toolchain.name() << "\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Running Dynamic Plugin Pipeline Test Suite (Story 3.4)\n";
    std::cout << "========================================\n";

    fs::path fixture_dir = "tests/fixtures/test_plugin_pipeline";
    cleanup_plugin_fixtures(fixture_dir);
    setup_plugin_fixtures(fixture_dir);

    // 1. Default toolchain (Clang or GCC)
    auto toolchain = cxxgen::find_toolchain();
    assert(toolchain != nullptr);
    std::cout << "[INFO] Using Primary Toolchain: " << toolchain->name() << "\n";
    test_plugin_pipeline(*toolchain);

    // 2. Also test with MSVC if available
    auto msvc = cxxgen::find_msvc_toolchain();
    if (msvc.has_value()) {
        std::cout << "\n[INFO] Testing with MSVC Toolchain: " << msvc->name() << "\n";
        test_plugin_pipeline(*msvc);
    }

    cleanup_plugin_fixtures(fixture_dir);

    std::cout << "========================================\n";
    std::cout << "All Story 3.4 Dynamic Plugin Tests Passed Successfully!\n";
    std::cout << "========================================\n";
    return 0;
}
