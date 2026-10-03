#include "cxxgen.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

void test_sanitizer_enum_and_operators() {
    using namespace cxxgen;

    assert(!Sanitizer::None);
    assert(!!Sanitizer::Address);
    assert(!!Sanitizer::Undefined);
    assert(!!Sanitizer::Leak);
    assert(!!Sanitizer::Thread);

    Sanitizer combo = Sanitizer::Address | Sanitizer::Undefined;
    assert((combo & Sanitizer::Address) == Sanitizer::Address);
    assert((combo & Sanitizer::Undefined) == Sanitizer::Undefined);
    assert((combo & Sanitizer::Leak) == Sanitizer::None);
    assert((combo & Sanitizer::Thread) == Sanitizer::None);

    Sanitizer combo2 = combo | Sanitizer::Leak;
    assert((combo2 & Sanitizer::Leak) == Sanitizer::Leak);

    Sanitizer inverted = ~Sanitizer::Address;
    assert((inverted & Sanitizer::Address) == Sanitizer::None);
    assert((inverted & Sanitizer::Undefined) == Sanitizer::Undefined);

    std::cout << "[PASS] test_sanitizer_enum_and_operators\n";
}

void test_parse_sanitizers() {
    using namespace cxxgen;

    assert(parse_sanitizers("none") == Sanitizer::None);
    assert(parse_sanitizers("") == Sanitizer::None);

    assert(parse_sanitizers("address") == Sanitizer::Address);
    assert(parse_sanitizers("asan") == Sanitizer::Address);
    assert(parse_sanitizers("undefined") == Sanitizer::Undefined);
    assert(parse_sanitizers("ubsan") == Sanitizer::Undefined);
    assert(parse_sanitizers("leak") == Sanitizer::Leak);
    assert(parse_sanitizers("lsan") == Sanitizer::Leak);
    assert(parse_sanitizers("thread") == Sanitizer::Thread);
    assert(parse_sanitizers("tsan") == Sanitizer::Thread);

    Sanitizer parsed_multi = parse_sanitizers("address, undefined, leak");
    assert((parsed_multi & Sanitizer::Address) == Sanitizer::Address);
    assert((parsed_multi & Sanitizer::Undefined) == Sanitizer::Undefined);
    assert((parsed_multi & Sanitizer::Leak) == Sanitizer::Leak);
    assert((parsed_multi & Sanitizer::Thread) == Sanitizer::None);

    Sanitizer parsed_all = parse_sanitizers("all");
    assert((parsed_all & Sanitizer::Address) == Sanitizer::Address);
    assert((parsed_all & Sanitizer::Undefined) == Sanitizer::Undefined);
    assert((parsed_all & Sanitizer::Leak) == Sanitizer::Leak);

    // Case-insensitive test
    Sanitizer parsed_ci = parse_sanitizers("AdDrEsS,UbSaN");
    assert((parsed_ci & Sanitizer::Address) == Sanitizer::Address);
    assert((parsed_ci & Sanitizer::Undefined) == Sanitizer::Undefined);

    std::cout << "[PASS] test_parse_sanitizers\n";
}

void test_target_sanitizer_methods() {
    using namespace cxxgen;

    Builder b;
    auto &t = b.add_executable({.name = "test_target", .sources = {"src/main.cpp"}});
    assert(t.sanitizers() == Sanitizer::None);

    t.sanitize(Sanitizer::Address);
    assert((t.sanitizers() & Sanitizer::Address) == Sanitizer::Address);

    t.sanitize("undefined,leak");
    assert((t.sanitizers() & Sanitizer::Address) == Sanitizer::Address);
    assert((t.sanitizers() & Sanitizer::Undefined) == Sanitizer::Undefined);
    assert((t.sanitizers() & Sanitizer::Leak) == Sanitizer::Leak);

    std::cout << "[PASS] test_target_sanitizer_methods\n";
}

void test_msvc_sanitizer_flags() {
    using namespace cxxgen;

    MsvcToolchain tc("cl.exe", "lib.exe", "link.exe", "19.38");

    CompileParams cparams{
        .source_file = "src/main.cpp",
        .object_file = "build/main.obj",
        .cpp_version = CppVersion::Cpp20,
        .optimize = Optimization::Debug,
        .sanitizers = Sanitizer::Address | Sanitizer::Undefined,
    };

    auto compile_cmd = tc.build_compile_command(cparams);
    std::string flat_compile = util::flatten_command(compile_cmd, false);

    // MSVC supports /fsanitize=address and requires /Zi
    assert(flat_compile.find("/fsanitize=address") != std::string::npos);
    assert(flat_compile.find("/Zi") != std::string::npos);
    // /RTC1 must NOT be present when /fsanitize=address is active
    assert(flat_compile.find("/RTC1") == std::string::npos);

    LinkParams lparams{
        .output_binary = "bin/main.exe",
        .target_type = TargetType::Executable,
        .object_files = {"build/main.obj"},
        .sanitizers = Sanitizer::Address,
    };
    auto link_cmd = tc.build_link_command(lparams);
    std::string flat_link = util::flatten_command(link_cmd, false);
    assert(flat_link.find("/DEBUG") != std::string::npos);

    std::cout << "[PASS] test_msvc_sanitizer_flags\n";
}

