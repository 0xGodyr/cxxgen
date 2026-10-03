#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void test_target_profile_parsing() {
    std::cout << "Testing TargetProfile parsing and architecture detection...\n";

    // 1. WebAssembly 32-bit
    auto wasm32_prof = cxxgen::TargetProfile::parse("wasm32");
    assert(wasm32_prof.is_wasm());
    assert(wasm32_prof.is_wasm32());
    assert(!wasm32_prof.is_wasm64());
    assert(wasm32_prof.arch == "wasm32");
    auto wasm32_defs = wasm32_prof.get_arch_defines();
    assert(std::find(wasm32_defs.begin(), wasm32_defs.end(), "__wasm__=1") != wasm32_defs.end());
    assert(std::find(wasm32_defs.begin(), wasm32_defs.end(), "__wasm32__=1") != wasm32_defs.end());

    // 2. WebAssembly 64-bit
    auto wasm64_prof = cxxgen::TargetProfile::parse("wasm64-unknown-emscripten");
    assert(wasm64_prof.is_wasm());
    assert(wasm64_prof.is_wasm64());
    assert(wasm64_prof.arch == "wasm64");
    assert(wasm64_prof.environment == "emscripten");
    auto wasm64_defs = wasm64_prof.get_arch_defines();
    assert(std::find(wasm64_defs.begin(), wasm64_defs.end(), "__wasm__=1") != wasm64_defs.end());
    assert(std::find(wasm64_defs.begin(), wasm64_defs.end(), "__wasm64__=1") != wasm64_defs.end());

    // 3. ARM64 / AArch64 Linux
    auto aarch64_prof = cxxgen::TargetProfile::parse("aarch64-linux-gnu");
    assert(aarch64_prof.is_aarch64());
    assert(!aarch64_prof.is_wasm());
    assert(aarch64_prof.is_linux());
    assert(aarch64_prof.arch == "aarch64");
    assert(aarch64_prof.os == "linux");
    auto aarch64_defs = aarch64_prof.get_arch_defines();
    assert(std::find(aarch64_defs.begin(), aarch64_defs.end(), "__aarch64__=1") != aarch64_defs.end());

    // 4. ARM 32-bit Bare-metal Embedded
    auto arm_prof = cxxgen::TargetProfile::parse("arm-none-eabi");
    assert(arm_prof.is_arm());
    assert(!arm_prof.is_aarch64());
    assert(arm_prof.arch == "arm");
    assert(arm_prof.os == "none");
    assert(arm_prof.environment == "eabi");
    auto arm_defs = arm_prof.get_arch_defines();
    assert(std::find(arm_defs.begin(), arm_defs.end(), "__arm__=1") != arm_defs.end());

    // 5. RISC-V 64-bit
    auto riscv64_prof = cxxgen::TargetProfile::parse("riscv64-unknown-elf");
    assert(riscv64_prof.is_riscv());
    assert(riscv64_prof.arch == "riscv64");
    auto riscv64_defs = riscv64_prof.get_arch_defines();
    assert(std::find(riscv64_defs.begin(), riscv64_defs.end(), "__riscv=1") != riscv64_defs.end());
    assert(std::find(riscv64_defs.begin(), riscv64_defs.end(), "__riscv_xlen=64") != riscv64_defs.end());

    // 6. RISC-V 32-bit
    auto riscv32_prof = cxxgen::TargetProfile::parse("riscv32-unknown-elf");
    assert(riscv32_prof.is_riscv());
    assert(riscv32_prof.arch == "riscv32");
    auto riscv32_defs = riscv32_prof.get_arch_defines();
    assert(std::find(riscv32_defs.begin(), riscv32_defs.end(), "__riscv=1") != riscv32_defs.end());
    assert(std::find(riscv32_defs.begin(), riscv32_defs.end(), "__riscv_xlen=32") != riscv32_defs.end());

    // 7. x86_64 Windows MSVC
    auto x86_win_prof = cxxgen::TargetProfile::parse("x86_64-pc-windows-msvc");
    assert(x86_win_prof.is_x86_64());
    assert(x86_win_prof.is_windows());
    assert(x86_win_prof.arch == "x86_64");
    assert(x86_win_prof.os == "windows");
    assert(x86_win_prof.environment == "msvc");
    auto x86_defs = x86_win_prof.get_arch_defines();
    assert(std::find(x86_defs.begin(), x86_defs.end(), "__x86_64__=1") != x86_defs.end());

    // 8. Apple Darwin / macOS
    auto darwin_prof = cxxgen::TargetProfile::parse("aarch64-apple-darwin");
    assert(darwin_prof.is_aarch64());
    assert(darwin_prof.is_darwin());
    assert(darwin_prof.vendor == "apple");
    assert(darwin_prof.os == "darwin");

    std::cout << "[PASS] TargetProfile parsing and architecture detection verified.\n";
}

