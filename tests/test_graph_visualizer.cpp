#include "cxxgen.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

void test_graph_ascii() {
    cxxgen::Builder b;

    auto &core_lib = b.add_static_library({
        .name = "core",
        .sources = {"src/core.cpp"},
    });

    auto &ui_lib = b.add_shared_library({
        .name = "ui",
        .sources = {"src/ui.cpp"},
    });
    ui_lib.link(core_lib);

    auto &app = b.add_executable({
        .name = "app",
        .sources = {"src/main.cpp"},
    });
    app.link(ui_lib);
    app.link_system_lib("pthread");

    std::string ascii = cxxgen::BuildGraphVisualizer::to_ascii(b);

    assert(ascii.find("cxxgen graph") != std::string::npos);
    assert(ascii.find("Target: ") != std::string::npos);
    assert(ascii.find("app") != std::string::npos);
    assert(ascii.find("ui") != std::string::npos);
    assert(ascii.find("core") != std::string::npos);
    assert(ascii.find("pthread") != std::string::npos);
    assert(ascii.find("Links:") != std::string::npos);

    std::cout << "[PASS] test_graph_ascii\n";
}

void test_graph_mermaid() {
    cxxgen::Builder b;

    auto &core = b.add_static_library({
        .name = "core_lib",
        .sources = {"src/core.cpp"},
    });

    auto &app = b.add_executable({
        .name = "my_app",
        .sources = {"src/main.cpp"},
    });
    app.link(core);

    std::string mermaid = cxxgen::BuildGraphVisualizer::to_mermaid(b);

    assert(mermaid.find("```mermaid") != std::string::npos);
    assert(mermaid.find("graph TD") != std::string::npos);
    assert(mermaid.find("my_app") != std::string::npos);
    assert(mermaid.find("core_lib") != std::string::npos);
    assert(mermaid.find("my_app --> core_lib") != std::string::npos);
    assert(mermaid.find("```") != std::string::npos);

    std::cout << "[PASS] test_graph_mermaid\n";
}

void test_graph_dot() {
    cxxgen::Builder b;

    auto &math = b.add_static_library({
        .name = "math",
        .sources = {"src/math.cpp"},
    });

    auto &sim = b.add_executable({
        .name = "sim",
        .sources = {"src/sim.cpp"},
    });
    sim.link(math);

    std::string dot = cxxgen::BuildGraphVisualizer::to_dot(b);

    assert(dot.find("digraph G {") != std::string::npos);
    assert(dot.find("\"sim\"") != std::string::npos);
    assert(dot.find("\"math\"") != std::string::npos);
    assert(dot.find("\"sim\" -> \"math\";") != std::string::npos);
    assert(dot.find("}") != std::string::npos);

    std::cout << "[PASS] test_graph_dot\n";
}

void test_graph_filtering() {
    cxxgen::Builder b;

    auto &base = b.add_static_library({
        .name = "base",
        .sources = {"src/base.cpp"},
    });

    auto &mid = b.add_static_library({
        .name = "mid",
        .sources = {"src/mid.cpp"},
    });
    mid.link(base);

    auto &top = b.add_executable({
        .name = "top",
        .sources = {"src/top.cpp"},
    });
    top.link(mid);

    auto &isolated = b.add_executable({
        .name = "isolated",
        .sources = {"src/isolated.cpp"},
    });
    (void)isolated;

    // Filter to "mid": should contain mid, base (upstream), and top (downstream), but NOT isolated
    auto filtered = cxxgen::BuildGraphVisualizer::get_filtered_targets(b, "mid");
    assert(filtered.find("mid") != filtered.end());
    assert(filtered.find("base") != filtered.end());
    assert(filtered.find("top") != filtered.end());
    assert(filtered.find("isolated") == filtered.end());

    // Mermaid output filtered to "mid"
    std::string mermaid = cxxgen::BuildGraphVisualizer::to_mermaid(b, "mid");
    assert(mermaid.find("mid") != std::string::npos);
    assert(mermaid.find("base") != std::string::npos);
    assert(mermaid.find("top") != std::string::npos);
    assert(mermaid.find("isolated") == std::string::npos);

    std::cout << "[PASS] test_graph_filtering\n";
}

