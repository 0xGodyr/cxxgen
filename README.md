# `cxxgen` — Modern C++ Generator & Build System

<p align="center">
  <strong>Fast, elegant, and native build ergonomics for modern C++ (C++20/C++23).</strong><br>
  <em>Single-header core • Zero dependencies • Universal project generators • High-performance DAG engine</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20%20%2F%2023-blue.svg" alt="C++20 / C++23">
  <img src="https://img.shields.io/badge/Dependencies-Zero%20(Stdlib%20Only)-brightgreen.svg" alt="Zero Dependencies">
  <img src="https://img.shields.io/badge/Platforms-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg" alt="Cross-Platform">
  <img src="https://img.shields.io/badge/Compilers-MSVC%20%7C%20Clang%20%7C%20GCC%20%7C%20AppleClang-orange.svg" alt="Compilers">
  <img src="https://img.shields.io/badge/Release-v1.0.0-success.svg" alt="Release v1.0.0">
</p>

---

## Why `cxxgen`?

Traditional C++ build systems force you to learn clunky domain-specific languages (DSLs) like CMake or Meson, debug cryptic syntax errors without IDE autocomplete, and install heavyweight external toolchains.

**`cxxgen` changes this entirely:**
* **Write Build Logic in C++**: Define your entire build graph in pure, strongly-typed C++20 (`build.cpp`). Benefit from compiler checks, IDE auto-completion, refactoring tools, and full language power.
* **Single-Header Invariant**: `include/cxxgen.hpp` is 100% self-contained standard C++ with **zero external dependencies**.
* **Zero-Install Fallback**: Anyone with a standard C++20 compiler can build your project without installing `cxxgen`:
  ```bash
  clang++ -std=c++20 -I<path-to-include> build.cpp -o build && ./build
  ```
* **Universal Meta-Build Generator**: Export your `build.cpp` on demand into native **CMake** (`CMakeLists.txt`), **Visual Studio / MSBuild** (`.sln` / `.vcxproj`), **Ninja** (`build.ninja`), and **Clang LSP Databases** (`compile_commands.json`).
* **Bidirectional Migration Engine**: Seamlessly import existing **CMake** projects (`CMakeLists.txt`) and **Visual Studio Solutions** (`.sln` / `.vcxproj`) into clean, idiomatic `build.cpp` scripts via `cxxgen import`.
* **Binary Asset Embedder**: Embed arbitrary binary assets (fonts, icons, shaders, JSON configs) directly into C++ headers as byte arrays or `constexpr std::array<uint8_t, N>` via `b.embed_file`.
* **Header Self-Containment Linter**: Compile project headers in isolation via `cxxgen check-headers` (`-fsyntax-only` / `/Zs`) to catch missing `#include` directives before release.
* **High-Performance DAG & Incremental Cache**: Content-hashed SHA-256 caching, automatic depfile header tracking (`-MMD`/`-MF` and MSVC `/showIncludes`), and a multi-threaded parallel scheduler.
* **Deterministic Dependency Lockfile (`cxxgen.lock`)**: Reproducible builds across machines with SHA-256 tree hashing, `--locked`/`--frozen` mode, and `cxxgen update`.
* **Binary Packaging & Release Bundler**: Bundle executables, libraries, public headers, and relocatable CMake configs into distributable `.zip` or `.tar.gz` packages via `cxxgen package`.
* **CMake Package Config Exporter**: Export `<Package>Config.cmake`, `<Package>Targets.cmake`, and version files so external CMake projects can consume your libraries with `find_package(<Package> REQUIRED)`.
* **JIT Build Script Compilation**: The `cxxgen` CLI caches the compiled `build.cpp` binary and invokes cached runs in **< 15ms**.
* **Full Artifact Ecosystem**: Executables, Static Libraries (`.lib`/`.a`), Shared Libraries / DLLs (`.dll`/`.so`/`.dylib` + import libraries), dynamically loadable Plugins, and Precompiled Headers (PCH).
* **Hardened Security & Process Isolation**: Robust defensive architecture eliminating shell command injection via OS environment blocks, preventing subprocess argument injection (`--` positional delimiters), blocking path traversal / Zip-slip exploits, and escaping C++ string literals during code synthesis.

---

## Quick Example (`build.cpp`)

