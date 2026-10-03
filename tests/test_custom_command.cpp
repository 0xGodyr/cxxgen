#include "cxxgen.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

static void setup_codegen_tool(const fs::path &tool_path) {
    fs::path src_path = tool_path.parent_path() / "codegen_src.cpp";
    std::ofstream src(src_path);
    src << "#include <fstream>\n"
        << "#include <string>\n"
        << "#include <iostream>\n"
        << "int main(int argc, char** argv) {\n"
        << "    if (argc > 1 && std::string(argv[1]) == \"--fail\") {\n"
        << "        std::cerr << \"Codegen intentional failure\" << std::endl;\n"
        << "        return 77;\n"
        << "    }\n"
        << "    if (argc < 3) return 1;\n"
        << "    std::ifstream in(argv[1]);\n"
        << "    std::string val;\n"
        << "    in >> val;\n"
        << "    std::ofstream out(argv[2]);\n"
        << "    out << \"int generated_value() { return \" << val << \"; }\\n\";\n"
        << "    return 0;\n"
        << "}\n";
    src.close();

    auto tc = cxxgen::find_toolchain();
    assert(tc != nullptr);

    cxxgen::Builder b;
    auto &exe = b.add_executable({
        .name = "codegen_tool",
        .sources = {src_path.string()},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .output_dir = tool_path.parent_path().string(),
    });
    auto res = b.build_target(exe, *tc);
    if (!res.success) {
        std::cerr << "Failed to build codegen_tool: " << res.error_message << "\n";
    }
    assert(res.success);
    assert(fs::exists(tool_path));
}

void test_custom_command_missing_output_and_up_to_date(const fs::path &codegen_exe) {
    std::cout << "Testing custom command missing output and up-to-date detection...\n";
    fs::path test_dir = "build/custom_cmd_test1";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path input_file = test_dir / "input.txt";
    fs::path output_file = test_dir / "generated.cpp";

    std::ofstream in_f(input_file);
    in_f << "42\n";
    in_f.close();

    cxxgen::Builder b;
    auto custom_step = b.add_custom_command({
        .name = "gen_code",
        .description = "Generate C++ code from input.txt",
        .command = {codegen_exe.string(), input_file.string(), output_file.string()},
        .inputs = {input_file.string()},
        .outputs = {output_file.string()},
    });

    assert(!custom_step->is_up_to_date());

    auto tc = cxxgen::find_toolchain();
    cxxgen::ExecutionContext ctx{.toolchain = *tc};

    // 1. First execution - output is missing, must run
    auto res1 = b.execute_step("gen_code", ctx);
    assert(res1.success());
    assert(res1.status == cxxgen::StepStatus::Success);
    assert(fs::exists(output_file));

    // Verify output file contents
    std::ifstream out_f(output_file);
    std::string content;
    std::getline(out_f, content);
    assert(content.find("42") != std::string::npos);
    out_f.close();

    // 2. Second execution - input hasn't changed, output exists -> up-to-date
    assert(custom_step->is_up_to_date());
    auto res2 = b.execute_step("gen_code", ctx);
    assert(res2.success());
    assert(res2.status == cxxgen::StepStatus::UpToDate);

    // 3. Third execution - modify input file -> must re-run
    std::ofstream in_mod(input_file);
    in_mod << "99\n";
    in_mod.close();

    // Ensure last_write_time is visibly newer than output (guaranteeing cross-platform resolution)
    auto out_time = fs::last_write_time(output_file, ec);
    fs::last_write_time(input_file, out_time + std::chrono::seconds(5), ec);

    assert(!custom_step->is_up_to_date());
    auto res3 = b.execute_step("gen_code", ctx);
    assert(res3.success());
    assert(res3.status == cxxgen::StepStatus::Success);

    std::ifstream out_mod(output_file);
    std::getline(out_mod, content);
    assert(content.find("99") != std::string::npos);
    out_mod.close();

    std::cout << "[PASS] Custom command output generation and timestamp checking verified!\n";
}

