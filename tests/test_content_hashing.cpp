#include "cxxgen.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

namespace fs = std::filesystem;

void test_xxhash64_unit() {
    std::cout << "Testing xxHash64 unit functions...\n";

    // Standard xxHash64 test vector for empty string with seed 0:
    // empty string -> 0xef46db3751d8e999ULL
    uint64_t h_empty = cxxgen::hash::xxhash64("", 0, 0);
    assert(h_empty == 0xef46db3751d8e999ULL);

    std::string hex_empty = cxxgen::hash::to_hex(h_empty);
    assert(hex_empty == "ef46db3751d8e999");
    assert(cxxgen::hash::from_hex(hex_empty) == h_empty);

    // Test non-empty string
    std::string test_str = "Antigravity C++ Builder (cxxgen) Content Hashing";
    uint64_t h1 = cxxgen::hash::xxhash64(test_str.data(), test_str.size(), 0);
    uint64_t h2 = cxxgen::hash::xxhash64(test_str.data(), test_str.size(), 0);
    assert(h1 == h2);
    assert(h1 != 0);

    // Different seed produces different hash
    uint64_t h_seed = cxxgen::hash::xxhash64(test_str.data(), test_str.size(), 12345);
    assert(h1 != h_seed);

    // Roundtrip hex conversion
    std::string hex1 = cxxgen::hash::to_hex(h1);
    assert(hex1.size() == 16);
    assert(cxxgen::hash::from_hex(hex1) == h1);

    // File hashing
    fs::path temp_file = "test_hash_temp.txt";
    {
        std::ofstream f(temp_file);
        f << test_str;
    }
    auto fhash = cxxgen::hash::hash_file(temp_file);
    assert(fhash.has_value());
    assert(*fhash == h1);
    std::error_code ec;
    fs::remove(temp_file, ec);

    std::cout << "[PASS] xxHash64 unit tests verified!\n";
}

void test_json_unit() {
    std::cout << "Testing JsonValue parser and serializer...\n";

    cxxgen::util::JsonValue root(std::map<std::string, cxxgen::util::JsonValue>{});
    root["version"] = 1;
    root["name"] = "cxxgen_build_state";
    root["enabled"] = true;

    std::vector<cxxgen::util::JsonValue> list;
    list.push_back("file1.cpp");
    list.push_back("file2.cpp");
    root["files"] = list;

    std::string json_str = root.serialize(0);
    assert(!json_str.empty());

    auto parsed_opt = cxxgen::util::JsonValue::parse(json_str);
    assert(parsed_opt.has_value());
    const auto &parsed = *parsed_opt;
    assert(parsed.is_object());
    assert(parsed["version"].as_int64() == 1);
    assert(parsed["name"].as_string() == "cxxgen_build_state");
    assert(parsed["enabled"].as_bool() == true);
    assert(parsed["files"].is_array());
    assert(parsed["files"].as_array().size() == 2);
    assert(parsed["files"][0].as_string() == "file1.cpp");
    assert(parsed["files"][1].as_string() == "file2.cpp");

    std::cout << "[PASS] JsonValue parser and serializer verified!\n";
}

void test_build_state_database_unit() {
    std::cout << "Testing BuildStateDatabase persistence...\n";

    fs::path test_db_path = "test_cache/build_state.json";
    std::error_code ec;
    fs::remove_all("test_cache", ec);

    auto &db = cxxgen::BuildStateDatabase::instance();
    db.clear();
    db.set_db_path(test_db_path);

    cxxgen::ObjectBuildState obs;
    obs.object_file = "obj/main.o";
    obs.source_file = "src/main.cpp";
    obs.source_hash = 0x12345678ULL;
    obs.source_mtime = 1000;
    obs.source_size = 500;
    obs.compiler_signature = "clang 19.1.0";
    obs.compile_flags = "-std=c++20 -O3";
    obs.fingerprint = "abcdef0123456789";
    obs.headers.push_back({"include/header1.h", 0x9999ULL, 1000});

    db.set_object_state(obs.object_file, obs);
    assert(db.has_object_state("obj/main.o"));
    assert(db.is_dirty());

    assert(db.save(test_db_path));
    assert(fs::exists(test_db_path));

    // Clear and reload
    db.clear();
    assert(!db.has_object_state("obj/main.o"));
    assert(db.load(test_db_path));
    assert(db.has_object_state("obj/main.o"));

    auto loaded = db.get_object_state("obj/main.o");
    assert(loaded.has_value());
    assert(loaded->source_file == "src/main.cpp");
    assert(loaded->source_hash == 0x12345678ULL);
    assert(loaded->source_mtime == 1000);
    assert(loaded->source_size == 500);
    assert(loaded->compiler_signature == "clang 19.1.0");
    assert(loaded->compile_flags == "-std=c++20 -O3");
    assert(loaded->fingerprint == "abcdef0123456789");
    assert(loaded->headers.size() == 1);
    assert(loaded->headers[0].path == "include/header1.h");
    assert(loaded->headers[0].hash == 0x9999ULL);

    fs::remove_all("test_cache", ec);
    db.clear();

    std::cout << "[PASS] BuildStateDatabase persistence verified!\n";
}

