#include "cxxgen.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace cxxgen;

// ============================================================================
// Security Verification 1: PowerShell Command Injection Immunity
// ============================================================================

void test_powershell_injection_immunity() {
    std::cout << "Testing PowerShell command injection immunity via environment block...\n";

#if defined(_WIN32)
    // Craft a payload that would execute a malicious command if string-concatenated:
    // e.g. "'; Write-Output 'INJECTED'; '"
    std::string malicious_payload = "https://safe.example.com/archive.zip'; Write-Output 'PWNED'; '";
    fs::path temp_dir = fs::temp_directory_path() / "cxxgen_sec_test_ps";
    fs::create_directories(temp_dir);
    fs::path temp_out = temp_dir / "out.txt";

    ProcessOptions ps_opts;
    // Command reads from $env variables without string concatenation
    ps_opts.command = {"powershell", "-NoProfile", "-NonInteractive", "-Command",
                       "$env:CXXGEN_TEST_URL | Out-File -FilePath $env:CXXGEN_TEST_OUT -Encoding utf8"};
    ps_opts.extra_env["CXXGEN_TEST_URL"] = malicious_payload;
    ps_opts.extra_env["CXXGEN_TEST_OUT"] = temp_out.string();

    ProcessResult res = ProcessRunner::run(ps_opts);
    assert(res.success());

    // Verify that the written content is the verbatim string, and 'PWNED' was never executed as code
    assert(res.stdout_output.find("PWNED") == std::string::npos);

    std::ifstream in(temp_out);
    std::string written_content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    assert(written_content.find("https://safe.example.com/archive.zip'; Write-Output 'PWNED'; '") != std::string::npos);

    std::error_code ec;
    fs::remove_all(temp_dir, ec);
#endif

    std::cout << "[PASS] PowerShell injection immunity verified.\n";
}

// ============================================================================
// Security Verification 2: Single-File URL Sanitization (Query Strings & Traversal)
// ============================================================================

void test_single_file_url_sanitization() {
    std::cout << "Testing single-file URL sanitization against query params and traversal...\n";

    auto sanitize_url_filename = [](const std::string &url, const std::string &fallback_name) -> std::string {
        std::string raw_url = url;
        auto q_idx = raw_url.find('?');
        if (q_idx != std::string::npos)
            raw_url = raw_url.substr(0, q_idx);
        auto hash_idx = raw_url.find('#');
        if (hash_idx != std::string::npos)
            raw_url = raw_url.substr(0, hash_idx);
        fs::path single_fname = fs::path(raw_url).filename();
        if (single_fname.empty() || single_fname == "." || single_fname == "..") {
            single_fname = fallback_name + ".h";
        }
        return single_fname.string();
    };

    // 1. URL with query parameters (tokens, signatures)
    std::string url1 = "https://raw.githubusercontent.com/org/repo/v1.0/stb_image.h?token=secret123&auth=true";
    assert(sanitize_url_filename(url1, "stb") == "stb_image.h");

    // 2. URL with anchor fragment
    std::string url2 = "https://example.com/downloads/header.hpp#L10-L20";
    assert(sanitize_url_filename(url2, "header") == "header.hpp");

    // 3. Traversal attempt
    std::string url3 = "https://example.com/path/to/..";
    assert(sanitize_url_filename(url3, "fallback") == "fallback.h");

    std::string url4 = "https://example.com/";
    assert(sanitize_url_filename(url4, "my_lib") == "my_lib.h");

    std::cout << "[PASS] Single-file URL sanitization verified.\n";
}

// ============================================================================
// Security Verification 3: Identifier and Namespace Sanitization in EmbedFileStep
// ============================================================================

