#include "cxxgen.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

namespace fs = std::filesystem;

void test_deterministic_guid() {
    std::cout << "Testing deterministic GUID generation...\n";

    std::string guid1 = cxxgen::util::generate_deterministic_guid("project:core_lib");
    std::string guid2 = cxxgen::util::generate_deterministic_guid("project:core_lib");
    std::string guid3 = cxxgen::util::generate_deterministic_guid("project:app");

    // 1. Same name yields identical GUID
    assert(guid1 == guid2);
    // 2. Different names yield distinct GUIDs
    assert(guid1 != guid3);

    // 3. Format is {XXXXXXXX-XXXX-5XXX-YXXX-XXXXXXXXXXXX}
    assert(guid1.front() == '{' && guid1.back() == '}');
    assert(guid1.size() == 38);
    assert(guid1[9] == '-');
    assert(guid1[14] == '-');
    assert(guid1[19] == '-');
    assert(guid1[24] == '-');

    // RFC 4122 version 5 check
    assert(guid1[15] == '5');
    // RFC 4122 variant check: 8, 9, A, or B
    char var = guid1[20];
    assert(var == '8' || var == '9' || var == 'A' || var == 'B');

    std::cout << "[PASS] Deterministic GUID generation verified: " << guid1 << "\n";
}