void test_emscripten_toolchain_driver() {
    std::cout << "Testing EmscriptenToolchain command generation & flag mapping...\n";

    cxxgen::EmscriptenToolchain em("em++", "emar", "em++", "emscripten-mock-3.1.50");
    assert(em.family() == cxxgen::CompilerFamily::Emscripten);
    assert(em.name().find("Emscripten") != std::string::npos);
    assert(em.compiler_path() == "em++");

    // 1. Compile command generation
    cxxgen::CompileParams cparams;
    cparams.source_file = "src/main.cpp";
    cparams.object_file = "obj/main.o";
    cparams.cpp_version = cxxgen::CppVersion::Cpp20;
    cparams.optimize = cxxgen::Optimization::ReleaseSmall;
    cparams.warnings_as_errors = true;
    cparams.exceptions = true;
    cparams.rtti = false;
    cparams.include_dirs = {"include", "vendor/wasm"};
    cparams.defines = {"FEATURE_WEBGL=1"};
    cparams.extra_flags = {"-sUSE_WEBGL2=1", "-sALLOW_MEMORY_GROWTH=1"};
    cparams.depfile = "obj/main.o.d";

    auto compile_cmd = em.build_compile_command(cparams);
    std::string flat_compile = cxxgen::util::flatten_command(compile_cmd);

    assert(flat_compile.find("em++") != std::string::npos);
    assert(flat_compile.find("-std=c++20") != std::string::npos);
    assert(flat_compile.find("-Oz") != std::string::npos);
    assert(flat_compile.find("-Werror") != std::string::npos);
    assert(flat_compile.find("-fexceptions") != std::string::npos);
    assert(flat_compile.find("-fno-rtti") != std::string::npos);
    assert(flat_compile.find("-D__EMSCRIPTEN__=1") != std::string::npos);
    assert(flat_compile.find("-DEMSCRIPTEN=1") != std::string::npos);
    assert(flat_compile.find("-D__wasm__=1") != std::string::npos);
    assert(flat_compile.find("-D__wasm32__=1") != std::string::npos);
    assert(flat_compile.find("-DFEATURE_WEBGL=1") != std::string::npos);
    assert(flat_compile.find("-Iinclude") != std::string::npos);
    assert(flat_compile.find("-Ivendor/wasm") != std::string::npos);
    assert(flat_compile.find("-sUSE_WEBGL2=1") != std::string::npos);
    assert(flat_compile.find("-sALLOW_MEMORY_GROWTH=1") != std::string::npos);
    assert(flat_compile.find("-MMD -MF obj/main.o.d") != std::string::npos ||
           flat_compile.find("-MD -MF obj/main.o.d") != std::string::npos);
    assert(flat_compile.find("-c src/main.cpp -o obj/main.o") != std::string::npos);

    // 2. Link command generation (.html output)
    cxxgen::LinkParams lparams_html;
    lparams_html.output_binary = "bin/game.html";
    lparams_html.object_files = {"obj/main.o", "obj/engine.o"};
    lparams_html.library_dirs = {"lib/wasm"};
    lparams_html.libraries = {"gl"};
    lparams_html.extra_flags = {"-sUSE_WEBGL2=1", "-sALLOW_MEMORY_GROWTH=1", "-sEXPORTED_FUNCTIONS=['_main','_step']",
                                "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']"};

    auto link_html_cmd = em.build_link_command(lparams_html);
    std::string flat_link_html = cxxgen::util::flatten_command(link_html_cmd);

    assert(flat_link_html.find("em++") != std::string::npos);
    assert(flat_link_html.find("obj/main.o") != std::string::npos);
    assert(flat_link_html.find("obj/engine.o") != std::string::npos);
    assert(flat_link_html.find("-o bin/game.html") != std::string::npos);
    assert(flat_link_html.find("-Llib/wasm") != std::string::npos);
    assert(flat_link_html.find("-lgl") != std::string::npos);
    assert(flat_link_html.find("-sUSE_WEBGL2=1") != std::string::npos);
    assert(flat_link_html.find("-sALLOW_MEMORY_GROWTH=1") != std::string::npos);
    assert(flat_link_html.find("-sEXPORTED_FUNCTIONS=['_main','_step']") != std::string::npos);
    assert(flat_link_html.find("-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']") != std::string::npos);

    // 3. Link command generation (standalone .wasm output)
    cxxgen::LinkParams lparams_wasm;
    lparams_wasm.output_binary = "bin/module.wasm";
    lparams_wasm.object_files = {"obj/math.o"};
    lparams_wasm.extra_flags = {"-sSTANDALONE_WASM=1", "--no-entry"};

    auto link_wasm_cmd = em.build_link_command(lparams_wasm);
    std::string flat_link_wasm = cxxgen::util::flatten_command(link_wasm_cmd);

    assert(flat_link_wasm.find("em++") != std::string::npos);
    assert(flat_link_wasm.find("-o bin/module.wasm") != std::string::npos);
    assert(flat_link_wasm.find("-sSTANDALONE_WASM=1") != std::string::npos);
    assert(flat_link_wasm.find("--no-entry") != std::string::npos);

    // 4. Archive command generation
    cxxgen::ArchiveParams aparams;
    aparams.output_lib = "lib/libengine.a";
    aparams.object_files = {"obj/main.o", "obj/engine.o"};

    auto archive_cmd = em.build_archive_command(aparams);
    std::string flat_archive = cxxgen::util::flatten_command(archive_cmd);
    assert(flat_archive.find("emar") != std::string::npos || flat_archive.find("em++") != std::string::npos);
    assert(flat_archive.find("lib/libengine.a") != std::string::npos);
    assert(flat_archive.find("obj/main.o") != std::string::npos);

    std::cout << "[PASS] EmscriptenToolchain command generation & flag mapping verified.\n";
}

