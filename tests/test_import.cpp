#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void test_cmake_tokenization() {
    std::cout << "Testing CMake tokenizer and variable expansion...\n";

    std::string cmake_text = R"(
        # Line comment
        #[[ Bracket comment with
            newlines and # symbols ]]
        project(SampleApp LANGUAGES CXX)
        set(SRC_LIST "src/main.cpp" "src/\"nested_quote\".cpp")
        add_executable(app ${SRC_LIST})
    )";

    auto tokens = cxxgen::CMakeImporter::tokenize(cmake_text);
    assert(!tokens.empty());

    // Verify tokens contain project, SampleApp, LANGUAGES, CXX, set, SRC_LIST, add_executable, app
    std::vector<std::string> token_strings;
    for (const auto &tok : tokens) {
        token_strings.push_back(tok.text);
    }

    auto has_token = [&](const std::string &val) {
        return std::find(token_strings.begin(), token_strings.end(), val) != token_strings.end();
    };

    assert(has_token("project"));
    assert(has_token("SampleApp"));
    assert(has_token("LANGUAGES"));
    assert(has_token("CXX"));
    assert(has_token("set"));
    assert(has_token("SRC_LIST"));
    assert(has_token("src/main.cpp"));
    assert(has_token("src/\"nested_quote\".cpp"));
    assert(has_token("add_executable"));
    assert(has_token("app"));

    std::cout << "[PASS] CMake tokenizer verified.\n";
}

void test_cmake_importer_single_target() {
    std::cout << "Testing CMakeImporter single-target project...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_import_cmake_single";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path cmake_file = test_dir / "CMakeLists.txt";
    std::ofstream out(cmake_file);
    out << "cmake_minimum_required(VERSION 3.20)\n"
        << "project(SingleApp LANGUAGES CXX)\n"
        << "set(CMAKE_CXX_STANDARD 20)\n"
        << "set(MY_SOURCES src/main.cpp src/helper.cpp)\n"
        << "add_executable(my_app ${MY_SOURCES})\n"
        << "target_include_directories(my_app PRIVATE include $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ext_inc>)\n"
        << "target_compile_definitions(my_app PRIVATE APP_VER=\"1.0.0\" DEBUG_MODE=1)\n"
        << "target_compile_options(my_app PRIVATE -Wall -Wextra)\n";
    out.close();

    std::string err;
    auto proj_opt = cxxgen::CMakeImporter::import_file(cmake_file, err);
    assert(proj_opt.has_value());
    assert(err.empty());

    const auto &proj = *proj_opt;
    assert(proj.project_name == "SingleApp");
    assert(proj.targets.size() == 1);

    const auto &t = proj.targets[0];
    assert(t.name == "my_app");
    assert(t.type == cxxgen::TargetType::Executable);
    assert(t.cpp_version == cxxgen::CppVersion::Cpp20);
    assert(t.sources.size() == 2);
    assert(t.sources[0] == "src/main.cpp");
    assert(t.sources[1] == "src/helper.cpp");
    assert(t.include_dirs.size() == 2);
    assert(t.include_dirs[0] == "include");
    assert(t.include_dirs[1] == "ext_inc");
    assert(t.defines.size() == 2);
    assert(t.defines[0] == "APP_VER=\"1.0.0\"");
    assert(t.defines[1] == "DEBUG_MODE=1");
    assert(t.compile_options.size() == 2);
    assert(t.compile_options[0] == "-Wall");
    assert(t.compile_options[1] == "-Wextra");

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] CMakeImporter single-target project verified.\n";
}