void test_custom_command_downstream_target_integration(const fs::path &codegen_exe) {
    std::cout << "Testing downstream target integration and automatic DAG chaining...\n";
    fs::path test_dir = "build/custom_cmd_test2";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path input_file = test_dir / "value.txt";
    fs::path generated_src = test_dir / "generated_code.cpp";
    fs::path main_src = test_dir / "main.cpp";

    std::ofstream in_f(input_file);
    in_f << "123\n";
    in_f.close();

    std::ofstream main_f(main_src);
    main_f << "int generated_value();\n"
           << "int main() {\n"
           << "    return (generated_value() == 123) ? 0 : 1;\n"
           << "}\n";
    main_f.close();

    cxxgen::Builder b;

    // Register custom command
    auto gen_step = b.add_custom_command({
        .name = "run_codegen",
        .command = {codegen_exe.string(), input_file.string(), generated_src.string()},
        .inputs = {input_file.string()},
        .outputs = {generated_src.string()},
    });

    // Register executable target which uses generated_code.cpp
    auto &app = b.add_executable({
        .name = "generated_consumer_app",
        .sources = {main_src.string(), generated_src.string()},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .output_dir = test_dir.string(),
    });

    // Verify automatic dependency resolution
    auto sorted = b.topological_sort("generated_consumer_app");
    assert(sorted.size() == 2);
    assert(sorted[0]->name() == gen_step->name());
    assert(sorted[1]->name() == app.name());

    auto tc = cxxgen::find_toolchain();
    cxxgen::ExecutionContext ctx{
        .toolchain = *tc,
        .thread_count = 2,
    };

    // Execute the target build step - will automatically run custom command first
    auto res = b.execute_step("generated_consumer_app", ctx);
    if (!res.success()) {
        std::cerr << "Build failed: " << res.error_message << "\n";
    }
    assert(res.success());
    assert(fs::exists(generated_src));

    fs::path exe_path = app.artifact_path(*tc);
    assert(fs::exists(exe_path));

    // Run the compiled binary
    cxxgen::ProcessOptions popts;
    popts.command = {exe_path.string()};
    auto pres = cxxgen::ProcessRunner::run(popts);
    assert(pres.success());
    assert(pres.exit_code == 0);

    std::cout << "[PASS] Downstream target auto-dependency and end-to-end execution verified!\n";
}

void test_custom_command_failure_aborts_downstream(const fs::path &codegen_exe) {
    std::cout << "Testing custom command failure aborting downstream compilation...\n";
    fs::path test_dir = "build/custom_cmd_test3";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path input_file = test_dir / "input.txt";
    fs::path generated_src = test_dir / "will_not_exist.cpp";
    fs::path main_src = test_dir / "main.cpp";

    std::ofstream in_f(input_file);
    in_f << "10\n";
    in_f.close();

    std::ofstream main_f(main_src);
    main_f << "int main() { return 0; }\n";
    main_f.close();

    cxxgen::Builder b;

    // Failing custom command (--fail causes exit code 77)
    auto failing_gen = b.add_custom_command({
        .name = "failing_gen",
        .command = {codegen_exe.string(), "--fail", input_file.string(), generated_src.string()},
        .inputs = {input_file.string()},
        .outputs = {generated_src.string()},
    });

    auto &app = b.add_executable({
        .name = "should_not_compile_app",
        .sources = {main_src.string(), generated_src.string()},
        .output_dir = test_dir.string(),
    });
    app.depend_on(failing_gen);

    auto tc = cxxgen::find_toolchain();
    cxxgen::ExecutionContext ctx{
        .toolchain = *tc,
        .thread_count = 2,
    };

    auto res = b.execute_step("should_not_compile_app", ctx);
    assert(!res.success());
    assert(res.exit_code == 77);
    assert(res.error_message.find("Codegen intentional failure") != std::string::npos);
    assert(!fs::exists(generated_src));

    std::cout << "[PASS] Custom command failure correctly aborted downstream target compilation!\n";
}

void test_custom_command_missing_expected_output() {
    std::cout << "Testing custom command that exits 0 but does not produce output...\n";
    fs::path test_dir = "build/custom_cmd_test4";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    cxxgen::Builder b;

    // Use a command that exits 0 without creating the declared output
    // On Windows, 'cmd /c exit 0' or 'whoami'
#if defined(_WIN32)
    std::vector<std::string> cmd = {"cmd.exe", "/c", "exit 0"};
#else
    std::vector<std::string> cmd = {"true"};
#endif

    auto step = b.add_custom_command({
        .name = "noop_gen",
        .command = cmd,
        .outputs = {(test_dir / "never_created.cpp").string()},
    });

    auto tc = cxxgen::find_toolchain();
    cxxgen::ExecutionContext ctx{.toolchain = *tc};

    auto res = b.execute_step("noop_gen", ctx);
    assert(!res.success());
    assert(res.error_message.find("expected output was not produced") != std::string::npos);

    std::cout << "[PASS] Detection of missing expected output verified!\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "Running Story 4.3: Custom Command Tests\n";
    std::cout << "===========================================\n";

    fs::path test_base = "build/custom_cmd_tools";
    std::error_code ec;
    fs::create_directories(test_base, ec);
#if defined(_WIN32)
    fs::path codegen_exe = fs::absolute(test_base / "codegen_tool.exe");
#else
    fs::path codegen_exe = fs::absolute(test_base / "codegen_tool");
#endif

    setup_codegen_tool(codegen_exe);

    test_custom_command_missing_output_and_up_to_date(codegen_exe);
    test_custom_command_downstream_target_integration(codegen_exe);
    test_custom_command_failure_aborts_downstream(codegen_exe);
    test_custom_command_missing_expected_output();

    std::cout << "\nAll Story 4.3 Tests Passed Successfully!\n";
    return 0;
}