void test_cross_compilation_flags() {
    std::cout << "Testing Clang & GCC cross-compilation flag propagation...\n";

    // 1. Clang cross-compilation (--target and --sysroot)
    cxxgen::ClangToolchain clang("clang++", "clang", "llvm-ar", "Clang 18");

    cxxgen::CompileParams clang_cparams;
    clang_cparams.source_file = "src/kernel.cpp";
    clang_cparams.object_file = "obj/kernel.o";
    clang_cparams.cpp_version = cxxgen::CppVersion::Cpp20;
    clang_cparams.target_triple = "aarch64-linux-gnu";
    clang_cparams.sysroot = "/opt/sysroots/aarch64";

    auto clang_cmd = clang.build_compile_command(clang_cparams);
    std::string flat_clang_compile = cxxgen::util::flatten_command(clang_cmd);

    assert(flat_clang_compile.find("--target=aarch64-linux-gnu") != std::string::npos);
    assert(flat_clang_compile.find("--sysroot=") != std::string::npos);
    assert(flat_clang_compile.find("/opt/sysroots/aarch64") != std::string::npos);
    assert(flat_clang_compile.find("-D__aarch64__=1") != std::string::npos);

    cxxgen::LinkParams clang_lparams;
    clang_lparams.output_binary = "bin/kernel";
    clang_lparams.object_files = {"obj/kernel.o"};
    clang_lparams.target_triple = "aarch64-linux-gnu";
    clang_lparams.sysroot = "/opt/sysroots/aarch64";

    auto clang_link_cmd = clang.build_link_command(clang_lparams);
    std::string flat_clang_link = cxxgen::util::flatten_command(clang_link_cmd);

    assert(flat_clang_link.find("--target=aarch64-linux-gnu") != std::string::npos);
    assert(flat_clang_link.find("--sysroot=") != std::string::npos);

    // 2. GCC cross-compilation (--sysroot and arch defines)
    cxxgen::GccToolchain gcc("aarch64-linux-gnu-g++", "aarch64-linux-gnu-gcc", "aarch64-linux-gnu-ar", "GCC aarch64");

    cxxgen::CompileParams gcc_cparams;
    gcc_cparams.source_file = "src/boot.cpp";
    gcc_cparams.object_file = "obj/boot.o";
    gcc_cparams.cpp_version = cxxgen::CppVersion::Cpp20;
    gcc_cparams.target_triple = "arm-none-eabi";
    gcc_cparams.sysroot = "/opt/arm-none-eabi";

    auto gcc_cmd = gcc.build_compile_command(gcc_cparams);
    std::string flat_gcc_compile = cxxgen::util::flatten_command(gcc_cmd);

    assert(flat_gcc_compile.find("--sysroot=") != std::string::npos);
    assert(flat_gcc_compile.find("/opt/arm-none-eabi") != std::string::npos);
    assert(flat_gcc_compile.find("-D__arm__=1") != std::string::npos);

    cxxgen::LinkParams gcc_lparams;
    gcc_lparams.output_binary = "bin/boot.elf";
    gcc_lparams.object_files = {"obj/boot.o"};
    gcc_lparams.target_triple = "arm-none-eabi";
    gcc_lparams.sysroot = "/opt/arm-none-eabi";

    auto gcc_link_cmd = gcc.build_link_command(gcc_lparams);
    std::string flat_gcc_link = cxxgen::util::flatten_command(gcc_link_cmd);

    assert(flat_gcc_link.find("--sysroot=") != std::string::npos);

    std::cout << "[PASS] Clang & GCC cross-compilation flag propagation verified.\n";
}

