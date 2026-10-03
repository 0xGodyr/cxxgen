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

void test_depfile_parser_edge_cases() {
    std::cout << "Testing depfile parser edge cases...\n";

    // 1. Simple rule
    std::string dep1 = "build/app.o: src/app.cpp inc/h1.h inc/h2.h\n";
    auto res1 = cxxgen::DepfileScanner::parse_dependencies(dep1);
    assert(res1.size() == 3);
    assert(res1[0] == "src/app.cpp");
    assert(res1[1] == "inc/h1.h");
    assert(res1[2] == "inc/h2.h");

    // 2. Multiline with backslash continuations
    std::string dep2 = "build/lib.o: src/lib.cpp \\\n"
                       "  inc/lib.h \\\n"
                       "  inc/detail/helper.h\n";
    auto res2 = cxxgen::DepfileScanner::parse_dependencies(dep2);
    assert(res2.size() == 3);
    assert(res2[0] == "src/lib.cpp");
    assert(res2[1] == "inc/lib.h");
    assert(res2[2] == "inc/detail/helper.h");

    // 3. Windows drive letters
    std::string dep3 = "C:/projects/app/build/app.o: C:/projects/app/src/app.cpp \\\n"
                       "  C:/projects/app/include/app.h\n";
    auto res3 = cxxgen::DepfileScanner::parse_dependencies(dep3);
    assert(res3.size() == 2);
    assert(res3[0] == "C:/projects/app/src/app.cpp");
    assert(res3[1] == "C:/projects/app/include/app.h");

    // 4. Windows paths with backslashes
    std::string dep4 = "build\\app.o: src\\app.cpp inc\\h1.h\n";
    auto res4 = cxxgen::DepfileScanner::parse_dependencies(dep4);
    assert(res4.size() == 2);
    assert(res4[0] == "src\\app.cpp");
    assert(res4[1] == "inc\\h1.h");

    // 5. Escaped spaces in paths
    std::string dep5 = "build/app.o: src/app.cpp \\\n"
                       "  C:/Program\\ Files/LLVM/include/stddef.h \\\n"
                       "  include/my\\ special\\ header.h\n";
    auto res5 = cxxgen::DepfileScanner::parse_dependencies(dep5);
    assert(res5.size() == 3);
    assert(res5[0] == "src/app.cpp");
    assert(res5[1] == "C:/Program Files/LLVM/include/stddef.h");
    assert(res5[2] == "include/my special header.h");

    // 6. Dummy rules emitted by -MP
    std::string dep6 = "build/app.o: src/app.cpp inc/h1.h\n\n"
                       "inc/h1.h:\n\n";
    auto res6 = cxxgen::DepfileScanner::parse_dependencies(dep6);
    assert(res6.size() == 2);
    assert(res6[0] == "src/app.cpp");
    assert(res6[1] == "inc/h1.h");

    // 7. Large depfile with dozens of included headers (60+ headers)
    std::string dep7 = "build/engine.o: src/engine.cpp";
    for (int i = 1; i <= 60; ++i) {
        dep7 += " \\\n  include/core/header_" + std::to_string(i) + ".hpp";
    }
    dep7 += "\n";
    auto res7 = cxxgen::DepfileScanner::parse_dependencies(dep7);
    assert(res7.size() == 61); // 1 source + 60 headers
    assert(res7[0] == "src/engine.cpp");
    assert(res7[60] == "include/core/header_60.hpp");

    // 8. In-memory caching and retrieval
    cxxgen::DepfileScanner::clear_cache();
    cxxgen::DepfileScanner::record_dependencies("obj_key.o.d", res7);
    auto cached = cxxgen::DepfileScanner::get_cached_dependencies("obj_key.o.d");
    assert(cached.size() == 61);
    assert(cached[0] == "src/engine.cpp");

    std::cout << "[PASS] Depfile parser edge cases and large file parsing verified!\n";
}

