#include "cxxgen.hpp"

#include <cassert>
#include <iostream>

class MockToolchain : public cxxgen::IToolchain {
public:
    [[nodiscard]] cxxgen::CompilerFamily family() const noexcept override { return cxxgen::CompilerFamily::Clang; }
    [[nodiscard]] std::string name() const override { return "MockClang"; }
    [[nodiscard]] std::string compiler_path() const override { return "clang++"; }
    [[nodiscard]] std::string archiver_path() const override { return "llvm-ar"; }
    [[nodiscard]] std::string linker_path() const override { return "clang++"; }

    [[nodiscard]] std::vector<std::string> build_compile_command(const cxxgen::CompileParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(compiler_path());
        cmd.push_back("-std=" + std::string(cxxgen::to_string_view(params.cpp_version)));
        if (params.optimize == cxxgen::Optimization::ReleaseFast) {
            cmd.push_back("-O3");
        }
        for (const auto &inc : params.include_dirs) {
            cmd.push_back("-I" + inc);
        }
        for (const auto &def : params.defines) {
            cmd.push_back("-D" + def);
        }
        cmd.push_back("-c");
        cmd.push_back(params.source_file);
        cmd.push_back("-o");
        cmd.push_back(params.object_file);
        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_archive_command(const cxxgen::ArchiveParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(archiver_path());
        cmd.push_back("rcs");
        cmd.push_back(params.output_lib);
        for (const auto &obj : params.object_files) {
            cmd.push_back(obj);
        }
        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_link_command(const cxxgen::LinkParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(linker_path());
        for (const auto &obj : params.object_files) {
            cmd.push_back(obj);
        }
        cmd.push_back("-o");
        cmd.push_back(params.output_binary);
        for (const auto &lib : params.libraries) {
            cmd.push_back("-l" + lib);
        }
        return cmd;
    }

    [[nodiscard]] std::vector<std::string> get_depfile_flags(std::string_view depfile) const override {
        return {"-MMD", "-MF", std::string(depfile)};
    }
};

void test_path_normalization() {
    using namespace cxxgen::util;

    assert(normalize_path("") == "");
    assert(normalize_path("foo/bar/baz.cpp", false) == "foo/bar/baz.cpp");
    assert(normalize_path("foo\\bar\\baz.cpp", false) == "foo/bar/baz.cpp");
    assert(normalize_path("foo//bar///baz.cpp", false) == "foo/bar/baz.cpp");

#if defined(_WIN32)
    assert(normalize_path("foo/bar/baz.cpp", true) == "foo\\bar\\baz.cpp");
    assert(normalize_path("foo\\bar\\baz.cpp", true) == "foo\\bar\\baz.cpp");
#else
    assert(normalize_path("foo/bar/baz.cpp", true) == "foo/bar/baz.cpp");
    assert(normalize_path("foo\\bar\\baz.cpp", true) == "foo/bar/baz.cpp");
#endif

    std::cout << "[PASS] Path normalization tests passed!\n";
}

void test_argument_quoting_and_flattening() {
    using namespace cxxgen::util;

    assert(quote_argument("main.cpp") == "main.cpp");
    assert(quote_argument("") == "\"\"");
    assert(quote_argument("hello world") == "\"hello world\"");
    assert(quote_argument("with\"quote") == "\"with\\\"quote\"");

    std::vector<std::string> cmd = {"clang++", "-O3", "C:\\My Projects\\main.cpp", "-o", "main.o"};
    std::string flat = flatten_command(cmd);
    assert(flat == "clang++ -O3 \"C:\\My Projects\\main.cpp\" -o main.o");

    std::cout << "[PASS] Argument quoting and flattening tests passed!\n";
}

void test_mock_toolchain_interface() {
    using namespace cxxgen;
    MockToolchain tc;

    assert(tc.family() == CompilerFamily::Clang);
    assert(tc.name() == "MockClang");

    CompileParams cparams{
        .source_file = "src/main.cpp",
        .object_file = "build/main.o",
        .cpp_version = CppVersion::Cpp20,
        .optimize = Optimization::ReleaseFast,
        .include_dirs = {"include", "vendor/include"},
        .defines = {"DEBUG=1", "APP_NAME=\"Demo\""},
    };
    auto compile_cmd = tc.build_compile_command(cparams);
    assert(compile_cmd.size() == 11);
    assert(compile_cmd[0] == "clang++");
    assert(compile_cmd[1] == "-std=c++20");
    assert(compile_cmd[2] == "-O3");
    assert(compile_cmd[3] == "-Iinclude");
    assert(compile_cmd[4] == "-Ivendor/include");
    assert(compile_cmd[5] == "-DDEBUG=1");
    assert(compile_cmd[6] == "-DAPP_NAME=\"Demo\"");
    assert(compile_cmd[7] == "-c");
    assert(compile_cmd[8] == "src/main.cpp");
    assert(compile_cmd[9] == "-o");
    assert(compile_cmd[10] == "build/main.o");

    ArchiveParams aparams{
        .output_lib = "libfoo.a",
        .object_files = {"a.o", "b.o"},
    };
    auto archive_cmd = tc.build_archive_command(aparams);
    assert(archive_cmd.size() == 5);
    assert(archive_cmd[0] == "llvm-ar");
    assert(archive_cmd[1] == "rcs");
    assert(archive_cmd[2] == "libfoo.a");

    LinkParams lparams{
        .output_binary = "bin/app.exe",
        .object_files = {"main.o"},
        .libraries = {"user32"},
    };
    auto link_cmd = tc.build_link_command(lparams);
    assert(link_cmd.size() == 5);
    assert(link_cmd[0] == "clang++");
    assert(link_cmd[1] == "main.o");
    assert(link_cmd[2] == "-o");
    assert(link_cmd[3] == "bin/app.exe");
    assert(link_cmd[4] == "-luser32");

    auto dep_flags = tc.get_depfile_flags("main.d");
    assert(dep_flags.size() == 3);
    assert(dep_flags[0] == "-MMD");
    assert(dep_flags[1] == "-MF");
    assert(dep_flags[2] == "main.d");

    std::cout << "[PASS] Mock toolchain interface tests passed!\n";
}

int main() {
    std::cout << "Running Story 2.1 Toolchain Interface Test Suite...\n";
    test_path_normalization();
    test_argument_quoting_and_flattening();
    test_mock_toolchain_interface();
    std::cout << "All Story 2.1 Tests Passed Successfully!\n";
    return 0;
}