```cpp
#include "cxxgen.hpp"

void build(cxxgen::Builder &b) {
    auto target   = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    // 1. Static core library
    auto &core = b.add_static_library({
        .name         = "engine",
        .sources      = {"src/engine.cpp"},
        .include_dirs = {"include"},
        .cpp_version  = cxxgen::CppVersion::Cpp20,
        .target       = target,
        .optimize     = optimize.mode,
    });

    // 2. Executable application linking core library
    auto &app = b.add_executable({
        .name         = "app",
        .sources      = {"src/main.cpp"},
        .include_dirs = {"include"},
        .cpp_version  = cxxgen::CppVersion::Cpp20,
        .target       = target,
        .optimize     = optimize.mode,
    });
    app.link(core);
    b.install_artifact(app);

    // 3. Automated unit test suite
    auto &test = b.add_test({
        .name         = "test_engine",
        .sources      = {"tests/test_engine.cpp"},
        .include_dirs = {"include"},
        .cpp_version  = cxxgen::CppVersion::Cpp20,
        .target       = target,
        .optimize     = optimize.mode,
    });
    test.link(core);
}

CXXGEN_MAIN(build)
```

---

## Installation & Getting Started

### 1. Build the Native CLI (`cxxgen`)
`cxxgen` builds itself! Simply compile the self-hosting build script using any standard C++20 compiler:

#### Windows (Clang / GCC / MSVC)
```powershell
# Using Clang or GCC
clang++ -std=c++20 -Iinclude build.cpp -o build_runner.exe
.\build_runner.exe build
# The CLI executable will be produced in bin\cxxgen.exe
```

#### Linux / macOS
```bash
clang++ -std=c++20 -Iinclude build.cpp -o build_runner
./build_runner build
# The CLI executable will be produced in bin/cxxgen
```

Add `bin/` to your system `PATH` to use `cxxgen` globally.

### 2. Scaffold a New Project
Create a complete C++ project in seconds:
```bash
mkdir my_project && cd my_project

# Scaffold an executable project with internal mathlib and Google Test / Google Mock
cxxgen init --type=exe

# Or scaffold a reusable library or dynamic plugin
cxxgen init --type=lib
cxxgen init --type=plugin
```

When you initialize an executable project (`cxxgen init --type=exe`), `cxxgen` generates a production-ready project structure:

```text
my_project/
├── build.cpp                     # Build script with Google Test & GMock v1.18.0 dependency
├── src/
│   ├── main.cpp                  # Application entrypoint using [[maybe_unused]]
│   └── mathlib/
│       ├── math.hpp              # Internal library public API (add, subtract, factorial, is_prime, ...)
│       └── math.cpp              # Library implementation
├── tests/
│   └── test_math.cpp             # Google Test assertions & GMock interface mocking
├── .gitignore                    # Sane defaults for build artifacts and temporary files
├── .clang-format                 # Modern C++ formatting rules
├── .clang-tidy                   # Static analysis rules
└── .clangd                       # Clangd LSP compilation database configuration
```

Key scaffolding features:
* **Modern Argument Attributes**: `src/main.cpp` uses modern C++ attributes (`int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)`).
* **Built-in Google Test & Google Mock**: `build.cpp` automatically declares the `googletest` dependency (pinned to `v1.18.0`), compiles a combined `gmock_main` static library (`gtest-all.cc`, `gmock-all.cc`, `gmock_main.cc`), and wires up the `test_math` test target.
* **Instant Out-of-the-Box Verification**: Running `cxxgen test` right after initialization executes unit tests and GMock mocks immediately with zero extra configuration.

### 3. Build, Run, and Test
```bash
# Build all install artifacts (and automatically emit/update compile_commands.json)
cxxgen build

# Run the executable (and pass arguments)
cxxgen run my_project -- --verbose

# Run all registered unit tests (including Google Test / GMock suites)
cxxgen test

# Format all C/C++ files in-place (like 'zig fmt')
cxxgen fmt .
```

---

## CLI Command Reference

`cxxgen` provides a rich, intuitive command-line interface with subcommands and flags:

| Command | Description | Example |
|:---|:---|:---|
| `cxxgen build` | Compiles/links default targets and auto-emits `compile_commands.json` | `cxxgen build -O ReleaseFast -j 8` |
| `cxxgen run <target>` | Builds target (if needed) and executes it with arguments | `cxxgen run app -- foo bar` |
| `cxxgen test [target]` | Discovers, builds, and executes test targets | `cxxgen test` or `cxxgen test test_calc` |
| `cxxgen import [fmt] [path]` | Imports CMakeLists.txt or MSVC solution/project to `build.cpp` | `cxxgen import` or `cxxgen import cmake --dry-run` |
| `cxxgen generate <fmt>` | Generates foreign build scripts (`cmake`, `msvc`, `ninja`, `compile_commands`) | `cxxgen generate cmake` |
| `cxxgen init [--type=...]` | Scaffolds a starter project (`exe`, `lib`, `plugin`) with tooling configs | `cxxgen init --type=lib` |
| `cxxgen fmt [paths...]` | Formats C/C++ files using `clang-format` (`--check` mode supported) | `cxxgen fmt .` or `cxxgen fmt --check src/` |
| `cxxgen lint [paths...]` | Runs `clang-tidy` static analysis checks (`--fix` supported) | `cxxgen lint .` or `cxxgen lint --fix src/` |
| `cxxgen check-headers [paths...]` | Lints headers in isolation with fast syntax checking for missing `#include`s | `cxxgen check-headers include/` |
| `cxxgen package [fmt]` | Bundles binaries, headers, and CMake config into `.zip` or `.tar.gz` | `cxxgen package` or `cxxgen package --format=zip` |
| `cxxgen update [pkg...]` | Updates locked dependency versions and tree hashes in `cxxgen.lock` | `cxxgen update` or `cxxgen update fmt` |
| `cxxgen cache [action] [arg]` | Inspects, clears, or configures global object cache (`stats`, `clean`, `max-size`) | `cxxgen cache stats` or `cxxgen cache max-size 10G` |
| `cxxgen watch` | Continuously monitors source files and rebuilds incrementally on save | `cxxgen watch` |
| `cxxgen doctor` | Inspects development environment, compilers, SDKs, and tools | `cxxgen doctor` |
| `cxxgen graph` | Visualizes target/step DAG (`ascii`, `mermaid`, `dot`, `--target=...`) | `cxxgen graph --format=mermaid` |
| `cxxgen toolchain [cmd]` | Discovers and inspects installed compilers (`detect` or `list`) | `cxxgen toolchain detect` |
| `cxxgen clean` | Removes compiled artifacts (`.cxxgen/`, `bin/`, `lib/`, `obj/`) | `cxxgen clean` |

### Common Flags
* `-O / --optimize <mode>`: Sets optimization level (`Debug`, `ReleaseFast`, `ReleaseSafe`, `ReleaseSmall`).
* `-j / --jobs <count>`: Number of parallel compilation threads (defaults to logical CPU cores).
* `--compiler <name>`: Explicitly select compiler family (`clang`, `gcc`, `msvc`, `emscripten`).
* `--target <triple>`: Cross-compilation target triple (e.g. `wasm32`, `aarch64-linux-gnu`).
* `--sysroot <path>`: Sysroot directory for cross-compilation headers and libraries.
* `--global-cache / --cache`: Enables global cross-project compilation cache (default: enabled).
* `--no-cache / --no-global-cache`: Disables global cross-project compilation cache.
* `--std-module`: Enables C++23 standard library module (`import std;`) support and auto-linking.
* `-v / --verbose`: Echoes raw compiler and linker commands to the console.
* `--locked / --frozen`: Enforces strict lockfile integrity; fails if `cxxgen.lock` would change or is missing entries.
* `--dry-run`: Previews synthesized `build.cpp` on `stdout` without writing to disk (`import`).
* `--force`: Forces overwriting existing files (`init`, `import`).
* `--out <path>`: Specifies custom output path (`import`, `package`).
* `--format <fmt>`: Specifies output format (`mermaid`, `dot`, `ascii` for `graph`; `zip`, `tar.gz` for `package`).
* `--check`: Checks formatting without writing to disk (for `fmt`).
* `--sanitize=<list>`: Enables sanitizers (`address`, `undefined`, `leak`, `thread`).
* `--coverage`: Enables code coverage profiling and instrumentation.
* `--fix`: Automatically applies fixes for `lint`.
* `--profile[=<file>]`: Records Chrome Tracing profile (`build_trace.json`) and prints top slowest steps.
* `--no-color`: Disables colored terminal output (also respects `NO_COLOR` environment variable).
* `-C / --directory / --work-dir <dir>`: Execute command in a specific working directory.

---

## Universal Project Exporters

Need to integrate with external CI/CD pipelines, IDEs, or legacy build environments? `cxxgen` can export your build graph to any foreign format with a single command:

### 1. CMake Generator (`cxxgen generate cmake`)
Produces a standard, modern `CMakeLists.txt` complete with target definitions, precompiled headers, include paths, and link libraries.

### 2. MSBuild / Visual Studio Generator (`cxxgen generate msvc`)
Produces native Visual Studio Solution (`.sln`) and Project (`.vcxproj`) files with Debug/Release configurations for x64/ARM64.

### 3. Ninja Generator (`cxxgen generate ninja`)
Produces an optimized `build.ninja` file for standalone Ninja invocation.

### 4. Compilation Database (`cxxgen generate compile_commands`)
Produces `compile_commands.json` enabling immediate zero-configuration IDE diagnostics for **clangd**, **VS Code**, **Neovim LSP**, and **CLion**. Note that `compile_commands.json` is also generated automatically during standard `cxxgen build` and `cxxgen run` invocations.