void test_content_hashing_end_to_end(const cxxgen::IToolchain &tc) {
    std::cout << "Testing content hashing end-to-end with toolchain: " << tc.name() << "...\n";

    fs::path fixture_dir = "test_hash_fixture";
    std::error_code ec;
    fs::remove_all(fixture_dir, ec);
    fs::remove_all(".cxxgen", ec);

    fs::create_directories(fixture_dir / "src", ec);
    fs::create_directories(fixture_dir / "include", ec);

    fs::path common_hdr = fixture_dir / "include" / "common.hpp";
    fs::path mod1_hdr = fixture_dir / "include" / "mod1.hpp";
    fs::path mod2_hdr = fixture_dir / "include" / "mod2.hpp";
    fs::path main_src = fixture_dir / "src" / "main.cpp";
    fs::path mod1_src = fixture_dir / "src" / "mod1.cpp";
    fs::path mod2_src = fixture_dir / "src" / "mod2.cpp";

    {
        std::ofstream f(common_hdr);
        f << "#pragma once\nconstexpr int BASE_FACTOR = 10;\n";
    }
    {
        std::ofstream f(mod1_hdr);
        f << "#pragma once\nint get_mod1_val();\n";
    }
    {
        std::ofstream f(mod2_hdr);
        f << "#pragma once\nint get_mod2_val();\n";
    }
    {
        std::ofstream f(mod1_src);
        f << "#include \"common.hpp\"\n#include \"mod1.hpp\"\nint get_mod1_val() { return 5 * BASE_FACTOR; }\n";
    }
    {
        std::ofstream f(mod2_src);
        f << "#include \"common.hpp\"\n#include \"mod2.hpp\"\nint get_mod2_val() { return 7 * BASE_FACTOR; }\n";
    }
    {
        std::ofstream f(main_src);
        f << "#include \"mod1.hpp\"\n#include \"mod2.hpp\"\n#include <iostream>\n"
          << "#ifdef DEBUG_FLAG\nconstexpr int DEBUG_EXTRA = 100;\n#else\nconstexpr int DEBUG_EXTRA = 0;\n#endif\n"
          << "int main() {\n"
          << "    int val = get_mod1_val() + get_mod2_val() + DEBUG_EXTRA;\n"
          << "    std::cout << val << \"\\n\";\n"
          << "    return 0;\n"
          << "}\n";
    }

    // 1. Initial Build
    cxxgen::Builder b;
    auto exe = b.add_executable({
        .name = "hash_app",
        .sources = {main_src.string(), mod1_src.string(), mod2_src.string()},
        .include_dirs = {(fixture_dir / "include").string()},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    auto res1 = b.build_target(exe, tc);
    if (!res1.success) {
        std::cerr << "Build target failed: " << res1.error_message << "\n";
    }
    assert(res1.success);
    assert(!res1.up_to_date);

    // Verify binary runs
    fs::path art_path = cxxgen::CompilationPipeline::get_artifact_path(exe, tc);
    assert(fs::exists(art_path));
    cxxgen::ProcessOptions popts;
    popts.command = {art_path.string()};
    auto pres1 = cxxgen::ProcessRunner::run(popts);
    assert(pres1.success());
    assert(cxxgen::util::trim(pres1.stdout_output) == "120"); // 50 + 70 + 0

    // Verify .cxxgen/cache/build_state.json exists
    fs::path state_json = cxxgen::BuildStateDatabase::default_db_path();
    assert(fs::exists(state_json));

    // 2. Acceptance Criteria 1: Back-to-Back build with no changes (< 50ms execution)
    auto t0 = std::chrono::steady_clock::now();
    auto res2 = b.build_target(exe, tc);
    auto t1 = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    assert(res2.success);
    assert(res2.up_to_date); // Zero files recompiled
    std::cout << "Back-to-back build execution time: " << elapsed_ms << " ms\n";
    assert(elapsed_ms < 50); // Under 50ms

    // 3. Touching files without changes must NOT recompile (content hash match)
    // Sleep to ensure a changed file timestamp quantum
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    {
        // Re-write identical content into mod1.cpp (simulates touch or git branch switch)
        std::ofstream f(mod1_src);
        f << "#include \"common.hpp\"\n#include \"mod1.hpp\"\nint get_mod1_val() { return 5 * BASE_FACTOR; }\n";
    }

    auto res3 = b.build_target(exe, tc);
    assert(res3.success);
    assert(res3.up_to_date); // Content hash matched! Zero files recompiled!

    // 4. Acceptance Criteria 2: Modifying a single file recompiles ONLY that object file and re-links
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    {
        std::ofstream f(mod2_src);
        f << "#include \"common.hpp\"\n#include \"mod2.hpp\"\nint get_mod2_val() { return 9 * BASE_FACTOR; }\n";
    }

    const char *obj_ext = (tc.family() == cxxgen::CompilerFamily::Msvc) ? ".obj" : ".o";
    fs::path obj_dir = fs::path(".cxxgen/obj") / exe.name();
    fs::path main_obj =
        obj_dir / (cxxgen::CompilationPipeline::sanitize_source_for_object(main_src.string()) + obj_ext);
    fs::path mod1_obj =
        obj_dir / (cxxgen::CompilationPipeline::sanitize_source_for_object(mod1_src.string()) + obj_ext);
    fs::path mod2_obj =
        obj_dir / (cxxgen::CompilationPipeline::sanitize_source_for_object(mod2_src.string()) + obj_ext);

    auto main_time_before = fs::last_write_time(main_obj);
    auto mod1_time_before = fs::last_write_time(mod1_obj);
    auto mod2_time_before = fs::last_write_time(mod2_obj);

    auto res4 = b.build_target(exe, tc);
    assert(res4.success);
    assert(!res4.up_to_date);

    auto main_time_after = fs::last_write_time(main_obj);
    auto mod1_time_after = fs::last_write_time(mod1_obj);
    auto mod2_time_after = fs::last_write_time(mod2_obj);

    // main and mod1 were NOT recompiled
    assert(main_time_before == main_time_after);
    assert(mod1_time_before == mod1_time_after);
    // mod2 WAS recompiled
    assert(mod2_time_after > mod2_time_before);

    // Verify relinked executable outputs 50 + 90 + 0 = 140
    auto pres4 = cxxgen::ProcessRunner::run(popts);
    assert(pres4.success());
    assert(cxxgen::util::trim(pres4.stdout_output) == "140");

    // 5. Acceptance Criteria 3: Changing compiler flag (adding define) invalidates cache and triggers recompilation
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    cxxgen::Builder b2;
    auto exe_debug = b2.add_executable({
        .name = "hash_app",
        .sources = {main_src.string(), mod1_src.string(), mod2_src.string()},
        .include_dirs = {(fixture_dir / "include").string()},
        .defines = {"DEBUG_FLAG"}, // Define changed!
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    auto res5 = b2.build_target(exe_debug, tc);
    assert(res5.success);
    assert(!res5.up_to_date); // Flags changed -> cache invalidated -> recompiled!

    auto pres5 = cxxgen::ProcessRunner::run(popts);
    assert(pres5.success());
    assert(cxxgen::util::trim(pres5.stdout_output) == "240"); // 50 + 90 + 100 = 240!

    // Cleanup
    fs::remove_all(fixture_dir, ec);
    fs::remove_all(".cxxgen", ec);

    std::cout << "[PASS] End-to-end content hashing verified with toolchain: " << tc.name() << "!\n";
}

int main() {
    std::cout << "===============================================\n";
    std::cout << "Running Story 5.3: Content Hashing & DB Tests\n";
    std::cout << "===============================================\n";

    test_xxhash64_unit();
    test_json_unit();
    test_build_state_database_unit();

    // End-to-end tests with Clang
    auto clang_tc = cxxgen::find_clang_toolchain();
    if (clang_tc) {
        test_content_hashing_end_to_end(*clang_tc);
    }

    // End-to-end tests with GCC
    auto gcc_tc = cxxgen::find_gcc_toolchain();
    if (gcc_tc) {
        test_content_hashing_end_to_end(*gcc_tc);
    }

    // End-to-end tests with MSVC
    auto msvc_tc = cxxgen::find_msvc_toolchain();
    if (msvc_tc) {
        test_content_hashing_end_to_end(*msvc_tc);
    }

    std::cout << "\nAll Story 5.3 Tests Passed Successfully!\n";
    return 0;
}
