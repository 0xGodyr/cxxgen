#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void test_auto_compile_commands_on_build() {
    fs::path test_dir = fs::current_path() / ".cxxgen_test_auto_cc";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "src", ec);

    // Create a dummy source
    {
        std::ofstream out(test_dir / "src" / "main.cpp");
        out << "int main() { return 0; }\n";
    }

    // Build script
    {
        std::ofstream out(test_dir / "build.cpp");
        out << "#include \"cxxgen.hpp\"\n"
            << "void build(cxxgen::Builder &b) {\n"
            << "    auto &app = b.add_executable({\n"
            << "        .name = \"test_auto_cc_app\",\n"
            << "        .sources = {\"src/main.cpp\"},\n"
            << "        .cpp_version = cxxgen::CppVersion::Cpp20,\n"
            << "    });\n"
            << "    b.install_artifact(app);\n"
            << "}\n"
            << "CXXGEN_MAIN(build)\n";
    }

    fs::copy_file(fs::absolute("include/cxxgen.hpp"), test_dir / "cxxgen.hpp", ec);

    fs::path cc_json = test_dir / "compile_commands.json";
    assert(!fs::exists(cc_json));

    // Prepare and run build
    cxxgen::JitRunnerConfig jcfg{
        .working_dir = test_dir,
        .build_script = test_dir / "build.cpp",
        .cxxgen_header = test_dir / "cxxgen.hpp",
        .verbose = false,
    };
    auto jres = cxxgen::JitRunner::prepare_runner(jcfg);
    assert(jres.success);

    int exit_code = cxxgen::JitRunner::execute_runner(jres.runner_binary, {"build"}, test_dir);
    assert(exit_code == 0);

    // compile_commands.json MUST exist now!
    assert(fs::exists(cc_json));

    // Verify it contains "main.cpp"
    std::ifstream in(cc_json);
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    assert(content.find("main.cpp") != std::string::npos);

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] test_auto_compile_commands_on_build\n";
}

int main() {
    std::cout << "Running test_auto_compile_commands...\n";
    test_auto_compile_commands_on_build();
    std::cout << "All auto compile_commands tests passed successfully!\n";
    return 0;
}
