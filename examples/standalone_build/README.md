# Standalone Build Example

This example demonstrates how a project can be structured and built using **`cxxgen`** as either a standalone executable build script or via the `cxxgen` CLI.

---

## Architecture Overview

```
standalone_build/
├── build.cpp            # Self-contained build script
├── include/
│   └── greeting.hpp     # Library header
├── src/
│   ├── greeting.cpp     # Library implementation
│   └── main.cpp         # Main application entry point
├── tests/
│   └── test.cpp         # Unit test runner
└── README.md
```

### Build Graph
- **`greeting`**: Static library compiled from `src/greeting.cpp`.
- **`hello_world`**: Executable compiled from `src/main.cpp` and linked with `greeting`.
- **`hello_test`**: Test executable compiled from `tests/test.cpp` and linked with `greeting`.

---

## Building and Running

### Option A: Using the `cxxgen` CLI
From this directory:

```bash
# Build all default install artifacts
cxxgen build

# Run the executable
cxxgen run hello_world

# Pass arguments to the executable
cxxgen run hello_world -- Alice

# Run the test suite
cxxgen test
```

### Option B: Compiling `build.cpp` Standalone
Since `build.cpp` uses the single-header `cxxgen.hpp`, it can be compiled and executed directly using any standard C++20 compiler:

#### Clang / GCC:
```bash
clang++ -std=c++20 -I../../include build.cpp -o my_build.exe
./my_build.exe build
./my_build.exe run hello_world
./my_build.exe test
```

#### MSVC:
```powershell
cl /std:c++20 /EHsc /I..\..\include build.cpp /Fe:my_build.exe
.\my_build.exe build
.\my_build.exe run hello_world
```
