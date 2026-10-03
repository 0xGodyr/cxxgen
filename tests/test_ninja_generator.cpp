#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void test_ninja_generator_unit() {
    std::cout << "Testing NinjaGenerator unit output generation...\n";

    cxxgen::Builder b;
    b.set_project_name("NinjaUnitTest");

    // 1. Interface
    auto &iface = b.add_interface_library({
        .name = "iface_hdr",
        .include_dirs = {"include/iface"},
        .defines = {"IFACE_ACTIVE=1"},
    });

    // 2. Static Lib
    auto &static_lib = b.add_static_library({
        .name = "core_static",
        .sources = {"core/src/core.cpp"},
        .include_dirs = {"core/include"},
        .defines = {"CORE_EXPORTS=1"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    static_lib.link(iface);

    // 3. Executable
    auto &app = b.add_executable({
        .name = "ninja_app",
        .sources = {"app/src/main.cpp"},
        .include_dirs = {"app/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .warnings_as_errors = true,
    });
    app.link(static_lib);

    // 4. Custom command
    b.add_custom_command({
        .name = "gen_header",
        .description = "Generate config header",
        .command = {"python", "scripts/gen.py"},
        .inputs = {"scripts/config.in"},
        .outputs = {"app/include/config.h"},
    });

    cxxgen::NinjaGenerator gen(b);
    std::string content = gen.generate();

    // Check version and rules
    assert(content.find("ninja_required_version = 1.8") != std::string::npos);
    assert(content.find("rule cxx_compile") != std::string::npos);
    assert(content.find("rule static_lib") != std::string::npos);
    assert(content.find("rule link_exe") != std::string::npos);
    assert(content.find("rule custom_cmd") != std::string::npos);

    // Check custom command
    assert(content.find("build app/include/config.h: custom_cmd scripts/config.in") != std::string::npos);

    // Check target objects and linking
    assert(content.find("core/src/core.cpp") != std::string::npos);
    assert(content.find("app/src/main.cpp") != std::string::npos);
    assert(content.find("core_static") != std::string::npos);
    assert(content.find("ninja_app") != std::string::npos);
    assert(content.find("default ") != std::string::npos);

    std::cout << "[PASS] NinjaGenerator unit output generation verified!\n";
}

void test_ninja_end_to_end_build() {
    std::cout << "Testing NinjaGenerator end-to-end execution and incremental rebuild...\n";

    fs::path fixture_dir = "test_ninja_fixture";
    std::error_code ec;
    fs::remove_all(fixture_dir, ec);

    fs::create_directories(fixture_dir / "lib" / "include", ec);
    fs::create_directories(fixture_dir / "lib" / "src", ec);
    fs::create_directories(fixture_dir / "app" / "src", ec);

    // 1. Create source files
    fs::path math_h = fixture_dir / "lib" / "include" / "math_lib.h";
    fs::path math_cpp = fixture_dir / "lib" / "src" / "math_lib.cpp";
    fs::path main_cpp = fixture_dir / "app" / "src" / "main.cpp";

    {
        std::ofstream f(math_h);
        f << "#pragma once\nint compute_answer();\n";
    }
    {
        std::ofstream f(math_cpp);
        f << "#include \"math_lib.h\"\nint compute_answer() { return 42; }\n";
    }
    {
        std::ofstream f(main_cpp);
        f << "#include \"math_lib.h\"\n#include <iostream>\nint main() {\n"
          << "    std::cout << \"Answer: \" << compute_answer() << \"\\n\";\n"
          << "    return (compute_answer() == 42) ? 0 : 1;\n"
          << "}\n";
    }

    // 2. Configure Builder
    cxxgen::Builder b;
    b.set_project_name("E2E_NinjaProject");

    auto &math_lib = b.add_static_library({
        .name = "math_lib",
        .sources = {"lib/src/math_lib.cpp"},
        .include_dirs = {"lib/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    auto &app = b.add_executable({
        .name = "e2e_app",
        .sources = {"app/src/main.cpp"},
        .include_dirs = {"lib/include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    app.link(math_lib);

    // 3. Generate build.ninja
    cxxgen::NinjaGenerator gen(b);
    fs::path ninja_file = fixture_dir / "build.ninja";
    assert(gen.generate_file(ninja_file));
    assert(fs::exists(ninja_file));

    // 4. Run ninja build
    cxxgen::ProcessOptions ninja_bld_opts;
    ninja_bld_opts.command = {"ninja"};
    ninja_bld_opts.working_directory = fixture_dir;
    auto bld_res = cxxgen::ProcessRunner::run(ninja_bld_opts);
    if (!bld_res.success()) {
        std::cerr << "Ninja build output:\n" << bld_res.combined_output() << "\n";
    }
    assert(bld_res.success());

    // 5. Verify and run the built binary
#if defined(_WIN32)
    fs::path exe_path = fixture_dir / "bin" / "e2e_app.exe";
#else
    fs::path exe_path = fixture_dir / "bin" / "e2e_app";
#endif
    assert(fs::exists(exe_path));

    cxxgen::ProcessOptions run_opts;
    run_opts.command = {exe_path.string()};
    auto run_res = cxxgen::ProcessRunner::run(run_opts);
    assert(run_res.success());
    assert(run_res.exit_code == 0);
    assert(cxxgen::util::trim(run_res.stdout_output) == "Answer: 42");

    // 6. Test no-op build (up-to-date)
    auto noop_res = cxxgen::ProcessRunner::run(ninja_bld_opts);
    assert(noop_res.success());
    assert(noop_res.stdout_output.find("no work to do") != std::string::npos);

    // 7. Test incremental rebuild: update math_lib.cpp
    {
        std::ofstream f(math_cpp);
        f << "#include \"math_lib.h\"\nint compute_answer() { return 84; }\n";
    }
    // Update main.cpp expectation to 84
    {
        std::ofstream f(main_cpp);
        f << "#include \"math_lib.h\"\n#include <iostream>\nint main() {\n"
          << "    std::cout << \"Answer: \" << compute_answer() << \"\\n\";\n"
          << "    return (compute_answer() == 84) ? 0 : 1;\n"
          << "}\n";
    }

    auto rebuild_res = cxxgen::ProcessRunner::run(ninja_bld_opts);
    assert(rebuild_res.success());

    auto run2_res = cxxgen::ProcessRunner::run(run_opts);
    assert(run2_res.success());
    assert(run2_res.exit_code == 0);
    assert(cxxgen::util::trim(run2_res.stdout_output) == "Answer: 84");

    // Cleanup
    fs::remove_all(fixture_dir, ec);

    std::cout << "[PASS] End-to-end Ninja execution and incremental rebuild verified!\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "Running Story 6.3: Ninja Generator Tests\n";
    std::cout << "===========================================\n";

    test_ninja_generator_unit();
    test_ninja_end_to_end_build();

    std::cout << "\nAll Story 6.3 Tests Passed Successfully!\n";
    return 0;
}