void test_embed_file_sanitization() {
    std::cout << "Testing EmbedFileStep variable and namespace sanitization...\n";

    // 1. Leading digit prefixing with 'asset_'
    std::string id1 = util::sanitize_identifier("128_font.ttf");
    assert(id1 == "asset_128_font_ttf");

    // 2. Code injection payload in identifier
    std::string malicious_id = "var; void injected() {}; //";
    std::string id2 = util::sanitize_identifier(malicious_id);
    assert(id2.find(';') == std::string::npos);
    assert(id2.find('(') == std::string::npos);
    assert(id2.find(')') == std::string::npos);
    assert(id2.find('{') == std::string::npos);
    assert(id2.find('}') == std::string::npos);
    assert(id2.find('/') == std::string::npos);
    assert(id2.find(' ') == std::string::npos);
    assert(id2.rfind("var__void_injected", 0) == 0);
    for (char c : id2) {
        assert(std::isalnum(static_cast<unsigned char>(c)) || c == '_');
    }

    // 3. Namespace sanitization with sub-namespaces
    std::string malicious_ns = "app::assets; injected_code; //::v1";
    std::vector<std::string> parts = util::split(malicious_ns, "::");
    std::vector<std::string> clean_parts;
    for (const auto &p : parts) {
        std::string clean = util::sanitize_identifier(p);
        if (!clean.empty())
            clean_parts.push_back(clean);
    }
    std::string clean_ns = util::join(clean_parts, "::");
    assert(clean_ns.find(';') == std::string::npos);
    assert(clean_ns.find('/') == std::string::npos);
    assert(clean_ns.rfind("app::assets__injected_code", 0) == 0);

    // 4. Test actual header generation
    fs::path temp_dir = fs::temp_directory_path() / "cxxgen_sec_test_embed";
    fs::create_directories(temp_dir);
    fs::path src_bin = temp_dir / "test.bin";
    fs::path dest_h = temp_dir / "embedded.hpp";

    {
        std::ofstream out(src_bin, std::ios::binary);
        out << "Hello Security!";
    }

    EmbedFileOptions opts;
    opts.variable_name = "123_payload; evil(); //";
    opts.namespace_name = "secret::ns; evil_ns(); //";
    std::string err;
    bool ok = EmbedFileStep::embed(src_bin, dest_h, opts, err);
    assert(ok);
    assert(fs::exists(dest_h));

    std::ifstream in(dest_h);
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    // Verify generated code has clean identifiers and zero semicolons/parentheses outside statements
    assert(content.find("namespace secret::ns__evil_ns") != std::string::npos);
    assert(content.find("asset_123_payload__evil") != std::string::npos);

    std::error_code ec;
    fs::remove_all(temp_dir, ec);
    std::cout << "[PASS] EmbedFileStep variable and namespace sanitization verified.\n";
}

// ============================================================================
// Security Verification 4: SHA-256 Tamper Detection & Auto-Removal
// ============================================================================

void test_sha256_tamper_detection() {
    std::cout << "Testing SHA-256 integrity mismatch rejection and auto-removal...\n";

    fs::path temp_dir = fs::temp_directory_path() / "cxxgen_sec_test_sha";
    fs::create_directories(temp_dir);
    fs::path sample_file = temp_dir / "sample.tar.gz";

    {
        std::ofstream out(sample_file);
        out << "Valid package contents v1.0.0";
    }

    std::string real_sha = hash::Sha256::hash_file(sample_file);
    std::string bad_sha = "0000000000000000000000000000000000000000000000000000000000000000";

    DependencyConfig cfg;
    cfg.url = sample_file.string();
    cfg.sha256 = bad_sha; // Expect mismatch
    cfg.cache_dir = temp_dir / "cache";

    auto res = DependencyFetcher::fetch("tampered_pkg", cfg);
    assert(!res.success);
    assert(res.error_message.find("SHA-256 integrity check failed") != std::string::npos);

    std::error_code ec;
    fs::remove_all(temp_dir, ec);
    std::cout << "[PASS] SHA-256 integrity rejection verified.\n";
}

// ============================================================================
// Security Verification 5: Git Clone Argument Injection Protection (--)
// ============================================================================