void test_nested_header_recompilation(cxxgen::IToolchain &tc) {
    std::cout << "Testing nested header change triggering recompilation with " << tc.name() << "...\n";
    fs::path test_dir = fs::path("build") / ("depfile_test_nested_" + std::to_string(static_cast<int>(tc.family())));
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "src", ec);
    fs::create_directories(test_dir / "include/nested", ec);

    fs::path deep_hdr = test_dir / "include/nested/deep.hpp";
    fs::path mid_hdr = test_dir / "include/mid.hpp";
    fs::path main_src = test_dir / "src/main.cpp";

    // Initial files
    std::ofstream(deep_hdr) << "#pragma once\nconstexpr int MAGIC_VAL = 100;\n";
    std::ofstream(mid_hdr) << "#pragma once\n#include \"nested/deep.hpp\"\nconstexpr int MID_VAL = MAGIC_VAL * 2;\n";
    std::ofstream(main_src)
        << "#include <iostream>\n#include \"mid.hpp\"\nint main() { return (MID_VAL == 200) ? 0 : 1; }\n";

    cxxgen::Builder b;
    auto &exe = b.add_executable({
        .name = "nested_dep_app_" + std::to_string(static_cast<int>(tc.family())),
        .sources = {main_src.string()},
        .include_dirs = {(test_dir / "include").string()},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .output_dir = test_dir.string(),
    });

    // 1. Initial build: compiles source and creates depfile
    auto res1 = b.build_target(exe, tc);
    assert(res1.success);
    assert(!res1.up_to_date);
    fs::path exe_path = exe.artifact_path(tc);
    assert(fs::exists(exe_path));

    // Run executable
    cxxgen::ProcessOptions popts;
    popts.command = {exe_path.string()};
    auto pres1 = cxxgen::ProcessRunner::run(popts);
    assert(pres1.success());
    assert(pres1.exit_code == 0);

    // Verify .d depfile exists
    fs::path obj_dir = fs::path(".cxxgen/obj") / exe.name();
    std::string safe_name = cxxgen::CompilationPipeline::sanitize_source_for_object(main_src.string());
    const char *obj_ext = (tc.family() == cxxgen::CompilerFamily::Msvc) ? ".obj" : ".o";
    fs::path obj_path = obj_dir / (safe_name + obj_ext);
    fs::path dep_path = obj_path.string() + ".d";

    assert(fs::exists(dep_path));
    auto parsed_deps = cxxgen::DepfileScanner::parse_file(dep_path);
    assert(!parsed_deps.empty());

    // Check that deep.hpp and mid.hpp are both discovered
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

    // 2. Second build: untouched -> must be UP TO DATE
    auto res2 = b.build_target(exe, tc);
    assert(res2.success);
    assert(res2.up_to_date);

    // 3. Modify nested/deep.hpp: must trigger recompilation!
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    {
        std::ofstream f(deep_hdr);
        f << "#pragma once\nconstexpr int MAGIC_VAL = 500;\n";
    }
    {
        std::ofstream f(main_src);
        f << "#include <iostream>\n#include \"mid.hpp\"\nint main() { return (MID_VAL == 1000) ? 0 : 1; }\n";
    }

    assert(!cxxgen::DepfileScanner::is_object_up_to_date(obj_path, dep_path));

    auto res3 = b.build_target(exe, tc);
    assert(res3.success);
    assert(!res3.up_to_date);

    auto pres3 = cxxgen::ProcessRunner::run(popts);
    assert(pres3.success());
    assert(pres3.exit_code == 0);

    // 4. Modify ONLY the deeply nested header without touching main.cpp!
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    {
        std::ofstream f(deep_hdr);
        f << "#pragma once\nconstexpr int MAGIC_VAL = 777;\n";
    }
    // Modify deep_hdr and check that main.cpp is recompiled even though main.cpp timestamp didn't change!
    assert(!cxxgen::DepfileScanner::is_object_up_to_date(obj_path, dep_path));

    auto res4 = b.build_target(exe, tc);
    assert(res4.success);
    assert(!res4.up_to_date);

    // Recompilation produced updated obj and exe!
    std::cout << "[PASS] Nested header recompilation verified with " << tc.name() << "!\n";
}