void test_artifact_extensions() {
    std::cout << "Testing artifact path and extension resolution for wasm and cross targets...\n";

    cxxgen::Builder b;
    cxxgen::ClangToolchain host_clang("clang++", "clang", "llvm-ar", "Clang Host");
    cxxgen::EmscriptenToolchain em("em++", "emcc", "emar", "Emscripten");

    // 1. Emscripten target with default name -> .wasm
    auto &wasm_exe = b.add_executable({.name = "web_app"});
    fs::path p1 = cxxgen::CompilationPipeline::get_artifact_path(wasm_exe, em);
    assert(p1.extension() == ".wasm" || p1.extension() == ".html");

    // 2. Target explicitly named with .html -> .html
    auto &html_exe = b.add_executable({.name = "index.html"});
    fs::path p2 = cxxgen::CompilationPipeline::get_artifact_path(html_exe, em);
    assert(p2.filename() == "index.html");

    // 3. Target explicitly named with .js -> .js
    auto &js_exe = b.add_executable({.name = "worker.js"});
    fs::path p3 = cxxgen::CompilationPipeline::get_artifact_path(js_exe, em);
    assert(p3.filename() == "worker.js");

    // 4. Target with target_triple = wasm32 on Clang -> .wasm
    auto &wasm32_target = b.add_executable({.name = "algo"});
    wasm32_target.set_target_triple("wasm32");
    fs::path p4 = cxxgen::CompilationPipeline::get_artifact_path(wasm32_target, host_clang);
    assert(p4.extension() == ".wasm");

    // 5. Cross-compilation to Linux from Windows host -> no .exe extension
    auto &linux_exe = b.add_executable({.name = "server"});
    linux_exe.set_target_triple("x86_64-linux-gnu");
    fs::path p5 = cxxgen::CompilationPipeline::get_artifact_path(linux_exe, host_clang);
    assert(p5.filename() == "server");
    assert(p5.extension() == "");

    // 6. Cross-compilation to Windows Mingw/MSVC -> .exe extension
    auto &win_exe = b.add_executable({.name = "tool"});
    win_exe.set_target_triple("x86_64-pc-windows-msvc");
    fs::path p6 = cxxgen::CompilationPipeline::get_artifact_path(win_exe, host_clang);
    assert(p6.filename() == "tool.exe");

    // 7. Shared library for Linux -> .so
    auto &linux_so = b.add_shared_library({.name = "plugin"});
    linux_so.set_target_triple("aarch64-linux-gnu");
    fs::path p7 = cxxgen::CompilationPipeline::get_artifact_path(linux_so, host_clang);
    assert(p7.filename() == "libplugin.so");

    // 8. Shared library for macOS -> .dylib
    auto &mac_dylib = b.add_shared_library({.name = "core"});
    mac_dylib.set_target_triple("aarch64-apple-darwin");
    fs::path p8 = cxxgen::CompilationPipeline::get_artifact_path(mac_dylib, host_clang);
    assert(p8.filename() == "libcore.dylib");

    // 9. Static library for wasm32 -> .a
    auto &wasm_lib = b.add_static_library({.name = "runtime"});
    wasm_lib.set_target_triple("wasm32");
    fs::path p9 = cxxgen::CompilationPipeline::get_artifact_path(wasm_lib, em);
    assert(p9.filename() == "libruntime.a");

    std::cout << "[PASS] Artifact path and extension resolution verified.\n";
}