---

## Bidirectional Migration: Project Importers (`cxxgen import`)

Migrating existing codebases to `cxxgen` is fast and fully automated. `cxxgen import` reverse-engineers foreign build configurations and synthesizes clean, idiomatic modern C++20 `build.cpp` scripts with zero manual effort:

### 1. Universal Auto-Discovery
Run `cxxgen import` without arguments in any foreign repository root:
```bash
cxxgen import
```
`cxxgen` automatically scans for `CMakeLists.txt`, `*.sln`, or `*.vcxproj` files, resolves project targets and dependencies, and writes `build.cpp`.

### 2. CMake Importer (`cxxgen import cmake`)
Translates `CMakeLists.txt` files (including nested `add_subdirectory` hierarchies) into native `build.cpp` declarations:
```bash
# Preview synthesized code on stdout without writing to disk
cxxgen import cmake path/to/cmake_project --dry-run

# Convert project and write to a custom script location
cxxgen import cmake path/to/cmake_project --out build.cpp
```
* Preserves target types (`add_executable`, `add_static_library`, `add_shared_library`, `add_plugin`, `add_interface_library`, `add_test`).
* Automatically unwraps generator expressions (such as `$<BUILD_INTERFACE:...>`).
* Normalizes include directories, compile definitions, and source paths relative to repository root.

### 3. Visual Studio / MSBuild Importer (`cxxgen import msvc`)
Converts legacy Visual Studio solutions (`.sln`) and MSBuild XML projects (`.vcxproj`) into cross-platform C++ build scripts:
```bash
cxxgen import msvc path/to/solution.sln
cxxgen import msvc path/to/App.vcxproj
```
* Parses XML configurations, source lists, include search paths, and precompiled headers (`.pch_header`).
* Extracts inter-project GUID dependencies and maps them to `.link()` relationships.
* Strips Windows platform boilerplate macros (`WIN32`, `_WINDOWS`, `_CONSOLE`, `_DEBUG`) to keep definitions concise and cross-platform ready.

### 4. Safety & Preview Flags
* `--dry-run`: Emits synthesized C++ code to `stdout` for inspection without creating or modifying files.
* `--force`: Overwrites destination file if `build.cpp` already exists (by default, `cxxgen import` protects against accidental overwrites).
* `--out <path>`: Specifies custom output destination path (defaults to `build.cpp`).

---

## Binary Asset Embedding & Header Containment

### 1. Build-Time Binary Asset Embedder (`b.embed_file`)
Embed arbitrary binary files (fonts, icons, shaders, JSON configs, binary blobs) directly into C++ headers with automatic incremental DAG change detection:
```cpp
// Embed asset as a constexpr byte array in a C++ header
b.embed_file("assets/font.ttf", "include/embedded/font.h", {
    .variable_name = "g_font_data",
    .as_constexpr = true,
    .null_terminate = false,
    .namespace_name = "assets",
});

auto &app = b.add_executable({
    .name = "app",
    .sources = {"src/main.cpp"},
    .include_dirs = {"include"},
});
// Target automatically tracks and depends on the embedded header step
```
Generated header provides:
* `inline constexpr std::array<uint8_t, N> <variable_name>`
* `inline std::string_view <variable_name>_string_view()`
* `inline const char* <variable_name>_c_str()` (when `null_terminate = true`)

### 2. Header Self-Containment Linter (`cxxgen check-headers`)
Ensure every project header compiles cleanly on its own without relying on accidental include order in translation units:
```bash
# Check all headers in standard project include directories
cxxgen check-headers

# Check specific headers or directories
cxxgen check-headers include/ src/
```
* Compiles each header in parallel across worker threads using ephemeral translation units (`#include "<header>"`).
* Uses fast syntax-only checking (`-fsyntax-only` for Clang/GCC, `/Zs` for MSVC) with strict warnings (`-Werror` / `/WX`) for maximum speed (~15ms per header) with zero object code generation overhead.
* Accurately reports missing `#include` directives and broken forward declarations.

---

## Modern C++20 / C++23 Modules Architecture

`cxxgen` features full, first-class native support for C++20 and C++23 Named Modules, Module Partitions, and the Standard Library Module (`import std;`).

### 1. Zero-Configuration Dependency Scanning (P1689R5)
Translation units containing `export module <name>;` and `import <name>;` are automatically scanned before compilation:
* **Vendor & Built-in Scanning**: Uses vendor tools (`clang-scan-deps` on Clang, `/scanDependencies` on MSVC) when available, and falls back to a zero-dependency built-in regex scanner with comment and string literal stripping.
* **Standard P1689R5 JSON**: Emits standard module dependency graphs compliant with ISO C++ P1689R5.
* **Dynamic Topological DAG Sequencing**: Automatically builds and topologically sorts the module dependency graph, detecting and reporting circular module dependencies (`CircularDependencyException`).

