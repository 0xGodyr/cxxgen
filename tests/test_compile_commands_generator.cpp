#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void test_compile_commands_unit() {
    std::cout << "Testing CompileCommandsGenerator unit and schema validation...\n";

    cxxgen::Builder b;
    b.set_project_name("TestCompileCommands");

    // 1. Interface library
    auto &iface = b.add_interface_library({
        .name = "common_iface",
        .include_dirs = {"include/common"},
        .defines = {"COMMON_MACRO=1"},
    });

    // 2. Static library
    auto &core_lib = b.add_static_library({
        .name = "core_lib",
        .sources = {"core/src/core.cpp", "core/src/util.cpp", "core/include/core.h"},
        .include_dirs = {"core/include"},
        .defines = {"CORE_BUILD=1"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    core_lib.link(iface);

    // 3. Executable
    auto &app = b.add_executable({
        .name = "main_app",
        .sources = {"app/src/main.cpp"},
        .include_dirs = {"app/include"},
        .defines = {"APP_MODE=1"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    app.link(core_lib);

    fs::path custom_work_dir = "C:/mock/project_root";
    cxxgen::CompileCommandsGenerator gen(b, nullptr, custom_work_dir);
    std::string json_str = gen.generate();

    // 1. Parse JSON to validate syntax and schema
    auto root_opt = cxxgen::util::JsonValue::parse(json_str);
    assert(root_opt.has_value());
    const auto &root = *root_opt;
    assert(root.is_array());

    // We have 3 source files total: core.cpp, util.cpp, main.cpp (core.h and iface are excluded)
    assert(root.size() == 3);

    bool found_core = false;
    bool found_util = false;
    bool found_main = false;

    for (size_t i = 0; i < root.size(); ++i) {
        const auto &entry = root[i];
        assert(entry.is_object());

        // Validate mandatory Clang compilation database fields
        assert(entry.contains("directory"));
        assert(entry["directory"].is_string());
        assert(entry["directory"].as_string() == "C:/mock/project_root");

        assert(entry.contains("command"));
        assert(entry["command"].is_string());
        std::string cmd = entry["command"].as_string();
        assert(!cmd.empty());

        assert(entry.contains("file"));
        assert(entry["file"].is_string());
        std::string file = entry["file"].as_string();

        assert(entry.contains("output"));
        assert(entry["output"].is_string());
        std::string output = entry["output"].as_string();

        if (file == "core/src/core.cpp") {
            found_core = true;
            assert(output.find("obj/core_lib/") != std::string::npos);
            assert(cmd.find("CORE_BUILD=1") != std::string::npos);
            assert(cmd.find("COMMON_MACRO=1") != std::string::npos);
        } else if (file == "core/src/util.cpp") {
            found_util = true;
            assert(output.find("obj/core_lib/") != std::string::npos);
        } else if (file == "app/src/main.cpp") {
            found_main = true;
            assert(output.find("obj/main_app/") != std::string::npos);
            assert(cmd.find("APP_MODE=1") != std::string::npos);
            assert(cmd.find("core/include") != std::string::npos);
            assert(cmd.find("include/common") != std::string::npos);
        }
    }

    assert(found_core);
    assert(found_util);
    assert(found_main);

    std::cout << "[PASS] CompileCommandsGenerator schema and contents verified!\n";
}

void test_compile_commands_file_generation() {
    std::cout << "Testing compile_commands.json file write and re-parse...\n";

    fs::path fixture_dir = "test_compile_commands_fixture";
    std::error_code ec;
    fs::remove_all(fixture_dir, ec);
    fs::create_directories(fixture_dir, ec);

    cxxgen::Builder b;
    b.add_executable({
        .name = "simple_app",
        .sources = {"main.cpp"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    cxxgen::CompileCommandsGenerator gen(b);
    fs::path target_file = fixture_dir / "compile_commands.json";
    assert(gen.generate_file(target_file));
    assert(fs::exists(target_file));

    std::ifstream in(target_file);
    assert(in.is_open());
    std::string file_content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    assert(!file_content.empty());

    auto parsed = cxxgen::util::JsonValue::parse(file_content);
    assert(parsed.has_value());
    assert(parsed->is_array());
    assert(parsed->size() == 1);
    assert(parsed->as_array()[0]["file"].as_string() == "main.cpp");

    fs::remove_all(fixture_dir, ec);
    std::cout << "[PASS] File generation and re-parse verified!\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "Running Story 6.4: Compile Commands Generator Tests\n";
    std::cout << "====================================================\n";

    test_compile_commands_unit();
    test_compile_commands_file_generation();

    std::cout << "\nAll Story 6.4 Tests Passed Successfully!\n";
    return 0;
}
