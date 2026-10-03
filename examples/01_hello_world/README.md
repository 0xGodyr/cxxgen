# 01 - Hello World (Beginner)

This is the simplest introduction to building C++ applications with **`cxxgen`**.

---

## What This Demonstrates
- Defining an executable target with `b.add_executable`.
- Specifying the C++ language standard (`cxxgen::CppVersion::Cpp20`).
- Marking install artifacts with `b.install_artifact`.
- The `CXXGEN_MAIN(build)` entrypoint macro.
- Compiling, running, and passing arguments to binaries.

---

## Project Structure

```
01_hello_world/
├── build.cpp            # Build declaration
├── src/
│   └── main.cpp         # C++20 application entrypoint
└── README.md            # Documentation
```

---

## Step-by-Step Walkthrough

### 1. Build Script (`build.cpp`)
In `cxxgen`, build scripts are written in standard, strongly typed C++:

```cpp
#include "cxxgen.hpp"

void build(cxxgen::Builder &b) {
    auto target = b.standard_target_options();
    auto optimize = b.standard_optimize_options();

    auto &exe = b.add_executable({
        .name = "hello_world",
        .sources = {"src/main.cpp"},
        .cpp_version = cxxgen::CppVersion::Cpp20,
        .target = target,
        .optimize = optimize.mode,
    });

    b.install_artifact(exe);
}

CXXGEN_MAIN(build)
```

Key points:
- `b.standard_target_options()` auto-detects host OS and machine architecture.
- `b.standard_optimize_options()` parses command-line flags like `-O Release`.
- Designated initializers (`.name = ...`, `.sources = ...`) provide readable, self-documenting configuration.
- `b.install_artifact(exe)` designates `hello_world` as a top-level product of `cxxgen build`.

---

## How to Build and Run

### Build the Application
```bash
cxxgen build
```

### Run the Application
```bash
cxxgen run hello_world
```

### Pass Command-Line Arguments
Arguments after `--` are forwarded verbatim to the target:
```bash
cxxgen run hello_world -- foo bar --flag
```

### Clean Build Artifacts
```bash
cxxgen clean
```