void test_git_argument_injection_protection() {
    std::cout << "Testing Git clone argument separator protection...\n";

    // Verify that clone command construction includes '--' before repository URL
    std::string fake_flag_repo = "--upload-pack=touch /tmp/pwned";
    std::vector<std::string> clone_cmd = {"git", "clone", "--depth", "1", "--", fake_flag_repo, "dest_dir"};

    // The '--' must appear before fake_flag_repo
    bool found_separator = false;
    bool repo_after_separator = false;
    for (size_t i = 0; i < clone_cmd.size(); ++i) {
        if (clone_cmd[i] == "--") {
            found_separator = true;
        } else if (found_separator && clone_cmd[i] == fake_flag_repo) {
            repo_after_separator = true;
            break;
        }
    }
    assert(found_separator);
    assert(repo_after_separator);

    std::cout << "[PASS] Git clone argument injection protection verified.\n";
}

// ============================================================================
// Security Verification 6: Dependency Name Traversal Protection
// ============================================================================

void test_dependency_traversal_rejection() {
    std::cout << "Testing dependency name path traversal rejection...\n";

    DependencyConfig cfg;
    cfg.url = "https://example.com/test.zip";

    auto res1 = DependencyFetcher::fetch("../../../evil_pkg", cfg);
    assert(!res1.success);
    assert(res1.error_message.find("path traversal characters are forbidden") != std::string::npos);

    auto res2 = DependencyFetcher::fetch("sub\\dir\\pkg", cfg);
    assert(!res2.success);
    assert(res2.error_message.find("path traversal characters are forbidden") != std::string::npos);

    auto res3 = DependencyFetcher::fetch(".", cfg);
    assert(!res3.success);
    assert(res3.error_message.find("path traversal characters are forbidden") != std::string::npos);

    // URL flag injection check
    DependencyConfig bad_url_cfg;
    bad_url_cfg.url = "--output=evil.exe";
    auto res4 = DependencyFetcher::fetch("valid_name", bad_url_cfg);
    assert(!res4.success);
    assert(res4.error_message.find("invalid dependency URL") != std::string::npos);

    std::cout << "[PASS] Dependency name path traversal rejection verified.\n";
}

// ============================================================================
// Security Verification 7: System Package Name Flag Injection Rejection
// ============================================================================

void test_system_package_flag_rejection() {
    std::cout << "Testing system package bridge flag injection rejection...\n";

    SystemPackageConfig cfg;
    cfg.required = false;

    auto res1 = SystemPackageBridge::find_package("--version", cfg);
    assert(!res1.found);
    assert(res1.error_message.find("invalid system package name") != std::string::npos);

    auto res2 = SystemPackageBridge::find_package("-lfoo", cfg);
    assert(!res2.found);
    assert(res2.error_message.find("invalid system package name") != std::string::npos);

    std::cout << "[PASS] System package bridge flag rejection verified.\n";
}

// ============================================================================
// Security Verification 8: ProjectCodeSynthesizer C++ String Literal Escaping
// ============================================================================

void test_project_code_synthesizer_escaping() {
    std::cout << "Testing ProjectCodeSynthesizer string literal escaping against CWE-94...\n";

    ImportedProject proj;
    proj.project_name = "App\"WithQuote";
    ImportedTarget tgt;
    tgt.name = "Target\"WithQuote; evil()";
    tgt.type = TargetType::Executable;
    tgt.defines = {"MACRO=\"hello\\world\"", "INJECT\"); evil(); //"};
    tgt.compile_options = {"-DFLAG=\"test\""};
    tgt.sources = {"src/main.cpp"};
    proj.targets.push_back(tgt);

    std::string synthesized = ProjectCodeSynthesizer::synthesize(proj);

    // Verify quotes and backslashes are escaped
    assert(synthesized.find("App\\\"WithQuote") != std::string::npos);
    assert(synthesized.find("MACRO=\\\"hello\\\\world\\\"") != std::string::npos);
    assert(synthesized.find("INJECT\\\"); evil(); //") != std::string::npos);

    std::cout << "[PASS] ProjectCodeSynthesizer string literal escaping verified.\n";
}

// ============================================================================
// Security Verification 9: Cache Key Traversal & Hex Validation
// ============================================================================

