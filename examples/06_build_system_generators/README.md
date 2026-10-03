# 06 - Foreign Build System Generators (Advanced)

One of **`cxxgen`**'s most powerful capabilities is functioning as a **Meta-Build Generator**. You can write your build logic once in clean C++ using `cxxgen`, and export fully functional build scripts for any foreign build system or IDE!

---

## What This Demonstrates
- Generating **CMake** projects (`CMakeLists.txt`).
- Generating **Visual Studio / MSBuild** solutions (`.sln` and `.vcxproj`).
- Generating ultra-fast **Ninja** build files (`build.ninja`).
- Generating **Clang JSON Compilation Databases** (`compile_commands.json`) for Language Server Protocols (LSP), clangd, VS Code, CLion, and Neovim.

---

## Project Structure

```
06_build_system_generators/
├── build.cpp                     # Universal build declaration
├── include/
│   └── core_math.hpp
├── src/
│   ├── core_math.cpp
│   └── main.cpp
└── README.md
```

---

## Supported Generator Backends

### 1. CMake Backend (`cxxgen generate cmake`)
Generates a modern `CMakeLists.txt` configured with target definitions, include paths, standard requirements, and link dependencies.

```bash
# Generate CMakeLists.txt
cxxgen generate cmake

# Build with standard CMake
cmake -B build
cmake --build build

# Run output
./build/generator_demo
```

### 2. MSBuild / Visual Studio Backend (`cxxgen generate msvc`)
Generates native Visual Studio solution (`.sln`) and project files (`.vcxproj`) containing Debug and Release configurations for x64 / ARM64.

```powershell
# Generate Solution and vcxproj files
cxxgen generate msvc

# Build with MSBuild on Windows
msbuild generator_demo.sln /p:Configuration=Release /p:Platform=x64

# Open in Visual Studio IDE
start generator_demo.sln
```

### 3. Ninja Backend (`cxxgen generate ninja`)
Generates an optimized `build.ninja` file for parallel build execution.

```bash
# Generate build.ninja
cxxgen generate ninja

# Execute build with Ninja
ninja

# Run output
./bin/generator_demo
```

### 4. Language Server / IDE Database (`cxxgen generate compile_commands`)
Generates `compile_commands.json` capturing the exact compiler flags, include directories, and macro definitions for each translation unit.

```bash
# Generate compile_commands.json
cxxgen generate compile_commands
```
Your editor (clangd, Neovim LSP, VS Code C/C++ extension, Sublime Text, or CLion) will immediately recognize symbols, includes, and diagnostics with zero configuration required!

---

## Direct Build with `cxxgen`
You can still build and run directly via `cxxgen` anytime without using external generators:
```bash
cxxgen build
cxxgen run generator_demo
```