void test_msvc_generator_unit() {
    std::cout << "Testing MsvcGenerator unit output generation...\n";

    cxxgen::Builder b;
    b.set_project_name("SampleMsvcProject");

    // 1. Interface target
    auto &iface = b.add_interface_library({
        .name = "common_iface",
        .include_dirs = {"include/common"},
        .defines = {"COMMON_ENABLED=1"},
    });

    // 2. Static lib
    auto &core_lib = b.add_static_library({
        .name = "core_lib",
        .sources = {"core/src/core.cpp", "core/include/core.h"},
        .include_dirs = {"core/include"},
        .defines = {"CORE_BUILD=1"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    core_lib.link(iface);

    // 3. Shared lib
    auto &math_dll = b.add_shared_library({
        .name = "math_dll",
        .sources = {"math/src/math.cpp", "math/include/math.h"},
        .include_dirs = {"math/include"},
        .defines = {"MATH_BUILD=1"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
    math_dll.link(core_lib);

    // 4. Executable
    auto &app = b.add_executable({
        .name = "main_app",
        .sources = {"app/src/main.cpp", "app/include/main.h"},
        .include_dirs = {"app/include"},
        .defines = {"APP_MODE=1"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .warnings_as_errors = true,
        .output_name = "custom_app",
    });
    app.link(math_dll);

    cxxgen::MsvcGenerator gen(b);

    // 1. Test .sln content
    std::string sln = gen.generate_sln();
    assert(sln.find("Microsoft Visual Studio Solution File, Format Version 12.00") != std::string::npos);
    assert(sln.find("# Visual Studio Version 17") != std::string::npos);
    assert(sln.find("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"main_app\", \"main_app.vcxproj\"") !=
           std::string::npos);
    assert(sln.find("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"core_lib\", \"core_lib.vcxproj\"") !=
           std::string::npos);
    assert(sln.find("GlobalSection(SolutionConfigurationPlatforms) = preSolution") != std::string::npos);
    assert(sln.find("Debug|x64 = Debug|x64") != std::string::npos);
    assert(sln.find("Release|x64 = Release|x64") != std::string::npos);
    assert(sln.find("GlobalSection(ProjectConfigurationPlatforms) = postSolution") != std::string::npos);
    assert(sln.find("SolutionGuid = " + gen.solution_guid()) != std::string::npos);

    // 2. Test core_lib.vcxproj content
    std::string core_vcx = gen.generate_vcxproj(core_lib);
    assert(core_vcx.find("<ConfigurationType>StaticLibrary</ConfigurationType>") != std::string::npos);
    assert(core_vcx.find("<LanguageStandard>stdcpp20</LanguageStandard>") != std::string::npos);
    assert(core_vcx.find("CORE_BUILD=1") != std::string::npos);
    assert(core_vcx.find("<ClCompile Include=\"core\\src\\core.cpp\" />") != std::string::npos);
    assert(core_vcx.find("<ClInclude Include=\"core\\include\\core.h\" />") != std::string::npos);
    assert(core_vcx.find("<ProjectReference Include=\"common_iface.vcxproj\">") != std::string::npos);

    // 3. Test math_dll.vcxproj content
    std::string math_vcx = gen.generate_vcxproj(math_dll);
    assert(math_vcx.find("<ConfigurationType>DynamicLibrary</ConfigurationType>") != std::string::npos);
    assert(math_vcx.find("MATH_DLL_EXPORTS") != std::string::npos);
    assert(math_vcx.find("<ProjectReference Include=\"core_lib.vcxproj\">") != std::string::npos);

    // 4. Test main_app.vcxproj content
    std::string app_vcx = gen.generate_vcxproj(app);
    assert(app_vcx.find("<ConfigurationType>Application</ConfigurationType>") != std::string::npos);
    assert(app_vcx.find("<TargetName>custom_app</TargetName>") != std::string::npos);
    assert(app_vcx.find("<TreatWarningAsError>true</TreatWarningAsError>") != std::string::npos);
    assert(app_vcx.find("<ProjectReference Include=\"math_dll.vcxproj\">") != std::string::npos);

    // 5. Test filters content
    std::string app_flt = gen.generate_vcxproj_filters(app);
    assert(app_flt.find("<Filter Include=\"Source Files\\app\\src\">") != std::string::npos);
    assert(app_flt.find("<Filter Include=\"Header Files\\app\\include\">") != std::string::npos);
    assert(app_flt.find("<ClCompile Include=\"app\\src\\main.cpp\">") != std::string::npos);
    assert(app_flt.find("<Filter>Source Files\\app\\src</Filter>") != std::string::npos);

    std::cout << "[PASS] MsvcGenerator unit generation verified!\n";
}

void test_msvc_end_to_end_build() {
    std::cout << "Testing MSVC end-to-end execution with MSBuild...\n";

    fs::path fixture_dir = "test_msvc_fixture";
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

    // 2. Configure Builder with paths relative to fixture_dir
    cxxgen::Builder b;
    b.set_project_name("E2E_MsvcProject");

    auto &math_lib = b.add_static_library({
        .name = "math_lib",
        .sources = {"lib/src/math_lib.cpp", "lib/include/math_lib.h"},
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

    // 3. Generate all MSVC files
    cxxgen::MsvcGenerator gen(b);
    assert(gen.generate_all(fixture_dir));

    assert(fs::exists(fixture_dir / "E2E_MsvcProject.sln"));
    assert(fs::exists(fixture_dir / "math_lib.vcxproj"));
    assert(fs::exists(fixture_dir / "math_lib.vcxproj.filters"));
    assert(fs::exists(fixture_dir / "e2e_app.vcxproj"));
    assert(fs::exists(fixture_dir / "e2e_app.vcxproj.filters"));

    // 4. Locate MSBuild on Windows
#if defined(_WIN32)
    std::string msbuild_path;
    // Check standard MSBuild path or search with vswhere
    fs::path standard_msbuild =
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe";
    if (fs::exists(standard_msbuild)) {
        msbuild_path = standard_msbuild.string();
    } else {
        // Try vswhere
        std::string vswhere_cmd = "\"%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe\" -latest "
                                  "-requires Microsoft.Component.MSBuild -find MSBuild\\**\\Bin\\MSBuild.exe";
        FILE *pipe = _popen(vswhere_cmd.c_str(), "r");
        if (pipe) {
            char buf[512];
            if (fgets(buf, sizeof(buf), pipe) != nullptr) {
                std::string line(buf);
                while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
                    line.pop_back();
                }
                if (fs::exists(line)) {
                    msbuild_path = line;
                }
            }
            _pclose(pipe);
        }
    }

    if (!msbuild_path.empty()) {
        std::cout << "Found MSBuild at: " << msbuild_path << "\n";

        // Build Release configuration
        cxxgen::ProcessOptions bld_opts;
        bld_opts.command = {
            msbuild_path, "E2E_MsvcProject.sln", "/p:Configuration=Release", "/p:Platform=x64", "/m",
        };
        bld_opts.working_directory = fixture_dir;
        auto bld_res = cxxgen::ProcessRunner::run(bld_opts);
        if (!bld_res.success()) {
            std::cerr << "MSBuild output:\n" << bld_res.combined_output() << "\n";
        }
        assert(bld_res.success());

        // Verify built executable
        fs::path exe_path = fixture_dir / "bin" / "Release" / "e2e_app.exe";
        assert(fs::exists(exe_path));

        cxxgen::ProcessOptions run_opts;
        run_opts.command = {exe_path.string()};
        auto run_res = cxxgen::ProcessRunner::run(run_opts);
        assert(run_res.success());
        assert(run_res.exit_code == 0);
        assert(cxxgen::util::trim(run_res.stdout_output) == "Answer: 42");

        std::cout << "[PASS] End-to-end MSBuild build and execution verified!\n";
    } else {
        std::cout << "[SKIP] MSBuild not found in standard paths, skipped end-to-end execution.\n";
    }
#else
    std::cout << "[SKIP] POSIX platform, skipped MSBuild execution.\n";
#endif

    // Cleanup
    fs::remove_all(fixture_dir, ec);
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "Running Story 6.2: MSVC Generator Tests\n";
    std::cout << "===========================================\n";

    test_deterministic_guid();
    test_msvc_generator_unit();
    test_msvc_end_to_end_build();

    std::cout << "\nAll Story 6.2 Tests Passed Successfully!\n";
    return 0;
}