void test_cli_cross_options() {
    std::cout << "Testing CLI parser with cross-compilation flags...\n";

    // 1. --target=<triple>
    {
        const char *argv[] = {"cxxgen", "build", "--target=aarch64-linux-gnu"};
        auto args = cxxgen::parse_cli_args(3, const_cast<char **>(argv));
        assert(args.command == "build");
        assert(args.target_triple == "aarch64-linux-gnu");
    }

    // 2. --target <triple>
    {
        const char *argv[] = {"cxxgen", "build", "--target", "wasm32"};
        auto args = cxxgen::parse_cli_args(4, const_cast<char **>(argv));
        assert(args.target_triple == "wasm32");
    }

    // 3. --sysroot=<path> and --triple=<triple>
    {
        const char *argv[] = {"cxxgen", "test", "--sysroot=/opt/sysroots/arm", "--triple=arm-none-eabi"};
        auto args = cxxgen::parse_cli_args(4, const_cast<char **>(argv));
        assert(args.command == "test");
        assert(args.sysroot == "/opt/sysroots/arm");
        assert(args.target_triple == "arm-none-eabi");
    }

    // 4. --compiler=emcc and --compiler=emscripten
    {
        const char *argv[] = {"cxxgen", "build", "--compiler=emcc"};
        auto args = cxxgen::parse_cli_args(3, const_cast<char **>(argv));
        assert(args.compiler.has_value());
        assert(*args.compiler == cxxgen::CompilerFamily::Emscripten);
    }
    {
        const char *argv[] = {"cxxgen", "build", "--compiler", "emscripten"};
        auto args = cxxgen::parse_cli_args(4, const_cast<char **>(argv));
        assert(args.compiler.has_value());
        assert(*args.compiler == cxxgen::CompilerFamily::Emscripten);
    }

    // 5. Graph command target filter vs target triple disambiguation
    {
        const char *argv[] = {"cxxgen", "graph", "--target=my_service"};
        auto args = cxxgen::parse_cli_args(3, const_cast<char **>(argv));
        assert(args.command == "graph");
        assert(args.graph_target == "my_service");
    }

    std::cout << "[PASS] CLI parser cross-compilation flags verified.\n";
}

void test_builder_target_cross_api() {
    std::cout << "Testing Builder & Target cross-compilation APIs...\n";

    cxxgen::Builder b;
    assert(b.target_triple().empty());
    assert(b.sysroot().empty());

    b.set_target_triple("wasm32-unknown-emscripten");
    b.set_sysroot("/opt/emsdk/upstream/emscripten/system");
    assert(b.target_triple() == "wasm32-unknown-emscripten");
    assert(b.sysroot() == "/opt/emsdk/upstream/emscripten/system");
    assert(b.standard_target_options().triple == "wasm32-unknown-emscripten");
    assert(b.standard_target_options().sysroot == "/opt/emsdk/upstream/emscripten/system");

    auto &t = b.add_executable({.name = "worker"});
    assert(t.target_triple().empty());
    t.set_target_triple("wasm32");
    t.set_sysroot("/opt/wasm32-sysroot");
    assert(t.target_triple() == "wasm32");
    assert(t.sysroot() == "/opt/wasm32-sysroot");

    std::cout << "[PASS] Builder & Target cross-compilation APIs verified.\n";
}