void test_clang_sanitizer_flags() {
    using namespace cxxgen;

    ClangToolchain tc("clang++", "llvm-ar", "clang++", "22.1");

    CompileParams cparams{
        .source_file = "src/main.cpp",
        .object_file = "build/main.o",
        .cpp_version = CppVersion::Cpp20,
        .sanitizers = Sanitizer::Address | Sanitizer::Undefined,
    };
    auto compile_cmd = tc.build_compile_command(cparams);
    std::string flat_compile = util::flatten_command(compile_cmd, false);
    assert(flat_compile.find("-fsanitize=address,undefined") != std::string::npos);
    assert(flat_compile.find("-fno-omit-frame-pointer") != std::string::npos);
    assert(flat_compile.find("-g") != std::string::npos);

    LinkParams lparams{
        .output_binary = "bin/main",
        .target_type = TargetType::Executable,
        .object_files = {"build/main.o"},
        .sanitizers = Sanitizer::Address | Sanitizer::Undefined,
    };
    auto link_cmd = tc.build_link_command(lparams);
    std::string flat_link = util::flatten_command(link_cmd, false);
    assert(flat_link.find("-fsanitize=address,undefined") != std::string::npos);

    std::cout << "[PASS] test_clang_sanitizer_flags\n";
}

void test_gcc_sanitizer_flags() {
    using namespace cxxgen;

    GccToolchain tc("g++", "ar", "g++", "13.2");

    CompileParams cparams{
        .source_file = "src/main.cpp",
        .object_file = "build/main.o",
        .cpp_version = CppVersion::Cpp20,
        .sanitizers = Sanitizer::Address | Sanitizer::Leak,
    };
    auto compile_cmd = tc.build_compile_command(cparams);
    std::string flat_compile = util::flatten_command(compile_cmd, false);
    assert(flat_compile.find("-fsanitize=address,leak") != std::string::npos);
    assert(flat_compile.find("-fno-omit-frame-pointer") != std::string::npos);
    assert(flat_compile.find("-g") != std::string::npos);

    LinkParams lparams{
        .output_binary = "bin/main",
        .target_type = TargetType::Executable,
        .object_files = {"build/main.o"},
        .sanitizers = Sanitizer::Address | Sanitizer::Leak,
    };
    auto link_cmd = tc.build_link_command(lparams);
    std::string flat_link = util::flatten_command(link_cmd, false);
    assert(flat_link.find("-fsanitize=address,leak") != std::string::npos);

    std::cout << "[PASS] test_gcc_sanitizer_flags\n";
}

void test_cli_sanitizer_parsing() {
    using namespace cxxgen;

    const char *argv1[] = {"cxxgen", "build", "--sanitize=address,undefined"};
    CliArgs args1 = parse_cli_args(3, const_cast<char **>(argv1));
    assert((args1.sanitizers & Sanitizer::Address) == Sanitizer::Address);
    assert((args1.sanitizers & Sanitizer::Undefined) == Sanitizer::Undefined);
    assert((args1.sanitizers & Sanitizer::Leak) == Sanitizer::None);

    const char *argv2[] = {"cxxgen", "test", "--sanitize", "leak,thread"};
    CliArgs args2 = parse_cli_args(4, const_cast<char **>(argv2));
    assert((args2.sanitizers & Sanitizer::Leak) == Sanitizer::Leak);
    assert((args2.sanitizers & Sanitizer::Thread) == Sanitizer::Thread);

    std::cout << "[PASS] test_cli_sanitizer_parsing\n";
}

int main() {
    std::cout << "=== Running Sanitizer Tests ===\n";
    test_sanitizer_enum_and_operators();
    test_parse_sanitizers();
    test_target_sanitizer_methods();
    test_msvc_sanitizer_flags();
    test_clang_sanitizer_flags();
    test_gcc_sanitizer_flags();
    test_cli_sanitizer_parsing();
    std::cout << "All Sanitizer tests passed successfully!\n";
    return 0;
}
