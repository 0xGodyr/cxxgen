#include "cxxgen.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

void test_target_dependency_chain() {
    std::cout << "Testing target dependency chain DAG...\n";
    cxxgen::Builder b;

    auto &math = b.add_static_library({
        .name = "math_lib",
        .sources = {"src/math.cpp"},
    });

    auto &core = b.add_static_library({
        .name = "core_lib",
        .sources = {"src/core.cpp"},
    });
    core.link(math);

    auto &app = b.add_executable({
        .name = "app_exe",
        .sources = {"src/main.cpp"},
    });
    app.link(core);

    auto sorted = b.topological_sort("app_exe");
    assert(sorted.size() == 3);
    assert(sorted[0]->name() == "math_lib");
    assert(sorted[1]->name() == "core_lib");
    assert(sorted[2]->name() == "app_exe");

    std::cout << "[PASS] Target dependency chain DAG sorted correctly!\n";
}

void test_custom_steps_and_dag() {
    std::cout << "Testing custom steps and DAG chaining...\n";
    cxxgen::Builder b;

    std::vector<std::string> execution_order;

    auto step_gen = b.add_step("codegen", "Generate source files", [&](cxxgen::ExecutionContext &) {
        execution_order.push_back("codegen");
        return cxxgen::StepResult::ok();
    });

    auto step_compile = b.add_step("compile_assets", "Compile shaders/assets", [&](cxxgen::ExecutionContext &) {
        execution_order.push_back("compile_assets");
        return cxxgen::StepResult::ok();
    });
    step_compile->depend_on(step_gen);

    auto &app = b.add_executable({
        .name = "my_app",
        .sources = {"src/main.cpp"},
    });
    // App depends on compile_assets
    app.build_step()->depend_on(step_compile);

    auto sorted = b.topological_sort("my_app");
    assert(sorted.size() == 3);
    assert(sorted[0]->name() == "codegen");
    assert(sorted[1]->name() == "compile_assets");
    assert(sorted[2]->name() == "my_app");

    // Test execution with dummy toolchain context
    cxxgen::ClangToolchain toolchain;
    cxxgen::ExecutionContext ctx{
        .toolchain = toolchain,
        .verbose = false,
        .dry_run = false,
        .thread_count = 1,
    };

    auto res = b.execute_step("compile_assets", ctx);
    assert(res.success());
    assert(execution_order.size() == 2);
    assert(execution_order[0] == "codegen");
    assert(execution_order[1] == "compile_assets");

    std::cout << "[PASS] Custom steps and DAG chaining verified!\n";
}

void test_cycle_detection_self() {
    std::cout << "Testing cycle detection: self-dependency (A -> A)...\n";
    cxxgen::Builder b;

    auto step_a = b.add_step("A", "Step A");
    step_a->depend_on(step_a);

    bool caught = false;
    try {
        b.topological_sort("A");
    } catch (const cxxgen::CircularDependencyException &ex) {
        caught = true;
        std::cout << "  Caught expected circular dependency: " << ex.what() << "\n";
        assert(!ex.cycle_path().empty());
        assert(ex.cycle_path().front() == "A");
        assert(ex.cycle_path().back() == "A");
    }
    assert(caught);
    std::cout << "[PASS] Self-dependency cycle detected!\n";
}

void test_cycle_detection_two_nodes() {
    std::cout << "Testing cycle detection: 2-node cycle (A -> B -> A)...\n";
    cxxgen::Builder b;

    auto step_a = b.add_step("node_A", "Node A");
    auto step_b = b.add_step("node_B", "Node B");

    step_a->depend_on(step_b);
    step_b->depend_on(step_a);

    bool caught = false;
    try {
        b.topological_sort("node_A");
    } catch (const cxxgen::CircularDependencyException &ex) {
        caught = true;
        std::cout << "  Caught expected circular dependency: " << ex.what() << "\n";
        assert(ex.cycle_path().size() >= 3);
        assert(ex.cycle_path().front() == ex.cycle_path().back());
    }
    assert(caught);
    std::cout << "[PASS] 2-node cycle detected!\n";
}