### 2. Two-Phase Compilation & BMI Caching (`.ifc` / `.pcm`)
* **Phase 1: Built Module Interface (BMI) Compilation**: Module interfaces (`.cppm`, `.ixx`) are compiled into binary interfaces (`.ifc` for MSVC, `.pcm` for Clang) before downstream translation units compile.
* **Phase 2: Source Compilation & Linking**: Downstream implementation units and consumers are passed reference flags (`/reference <name>=<bmi>` for MSVC, `-fmodule-file=<name>=<bmi>` for Clang) and linked with the compiled module object files.
* **Persistent BMI Cache**: Built module interfaces are cached in `~/.cxxgen/cache/bmi/` indexed by source content hash, compiler version, compilation flags, and dependent module BMI hashes.

### 3. C++23 Standard Library Module (`import std;`)
Replace `#include <vector>`, `<iostream>`, `<algorithm>`, and heavy STL headers with a single instant `import std;`:
```cpp
// Enable standard library module in build.cpp
auto &app = b.add_executable({
    .name = "app",
    .sources = {"src/main.cpp"},
    .cpp_version = cxxgen::CppVersion::Cpp23,
    .enable_std_module = true,
});

// Or enable project-wide via Builder or CLI:
b.enable_std_module(); // in build.cpp
// or via CLI:
// cxxgen build --std-module
```
In your C++ source code:
```cpp
import std;

int main() {
    std::println("Hello, Modern C++23 Modules!");
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    return 0;
}
```
* **Auto-Discovery**: Automatically discovers host compiler's `std.ixx` (MSVC) or `std.cppm` (Clang libc++).
* **Global Precompilation**: Compiles the standard library module interface once and caches `std.ifc` / `std.pcm` and `std.obj` in `~/.cxxgen/cache/bmi/std/` across all projects.

---

## Cross-Compilation & WebAssembly (Wasm)

### 1. WebAssembly with Emscripten (`emcc` / `em++`)
Compile C++ applications to WebAssembly for execution in web browsers or Node.js runtimes:

```bash
# Compile to WebAssembly with automatic Emscripten SDK detection
cxxgen build --compiler=emcc

# Or by target triple
cxxgen build --target=wasm32
```

In `build.cpp`, declare web targets with standard or custom output shells:
```cpp
auto &web_game = b.add_executable({
    .name = "index.html", // or "game.wasm" / "worker.js"
    .sources = {"src/main.cpp", "src/render.cpp"},
    .include_dirs = {"include"},
});

// Emscripten flags are forwarded directly to compiler and linker
web_game.config().compile_options.push_back("-sUSE_WEBGL2=1");
web_game.config().link_options.push_back("-sALLOW_MEMORY_GROWTH=1");
web_game.config().link_options.push_back("-sEXPORTED_FUNCTIONS=['_main','_step']");
```

### 2. Multi-Architecture Target Profiles (`--target` & `--sysroot`)
Cross-compile effortlessly for Linux ARM64 servers, Raspberry Pi, RISC-V, or bare-metal embedded microcontrollers:

```bash
# Cross-compile for ARM64 Linux with custom sysroot
cxxgen build --target=aarch64-linux-gnu --sysroot=/opt/sysroots/aarch64

# Cross-compile for bare-metal Cortex-M embedded target
cxxgen build --target=arm-none-eabi --sysroot=/opt/arm-none-eabi
```

* **Automatic Architecture Defines**: `cxxgen` automatically injects target preprocessor definitions (`__wasm__`, `__wasm32__`, `__aarch64__`, `__arm__`, `__riscv`, `__x86_64__`).
* **Sysroot Library Resolution**: Automatically passes `--sysroot=<path>` and configures `-L<sysroot>/lib` and `-L<sysroot>/usr/lib` search paths.
* **Declarative API in `build.cpp`**:
  ```cpp
  // Project-wide target triple and sysroot
  b.set_target_triple("aarch64-linux-gnu");
  b.set_sysroot("/opt/sysroots/aarch64");

  // Or configure per target
  auto &worker = b.add_executable({.name = "worker", .sources = {"src/worker.cpp"}});
  worker.set_target_triple("wasm32");
  ```

---

## Package & Dependency Management

### 1. Remote Git & HTTP / Archive Dependencies
Fetch, cache, and import remote repositories or release archives directly in `build.cpp`:

