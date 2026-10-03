#include "cxxgen.hpp"
#include "embedded_cxxgen.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char **argv) {
    cxxgen::terminal::init_console();
    try {
        cxxgen::CliArgs args = cxxgen::parse_cli_args(argc, argv);

        if (args.show_help) {
            cxxgen::print_help(std::cout);
            return 0;
        }

        if (args.show_version) {
            cxxgen::print_version(std::cout);
            return 0;
        }

        cxxgen::GlobalObjectCache::instance().set_enabled(args.global_cache);

        if (args.command == "cache") {
            return cxxgen::GlobalObjectCache::instance().run_cli_cache(args);
        }

        if (args.command == "toolchain") {
            std::cout << "[cxxgen] Toolchain Discovery:\n";
            auto clang = cxxgen::find_clang_toolchain();
            auto gcc = cxxgen::find_gcc_toolchain();
            auto msvc = cxxgen::find_msvc_toolchain();
            auto em = cxxgen::find_emscripten_toolchain();
            std::cout << "  Clang:      " << (clang ? clang->name() + " -> " + clang->compiler_path() : "Not detected")
                      << "\n";
            std::cout << "  GCC:        " << (gcc ? gcc->name() + " -> " + gcc->compiler_path() : "Not detected")
                      << "\n";
            std::cout << "  MSVC:       " << (msvc ? msvc->name() + " -> " + msvc->compiler_path() : "Not detected")
                      << "\n";
            std::cout << "  Emscripten: " << (em ? em->name() + " -> " + em->compiler_path() : "Not detected") << "\n";
            cxxgen::TargetOptions topt;
            topt.triple = args.target_triple;
            topt.sysroot = args.sysroot;
            auto def = cxxgen::find_toolchain(args.compiler.value_or(cxxgen::CompilerFamily::Auto), topt);
            if (def) {
                std::cout << "[cxxgen] Selected toolchain: " << def->name() << "\n";
            } else {
                std::cout << "[cxxgen] No compatible toolchain detected.\n";
            }
            return 0;
        }

        if (args.command == "init") {
            cxxgen::ProjectScaffolderConfig scfg;
            scfg.target_dir = args.work_dir.empty() ? std::filesystem::current_path() : args.work_dir;
            scfg.type = args.init_type;
            scfg.force = args.force;
            scfg.verbose = args.verbose;
            scfg.header_content = cxxgen::embedded::cxxgen_hpp_string_view();
            auto sres = cxxgen::ProjectScaffolder::scaffold(scfg);
            if (!sres.success) {
                std::cerr << sres.error_message << "\n";
                return 1;
            }
            std::cout << "[cxxgen] Initialized new " << args.init_type << " project in " << scfg.target_dir.string()
                      << "\n";
            return 0;
        }

        if (args.command == "fmt") {
            auto fmt_binary = cxxgen::find_clang_format();
            if (!fmt_binary) {
                std::cerr << "cxxgen error: 'clang-format' executable not found on PATH or alongside compiler.\n";
                std::cerr << "Please install clang-format (e.g. via LLVM or Scoop: 'scoop install llvm').\n";
                return 1;
            }

            cxxgen::FormatConfig fcfg;
            fcfg.paths = args.fmt_paths.empty()
                             ? std::vector<std::filesystem::path>{"."}
                             : std::vector<std::filesystem::path>(args.fmt_paths.begin(), args.fmt_paths.end());
            fcfg.check = args.check_only;
            fcfg.verbose = args.verbose;

            auto fres = cxxgen::CodeFormatter::format_all(*fmt_binary, fcfg);
            if (!fres.success) {
                std::cerr << fres.error_message << "\n";
                return 1;
            }
            if (args.check_only) {
                if (fres.mismatched_files.empty()) {
                    std::cout << "[cxxgen fmt] All " << fres.files_scanned << " file(s) are properly formatted.\n";
                    return 0;
                } else {
                    std::cerr << "[cxxgen fmt] " << fres.mismatched_files.size() << " file(s) need formatting:\n";
                    for (const auto &f : fres.mismatched_files) {
                        std::cerr << "  - " << f << "\n";
                    }
                    return 1;
                }
            } else {
                std::cout << "[cxxgen fmt] Formatted " << fres.files_formatted << " file(s) (" << fres.files_scanned
                          << " scanned).\n";
                return 0;
            }
        }

        if (args.command == "doctor") {
            return cxxgen::SystemDoctor::run_doctor(args.verbose);
        }

        if (args.command == "import") {
            std::filesystem::path start_dir = args.work_dir.empty() ? std::filesystem::current_path() : args.work_dir;
            cxxgen::ImportOptions iopts;
            iopts.output_path = args.import_output.empty() ? (start_dir / "build.cpp") : args.import_output;
            iopts.force = args.force;
            iopts.dry_run = args.dry_run;
            iopts.verbose = args.verbose;
            iopts.format = args.import_format;

            std::filesystem::path target_path = start_dir;
            if (!args.import_args.empty()) {
                std::string first = args.import_args[0];
                if (first == "cmake" || first == "msvc") {
                    iopts.format = first;
                    if (args.import_args.size() > 1) {
                        target_path = args.import_args[1];
                    }
                } else {
                    target_path = first;
                }
            }

            auto ires = cxxgen::ProjectImporter::import_project(target_path, iopts);
            if (!ires.success) {
                std::cerr << ires.error_message << "\n";
                return 1;
            }
            if (iopts.dry_run) {
                std::cout << ires.generated_code;
            } else {
                std::cout << "[cxxgen import] Successfully generated " << iopts.output_path.string() << " from "
                          << ires.detected_format << " (" << ires.targets_imported << " target(s) imported).\n";
            }
            return 0;
        }

        std::filesystem::path start_dir = args.work_dir.empty() ? std::filesystem::current_path() : args.work_dir;
        auto script_opt = cxxgen::JitRunner::find_build_script(start_dir);
        if (!script_opt) {
            if (args.command == "lint") {
                auto tidy_binary = cxxgen::find_clang_tidy();
                if (!tidy_binary) {
                    std::cerr << "cxxgen error: 'clang-tidy' executable not found on PATH or alongside compiler.\n";
                    std::cerr << "Please install clang-tidy (e.g. via LLVM or Scoop: 'scoop install llvm').\n";
                    return 1;
                }
                cxxgen::LintConfig lcfg;
                lcfg.paths = args.lint_paths.empty()
                                 ? std::vector<std::filesystem::path>{"."}
                                 : std::vector<std::filesystem::path>(args.lint_paths.begin(), args.lint_paths.end());
                lcfg.fix = args.fix;
                lcfg.verbose = args.verbose;
                lcfg.build_path = start_dir;
                auto lres = cxxgen::CodeLinter::lint_all(*tidy_binary, lcfg);
                if (!lres.report.empty()) {
                    std::cout << lres.report;
                    if (lres.report.back() != '\n')
                        std::cout << "\n";
                }
                if (!lres.success && !lres.error_message.empty()) {
                    std::cerr << lres.error_message << "\n";
                    return 1;
                }
                std::cout << "[cxxgen lint] Scanned " << lres.files_scanned << " file(s): " << lres.warnings_count
                          << " warning(s), " << lres.errors_count << " error(s).\n";
                return (lres.errors_count > 0) ? 1 : 0;
            }
            if (args.command == "check-headers") {
                cxxgen::TargetOptions topt;
                topt.triple = args.target_triple;
                topt.sysroot = args.sysroot;
                auto tc = cxxgen::find_toolchain(args.compiler.value_or(cxxgen::CompilerFamily::Auto), topt);
                if (!tc) {
                    std::cerr << "cxxgen error: no compatible toolchain found on system\n";
                    return 1;
                }
                cxxgen::HeaderCheckConfig hcfg;
                hcfg.work_dir = start_dir;
                hcfg.verbose = args.verbose;
                hcfg.jobs = args.jobs;
                for (const auto &p : args.check_headers_paths) {
                    hcfg.paths.emplace_back(p);
                }
                auto hres = cxxgen::HeaderChecker::check_all(*tc, hcfg);
                if (!hres.report.empty()) {
                    std::cout << hres.report;
                    if (hres.report.back() != '\n')
                        std::cout << "\n";
                }
                return hres.success ? 0 : 1;
            }
            std::cerr << "cxxgen error: could not locate 'build.cpp' in current directory or any parent directory.\n";
            std::cerr << "Run 'cxxgen init' to scaffold a new project.\n";
            return 1;
        }

        auto header_opt = cxxgen::JitRunner::find_cxxgen_header(*script_opt);
        std::filesystem::path header_path;
        if (header_opt) {
            header_path = *header_opt;
        } else {
            std::filesystem::path fallback_dir = script_opt->parent_path() / ".cxxgen" / "include" / "cxxgen";
            std::error_code ec;
            std::filesystem::create_directories(fallback_dir, ec);
            std::filesystem::path fallback_header = fallback_dir / "cxxgen.hpp";
            std::ofstream out(fallback_header, std::ios::trunc | std::ios::binary);
            if (out.is_open()) {
                auto emb = cxxgen::embedded::cxxgen_hpp_string_view();
                out.write(emb.data(), emb.size());
                header_path = fallback_header;
            } else {
                header_path = "include/cxxgen/cxxgen.hpp";
            }
        }

        cxxgen::JitRunnerConfig jcfg;
        jcfg.working_dir = script_opt->parent_path();
        jcfg.build_script = *script_opt;
        jcfg.cxxgen_header = header_path;
        jcfg.verbose = args.verbose;
        jcfg.toolchain = cxxgen::find_toolchain(args.compiler.value_or(cxxgen::CompilerFamily::Auto));

        auto jres = cxxgen::JitRunner::prepare_runner(jcfg);
        if (!jres.success) {
            std::cerr << "cxxgen error: failed to compile build script:\n" << jres.error_message << "\n";
            return jres.exit_code != 0 ? jres.exit_code : 1;
        }

        std::vector<std::string> forward_args;
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-C" || arg == "--work-dir") {
                ++i;
                continue;
            }
            if (arg.rfind("--work-dir=", 0) == 0) {
                continue;
            }
            forward_args.emplace_back(arg);
        }

        int exit_code = cxxgen::JitRunner::execute_runner(jres.runner_binary, forward_args, jcfg.working_dir);
        return exit_code;

    } catch (const std::exception &e) {
        std::cerr << "cxxgen error: " << e.what() << "\n";
        return 1;
    }
}