void test_cycle_detection_three_nodes() {
    std::cout << "Testing cycle detection: 3-node cycle (A -> B -> C -> A)...\n";
    cxxgen::Builder b;

    auto step_a = b.add_step("step_A", "Step A");
    auto step_b = b.add_step("step_B", "Step B");
    auto step_c = b.add_step("step_C", "Step C");

    step_a->depend_on(step_b);
    step_b->depend_on(step_c);
    step_c->depend_on(step_a);

    bool caught = false;
    try {
        b.topological_sort("step_A");
    } catch (const cxxgen::CircularDependencyException &ex) {
        caught = true;
        std::cout << "  Caught expected circular dependency: " << ex.what() << "\n";
        assert(ex.cycle_path().size() == 4);
        assert(ex.cycle_path().front() == ex.cycle_path().back());
    }
    assert(caught);
    std::cout << "[PASS] 3-node cycle detected!\n";
}

void test_cycle_detection_targets() {
    std::cout << "Testing cycle detection with targets (lib1 <-> lib2)...\n";
    cxxgen::Builder b;

    auto &lib1 = b.add_static_library({.name = "lib1", .sources = {"src1.cpp"}});
    auto &lib2 = b.add_static_library({.name = "lib2", .sources = {"src2.cpp"}});

    lib1.link(lib2);
    lib2.link(lib1);

    bool caught = false;
    try {
        b.topological_sort("lib1");
    } catch (const cxxgen::CircularDependencyException &ex) {
        caught = true;
        std::cout << "  Caught expected circular dependency in targets: " << ex.what() << "\n";
        assert(ex.cycle_path().size() >= 3);
    }
    assert(caught);
    std::cout << "[PASS] Target cyclic dependency detected!\n";
}

void test_step_failure_stops_execution() {
    std::cout << "Testing step failure propagation...\n";
    cxxgen::Builder b;

    int run_count = 0;

    auto step1 = b.add_step("failing_step", "This step fails", [&](cxxgen::ExecutionContext &) {
        run_count++;
        return cxxgen::StepResult::fail("Intentional error", 42);
    });

    auto step2 = b.add_step("dependent_step", "Should not run", [&](cxxgen::ExecutionContext &) {
        run_count++;
        return cxxgen::StepResult::ok();
    });
    step2->depend_on(step1);

    cxxgen::ClangToolchain toolchain;
    cxxgen::ExecutionContext ctx{
        .toolchain = toolchain,
    };

    auto res = b.execute_step("dependent_step", ctx);
    assert(!res.success());
    assert(res.exit_code == 42);
    assert(res.error_message == "Intentional error");
    assert(run_count == 1); // Second step was not executed

    std::cout << "[PASS] Step failure correctly aborted downstream execution!\n";
}

void test_diamond_dag_topological_sort() {
    std::cout << "Testing diamond DAG (A depends on B and C, both depend on D)...\n";
    cxxgen::Builder b;

    auto d = b.add_step("D", "Leaf node");
    auto b_step = b.add_step("B", "Branch B");
    auto c_step = b.add_step("C", "Branch C");
    auto a = b.add_step("A", "Root node");

    b_step->depend_on(d);
    c_step->depend_on(d);
    a->depend_on(b_step);
    a->depend_on(c_step);

    auto sorted = b.topological_sort("A");
    assert(sorted.size() == 4);
    // D must be before B and C; B and C must be before A
    size_t pos_d = 0, pos_b = 0, pos_c = 0, pos_a = 0;
    for (size_t i = 0; i < sorted.size(); ++i) {
        if (sorted[i]->name() == "D")
            pos_d = i;
        if (sorted[i]->name() == "B")
            pos_b = i;
        if (sorted[i]->name() == "C")
            pos_c = i;
        if (sorted[i]->name() == "A")
            pos_a = i;
    }
    assert(pos_d < pos_b);
    assert(pos_d < pos_c);
    assert(pos_b < pos_a);
    assert(pos_c < pos_a);

    std::cout << "[PASS] Diamond DAG topologically sorted accurately!\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "Running Story 4.1: DAG Step Model Tests\n";
    std::cout << "===========================================\n";

    test_target_dependency_chain();
    test_custom_steps_and_dag();
    test_cycle_detection_self();
    test_cycle_detection_two_nodes();
    test_cycle_detection_three_nodes();
    test_cycle_detection_targets();
    test_step_failure_stops_execution();
    test_diamond_dag_topological_sort();

    std::cout << "\nAll Story 4.1 Tests Passed Successfully!\n";
    return 0;
}
