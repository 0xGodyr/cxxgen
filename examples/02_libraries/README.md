# 02 - Static and Shared Libraries (Beginner / Intermediate)

This example demonstrates how to build and link both **static libraries** (`.lib` / `.a`) and **shared dynamic libraries** (`.dll` / `.so` / `.dylib`) using `cxxgen`.

---

## What This Demonstrates
- Creating static libraries with `b.add_static_library`.
- Creating dynamic shared libraries with `b.add_shared_library`.
- Defining compiler preprocessor symbols (e.g. `BUILDING_STRING_SHARED`).
- Cross-platform dynamic symbol export macros (`__declspec(dllexport)` / `__attribute__((visibility("default")))`).
- Linking multiple libraries into a consumer executable with `app.link(lib)`.
- Automatic DAG scheduling: `cxxgen` compiles both libraries in parallel before linking `app`.

---

## Project Structure

```
02_libraries/
├── build.cpp                       # Build declaration
├── math_static/                    # Static library module
│   ├── include/math_static.hpp
│   └── src/math_static.cpp
├── string_shared/                  # Shared dynamic library module
│   ├── include/string_shared.hpp
│   └── src/string_shared.cpp
├── app/                            # Consumer executable
│   └── src/main.cpp
└── README.md
```

---

## Architectural Breakdown

### 1. Static Library (`math_static`)
Static libraries are compiled into native object archives:
```cpp
auto &math_lib = b.add_static_library({
    .name = "math_static",
    .sources = {"math_static/src/math_static.cpp"},
    .include_dirs = {"math_static/include"},
    .cpp_version = cxxgen::CppVersion::Cpp20,
});
```
On Windows MSVC, this generates `lib/math_static.lib` using `lib.exe`.  
On Clang/GCC, this generates `lib/libmath_static.a` using `llvm-ar` or `ar`.

### 2. Shared Dynamic Library (`string_shared`)
Dynamic libraries require symbol visibility management:
```cpp
#if defined(_WIN32)
  #if defined(BUILDING_STRING_SHARED)
    #define STRING_SHARED_API __declspec(dllexport)
  #else
    #define STRING_SHARED_API __declspec(dllimport)
  #endif
#else
  #define STRING_SHARED_API __attribute__((visibility("default")))
#endif
```
In `build.cpp`:
```cpp
auto &str_lib = b.add_shared_library({
    .name = "string_shared",
    .sources = {"string_shared/src/string_shared.cpp"},
    .include_dirs = {"string_shared/include"},
    .defines = {"BUILDING_STRING_SHARED"},
    .cpp_version = cxxgen::CppVersion::Cpp20,
});
```
On Windows, `cxxgen` automatically handles import library generation (`bin/string_shared.lib` and `bin/string_shared.dll`).

### 3. Linking into Executable
```cpp
auto &app = b.add_executable({
    .name = "library_consumer",
    .sources = {"app/src/main.cpp"},
    .include_dirs = {"math_static/include", "string_shared/include"},
    .cpp_version = cxxgen::CppVersion::Cpp20,
});

app.link(math_lib);
app.link(str_lib);
```
`cxxgen` automatically establishes dependency edges in the execution DAG and passes the proper linker flags and library search directories.

---

## How to Build and Run

### Build All Targets
```bash
cxxgen build
```

### Run the Application
```bash
cxxgen run library_consumer
```

Expected Output:
```
=== Testing Static Library (math_static) ===
12 + 25 = 37
12 * 25 = 300
5! = 120

=== Testing Shared Library (string_shared) ===
Upper : HELLO FROM DYNAMIC CXXGEN SHARED LIBRARY!
Lower : hello from dynamic cxxgen shared library!
Joined: cxxgen -> modern -> fast -> c++20
```
