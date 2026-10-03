#include "cxxgen.hpp"

#include <cassert>
#include <chrono>
#include <iostream>

void test_msvc_command_construction() {
    using namespace cxxgen;
    MsvcToolchain tc("cl.exe", "link.exe", "lib.exe", "19.44");

    assert(tc.family() == CompilerFamily::Msvc);
    assert(tc.compiler_path() == "cl.exe");
    assert(tc.archiver_path() == "lib.exe");
    assert(tc.linker_path() == "link.exe");

    // 1. Test compile command (ReleaseFast, C++20)
    CompileParams cparams{
        .source_file = "src/main.cpp",
        .object_file = "build/main.obj",
        .cpp_version = CppVersion::Cpp20,
        .optimize = Optimization::ReleaseFast,
        .warnings_as_errors = true,
        .exceptions = true,
        .rtti = false,
        .include_dirs = {"include", "vendor/include"},
        .defines = {"WIN32", "NDEBUG"},
    };
    auto compile_cmd = tc.build_compile_command(cparams);
    std::string flat_compile = util::flatten_command(compile_cmd);

    assert(flat_compile.find("cl.exe") != std::string::npos);
    assert(flat_compile.find("/nologo") != std::string::npos);
    assert(flat_compile.find("/showIncludes") != std::string::npos);
    assert(flat_compile.find("/std:c++20") != std::string::npos);
    assert(flat_compile.find("/O2") != std::string::npos);
    assert(flat_compile.find("/MD") != std::string::npos);
    assert(flat_compile.find("/EHsc") != std::string::npos);
    assert(flat_compile.find("/GR-") != std::string::npos);
    assert(flat_compile.find("/W4") != std::string::npos);
    assert(flat_compile.find("/WX") != std::string::npos);
    assert(flat_compile.find("/DWIN32") != std::string::npos);
    assert(flat_compile.find("/DNDEBUG") != std::string::npos);
    assert(flat_compile.find("/c") != std::string::npos);

    // 2. Test archive command
    ArchiveParams aparams{
        .output_lib = "lib/engine.lib",
        .object_files = {"build/engine.obj", "build/math.obj"},
    };
    auto archive_cmd = tc.build_archive_command(aparams);
    std::string flat_archive = util::flatten_command(archive_cmd);
    assert(flat_archive.find("lib.exe /nologo /OUT:") != std::string::npos);

    // 3. Test link command for DLL with import library
    LinkParams dll_params{
        .output_binary = "bin/plugin.dll",
        .target_type = TargetType::SharedLibrary,
        .subsystem = WindowsSubsystem::Console,
        .object_files = {"build/plugin.obj"},
        .library_dirs = {"lib"},
        .libraries = {"engine", "user32.lib"},
        .import_library = "bin/plugin.lib",
    };
    auto link_cmd = tc.build_link_command(dll_params);
    std::string flat_link = util::flatten_command(link_cmd);

    assert(flat_link.find("link.exe /nologo /OUT:") != std::string::npos);
    assert(flat_link.find("/DLL") != std::string::npos);
    assert(flat_link.find("/IMPLIB:") != std::string::npos);
    assert(flat_link.find("/SUBSYSTEM:CONSOLE") != std::string::npos);
    assert(flat_link.find("/LIBPATH:") != std::string::npos);
    assert(flat_link.find("engine.lib") != std::string::npos);
    assert(flat_link.find("user32.lib") != std::string::npos);

    std::cout << "[PASS] MSVC command construction tests passed!\n";
}

void test_msvc_environment_detection() {
    using namespace cxxgen;
#if defined(_WIN32)
    auto start = std::chrono::high_resolution_clock::now();
    auto env = find_msvc_environment(true);
    auto duration_first =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);

    std::cout << "[INFO] First MSVC discovery completed in " << duration_first.count() << " ms.\n";

    if (env.found) {
        std::cout << "[INFO] Detected Visual Studio: " << env.installation_path << "\n";
        std::cout << "[INFO] MSVC Version: " << env.version << "\n";
        std::cout << "[INFO] cl.exe: " << env.cl_path << "\n";
        assert(std::filesystem::exists(env.cl_path));
        assert(std::filesystem::exists(env.link_path));
        assert(std::filesystem::exists(env.lib_path));

        // Test repeat cached lookup speed
        auto cache_start = std::chrono::high_resolution_clock::now();
        auto cached_env = find_msvc_environment(false);
        auto duration_cache = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - cache_start);

        std::cout << "[INFO] Cached MSVC discovery completed in " << duration_cache.count() << " microseconds.\n";
        assert(cached_env.found);
        // Acceptance criteria: Cached environment loads rapidly on repeat builds (< 50ms in unoptimized test)
        assert(duration_cache.count() < 50000);
    } else {
        std::cout << "[WARN] MSVC not detected on this machine; skipping path existence checks.\n";
    }
#else
    std::cout << "[INFO] Non-Windows OS; MSVC environment discovery not applicable.\n";
#endif
    std::cout << "[PASS] MSVC environment discovery tests passed!\n";
}

int main() {
    std::cout << "Running Story 2.2 MSVC Toolchain Test Suite...\n";
    test_msvc_command_construction();
    test_msvc_environment_detection();
    std::cout << "All Story 2.2 Tests Passed Successfully!\n";
    return 0;
}
