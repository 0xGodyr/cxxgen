#include "cxxgen.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

void test_enums_and_strings() {
    using namespace cxxgen;

    // CppVersion conversions
    assert(to_string_view(CppVersion::Cpp20) == "c++20");
    assert(to_string(CppVersion::Cpp23) == "c++23");
    assert(parse_cpp_version("c++20") == CppVersion::Cpp20);
    assert(parse_cpp_version("cpp23") == CppVersion::Cpp23);
    assert(parse_cpp_version("c++2b") == CppVersion::Cpp23);
    assert(!parse_cpp_version("invalid").has_value());

    // Optimization conversions
    assert(to_string_view(Optimization::ReleaseFast) == "ReleaseFast");
    assert(to_string(Optimization::Debug) == "Debug");
    assert(parse_optimization("releasefast") == Optimization::ReleaseFast);
    assert(parse_optimization("release") == Optimization::ReleaseFast);
    assert(parse_optimization("debug") == Optimization::Debug);
    assert(parse_optimization("small") == Optimization::ReleaseSmall);
    assert(!parse_optimization("unknown").has_value());

    // TargetType conversions
    assert(to_string_view(TargetType::Executable) == "Executable");
    assert(to_string_view(TargetType::SharedLibrary) == "SharedLibrary");
    assert(to_string_view(TargetType::StaticLibrary) == "StaticLibrary");
    assert(to_string_view(TargetType::Plugin) == "Plugin");
    assert(parse_target_type("exe") == TargetType::Executable);
    assert(parse_target_type("dll") == TargetType::SharedLibrary);
    assert(parse_target_type("plugin") == TargetType::Plugin);
    assert(parse_target_type("module") == TargetType::Plugin);
    assert(parse_target_type("interface") == TargetType::Interface);

    // CompilerFamily conversions
    assert(to_string_view(CompilerFamily::Msvc) == "MSVC");
    assert(to_string_view(CompilerFamily::Clang) == "Clang");
    assert(to_string_view(CompilerFamily::Gcc) == "GCC");
    assert(parse_compiler_family("cl") == CompilerFamily::Msvc);
    assert(parse_compiler_family("clang++") == CompilerFamily::Clang);
    assert(parse_compiler_family("g++") == CompilerFamily::Gcc);

    // Stream operators
    std::ostringstream ss;
    ss << CppVersion::Cpp20 << " " << Optimization::ReleaseFast << " " << TargetType::Plugin;
    assert(ss.str() == "c++20 ReleaseFast Plugin");

    std::cout << "[PASS] Enum and string conversion tests passed!\n";
}

void test_designated_initializers() {
    using namespace cxxgen;

    TargetConfig config{
        .name = "sandbox_engine",
        .type = TargetType::SharedLibrary,
        .sources = {"src/engine.cpp", "src/renderer.cpp"},
        .headers = {"include/engine.hpp"},
        .include_dirs = {"include", "vendor/include"},
        .defines = {"ENGINE_DLL", "BUILD_SHARED=1"},
        .compile_options = {"-Wall"},
        .link_options = {},
        .link_libraries = {"user32"},
        .cpp_version = CppVersion::Cpp20,
        .c_version = std::nullopt,
        .optimize = Optimization::ReleaseFast,
        .warnings_as_errors = true,
        .exceptions = true,
        .rtti = false,
        .subsystem = WindowsSubsystem::Console,
        .output_dir = "bin",
        .output_name = "sandbox_engine.dll",
        .pch_header = "include/pch.hpp",
        .pch_source = "src/pch.cpp",
    };

    assert(config.name == "sandbox_engine");
    assert(config.type == TargetType::SharedLibrary);
    assert(config.sources.size() == 2);
    assert(config.headers.size() == 1);
    assert(config.include_dirs.size() == 2);
    assert(config.defines.size() == 2);
    assert(config.cpp_version == CppVersion::Cpp20);
    assert(config.optimize == Optimization::ReleaseFast);
    assert(config.warnings_as_errors == true);
    assert(config.rtti == false);
    assert(config.output_name == "sandbox_engine.dll");
    assert(config.pch_header == "include/pch.hpp");

    std::cout << "[PASS] Designated initializer tests passed!\n";
}

int main() {
    std::cout << "Running Story 1.1 Core Types Test Suite...\n";
    test_enums_and_strings();
    test_designated_initializers();
    std::cout << "All Story 1.1 Tests Passed Successfully!\n";
    return 0;
}