void test_cmake_importer_multi_target_and_subdir() {
    std::cout << "Testing CMakeImporter multi-target project with add_subdirectory...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_import_cmake_multi";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir / "core", ec);

    // Subdirectory CMakeLists.txt
    fs::path sub_cmake = test_dir / "core" / "CMakeLists.txt";
    std::ofstream sub_out(sub_cmake);
    sub_out << "add_library(core_static STATIC src/core.cpp)\n"
            << "target_include_directories(core_static PUBLIC include)\n"
            << "add_library(math_dyn SHARED src/math.cpp)\n";
    sub_out.close();

    // Root CMakeLists.txt
    fs::path root_cmake = test_dir / "CMakeLists.txt";
    std::ofstream root_out(root_cmake);
    root_out << "cmake_minimum_required(VERSION 3.20)\n"
             << "project(MultiEngine)\n"
             << "add_library(common_iface INTERFACE)\n"
             << "target_include_directories(common_iface INTERFACE include/common)\n"
             << "add_subdirectory(core)\n"
             << "add_executable(app src/main.cpp)\n"
             << "target_link_libraries(app PRIVATE core_static common_iface ws2_32)\n"
             << "add_test(NAME app_test COMMAND app)\n";
    root_out.close();

    std::string err;
    auto proj_opt = cxxgen::CMakeImporter::import_file(root_cmake, err);
    assert(proj_opt.has_value());

    const auto &proj = *proj_opt;
    assert(proj.project_name == "MultiEngine");
    assert(proj.targets.size() == 4);

    // Verify common_iface
    auto it_iface =
        std::find_if(proj.targets.begin(), proj.targets.end(), [](const auto &t) { return t.name == "common_iface"; });
    assert(it_iface != proj.targets.end());
    assert(it_iface->type == cxxgen::TargetType::Interface);
    assert(!it_iface->include_dirs.empty() && it_iface->include_dirs[0] == "include/common");

    // Verify core_static
    auto it_core =
        std::find_if(proj.targets.begin(), proj.targets.end(), [](const auto &t) { return t.name == "core_static"; });
    assert(it_core != proj.targets.end());
    assert(it_core->type == cxxgen::TargetType::StaticLibrary);
    assert(!it_core->sources.empty() && it_core->sources[0] == "core/src/core.cpp");
    assert(!it_core->include_dirs.empty() && it_core->include_dirs[0] == "core/include");

    // Verify math_dyn
    auto it_math =
        std::find_if(proj.targets.begin(), proj.targets.end(), [](const auto &t) { return t.name == "math_dyn"; });
    assert(it_math != proj.targets.end());
    assert(it_math->type == cxxgen::TargetType::SharedLibrary);
    assert(!it_math->sources.empty() && it_math->sources[0] == "core/src/math.cpp");

    // Verify app
    auto it_app = std::find_if(proj.targets.begin(), proj.targets.end(), [](const auto &t) { return t.name == "app"; });
    assert(it_app != proj.targets.end());
    assert(it_app->is_test);
    assert(it_app->sources.size() == 1 && it_app->sources[0] == "src/main.cpp");
    assert(std::find(it_app->linked_targets.begin(), it_app->linked_targets.end(), "core_static") !=
           it_app->linked_targets.end());
    assert(std::find(it_app->linked_targets.begin(), it_app->linked_targets.end(), "common_iface") !=
           it_app->linked_targets.end());
    assert(std::find(it_app->system_libs.begin(), it_app->system_libs.end(), "ws2_32") != it_app->system_libs.end());

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] CMakeImporter multi-target and add_subdirectory verified.\n";
}

