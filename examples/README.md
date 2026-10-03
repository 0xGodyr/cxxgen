# cxxgen Examples Catalog

Welcome to the **`cxxgen`** example showcase! This directory contains a progressive, hands-on series of examples ranging from fundamental beginner concepts to advanced master-level multi-stage DAG pipelines and enterprise monorepos.

All examples are 100% self-contained, compile with strict warning flags (`-Wall -Wextra -Werror` / `/W4 /WX`), and run seamlessly with the `cxxgen` CLI or standalone C++20 compilers.

---

## Example Directory & Difficulty Matrix

| # | Example | Level | Key Concepts & APIs Demonstrated |
|:---:|:---|:---:|:---|
| **01** | [**01_hello_world**](01_hello_world/README.md) | **Beginner** | • `b.add_executable`<br>• `b.install_artifact`<br>• C++20 standard specification<br>• Argument forwarding via `--` |
| **02** | [**02_libraries**](02_libraries/README.md) | **Beginner** | • Static libraries (`b.add_static_library`)<br>• Dynamic shared libraries (`b.add_shared_library`)<br>• Symbol visibility export macros<br>• Target linking (`app.link(lib)`) |
| **03** | [**03_precompiled_headers**](03_precompiled_headers/README.md) | **Intermediate** | • `.pch_header` acceleration<br>• Heavy STL compilation speedup<br>• Cross-compiler PCH (MSVC `/Yu`, Clang `-include-pch`, GCC `.gch`) |
| **04** | [**04_unit_testing**](04_unit_testing/README.md) | **Intermediate** | • Automated testing with `b.add_test`<br>• Multi-suite test runner (`cxxgen test`)<br>• Targeting specific test suites<br>• Google Test & Google Mock integration<br>• Regression verification |
| **05** | [**05_dynamic_plugins**](05_dynamic_plugins/README.md) | **Advanced** | • Modular plugins via `b.add_plugin`<br>• Dedicated output directories (`.output_dir`)<br>• Runtime OS loading (`LoadLibrary` / `dlopen`)<br>• C ABI plugin interfaces |
| **06** | [**06_build_system_generators**](06_build_system_generators/README.md) | **Advanced** | • **CMake** export (`cxxgen generate cmake`)<br>• **MSBuild / Visual Studio** export (`cxxgen generate msvc`)<br>• **Ninja** export (`cxxgen generate ninja`)<br>• **compile_commands.json** LSP database |
| **07** | [**07_external_dependencies**](07_external_dependencies/README.md) | **Advanced** | • System package discovery (`b.find_system_package`)<br>• `pkg-config` and platform SDK fallback<br>• Git dependency management (`b.dependency`)<br>• Shallow cloning & tag pinning<br>• Automatic flag and include propagation |
| **08** | [**08_custom_codegen_pipeline**](08_custom_codegen_pipeline/README.md) | **Master** | • Multi-stage DAG code generation<br>• Tool bootstrapping & execution<br>• `b.add_custom_command`<br>• Step dependency edges (`depend_on`)<br>• Incremental asset rebuilding |
| **09** | [**09_enterprise_workspace**](09_enterprise_workspace/README.md) | **Master** | • Enterprise monorepo workspace<br>• Multi-client / daemon / test target composition<br>• Shared compiler options & configurations<br>• Production install layout |
| **10** | [**10_cpp20_modules**](10_cpp20_modules/README.md) | **Master** | • C++20 / C++23 Modules (`export module`, `import`)<br>• P1689 dependency scanning<br>• Two-phase BMI compilation (`.ifc` / `.pcm` / `.gcm`)<br>• `import std;` standard library module support |
| **--** | [**standalone_build**](standalone_build/README.md) | **Beginner** | • Compiling without installing the `cxxgen` CLI<br>• Direct compiler execution of `build.cpp` |

---

## Quick Start: How to Run Any Example

### Using the `cxxgen` CLI
Navigate into any example folder and run:

```bash
# Build the example
cxxgen build

# Run primary target
cxxgen run <target_name>

# Run unit tests (if applicable)
cxxgen test

# Clean artifacts
cxxgen clean
```

### Standalone Compilation (No CLI Required)
Because `cxxgen` is a zero-dependency header-only library (`include/cxxgen.hpp`), every single example can be compiled and executed directly with your system C++ compiler:

```bash
# Windows / Linux / macOS (Clang or GCC)
clang++ -std=c++20 -I../../include build.cpp -o my_build.exe
./my_build.exe build
./my_build.exe run <target_name>

# Windows (MSVC)
cl /std:c++20 /EHsc /I..\..\include build.cpp /Fe:my_build.exe
.\my_build.exe build
.\my_build.exe run <target_name>
```
