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

void test_show_includes_parser_unit() {
    std::cout << "Testing ShowIncludesScanner unit parser and filter...\n";

    std::string sample_output =
        "main.cpp\r\n"
        "Note: including file: C:\\Program Files\\Microsoft Visual Studio\\2022\\VC\\Tools\\MSVC\\include\\iostream\r\n"
        "Note: including file:  C:\\Program Files\\Microsoft Visual Studio\\2022\\VC\\Tools\\MSVC\\include\\istream\r\n"
        "Note: including file:   C:\\Program Files\\Microsoft Visual "
        "Studio\\2022\\VC\\Tools\\MSVC\\include\\ostream\r\n"
        "Note: including file:    C:\\projects\\app\\include\\config.hpp\r\n"
        "Note: including file:     C:\\projects\\app\\include\\version.hpp\r\n"
        "main.cpp(25): warning C4100: 'argc': unreferenced formal parameter\r\n"
        "main.cpp(26): warning C4189: 'unused': local variable is initialized but not referenced\r\n";

    auto parsed = cxxgen::ShowIncludesScanner::parse(sample_output);

    // 1. Accurately discover all 5 included headers
    assert(parsed.dependencies.size() == 5);
    assert(parsed.dependencies[0] ==
           "C:\\Program Files\\Microsoft Visual Studio\\2022\\VC\\Tools\\MSVC\\include\\iostream");
    assert(parsed.dependencies[1] ==
           "C:\\Program Files\\Microsoft Visual Studio\\2022\\VC\\Tools\\MSVC\\include\\istream");
    assert(parsed.dependencies[2] ==
           "C:\\Program Files\\Microsoft Visual Studio\\2022\\VC\\Tools\\MSVC\\include\\ostream");
    assert(parsed.dependencies[3] == "C:\\projects\\app\\include\\config.hpp");
    assert(parsed.dependencies[4] == "C:\\projects\\app\\include\\version.hpp");

    // 2. Clean terminal output without 'Note: including file:' clutter
    assert(parsed.clean_output.find("Note: including file:") == std::string::npos);
    assert(parsed.clean_output.find("main.cpp") != std::string::npos);
    assert(parsed.clean_output.find("warning C4100") != std::string::npos);
    assert(parsed.clean_output.find("warning C4189") != std::string::npos);

    // 3. Case-insensitive prefix check
    std::string lower_sample = "note: including file: C:\\test\\header.h\ncode.cpp\n";
    auto lower_parsed = cxxgen::ShowIncludesScanner::parse(lower_sample);
    assert(lower_parsed.dependencies.size() == 1);
    assert(lower_parsed.dependencies[0] == "C:\\test\\header.h");
    assert(lower_parsed.clean_output.find("note: including file:") == std::string::npos);
    assert(lower_parsed.clean_output.find("code.cpp") != std::string::npos);

    // 4. Custom localized prefix
    std::string french_sample = "Remarque : inclusion du fichier : C:\\test\\french.h\ncode.cpp\n";
    auto french_parsed = cxxgen::ShowIncludesScanner::parse(french_sample, "Remarque : inclusion du fichier :");
    assert(french_parsed.dependencies.size() == 1);
    assert(french_parsed.dependencies[0] == "C:\\test\\french.h");
    assert(french_parsed.clean_output.find("Remarque : inclusion du fichier :") == std::string::npos);

    // 5. Test depfile emission
    fs::path temp_dep = "build/test_msvc_emit.d";
    std::error_code ec;
    fs::remove(temp_dep, ec);
    bool ok =
        cxxgen::ShowIncludesScanner::write_depfile(temp_dep, "build/main.obj", "src/main.cpp", parsed.dependencies);
    assert(ok);
    assert(fs::exists(temp_dep));

    auto roundtrip_deps = cxxgen::DepfileScanner::parse_file(temp_dep);
    assert(roundtrip_deps.size() == 6); // 1 source + 5 headers
    fs::remove(temp_dep, ec);

    std::cout << "[PASS] ShowIncludesScanner unit parser, filtering, and depfile emission verified!\n";
}

