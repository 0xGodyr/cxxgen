#include "cxxgen.hpp"

#include <cassert>
#include <iostream>

void test_gcc_command_construction() {
    using namespace cxxgen;
    GccToolchain tc("g++", "ar", "g++", "15.2.0");

    assert(tc.family() == CompilerFamily::Gcc);
    assert(tc.compiler_path() == "g++");
    assert(tc.archiver_path() == "ar");
    assert(tc.linker_path() == "g++");

    // 1. Test compile command (ReleaseFast, C++20, PIC, Depfile)
    CompileParams cparams{
        .source_file = "src/main.cpp",
        .object_file = "build/main.o",
        .cpp_version = CppVersion::Cpp20,
        .optimize = Optimization::ReleaseFast,
        .warnings_as_errors = true,
        .exceptions = false,
        .rtti = false,
        .position_independent_code = true,
        .include_dirs = {"include", "vendor/include"},
        .defines = {"DEBUG=1", "BUILD_SHARED=1"},
        .depfile = "build/main.d",
    };
    auto compile_cmd = tc.build_compile_command(cparams);
    std::string flat_compile = util::flatten_command(compile_cmd, false);

    assert(flat_compile.find("g++") != std::string::npos);
    assert(flat_compile.find("-std=c++20") != std::string::npos);
    assert(flat_compile.find("-O3") != std::string::npos);
    assert(flat_compile.find("-DNDEBUG") != std::string::npos);
    assert(flat_compile.find("-fPIC") != std::string::npos);
    assert(flat_compile.find("-fno-exceptions") != std::string::npos);
    assert(flat_compile.find("-fno-rtti") != std::string::npos);
    assert(flat_compile.find("-Wall") != std::string::npos);
    assert(flat_compile.find("-Wextra") != std::string::npos);
    assert(flat_compile.find("-Werror") != std::string::npos);
    assert(flat_compile.find("-Iinclude") != std::string::npos);
    assert(flat_compile.find("-DDEBUG=1") != std::string::npos);
    assert(flat_compile.find("-MMD -MF build/main.d") != std::string::npos);
    assert(flat_compile.find("-c src/main.cpp -o build/main.o") != std::string::npos);

    // 2. Test archive command
    ArchiveParams aparams{
        .output_lib = "lib/libengine.a",
        .object_files = {"build/a.o", "build/b.o"},
    };
    auto archive_cmd = tc.build_archive_command(aparams);
    std::string flat_archive = util::flatten_command(archive_cmd, false);
    assert(flat_archive.find("ar rcs lib/libengine.a build/a.o build/b.o") != std::string::npos);

    // 3. Test link command (Shared library)
    LinkParams link_params{
        .output_binary = "bin/libplugin.so",
        .target_type = TargetType::SharedLibrary,
        .object_files = {"build/plugin.o"},
        .library_dirs = {"lib"},
        .libraries = {"engine", "pthread"},
    };
    auto link_cmd = tc.build_link_command(link_params);
    std::string flat_link = util::flatten_command(link_cmd, false);
    assert(flat_link.find("g++ -shared -o bin/libplugin.so build/plugin.o -Llib -lengine -lpthread") !=
           std::string::npos);

    std::cout << "[PASS] GCC command construction tests passed!\n";
}

void test_gcc_detection() {
    using namespace cxxgen;
    auto gcc_opt = find_gcc_toolchain();
    assert(gcc_opt.has_value());

    std::cout << "[INFO] Detected GCC: " << gcc_opt->compiler_path() << "\n";
    std::cout << "[INFO] GCC Version: " << gcc_opt->name() << "\n";
    assert(std::filesystem::exists(gcc_opt->compiler_path()));

    std::cout << "[PASS] GCC auto-detection tests passed!\n";
}

int main() {
    std::cout << "Running Story 2.4 GCC Toolchain Test Suite...\n";
    test_gcc_command_construction();
    test_gcc_detection();
    std::cout << "All Story 2.4 Tests Passed Successfully!\n";
    return 0;
}
