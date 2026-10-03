# 07 - External Dependency Management (Advanced)

This example demonstrates how to integrate external dependencies into a `cxxgen` project via **System Package Discovery** (`pkg-config` / environment search) and **Remote Git Dependencies**.

---

## What This Demonstrates
- Querying installed system packages using `b.find_system_package`.
- Optional vs. Required package handling (`.required = false`).
- Automatically propagating include paths, preprocessor defines, and linker flags via `app.link(pkg)`.
- Providing actionable user hints for missing packages (`.install_hint = "..."`).
- Declaring remote git dependencies with branch/tag/commit pinning.

---

## Project Structure

```
07_external_dependencies/
├── build.cpp                     # Build script with dependency discovery
├── src/
│   └── main.cpp                  # Application leveraging discovered packages
└── README.md
```

---

## Technical Details

### 1. System Package Discovery (`b.find_system_package`)
```cpp
auto threads_pkg = b.find_system_package("threads", {
    .required = false,
    .install_hint = "Standard C++ threading or pthread",
});

if (threads_pkg) {
    app.link(threads_pkg);
    app.config().defines.push_back("HAVE_THREADS");
}
```

How it works:
1. `cxxgen` queries `pkg-config` if present on the system.
2. If `pkg-config` is unavailable or the package is not found, `cxxgen` inspects standard platform SDK paths (Windows SDK, `/usr/include`, `/usr/lib`, MinGW sysroot).
3. If found, all include paths and link flags are packaged into a strongly typed `SystemPackage` structure.
4. Calling `app.link(pkg)` seamlessly wires these paths into the compilation and link commands.

### 2. Remote Git Dependencies (`b.dependency`)
You can declare git dependencies directly in `build.cpp`:
```cpp
auto &googletest = b.dependency("googletest", {
    .git = "https://github.com/google/googletest.git",
    .tag = "v1.18.0",
});

// Compile a library directly from the fetched dependency sources
auto &gmock_lib = googletest.add_static_library({
    .name = "gmock_main",
    .sources = {
        "googletest/src/gtest-all.cc",
        "googlemock/src/gmock-all.cc",
        "googlemock/src/gmock_main.cc",
    },
    .include_dirs = {
        "googletest/include",
        "googletest",
        "googlemock/include",
        "googlemock",
    },
    .cpp_version = cxxgen::CppVersion::Cpp20,
});
```

Key dependency management behaviors:
* **Fast Shallow Clones**: Remote git repositories are fetched with `--depth 1 --branch <tag>` for minimal bandwidth and disk usage.
* **Persistent Disk Cache**: Dependencies are cached in `~/.cxxgen/pkg/<name>_<hash>` and shared across builds.
* **Deterministic Lockfile**: Exact resolved commit hashes are pinned in `cxxgen.lock` for reproducible builds across team members and CI pipelines.
* **Child Target Importing**: If the remote repository provides a `build.cpp`, `cxxgen` automatically imports its targets. You can also declare custom static/shared libraries directly from the fetched dependency root using `dep.add_static_library` or `dep.add_shared_library`.
* **System Library Linking**: For platform-specific runtime dependencies (like `-lpthread`), use `target.add_system_library("pthread")`.

---

## How to Build and Run

### Build the Target
```bash
cxxgen build
```

### Run the Application
```bash
cxxgen run dependency_demo
```

Expected Output:
```
========================================
    External Dependency Discovery Demo  
========================================
[System Package] Threading package linked successfully!
  [Thread 1] Running concurrent work with thread support...
  [Thread 2] Running concurrent work with thread support...
  [Thread 3] Running concurrent work with thread support...
[Custom Define] Macro injected from dependency package!

Dependencies resolved and linked cleanly by cxxgen!
```