void test_msvc_showincludes_end_to_end() {
    std::cout << "Testing MSVC /showIncludes end-to-end compiler execution...\n";

    auto msvc = cxxgen::find_msvc_toolchain();
    if (!msvc) {
        std::cout << "[SKIP] MSVC toolchain not found on this machine, skipping live cl.exe test.\n";
        return;
    }

    std::cout << "Using MSVC Toolchain: " << msvc->name() << "\n";
    fs::path test_dir = "build/msvc_showincludes_e2e";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "src", ec);
    fs::create_directories(test_dir / "include/nested", ec);

    fs::path deep_hdr = test_dir / "include/nested/deep.hpp";
    fs::path mid_hdr = test_dir / "include/mid.hpp";
    fs::path main_src = test_dir / "src/main.cpp";

    std::ofstream(deep_hdr) << "#pragma once\nconstexpr int DEEP_VAL = 42;\n";
    std::ofstream(mid_hdr) << "#pragma once\n#include \"nested/deep.hpp\"\nconstexpr int MID_VAL = DEEP_VAL * 2;\n";
    std::ofstream(main_src)
        << "#include <iostream>\n#include \"mid.hpp\"\nint main() { return (MID_VAL == 84) ? 0 : 1; }\n";

    cxxgen::Builder b;
    auto &exe = b.add_executable({
        .name = "msvc_includes_app",
        .sources = {main_src.string()},
        .include_dirs = {(test_dir / "include").string()},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .output_dir = test_dir.string(),
    });

    // 1. Initial build: runs cl.exe /showIncludes, extracts dependencies, writes .d depfile
    auto res1 = b.build_target(exe, *msvc);
    if (!res1.success) {
        std::cerr << "MSVC initial build failed: " << res1.error_message << "\n";
    }
    assert(res1.success);
    assert(!res1.up_to_date);

    // Verify .d depfile was generated for MSVC .obj
    fs::path obj_dir = fs::path(".cxxgen/obj") / exe.name();
    std::string safe_name = cxxgen::CompilationPipeline::sanitize_source_for_object(main_src.string());
    fs::path obj_path = obj_dir / (safe_name + ".obj");
    fs::path dep_path = obj_path.string() + ".d";

    assert(fs::exists(dep_path));
    auto parsed_deps = cxxgen::DepfileScanner::parse_file(dep_path);
    assert(!parsed_deps.empty());

    // Check that mid.hpp and deep.hpp were captured
    bool found_mid = false;
    bool found_deep = false;
    for (const auto &d : parsed_deps) {
        fs::path dp(d);
        if (dp.filename() == "mid.hpp")
            found_mid = true;
        if (dp.filename() == "deep.hpp")
            found_deep = true;
    }
    assert(found_mid);
    assert(found_deep);

    // Verify binary executes
    fs::path exe_path = exe.artifact_path(*msvc);
    assert(fs::exists(exe_path));
    cxxgen::ProcessOptions popts;
    popts.command = {exe_path.string()};
    auto pres1 = cxxgen::ProcessRunner::run(popts);
    assert(pres1.success());
    assert(pres1.exit_code == 0);

    // 2. Second build: untouched -> must be UP TO DATE!
    auto res2 = b.build_target(exe, *msvc);
    assert(res2.success);
    assert(res2.up_to_date);

    // 3. Modify nested deep.hpp -> triggers MSVC recompilation!
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    {
        std::ofstream f(deep_hdr);
        f << "#pragma once\nconstexpr int DEEP_VAL = 50;\n";
    }
    {
        std::ofstream f(main_src);
        f << "#include <iostream>\n#include \"mid.hpp\"\nint main() { return (MID_VAL == 100) ? 0 : 1; }\n";
    }

    assert(!cxxgen::DepfileScanner::is_object_up_to_date(obj_path, dep_path));

    auto res3 = b.build_target(exe, *msvc);
    assert(res3.success);
    assert(!res3.up_to_date);

    auto pres3 = cxxgen::ProcessRunner::run(popts);
    assert(pres3.success());
    assert(pres3.exit_code == 0);

    std::cout << "[PASS] MSVC /showIncludes end-to-end compilation and incremental rebuild verified!\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "Running Story 5.2: MSVC /showIncludes Tests\n";
    std::cout << "===========================================\n";

    test_show_includes_parser_unit();
    test_msvc_showincludes_end_to_end();

    std::cout << "\nAll Story 5.2 Tests Passed Successfully!\n";
    return 0;
}
