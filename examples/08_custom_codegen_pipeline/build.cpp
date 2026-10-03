#if __has_include("cxxgen.hpp")
#include "cxxgen.hpp"
#else
#include "../../include/cxxgen.hpp"
#endif

void build(cxxgen::Builder &b) {
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    // 1. Build the custom code generation tool
    auto &tool = b.add_executable({
        .name = "asset_compiler",
        .sources = {"tools/asset_compiler.cpp"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
        .output_dir = "bin/tools",
    });

    // 2. Define custom command that executes the compiler tool
#if defined(_WIN32)
    std::string tool_path = "bin/tools/asset_compiler.exe";
#else
    std::string tool_path = "bin/tools/asset_compiler";
#endif

    auto codegen_step = b.add_custom_command({
        .description = "Compiling assets/ui_strings.txt to C++ sources",
        .command =
            {
                tool_path,
                "assets/ui_strings.txt",
                "generated",
            },
        .inputs = {"assets/ui_strings.txt"},
        .outputs =
            {
                "generated/ui_strings.hpp",
                "generated/ui_strings.cpp",
            },
    });

    // Codegen requires tool executable to be built first
    codegen_step->depend_on(tool);

    // 3. Define the main game client that compiles generated sources
    auto &client = b.add_executable({
        .name = "game_client",
        .sources =
            {
                "src/main.cpp",
                "generated/ui_strings.cpp",
            },
        .include_dirs = {"generated"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });

    // Client compilation requires codegen step to complete
    client.depend_on(codegen_step);

    b.install_artifact(client);
}

CXXGEN_MAIN(build)