void test_multi_source_partial_recompilation(cxxgen::IToolchain &tc) {
    std::cout << "Testing multi-source partial recompilation with " << tc.name() << "...\n";
    fs::path test_dir = fs::path("build") / ("depfile_test_multi_" + std::to_string(static_cast<int>(tc.family())));
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "src", ec);
    fs::create_directories(test_dir / "include", ec);

    fs::path common_hdr = test_dir / "include/common.hpp";
    fs::path src1 = test_dir / "src/part1.cpp";
    fs::path src2 = test_dir / "src/part2.cpp";
    fs::path main_src = test_dir / "src/main.cpp";

    std::ofstream(common_hdr) << "#pragma once\nint get_common() { return 10; }\n";
    std::ofstream(src1) << "#include \"common.hpp\"\nint part1() { return get_common(); }\n";
    std::ofstream(src2) << "int part2() { return 20; }\n";
    std::ofstream(main_src) << "int part1(); int part2(); int main() { return (part1() + part2() == 30) ? 0 : 1; }\n";

    cxxgen::Builder b;
    auto &exe = b.add_executable({
        .name = "multi_dep_app_" + std::to_string(static_cast<int>(tc.family())),
        .sources = {src1.string(), src2.string(), main_src.string()},
        .include_dirs = {(test_dir / "include").string()},
        .output_dir = test_dir.string(),
    });

    auto res1 = b.build_target(exe, tc);
    assert(res1.success);
    assert(!res1.up_to_date);

    fs::path obj_dir = fs::path(".cxxgen/obj") / exe.name();
    const char *obj_ext = (tc.family() == cxxgen::CompilerFamily::Msvc) ? ".obj" : ".o";

    fs::path obj1 = obj_dir / (cxxgen::CompilationPipeline::sanitize_source_for_object(src1.string()) + obj_ext);
    fs::path obj2 = obj_dir / (cxxgen::CompilationPipeline::sanitize_source_for_object(src2.string()) + obj_ext);

    auto t1_before = fs::last_write_time(obj1, ec);
    auto t2_before = fs::last_write_time(obj2, ec);

    // Sleep across clock tick, then modify common_hdr and main_src
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    // Modify common_hdr -> only src1 should be out-of-date, src2 must remain up-to-date!
    std::ofstream(common_hdr) << "#pragma once\nint get_common() { return 100; }\n";
    std::ofstream(main_src) << "int part1(); int part2(); int main() { return (part1() + part2() == 120) ? 0 : 1; }\n";

    fs::path dep1 = obj1.string() + ".d";
    fs::path dep2 = obj2.string() + ".d";
    assert(!cxxgen::DepfileScanner::is_object_up_to_date(obj1, dep1));
    assert(cxxgen::DepfileScanner::is_object_up_to_date(obj2, dep2));

    auto res2 = b.build_target(exe, tc);
    assert(res2.success);

    auto t1_after = fs::last_write_time(obj1, ec);
    auto t2_after = fs::last_write_time(obj2, ec);

    // obj1 was recompiled (timestamp updated towards now)
    assert(t1_after > t1_before);
    // obj2 was skipped (timestamp unchanged)
    assert(t2_after == t2_before);

    fs::path exe_path = exe.artifact_path(tc);
    cxxgen::ProcessOptions popts;
    popts.command = {exe_path.string()};
    auto pres2 = cxxgen::ProcessRunner::run(popts);
    assert(pres2.success());
    assert(pres2.exit_code == 0);

    std::cout << "[PASS] Multi-source partial recompilation verified with " << tc.name() << "!\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "Running Story 5.1: GCC / Clang Depfile Tests\n";
    std::cout << "===========================================\n";

    test_depfile_parser_edge_cases();

    auto clang = cxxgen::find_clang_toolchain();
    if (clang) {
        test_nested_header_recompilation(*clang);
        test_multi_source_partial_recompilation(*clang);
    }

    auto gcc = cxxgen::find_gcc_toolchain();
    if (gcc) {
        test_nested_header_recompilation(*gcc);
        test_multi_source_partial_recompilation(*gcc);
    }

    std::cout << "\nAll Story 5.1 Tests Passed Successfully!\n";
    return 0;
}