```cpp
// Git repository dependency with tag or commit SHA
auto fmt = b.dependency("fmt", {
    .git = "https://github.com/fmtlib/fmt.git",
    .tag = "10.2.1",
});

// Direct HTTP / HTTPS zip or tarball with SHA-256 integrity validation
auto sqlite = b.dependency("sqlite", {
    .url = "https://www.sqlite.org/2024/sqlite-amalgamation-3450200.zip",
    .sha256 = "8d3e23b126...",
});
```

* Archives (`.zip`, `.tar.gz`) are verified against declared SHA-256 hashes prior to extraction.
* Extracted dependencies are persistently cached in `~/.cxxgen/pkg/<hash>` and their exported include paths and targets are seamlessly available.

### 2. Deterministic Lockfile (`cxxgen.lock`)
`cxxgen` automatically tracks remote dependencies in a deterministic `cxxgen.lock` file recording exact resolved commit SHAs, source URLs, and tree content hashes.

* **Strict CI / Frozen Mode**: Run builds with `--locked` or `--frozen`:
  ```bash
  cxxgen build --locked
  ```
  Fails immediately if `cxxgen.lock` is missing dependency records or if upstream resolutions have changed or been tampered with.
* **Controlled Upgrades**: Update dependencies deliberately using `cxxgen update`:
  ```bash
  # Update all dependencies
  cxxgen update

  # Update a specific dependency
  cxxgen update fmt
  ```

### 3. Binary Packaging & Release Bundles (`cxxgen package`)
Declare package metadata in `build.cpp` and bundle your application or library with a single command:

```cpp
b.set_package({
    .package_name = "AwesomeEngine",
    .version      = "2.4.0",
    .output_dir   = "dist",
    .include_dirs = {"include"},
    .artifacts    = {&engine_lib, &tools_exe},
    .extra_files  = {"README.md", "LICENSE"},
    .format       = cxxgen::PackageFormat::Zip, // or TarGz
});
```

Execute `cxxgen package` to build all targets and generate a clean release archive:
```bash
cxxgen package
# Creates dist/AwesomeEngine-2.4.0-windows-x64.zip
```
The resulting archive contains:
* `bin/` — executable binaries and dynamic DLLs
* `lib/` — static archives (`.lib` / `.a`) and import libraries
* `include/` — public header trees
* `lib/cmake/<Package>/` — relocatable CMake package configuration files

### 4. Relocatable CMake Config Exporter (`<Package>Config.cmake`)
Installed and packaged libraries automatically generate modern relocatable CMake configuration files:
* `<Package>Config.cmake`
* `<Package>ConfigVersion.cmake` (with exact semantic version compatibility checking)
* `<Package>Targets.cmake` (defining `add_library(<Package>::<Target> IMPORTED)`)

Foreign CMake projects can consume `cxxgen`-packaged libraries with standard CMake syntax:
```cmake
list(APPEND CMAKE_PREFIX_PATH "path/to/dist/AwesomeEngine")
find_package(AwesomeEngine 2.4 REQUIRED)

target_link_libraries(MyCMakeApp PRIVATE AwesomeEngine::engine)
```

### 5. System Package Discovery
Discover installed system libraries via `pkg-config` or standard platform SDK paths:
```cpp
auto zlib = b.find_system_package("zlib", {
    .required = false,
    .install_hint = "Install via scoop (Windows) or apt install zlib1g-dev (Linux)",
});

if (zlib) {
    app.link(zlib);
    app.config().defines.push_back("HAVE_ZLIB");
}
```

### 6. System Libraries & Platform Dependencies (`add_system_library`)
For direct toolchain/OS runtime libraries (such as POSIX `-lpthread`, `-ldl`, `-lm` on Linux/macOS, or Windows SDK system libraries like `ws2_32`), use the `add_system_library` convenience method:
```cpp
auto &test_math = b.add_test({
    .name = "test_math",
    .sources = {"tests/test_math.cpp"},
});

#if !defined(_WIN32)
    test_math.add_system_library("pthread");
#endif
```

### 7. Global Cross-Project Object Cache (`ccache` / `sccache` Built-in)
`cxxgen` features a built-in content-addressable compilation cache stored in `~/.cxxgen/cache/`:
* **Direct Mode**: Fast SHA-256 source hash manifests enable **sub-millisecond (< 1ms)** cross-directory and cross-branch cache hits on clean checkouts.
* **LRU Eviction**: Automatically limits storage to configurable sizes (default 5 GB).
* **CLI Management**:
  ```bash
  cxxgen cache stats         # View hits, misses, storage size, and hit rates
  cxxgen cache clean         # Clear cache objects and reclaim disk storage
  cxxgen cache max-size 10G  # Set custom maximum cache size
  ```