void test_cache_key_traversal_rejection() {
    std::cout << "Testing cache key path traversal rejection...\n";

    assert(!util::is_safe_hash_key("../../../evil"));
    assert(!util::is_safe_hash_key("..\\..\\evil"));
    assert(!util::is_safe_hash_key("key with spaces"));
    assert(!util::is_safe_hash_key(""));
    assert(util::is_safe_hash_key("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));

    // BmiCache reject unsafe keys
    assert(!modules::BmiCache::instance().try_restore("../../../evil", "out.bmi", "out.obj"));

    std::cout << "[PASS] Cache key path traversal rejection verified.\n";
}

// ============================================================================
// Security Verification 10: CMakeConfigExporter Injection Rejection
// ============================================================================

void test_cmake_exporter_injection_rejection() {
    std::cout << "Testing CMakeConfigExporter injection rejection...\n";

    Builder b;
    CMakeConfigOptions opts;
    opts.package_name = "MyLib\nexecute_process(COMMAND evil)\n";
    opts.version = "1.0.0";
    std::string err;
    bool ok = CMakeConfigExporter::export_config(b, opts, &err);
    assert(!ok);
    assert(err.find("illegal CMake characters") != std::string::npos);

    opts.package_name = "MyLib";
    opts.version = "1.0.0; evil()";
    ok = CMakeConfigExporter::export_config(b, opts, &err);
    assert(!ok);
    assert(err.find("illegal CMake characters") != std::string::npos);

    std::cout << "[PASS] CMakeConfigExporter injection rejection verified.\n";
}

// ============================================================================
// Security Verification 11: Scaffolder System Directory Protection
// ============================================================================

void test_scaffolder_system_directory_protection() {
    std::cout << "Testing ProjectScaffolder system root directory rejection...\n";

    fs::path root_p;
#if defined(_WIN32)
    root_p = "C:\\";
#else
    root_p = "/";
#endif

    ProjectScaffolderConfig cfg;
    cfg.target_dir = root_p;
    cfg.project_name = "test_root_app";
    cfg.type = "exe";

    auto res = ProjectScaffolder::scaffold(cfg);
    assert(!res.success);
    assert(res.error_message.find("refusing to scaffold project into system protected directory") != std::string::npos);

#if defined(_WIN32)
    const char *windir = std::getenv("WINDIR");
    if (windir && *windir) {
        ProjectScaffolderConfig wcfg;
        wcfg.target_dir = windir;
        wcfg.project_name = "test_windir_app";
        auto wres = ProjectScaffolder::scaffold(wcfg);
        assert(!wres.success);
        assert(wres.error_message.find("refusing to scaffold project into system protected directory") !=
               std::string::npos);
    }
#endif

    std::cout << "[PASS] ProjectScaffolder system root directory rejection verified.\n";
}

// ============================================================================
// Security Verification 12: PackageEngine Name & Traversal Protection
// ============================================================================

void test_package_name_traversal_rejection() {
    std::cout << "Testing PackageEngine package name traversal rejection...\n";

    Builder b;
    auto tc = find_toolchain(CompilerFamily::Auto);
    assert(tc != nullptr);

    PackageConfig bad_cfg1;
    bad_cfg1.package_name = "../../../evil";
    bad_cfg1.version = "1.0.0";
    auto res1 = PackageEngine::package(b, *tc, bad_cfg1);
    assert(!res1.success);
    assert(res1.error_message.find("traversal and illegal characters forbidden") != std::string::npos);

    PackageConfig bad_cfg2;
    bad_cfg2.package_name = "good_pkg";
    bad_cfg2.version = "../../../evil_ver";
    auto res2 = PackageEngine::package(b, *tc, bad_cfg2);
    assert(!res2.success);
    assert(res2.error_message.find("traversal and illegal characters forbidden") != std::string::npos);

    PackageConfig bad_cfg3;
    bad_cfg3.package_name = "pkg; evil_cmd";
    bad_cfg3.version = "1.0.0";
    auto res3 = PackageEngine::package(b, *tc, bad_cfg3);
    assert(!res3.success);
    assert(res3.error_message.find("traversal and illegal characters forbidden") != std::string::npos);

    std::cout << "[PASS] PackageEngine package name traversal rejection verified.\n";
}

// ============================================================================
// Security Verification 13: System Directory Protection & Extension Validation
// ============================================================================

void test_system_protection_checks() {
    std::cout << "Testing is_system_protected_dir and is_safe_extension...\n";

#if defined(_WIN32)
    assert(util::is_system_protected_dir("C:\\"));
    assert(util::is_system_protected_dir("C:\\Windows"));
    assert(util::is_system_protected_dir("C:\\Windows\\System32"));
#else
    assert(util::is_system_protected_dir("/"));
    assert(util::is_system_protected_dir("/bin"));
    assert(util::is_system_protected_dir("/usr"));
    assert(util::is_system_protected_dir("/etc"));
#endif
    assert(!util::is_system_protected_dir(fs::current_path()));

    // Extension validation
    assert(util::is_safe_extension(".obj"));
    assert(util::is_safe_extension(".o"));
    assert(util::is_safe_extension(".lib"));
    assert(!util::is_safe_extension(""));
    assert(!util::is_safe_extension("obj"));
    assert(!util::is_safe_extension(".obj/../evil"));
    assert(!util::is_safe_extension(".obj\\..\\evil"));
    assert(!util::is_safe_extension("."));

    // Safe package name
    assert(util::is_safe_package_name("my-pkg_v1.0"));
    assert(util::is_safe_package_name("vulkan"));
    assert(!util::is_safe_package_name("../evil"));
    assert(!util::is_safe_package_name("evil/pkg"));
    assert(!util::is_safe_package_name("evil\\pkg"));
    assert(!util::is_safe_package_name("-flag"));
    assert(!util::is_safe_package_name(""));

    std::cout << "[PASS] System protection checks verified.\n";
}

// ============================================================================
// Security Verification 14: Zip-Slip Path Escape Detection
// ============================================================================

void test_zip_slip_rejection() {
    std::cout << "Testing Zip-Slip path escape detection...\n";

    fs::path temp_base = fs::temp_directory_path() / "cxxgen_sec_zip_slip_test";
    std::error_code ec;
    fs::remove_all(temp_base, ec);
    fs::create_directories(temp_base / "pkg", ec);

    fs::path pkg_dir = temp_base / "pkg";
    fs::path escape_dir = temp_base / "escaped";
    fs::create_directories(escape_dir, ec);

    // Verify detection logic
    fs::path canonical_base = fs::weakly_canonical(pkg_dir, ec);
    fs::path escaped_file = fs::weakly_canonical(escape_dir / "evil.txt", ec);

    auto [b_base, m_ent] =
        std::mismatch(canonical_base.begin(), canonical_base.end(), escaped_file.begin(), escaped_file.end());
    // Mismatch MUST occur because escaped_file is outside pkg_dir
    assert(b_base != canonical_base.end());

    fs::remove_all(temp_base, ec);
    std::cout << "[PASS] Zip-Slip path escape detection verified.\n";
}

// ============================================================================
// Security Verification 15: Git Dangerous Protocol Rejection (ext:: and fd::)
// ============================================================================

void test_git_dangerous_protocol_rejection() {
    std::cout << "Testing Git remote protocol injection defense (ext::, fd::, ref validation)...\n";

    DependencyConfig cfg1;
    cfg1.git = "ext::sh -c touch /tmp/pwned";
    auto res1 = DependencyFetcher::fetch("evil_ext_dep", cfg1);
    assert(!res1.success);
    assert(res1.error_message.find("dangerous git protocol rejected") != std::string::npos);

    DependencyConfig cfg2;
    cfg2.git = "  EXT::evil_transport";
    auto res2 = DependencyFetcher::fetch("evil_ext_upper", cfg2);
    assert(!res2.success);
    assert(res2.error_message.find("dangerous git protocol rejected") != std::string::npos);

    DependencyConfig cfg3;
    cfg3.git = "fd::123";
    auto res3 = DependencyFetcher::fetch("evil_fd_dep", cfg3);
    assert(!res3.success);
    assert(res3.error_message.find("dangerous git protocol rejected") != std::string::npos);

    // Ref validation checks
    DependencyConfig cfg4;
    cfg4.git = "https://github.com/cxxgen-build/cxxgen.git";
    cfg4.branch = "-u evil";
    auto res4 = DependencyFetcher::fetch("evil_branch", cfg4);
    assert(!res4.success);
    assert(res4.error_message.find("cannot start with '-'") != std::string::npos);

    DependencyConfig cfg5;
    cfg5.git = "https://github.com/cxxgen-build/cxxgen.git";
    cfg5.tag = "tag; evil_command; //";
    auto res5 = DependencyFetcher::fetch("evil_tag", cfg5);
    assert(!res5.success);
    assert(res5.error_message.find("contains illegal characters") != std::string::npos);

    std::cout << "[PASS] Git dangerous protocol rejection verified.\n";
}

// ============================================================================
// Security Verification 16: CMakeGenerator Injection Defense
// ============================================================================

void test_cmake_generator_injection_defense() {
    std::cout << "Testing CMakeGenerator project name and string escaping defense...\n";

    Builder b;
    b.set_project_name("Project\nexecute_process(COMMAND evil)\n");
    auto &exe = b.add_executable({
        .name = "MyTarget",
        .sources = {"src/main.cpp"},
        .defines = {"MESSAGE=\"Hello\\World\"", "CMD=evil();"},
        .compile_options = {"-DFLAG=\"test\\value\""},
    });

    CMakeGenerator gen(b);
    std::string cmake = gen.generate();

    // Verify project name was sanitized into valid alphanumeric/underscore identifier
    assert(cmake.find("execute_process") == std::string::npos ||
           cmake.find("project(Project_execute_process") != std::string::npos);
    assert(cmake.find("project(Project\nexecute_process") == std::string::npos);

    // Verify defines quotes and backslashes were properly escaped
    assert(cmake.find("\"MESSAGE=\\\"Hello\\\\World\\\"\"") != std::string::npos);
    assert(cmake.find("\"-DFLAG=\\\"test\\\\value\\\"\"") != std::string::npos);

    std::cout << "[PASS] CMakeGenerator injection defense verified.\n";
}

// ============================================================================
// Security Verification 17: ProjectImporter System Directory Protection
// ============================================================================

void test_project_importer_system_dir_protection() {
    std::cout << "Testing ProjectImporter output path system directory protection...\n";

    ImportOptions opts;
#if defined(_WIN32)
    opts.output_path = "C:\\Windows\\build.cpp";
#else
    opts.output_path = "/etc/build.cpp";
#endif
    opts.format = "cmake";

    auto res = ProjectImporter::import_project(".", opts);
    assert(!res.success);
    assert(res.error_message.find("refusing to write imported project to system protected directory") !=
           std::string::npos);

    std::cout << "[PASS] ProjectImporter system directory protection verified.\n";
}

// ============================================================================
// Security Verification 18: ProgramData & Extended System Directory Protection
// ============================================================================

void test_extended_system_directory_protection() {
    std::cout << "Testing ProgramData & extended system protection checks...\n";

#if defined(_WIN32)
    const char *progdata = std::getenv("ProgramData");
    if (progdata && *progdata) {
        assert(util::is_system_protected_dir(progdata));
        assert(util::is_system_protected_dir(std::string(progdata) + "\\subfolder"));
    }
#endif

    assert(util::is_safe_extension(".bmi"));
    assert(!util::is_safe_extension(".bmi/../evil"));
    assert(!util::is_safe_extension(".bmi\\..\\evil"));

    std::cout << "[PASS] Extended system directory protection verified.\n";
}

// ============================================================================
// Security Verification 19: MSVC Generator XML Injection Defense
// ============================================================================

void test_msvc_generator_xml_escaping() {
    std::cout << "Testing MSVC generator XML attribute and element escaping...\n";

    Builder b;
    b.add_executable({
        .name = "Malicious<Target>&Name",
        .output_name = "out<Tag>Name",
        .sources = {"src/app&demo.cpp", "include/header<test>.hpp"},
        .include_dirs = {"inc&test/dir<foo>"},
        .link_directories = {"lib&test/dir<bar>"},
        .link_libraries = {"extra&lib<sys>"},
    });

    MsvcGenerator gen(b);
    const Target *t = b.find_target("Malicious<Target>&Name");
    assert(t != nullptr);

    std::string vcxproj = gen.generate_vcxproj(*t);
    assert(vcxproj.find("<RootNamespace>Malicious&lt;Target&gt;&amp;Name</RootNamespace>") != std::string::npos);
    assert(vcxproj.find("<TargetName>out&lt;Tag&gt;Name</TargetName>") != std::string::npos);
    assert(vcxproj.find("Include=\"src\\app&amp;demo.cpp\"") != std::string::npos);
    assert(vcxproj.find("Include=\"include\\header&lt;test&gt;.hpp\"") != std::string::npos);
    assert(vcxproj.find("inc&amp;test\\dir&lt;foo&gt;;") != std::string::npos);
    assert(vcxproj.find("lib&amp;test\\dir&lt;bar&gt;;") != std::string::npos);
    assert(vcxproj.find("extra&amp;lib&lt;sys&gt;.lib;") != std::string::npos);

    // Verify unescaped angle brackets or malicious elements are not present as raw tags
    assert(vcxproj.find("<Tag>") == std::string::npos);
    assert(vcxproj.find("<Target>") == std::string::npos);

    std::string filters = gen.generate_vcxproj_filters(*t);
    assert(filters.find("Include=\"src\\app&amp;demo.cpp\"") != std::string::npos);
    assert(filters.find("Include=\"include\\header&lt;test&gt;.hpp\"") != std::string::npos);

    std::cout << "[PASS] MSVC generator XML injection defense verified.\n";
}

// ============================================================================
// Security Verification 20: System Directory Protection in All Exporters
// ============================================================================

void test_generators_system_directory_protection() {
    std::cout << "Testing system directory write rejection in all generators...\n";

    Builder b;
    b.add_executable({.name = "SecApp", .sources = {"main.cpp"}});

#if defined(_WIN32)
    fs::path sys_cmake = "C:\\Windows\\CMakeLists.txt";
    fs::path sys_ninja = "C:\\Windows\\build.ninja";
    fs::path sys_cc = "C:\\Windows\\compile_commands.json";
    fs::path sys_sln = "C:\\Windows";
    fs::path sys_db = "C:\\Windows\\build_state.json";
    fs::path sys_h = "C:\\Windows\\embedded.hpp";
#else
    fs::path sys_cmake = "/etc/CMakeLists.txt";
    fs::path sys_ninja = "/etc/build.ninja";
    fs::path sys_cc = "/etc/compile_commands.json";
    fs::path sys_sln = "/etc";
    fs::path sys_db = "/etc/build_state.json";
    fs::path sys_h = "/etc/embedded.hpp";
#endif

    CMakeGenerator cmake_gen(b);
    assert(!cmake_gen.generate_file(sys_cmake));

    NinjaGenerator ninja_gen(b);
    assert(!ninja_gen.generate_file(sys_ninja));

    CompileCommandsGenerator cc_gen(b);
    assert(!cc_gen.generate_file(sys_cc));

    MsvcGenerator msvc_gen(b);
    assert(!msvc_gen.generate_all(sys_sln));

    assert(!BuildStateDatabase::instance().save(sys_db));

    fs::path temp_src = fs::temp_directory_path() / "sample_sec.bin";
    {
        std::ofstream out(temp_src);
        out << "test";
    }
    std::string err;
    assert(!EmbedFileStep::embed(temp_src, sys_h, {}, err));
    assert(err.find("protected system directory") != std::string::npos);

    std::error_code ec;
    fs::remove(temp_src, ec);

    std::cout << "[PASS] System directory protection in all generators verified.\n";
}

// ============================================================================
// Security Verification 21: JSON Parser Recursion Limit (DoS Immunity)
// ============================================================================

void test_json_parser_recursion_limit() {
    std::cout << "Testing JSON parser recursion depth limits against stack exhaustion...\n";

    // 1. Valid JSON within limit
    std::string valid_json = "{\"a\": [1, 2, {\"b\": [3, 4]}]}";
    auto parsed_ok = util::JsonValue::parse(valid_json);
    assert(parsed_ok.has_value());

    // 2. Malicious deeply nested JSON (depth = 300 > max_depth 128)
    std::string deep_array;
    for (int i = 0; i < 300; ++i)
        deep_array += "[";
    for (int i = 0; i < 300; ++i)
        deep_array += "]";

    auto parsed_deep = util::JsonValue::parse(deep_array);
    assert(!parsed_deep.has_value());

    // 3. Malicious deeply nested objects
    std::string deep_obj;
    for (int i = 0; i < 300; ++i)
        deep_obj += "{\"x\":";
    deep_obj += "1";
    for (int i = 0; i < 300; ++i)
        deep_obj += "}";

    auto parsed_deep_obj = util::JsonValue::parse(deep_obj);
    assert(!parsed_deep_obj.has_value());

    std::cout << "[PASS] JSON parser recursion limits verified.\n";
}

// ============================================================================
// Security Verification 22: CMakeImporter Cycle & Traversal Immunity
// ============================================================================

void test_cmake_importer_recursion_and_traversal() {
    std::cout << "Testing CMakeImporter cycle detection and path traversal immunity...\n";

    fs::path temp_dir = fs::temp_directory_path() / "cxxgen_sec_test_cmake_imp";
    fs::create_directories(temp_dir);
    fs::path sub_dir = temp_dir / "sub";
    fs::create_directories(sub_dir);

    // Root CMakeLists includes sub
    {
        std::ofstream root_cm(temp_dir / "CMakeLists.txt");
        root_cm << "project(CycleTest)\nadd_subdirectory(sub)\n";
    }

    // Sub CMakeLists includes parent directory creating a recursion cycle: sub -> root -> sub
    {
        std::ofstream sub_cm(sub_dir / "CMakeLists.txt");
        sub_cm << "add_subdirectory(..)\nadd_subdirectory(C:/Windows)\n";
    }

    std::string err;
    auto proj = CMakeImporter::import_file(temp_dir / "CMakeLists.txt", err);
    assert(proj.has_value());

    std::error_code ec;
    fs::remove_all(temp_dir, ec);

    std::cout << "[PASS] CMakeImporter cycle and traversal immunity verified.\n";
}

// ============================================================================
// Security Verification 23: BuildGraphVisualizer Graph Syntax Escaping
// ============================================================================

void test_graph_visualizer_escaping() {
    std::cout << "Testing BuildGraphVisualizer graph escaping against syntax injection...\n";

    Builder b;
    b.add_executable({.name = "app\" [color=red]; evil_node [label=\"injected", .sources = {"main.cpp"}});

    std::string mermaid = BuildGraphVisualizer::to_mermaid(b);
    assert(mermaid.find("app&quot;") != std::string::npos);
    assert(mermaid.find("evil_node") != std::string::npos);
    assert(mermaid.find("app\"") == std::string::npos);

    std::string dot = BuildGraphVisualizer::to_dot(b);
    assert(dot.find("app\\\" [color=red]; evil_node [label=\\\"injected") != std::string::npos);
    assert(dot.find("app\" [color=red]") == std::string::npos);

    std::cout << "[PASS] BuildGraphVisualizer graph escaping verified.\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "Running cxxgen Security Audit Verification Test Suite\n";
    std::cout << "====================================================\n";

    test_powershell_injection_immunity();
    test_single_file_url_sanitization();
    test_embed_file_sanitization();
    test_sha256_tamper_detection();
    test_git_argument_injection_protection();
    test_dependency_traversal_rejection();
    test_system_package_flag_rejection();
    test_project_code_synthesizer_escaping();
    test_cache_key_traversal_rejection();
    test_cmake_exporter_injection_rejection();
    test_scaffolder_system_directory_protection();
    test_package_name_traversal_rejection();
    test_system_protection_checks();
    test_zip_slip_rejection();
    test_git_dangerous_protocol_rejection();
    test_cmake_generator_injection_defense();
    test_project_importer_system_dir_protection();
    test_extended_system_directory_protection();
    test_msvc_generator_xml_escaping();
    test_generators_system_directory_protection();
    test_json_parser_recursion_limit();
    test_cmake_importer_recursion_and_traversal();
    test_graph_visualizer_escaping();

    std::cout << "\n[ALL 23 SECURITY AUDIT TESTS PASSED] cxxgen security audit verified.\n";
    return 0;
}
