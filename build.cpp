#include "include/cxxgen.hpp"

void build(cxxgen::Builder &b) {
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    auto embed_step = b.embed_file("include/cxxgen.hpp", "src/cli/embedded_cxxgen.hpp",
                                   {
                                       .variable_name = "cxxgen_hpp",
                                       .as_constexpr = false,
                                       .null_terminate = false,
                                       .namespace_name = "cxxgen::embedded",
                                   });

    auto cxxgen_cli = b.add_executable({
        .name = "cxxgen",
        .sources = {"src/cli/main.cpp"},
        .headers = {"src/cli/embedded_cxxgen.hpp"},
        .include_dirs = {"include", "src/cli"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });
    cxxgen_cli.depend_on(embed_step);
    b.install_artifact(cxxgen_cli);

    b.set_project_name("cxxgen");
    b.set_package({
        .package_name = "cxxgen",
        .version = "1.0.0",
        .output_dir = "dist",
        .include_dirs = {"include"},
        .artifacts = {"cxxgen"},
        .extra_files = {"README.md", "LICENSE"},
        .format = "zip",
    });

    b.add_test({
        .name = "test_sanitizers",
        .sources = {"tests/test_sanitizers.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_linter",
        .sources = {"tests/test_linter.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_coverage",
        .sources = {"tests/test_coverage.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_import",
        .sources = {"tests/test_import.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_embed_and_headers",
        .sources = {"tests/test_embed_and_headers.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_packaging_and_hygiene",
        .sources = {"tests/test_packaging_and_hygiene.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_cross_and_wasm",
        .sources = {"tests/test_cross_and_wasm.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_ergonomics_and_cache",
        .sources = {"tests/test_ergonomics_and_cache.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_modules",
        .sources = {"tests/test_modules.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_security_audit",
        .sources = {"tests/test_security_audit.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_scaffolder",
        .sources = {"tests/test_scaffolder.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_parallel_scheduler",
        .sources = {"tests/test_parallel_scheduler.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_compiler_diagnostics",
        .sources = {"tests/test_compiler_diagnostics.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });

    b.add_test({
        .name = "test_file_watcher",
        .sources = {"tests/test_file_watcher.cpp"},
        .include_dirs = {"include"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
    });
}

CXXGEN_MAIN(build)