### 8. Template Demangler & Concept Diagnostic Cards
Debugging modern C++ template metaprogramming or C++20 concept failures is simplified with automatic diagnostic interception:
* **Type Simplification**: Verbose internal template types like `std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>` are simplified to `std::string`, `std::vector<T>`, `std::unique_ptr<T>`, etc.
* **Concept Constraint Extraction**: Core unsatisfied requirements and failure reasons are extracted and highlighted.
* **Collapsed Instantiation Backtraces**: Redundant internal standard library instantiation frames (`<vector>`, `<bits/...>`, `xutility`) are collapsed (`... [N stdlib frames hidden] ...`), keeping user code frames prominent.
* **Box Border Summary Cards**: Renders colorized card UI with location, snippet lines, carets, and root cause.

### 9. Hardened Security Model & Process Isolation
`cxxgen` is engineered with a zero-trust defensive security posture designed to safely build untrusted or third-party codebases:
* **Shell & Command Injection Immunity (CWE-78)**: Subprocess execution via PowerShell or shell wrappers strictly avoids string concatenation. Dynamic parameters (download URLs, extract paths, staging destinations) are transmitted purely through the OS process environment block (`$env:VAR`), ensuring arguments are treated as pure string data and never evaluated as code.
* **Argument Injection Delimiters (CWE-88)**: Subprocess invocations (including `git clone`, `git checkout`, and `curl`) employ explicit `--` positional argument delimiters and validate ref names to ensure maliciously named URLs, tags, branches, or commits (e.g., `--upload-pack=...`) cannot be misparsed as flags.
* **Path Traversal & Zip-Slip Protection (CWE-22)**: Package and dependency names are strictly validated against directory traversal separators (`/`, `\\`, `..`). Extracted archive paths and cached object keys are strictly confined to authorized project and cache directories.
* **C++ Code Synthesis Escaping (CWE-94)**: Imported CMake/MSVC project specifications, resource embedder variable names, and CMake export configs escape quotes, newlines, and non-printable bytes into valid C++ string literals (`util::escape_c_string`), preventing code execution during `build.cpp` compilation.
* **Cryptographic Cache Integrity**: Global object cache and module BMI cache entries are verified using SHA-256 hashes and alphanumeric key whitelisting (`util::is_safe_hash_key`).

---

## Developer Ergonomics, Tooling & Diagnostics

### 1. Continuous Watch Mode (`cxxgen watch`)
Keep development fast and focused with automatic incremental builds on file change:
```bash
cxxgen watch
```
* **Smart Change Debouncing**: Collects filesystem events with intelligent debouncing (150ms default) to avoid triggering intermediate builds during multi-file editor saves.
* **Incremental Delta Builds**: Only re-executes the dirty slice of the DAG, achieving rebuild latency often under **50ms**.
* **Clean Terminal Feedback**: Prints build time deltas and diagnostic error summaries without exiting.

### 2. Environment Diagnostics (`cxxgen doctor`)
Verify your developer workstation's C++ toolchains, SDKs, and build utilities in a single command:
```bash
cxxgen doctor
```
Inspects and reports installed versions and paths for:
* **Compilers**: Clang (`clang++`), GCC (`g++`), Microsoft Visual C++ (`cl.exe`), Emscripten (`em++`).
* **Platform SDKs**: Windows 10/11 SDK, Universal C Runtime (UCRT), Linux libc headers, macOS Command Line Tools.
* **Developer Utilities**: `clang-format`, `clang-tidy`, `clangd` LSP, `llvm-profdata`, `llvm-cov`, `git`, `ninja`, and `pkg-config`.
* **Actionable Recommendations**: Clear, platform-specific install hints (Scoop, Winget, Apt, Brew) for missing tools.

### 3. Toolchain Discovery & Inspection (`cxxgen toolchain`)
Inspect discovered compilers and switch between toolchain families dynamically:
```bash
# Detect and display the primary compiler and all discovered toolchains
cxxgen toolchain detect

# List all available compilers on PATH and SDK installation paths
cxxgen toolchain list

# Force build using a specific compiler family
cxxgen build --compiler=clang
cxxgen build --compiler=gcc
cxxgen build --compiler=msvc
```

### 4. Build Graph Visualization (`cxxgen graph`)
Visualize targets, compilation steps, and execution DAG ordering across your project:
```bash
# Render ASCII tree in terminal
cxxgen graph

# Generate Mermaid flowchart diagram
cxxgen graph --format=mermaid

# Generate Graphviz DOT diagram for rendering with dot
cxxgen graph --format=dot

# Filter graph for a specific target
cxxgen graph --target=app --format=mermaid
```