void test_msvc_importer_vcxproj() {
    std::cout << "Testing MsvcImporter .vcxproj parsing...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_import_msvc_vcxproj";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path vcx_file = test_dir / "Engine.vcxproj";
    std::ofstream out(vcx_file);
    out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        << "<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
        << "  <PropertyGroup Label=\"Globals\">\n"
        << "    <ProjectGuid>{ABCDEF12-3456-7890-ABCD-EF1234567890}</ProjectGuid>\n"
        << "    <RootNamespace>Engine</RootNamespace>\n"
        << "  </PropertyGroup>\n"
        << "  <PropertyGroup>\n"
        << "    <ConfigurationType>StaticLibrary</ConfigurationType>\n"
        << "  </PropertyGroup>\n"
        << "  <ItemDefinitionGroup>\n"
        << "    <ClCompile>\n"
        << "      <LanguageStandard>stdcpp20</LanguageStandard>\n"
        << "      "
           "<AdditionalIncludeDirectories>include;third_party\\include;%(AdditionalIncludeDirectories)</"
           "AdditionalIncludeDirectories>\n"
        << "      "
           "<PreprocessorDefinitions>ENGINE_EXPORTS;CUSTOM_MACRO=1;WIN32;_CONSOLE;%(PreprocessorDefinitions)</"
           "PreprocessorDefinitions>\n"
        << "      <PrecompiledHeaderFile>pch.h</PrecompiledHeaderFile>\n"
        << "    </ClCompile>\n"
        << "    <Link>\n"
        << "      <AdditionalDependencies>ws2_32.lib;kernel32.lib;%(AdditionalDependencies)</AdditionalDependencies>\n"
        << "    </Link>\n"
        << "  </ItemDefinitionGroup>\n"
        << "  <ItemGroup>\n"
        << "    <ClCompile Include=\"src\\engine.cpp\" />\n"
        << "    <ClCompile Include=\"src\\renderer.cpp\" />\n"
        << "  </ItemGroup>\n"
        << "  <ItemGroup>\n"
        << "    <ProjectReference Include=\"..\\Core\\Core.vcxproj\">\n"
        << "      <Project>{11223344-5566-7788-99AA-BBCCDDEEFF00}</Project>\n"
        << "    </ProjectReference>\n"
        << "  </ItemGroup>\n"
        << "</Project>\n";
    out.close();

    std::string err;
    auto proj_opt = cxxgen::MsvcImporter::import_project(vcx_file, err);
    assert(proj_opt.has_value());

    const auto &proj = *proj_opt;
    assert(proj.targets.size() == 1);
    const auto &t = proj.targets[0];
    assert(t.name == "Engine");
    assert(t.type == cxxgen::TargetType::StaticLibrary);
    assert(t.cpp_version == cxxgen::CppVersion::Cpp20);
    assert(t.pch_header == "pch.h");

    // Sources normalized to forward slash
    assert(t.sources.size() == 2);
    assert(t.sources[0] == "src/engine.cpp");
    assert(t.sources[1] == "src/renderer.cpp");

    // Includes
    assert(t.include_dirs.size() == 2);
    assert(t.include_dirs[0] == "include");
    assert(t.include_dirs[1] == "third_party/include");

    // Boilerplate WIN32 and _CONSOLE filtered out, custom macros kept
    assert(std::find(t.defines.begin(), t.defines.end(), "ENGINE_EXPORTS") != t.defines.end());
    assert(std::find(t.defines.begin(), t.defines.end(), "CUSTOM_MACRO=1") != t.defines.end());
    assert(std::find(t.defines.begin(), t.defines.end(), "WIN32") == t.defines.end());
    assert(std::find(t.defines.begin(), t.defines.end(), "_CONSOLE") == t.defines.end());

    // Boilerplate kernel32.lib filtered out, ws2_32.lib kept
    assert(std::find(t.system_libs.begin(), t.system_libs.end(), "ws2_32.lib") != t.system_libs.end());
    assert(std::find(t.system_libs.begin(), t.system_libs.end(), "kernel32.lib") == t.system_libs.end());

    // Project reference Core extracted
    assert(std::find(t.linked_targets.begin(), t.linked_targets.end(), "Core") != t.linked_targets.end());

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] MsvcImporter .vcxproj parsing verified.\n";
}

void test_msvc_importer_solution() {
    std::cout << "Testing MsvcImporter .sln solution parsing...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_import_msvc_sln";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    // Create App.vcxproj
    std::ofstream app_vcx(test_dir / "App.vcxproj");
    app_vcx << "<Project><PropertyGroup><ConfigurationType>Application</ConfigurationType></PropertyGroup>"
            << "<ItemGroup><ClCompile Include=\"src/main.cpp\" /></ItemGroup></Project>";
    app_vcx.close();

    // Create Math.vcxproj
    std::ofstream math_vcx(test_dir / "Math.vcxproj");
    math_vcx << "<Project><PropertyGroup><ConfigurationType>StaticLibrary</ConfigurationType></PropertyGroup>"
             << "<ItemGroup><ClCompile Include=\"src/math.cpp\" /></ItemGroup></Project>";
    math_vcx.close();

    // Create App.sln with GUID dependency from App onto Math
    std::ofstream sln(test_dir / "App.sln");
    sln << "Microsoft Visual Studio Solution File, Format Version 12.00\n"
        << "# Visual Studio Version 17\n"
        << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"App\", \"App.vcxproj\", "
           "\"{AAA11111-2222-3333-4444-555555555555}\"\n"
        << "\tProjectSection(ProjectDependencies) = postProject\n"
        << "\t\t{BBB11111-2222-3333-4444-555555555555} = {BBB11111-2222-3333-4444-555555555555}\n"
        << "\tEndProjectSection\n"
        << "EndProject\n"
        << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"Math\", \"Math.vcxproj\", "
           "\"{BBB11111-2222-3333-4444-555555555555}\"\n"
        << "EndProject\n";
    sln.close();

    std::string err;
    auto proj_opt = cxxgen::MsvcImporter::import_solution(test_dir / "App.sln", err);
    assert(proj_opt.has_value());

    const auto &proj = *proj_opt;
    assert(proj.targets.size() == 2);

    auto it_app = std::find_if(proj.targets.begin(), proj.targets.end(), [](const auto &t) { return t.name == "App"; });
    auto it_math =
        std::find_if(proj.targets.begin(), proj.targets.end(), [](const auto &t) { return t.name == "Math"; });
    assert(it_app != proj.targets.end());
    assert(it_math != proj.targets.end());

    assert(it_app->type == cxxgen::TargetType::Executable);
    assert(it_math->type == cxxgen::TargetType::StaticLibrary);

    // App links to Math via SLN GUID dependency
    assert(std::find(it_app->linked_targets.begin(), it_app->linked_targets.end(), "Math") !=
           it_app->linked_targets.end());

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] MsvcImporter .sln solution parsing verified.\n";
}