void test_live_wasm_compilation() {
    std::cout << "Testing live WebAssembly compilation and bytecode verification with Clang/wasm-ld...\n";

    auto clang = cxxgen::find_clang_toolchain();
    if (!clang) {
        std::cout << "[SKIP] LLVM Clang not found for live Wasm test.\n";
        return;
    }

    fs::path temp_dir = fs::current_path() / ".cxxgen_test_wasm_live";
    std::error_code ec;
    fs::remove_all(temp_dir, ec);
    fs::create_directories(temp_dir, ec);

    fs::path src_file = temp_dir / "math.cpp";
    {
        std::ofstream f(src_file);
        f << "extern \"C\" {\n"
          << "    int add_numbers(int a, int b) {\n"
          << "        return a + b;\n"
          << "    }\n"
          << "    int multiply_numbers(int a, int b) {\n"
          << "        return a * b;\n"
          << "    }\n"
          << "}\n";
    }

    fs::path obj_file = temp_dir / "math.o";
    fs::path wasm_file = temp_dir / "math.wasm";

    // 1. Compile C++ to WebAssembly object file
    cxxgen::CompileParams cp;
    cp.source_file = src_file.string();
    cp.object_file = obj_file.string();
    cp.cpp_version = cxxgen::CppVersion::Cpp20;
    cp.optimize = cxxgen::Optimization::ReleaseFast;
    cp.target_triple = "wasm32";
    cp.extra_flags = {"-nostdlib"};

    auto compile_cmd = clang->build_compile_command(cp);
    cxxgen::ProcessResult c_res = cxxgen::ProcessRunner::run(compile_cmd);
    if (!c_res.success()) {
        std::cout << "[SKIP] Host Clang does not support wasm32 target directly: " << c_res.combined_output() << "\n";
        fs::remove_all(temp_dir, ec);
        return;
    }
    assert(fs::exists(obj_file));

    // 2. Link object into standalone .wasm binary
    cxxgen::LinkParams lp;
    lp.output_binary = wasm_file.string();
    lp.object_files = {obj_file.string()};
    lp.target_triple = "wasm32";
    lp.extra_flags = {"-nostdlib", "-Wl,--no-entry", "-Wl,--export-all"};

    auto link_cmd = clang->build_link_command(lp);
    cxxgen::ProcessResult l_res = cxxgen::ProcessRunner::run(link_cmd);
    if (!l_res.success()) {
        std::cout << "[SKIP] Host wasm linker not available: " << l_res.combined_output() << "\n";
        fs::remove_all(temp_dir, ec);
        return;
    }
    assert(fs::exists(wasm_file));

    // 3. Verify WebAssembly bytecode magic bytes: '\0asm' (0x00 0x61 0x73 0x6d)
    std::ifstream wasm_in(wasm_file, std::ios::binary);
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(wasm_in)), std::istreambuf_iterator<char>());
    wasm_in.close();

    assert(bytes.size() >= 8);
    assert(bytes[0] == 0x00 && bytes[1] == 0x61 && bytes[2] == 0x73 && bytes[3] == 0x6D); // \0asm
    assert(bytes[4] == 0x01 && bytes[5] == 0x00 && bytes[6] == 0x00 && bytes[7] == 0x00); // version 1

    fs::remove_all(temp_dir, ec);
    std::cout << "[PASS] Live WebAssembly compilation and bytecode magic header verified (size: " << bytes.size()
              << " bytes).\n";
}

int main() {
    std::cout << "================================================================================\n";
    std::cout << "Starting Test Suite: Cross-Compilation & WebAssembly (Wasm)\n";
    std::cout << "================================================================================\n";

    test_target_profile_parsing();
    test_emscripten_toolchain_driver();
    test_cross_compilation_flags();
    test_artifact_extensions();
    test_cli_cross_options();
    test_builder_target_cross_api();
    test_live_wasm_compilation();

    std::cout << "================================================================================\n";
    std::cout << "All Cross-Compilation & WebAssembly (Wasm) tests passed successfully!\n";
    std::cout << "================================================================================\n";
    return 0;
}
