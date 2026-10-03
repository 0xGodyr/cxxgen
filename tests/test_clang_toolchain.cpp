#include "cxxgen.hpp"

#include <cassert>
#include <iostream>

void test_clang_command_construction() {
    using namespace cxxgen;
    ClangToolchain tc("clang++", "llvm-ar", "clang++", "22.1");

    assert(tc.family() == CompilerFamily::Clang);
    assert(tc.compiler_path() == "clang++");
    assert(tc.archiver_path() == "llvm-ar");
    assert(tc.linker_path() == "clang++");

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

    assert(flat_compile.find("clang++") != std::string::npos);
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
    assert(flat_archive.find("llvm-ar rcs lib/libengine.a build/a.o build/b.o") != std::string::npos);

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
    assert(flat_link.find("clang++ -shared -o bin/libplugin.so build/plugin.o -Llib -lengine -lpthread") !=
           std::string::npos);

    std::cout << "[PASS] Clang command construction tests passed!\n";
}

void test_clang_detection() {
    using namespace cxxgen;
    auto clang_opt = find_clang_toolchain();
    assert(clang_opt.has_value());

    std::cout << "[INFO] Detected Clang: " << clang_opt->compiler_path() << "\n";
    std::cout << "[INFO] Clang Version: " << clang_opt->name() << "\n";
    assert(std::filesystem::exists(clang_opt->compiler_path()));

    std::cout << "[PASS] Clang auto-detection tests passed!\n";
}

int main() {
    std::cout << "Running Story 2.3 Clang Toolchain Test Suite...\n";
    test_clang_command_construction();
    test_clang_detection();
    std::cout << "All Story 2.3 Tests Passed Successfully!\n";
    return 0;
}