void test_project_code_synthesizer_and_syntax() {
    std::cout << "Testing ProjectCodeSynthesizer and generated code syntax...\n";

    cxxgen::ImportedProject proj;
    proj.project_name = "Kebab-App.Test";

    // 1. Interface
    cxxgen::ImportedTarget iface;
    iface.name = "iface-lib";
    iface.type = cxxgen::TargetType::Interface;
    iface.include_dirs = {"include/iface"};
    proj.targets.push_back(iface);

    // 2. Static Lib
    cxxgen::ImportedTarget static_lib;
    static_lib.name = "core-lib";
    static_lib.type = cxxgen::TargetType::StaticLibrary;
    static_lib.sources = {"src/core.cpp"};
    static_lib.include_dirs = {"include/core"};
    static_lib.defines = {"CORE_BUILD=1"};
    static_lib.cpp_version = cxxgen::CppVersion::Cpp20;
    static_lib.linked_targets = {"iface-lib"};
    proj.targets.push_back(static_lib);

    // 3. Executable
    cxxgen::ImportedTarget exe;
    exe.name = "kebab-app";
    exe.type = cxxgen::TargetType::Executable;
    exe.sources = {"src/main.cpp"};
    exe.include_dirs = {"include"};
    exe.defines = {"APP_DEBUG=1"};
    exe.cpp_version = cxxgen::CppVersion::Cpp20;
    exe.linked_targets = {"core-lib"};
    exe.system_libs = {"ws2_32"};
    proj.targets.push_back(exe);

    std::string synthesized = cxxgen::ProjectCodeSynthesizer::synthesize(proj);
    assert(!synthesized.empty());

    // Verify key C++20 constructs in synthesized code
    assert(synthesized.find("#include \"include/cxxgen.hpp\"") != std::string::npos);
    assert(synthesized.find("void build(cxxgen::Builder &b)") != std::string::npos);
    assert(synthesized.find("CXXGEN_MAIN(build)") != std::string::npos);
    assert(synthesized.find("b.add_interface_library") != std::string::npos);
    assert(synthesized.find("b.add_static_library") != std::string::npos);
    assert(synthesized.find("b.add_executable") != std::string::npos);
    assert(synthesized.find("kebab_app.link(core_lib);") != std::string::npos);
    assert(synthesized.find("kebab_app.link_system_lib(\"ws2_32\");") != std::string::npos);
    assert(synthesized.find("b.install_artifact(kebab_app);") != std::string::npos);

    // Verify syntax by invoking clang++ -fsyntax-only on the generated code
    fs::path test_dir = fs::current_path() / ".cxxgen_test_synthesizer_syntax";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path temp_build_cpp = test_dir / "build.cpp";
    std::ofstream out(temp_build_cpp);
    out << synthesized;
    out.close();

    cxxgen::ProcessOptions opts;
    opts.command = {"clang++",
                    "-std=c++20",
                    "-fsyntax-only",
                    "-I" + fs::absolute("include").string(),
                    "-I" + (fs::absolute("include").parent_path()).string(),
                    temp_build_cpp.string()};
    opts.working_directory = test_dir;
    auto res = cxxgen::ProcessRunner::run(opts);
    if (!res.success()) {
        std::cerr << "Syntax check compiler error:\n" << res.combined_output() << "\n";
    }
    assert(res.success());

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] ProjectCodeSynthesizer syntax check verified.\n";
}