### 5. Code Formatting & Linting (`cxxgen fmt` & `cxxgen lint`)
Native formatting and static analysis integrated directly into the build system:
```bash
# Format all C/C++ files in-place using clang-format
cxxgen fmt .

# Dry-run format verification (fails with non-zero exit code if unformatted; ideal for CI)
cxxgen fmt --check src/ include/

# Run clang-tidy static analysis across project translation units
cxxgen lint src/

# Run clang-tidy and automatically apply suggested fixes
cxxgen lint --fix src/
```

### 6. Sanitizers, Code Coverage & Profiling
* **Address, Undefined, Leak, and Thread Sanitizers**:
  ```bash
  cxxgen test --sanitize=address,undefined
  cxxgen run app --sanitize=thread
  ```
* **Source-Based Code Coverage Profiling**:
  ```bash
  cxxgen test --coverage
  # Instruments binaries with source coverage (-fprofile-instr-generate)
  ```
* **Build Tracing & Performance Profiling**:
  ```bash
  cxxgen build --profile=build_trace.json
  # Records Chrome Tracing / Perfetto JSON profile and reports slowest compilation steps
  ```

---

## Hands-On Examples Catalog

Explore our comprehensive, progressive examples located in [`examples/`](examples/README.md):

| Example | Level | Key Concepts & APIs Demonstrated |
|:---|:---:|:---|
| [**01_hello_world**](examples/01_hello_world/README.md) | **Beginner** | Basic executable, install artifacts, CLI argument forwarding (`--`). |
| [**02_libraries**](examples/02_libraries/README.md) | **Beginner** | Static libraries, shared DLLs, export macros, multi-library linking. |
| [**03_precompiled_headers**](examples/03_precompiled_headers/README.md) | **Intermediate** | `.pch_header` acceleration for heavy standard library headers. |
| [**04_unit_testing**](examples/04_unit_testing/README.md) | **Intermediate** | Multi-suite test runner integration (`b.add_test`, `cxxgen test`). |
| [**05_dynamic_plugins**](examples/05_dynamic_plugins/README.md) | **Advanced** | Standalone loadable plugins (`b.add_plugin`, `LoadLibrary`/`dlopen`). |
| [**06_build_system_generators**](examples/06_build_system_generators/README.md) | **Advanced** | Exporting to CMake, Visual Studio MSBuild, Ninja, and LSP database. |
| [**07_external_dependencies**](examples/07_external_dependencies/README.md) | **Advanced** | `pkg-config` system discovery, git dependencies, include/flag propagation. |
| [**08_custom_codegen_pipeline**](examples/08_custom_codegen_pipeline/README.md) | **Master** | Multi-stage DAG codegen tool bootstrapping (`b.add_custom_command`). |
| [**09_enterprise_workspace**](examples/09_enterprise_workspace/README.md) | **Master** | Monorepo architecture with core engine, CLI tool, daemon, and test suite. |
| [**10_cpp20_modules**](examples/10_cpp20_modules/README.md) | **Master** | C++20 / C++23 Modules, P1689 scanning, two-phase BMI generation, and `import std;`. |
| [**standalone_build**](examples/standalone_build/README.md) | **Beginner** | Zero-install compilation without the `cxxgen` CLI binary. |

---

## Architecture & Core Design

```
cxxgen/
├── include/
│   └── cxxgen.hpp         # The entire engine in a single, zero-dependency C++20 header
├── src/
│   └── cli/             # High-performance native CLI entrypoint (main.cpp)
├── examples/            # 10 comprehensive example projects (Beginner to Master)
├── tests/               # 14 comprehensive regression test targets (including security audit verification) with 100% pass rate
├── build.cpp            # Self-hosting build declaration for cxxgen itself
└── tasks.md             # Complete 18-Epic engineering specification
```

### Invariants & Guarantees
1. **Header-Only Simplicity**: `include/cxxgen.hpp` requires only the standard C++20 library. No external submodules, no third-party libraries, no native shared library dependencies.
2. **Cross-Platform Parity**: First-class support for Windows MSVC (with automatic `vswhere` detection), Windows Clang/GCC (MinGW), Linux GCC/Clang, and macOS AppleClang.
3. **Strict Warning-Free Quality**: The codebase compiles with zero warnings under `-Wall -Wextra -Werror` and `/W4 /WX`.
4. **Reproducible Incremental Caching**: Source and header dependencies are hashed with SHA-256; unchanged translation units are skipped instantly.
5. **Zero-Trust Hardened Execution**: Defensive argument sanitization, positional delimiters (`--`), path traversal immunity, and OS environment parameter passing prevent code injection and unauthorized filesystem access.

---

## Contributing

`cxxgen` follows a strict three-tier branching model:
* `main`: Production and stable release milestones (`v1.0.0`).
* `dev`: Active integration and staging branch.
* Dedicated feature / story branches created from `dev`.

---

## License

`cxxgen` is licensed under the [MIT License](LICENSE).