void test_graph_run_and_errors() {
    cxxgen::Builder b;
    b.add_executable({.name = "tool", .sources = {"src/tool.cpp"}});

    std::ostringstream oss;
    int code_ok = cxxgen::BuildGraphVisualizer::run(b, "ascii", "", oss);
    assert(code_ok == 0);
    assert(oss.str().find("tool") != std::string::npos);

    std::ostringstream oss_mermaid;
    code_ok = cxxgen::BuildGraphVisualizer::run(b, "mermaid", "tool", oss_mermaid);
    assert(code_ok == 0);
    assert(oss_mermaid.str().find("graph TD") != std::string::npos);

    // Non-existent target
    int code_err = cxxgen::BuildGraphVisualizer::run(b, "ascii", "non_existent_target", oss);
    assert(code_err == 1);

    std::cout << "[PASS] test_graph_run_and_errors\n";
}

void test_graph_cli_parsing() {
    {
        const char *argv[] = {"cxxgen", "graph"};
        auto args = cxxgen::parse_cli_args(2, const_cast<char **>(argv));
        assert(args.command == "graph");
        assert(args.graph_format == "ascii");
        assert(args.graph_target.empty());
    }
    {
        const char *argv[] = {"cxxgen", "graph", "--format=mermaid", "--target=myapp"};
        auto args = cxxgen::parse_cli_args(4, const_cast<char **>(argv));
        assert(args.command == "graph");
        assert(args.graph_format == "mermaid");
        assert(args.graph_target == "myapp");
    }
    {
        const char *argv[] = {"cxxgen", "graph", "--format", "dot", "--target", "core"};
        auto args = cxxgen::parse_cli_args(6, const_cast<char **>(argv));
        assert(args.command == "graph");
        assert(args.graph_format == "dot");
        assert(args.graph_target == "core");
    }
    {
        const char *argv[] = {"cxxgen", "graph", "engine"};
        auto args = cxxgen::parse_cli_args(3, const_cast<char **>(argv));
        assert(args.command == "graph");
        assert(args.target_or_step == "engine");
    }
    std::cout << "[PASS] test_graph_cli_parsing\n";
}

void test_graph_hierarchical_dag() {
    cxxgen::Builder b;

    auto &exe = b.add_executable({
        .name = "attempt1",
        .sources = {"src/main.cpp"},
    });
    b.install_artifact(exe);

    auto run_cmd = b.add_run_artifact(exe);
    auto &run_step = b.step("run", "Run the app");
    run_step.depend_on(run_cmd);

    std::string out = cxxgen::BuildGraphVisualizer::to_tree(b, "", /*force_ascii=*/true);

    // Verify header and sections
    assert(out.find("cxxgen graph - Build Dependency Graph") != std::string::npos);
    assert(out.find("Targets:") != std::string::npos);
    assert(out.find("Target: attempt1 (Executable)") != std::string::npos);
    assert(out.find("Execution Steps (DAG):") != std::string::npos);

    // Verify install step depends on attempt1
    assert(out.find("Step: install - Install build artifacts") != std::string::npos);

    // Verify run step depends on run:attempt1 which depends on attempt1
    assert(out.find("Step: run - Run the app") != std::string::npos);
    assert(out.find("Step: run:attempt1 - Run target attempt1") != std::string::npos);

    // Verify pure ASCII glyphs used
    assert(out.find("\\-- ") != std::string::npos || out.find("|-- ") != std::string::npos);

    std::cout << "[PASS] test_graph_hierarchical_dag\n";
}

void test_graph_ascii_pure_fallback() {
    cxxgen::Builder b;
    b.add_executable({.name = "testapp", .sources = {"src/test.cpp"}});

    std::string out = cxxgen::BuildGraphVisualizer::to_tree(b, "", /*force_ascii=*/true);
    // In pure ASCII mode, none of the UTF-8 box characters should be present
    assert(out.find("\xe2\x94\x94") == std::string::npos); // └──
    assert(out.find("\xe2\x94\x9c") == std::string::npos); // ├──
    assert(out.find("\xe2\x94\x82") == std::string::npos); // │

    std::cout << "[PASS] test_graph_ascii_pure_fallback\n";
}

int main() {
    std::cout << "Running test_graph_visualizer...\n";
    test_graph_ascii();
    test_graph_mermaid();
    test_graph_dot();
    test_graph_filtering();
    test_graph_run_and_errors();
    test_graph_cli_parsing();
    test_graph_hierarchical_dag();
    test_graph_ascii_pure_fallback();
    std::cout << "All graph visualizer tests passed successfully!\n";
    return 0;
}