void test_project_importer_cli_and_safety() {
    std::cout << "Testing ProjectImporter auto-discovery, safety checks, and options...\n";

    fs::path test_dir = fs::current_path() / ".cxxgen_test_importer_safety";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    // 1. Create a dummy CMakeLists.txt
    fs::path cmake_file = test_dir / "CMakeLists.txt";
    std::ofstream out(cmake_file);
    out << "cmake_minimum_required(VERSION 3.20)\n"
        << "project(SafetyTest LANGUAGES CXX)\n"
        << "add_executable(safety_app src/main.cpp)\n";
    out.close();

    // 2. Auto-discovery without specifying format or file name
    cxxgen::ImportOptions opts;
    opts.output_path = test_dir / "build.cpp";
    auto res1 = cxxgen::ProjectImporter::import_project(test_dir, opts);
    assert(res1.success);
    assert(res1.detected_format == "cmake");
    assert(res1.targets_imported == 1);
    assert(fs::exists(test_dir / "build.cpp"));

    // 3. Collision protection: attempt import without --force should fail
    auto res2 = cxxgen::ProjectImporter::import_project(test_dir, opts);
    assert(!res2.success);
    assert(res2.error_message.find("already exists") != std::string::npos);

    // 4. Force overwrite
    opts.force = true;
    auto res3 = cxxgen::ProjectImporter::import_project(test_dir, opts);
    assert(res3.success);

    // 5. Dry-run mode: does not write file
    fs::path dry_out = test_dir / "dry_build.cpp";
    opts.output_path = dry_out;
    opts.dry_run = true;
    auto res4 = cxxgen::ProjectImporter::import_project(test_dir, opts);
    assert(res4.success);
    assert(!res4.generated_code.empty());
    assert(!fs::exists(dry_out));

    // 6. Custom output path
    fs::path custom_out = test_dir / "custom" / "my_build.cpp";
    opts.dry_run = false;
    opts.output_path = custom_out;
    auto res5 = cxxgen::ProjectImporter::import_project(test_dir, opts);
    assert(res5.success);
    assert(fs::exists(custom_out));

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] ProjectImporter auto-discovery and safety checks verified.\n";
}

void test_round_trip_cmake_generation_and_import() {
    std::cout << "Testing round-trip: cxxgen -> CMake -> cxxgen import...\n";

    // 1. Create a cxxgen project in memory
    cxxgen::Builder b;
    b.set_project_name("RoundTripProj");

    auto &engine = b.add_static_library({
        .name = "engine",
        .sources = {"engine/src/engine.cpp"},
        .include_dirs = {"engine/include"},
        .defines = {"ENGINE_LIB=1"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    auto &game = b.add_executable({
        .name = "game",
        .sources = {"game/src/main.cpp"},
        .include_dirs = {"game/include"},
        .defines = {"GAME_BUILD=1"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    game.link(engine);

    // 2. Generate CMakeLists.txt using CMakeGenerator
    cxxgen::CMakeGenerator gen(b);
    std::string cmake_content = gen.generate();

    fs::path test_dir = fs::current_path() / ".cxxgen_test_roundtrip";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir, ec);

    fs::path cmake_file = test_dir / "CMakeLists.txt";
    std::ofstream out(cmake_file);
    out << cmake_content;
    out.close();

    // 3. Import back using CMakeImporter
    std::string err;
    auto proj_opt = cxxgen::CMakeImporter::import_file(cmake_file, err);
    assert(proj_opt.has_value());

    const auto &proj = *proj_opt;
    assert(proj.project_name == "RoundTripProj");
    assert(proj.targets.size() == 2);

    auto it_engine =
        std::find_if(proj.targets.begin(), proj.targets.end(), [](const auto &t) { return t.name == "engine"; });
    auto it_game =
        std::find_if(proj.targets.begin(), proj.targets.end(), [](const auto &t) { return t.name == "game"; });

    assert(it_engine != proj.targets.end());
    assert(it_game != proj.targets.end());

    assert(it_engine->type == cxxgen::TargetType::StaticLibrary);
    assert(it_game->type == cxxgen::TargetType::Executable);

    assert(std::find(it_game->linked_targets.begin(), it_game->linked_targets.end(), "engine") !=
           it_game->linked_targets.end());

    fs::remove_all(test_dir, ec);
    std::cout << "[PASS] Round-trip CMake generation and import verified.\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "Running Epic 18: Build System Importer Tests\n";
    std::cout << "===========================================\n";

    test_cmake_tokenization();
    test_cmake_importer_single_target();
    test_cmake_importer_multi_target_and_subdir();
    test_msvc_importer_vcxproj();
    test_msvc_importer_solution();
    test_project_code_synthesizer_and_syntax();
    test_project_importer_cli_and_safety();
    test_round_trip_cmake_generation_and_import();

    std::cout << "\nAll Epic 18 Importer Tests Passed Successfully!\n";
    return 0;
}
