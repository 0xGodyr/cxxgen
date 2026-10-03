# 05 - Dynamic Plugin Architecture (Advanced)

This example demonstrates how to build and consume **dynamically loaded plugins** (`.dll` / `.so` / `.dylib`) in modern C++20 using `cxxgen`.

---

## What This Demonstrates
- Using `b.add_plugin` to build standalone, loadable dynamic libraries.
- Directing plugin binaries into a dedicated folder (`.output_dir = "bin/plugins"`).
- Designing a stable C ABI entrypoint (`extern "C" PLUGIN_EXPORT IPlugin* create_plugin()`).
- Loading dynamic plugins at runtime using OS APIs:
  - `LoadLibraryA` & `GetProcAddress` on Windows.
  - `dlopen` & `dlsym` on Linux / macOS.
- Safe dynamic dispatch and lifetime management.

---

## Project Structure

```
05_dynamic_plugins/
├── build.cpp                             # Build declarations
├── include/
│   └── plugin_interface.hpp              # Shared abstract plugin interface
├── plugins/
│   ├── plugin_rot13/                     # ROT13 Caesar cipher plugin
│   │   └── src/plugin_rot13.cpp
│   └── plugin_reverse/                   # String reversal plugin
│       └── src/plugin_reverse.cpp
├── host/                                 # Host runner executable
│   └── src/main.cpp
└── README.md
```

---

## Technical Details

### 1. Abstract Plugin Interface (`include/plugin_interface.hpp`)
```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual std::string process(std::string_view input) const = 0;
};

using CreatePluginFn = IPlugin *(*)();
```

### 2. Declaring Plugins in `build.cpp`
Plugins do not require an import library; they are compiled as loadable shared modules:
```cpp
auto &plug_rot13 = b.add_plugin({
    .name = "plugin_rot13",
    .sources = {"plugins/plugin_rot13/src/plugin_rot13.cpp"},
    .include_dirs = {"include"},
    .defines = {"BUILDING_PLUGIN"},
    .output_dir = "bin/plugins",
});
b.install_artifact(plug_rot13);
```

### 3. Dynamic Runtime Loading
The host application uses standard platform loaders (`LoadLibraryA` / `dlopen`) to resolve `create_plugin` without compile-time linking dependencies.

---

## How to Build and Run

### Build Host and Plugins
```bash
cxxgen build
```

### Run Host with Default Sample Text
```bash
cxxgen run plugin_host
```

### Run Host with Custom Text
```bash
cxxgen run plugin_host -- "Hello World! Antigravity C++ Builder"
```

Expected Output:
```
========================================
      cxxgen Dynamic Plugin Host          
========================================
Input Text: "Hello World! Antigravity C++ Builder"

Loading: bin/plugins\plugin_rot13.dll ...
  -> [OK] Loaded Plugin: "ROT13 Cipher Plugin"
  -> [RESULT]: Uryyb Jbeyq! Nagvtenivgl P++ Ohvyqre

Loading: bin/plugins\plugin_reverse.dll ...
  -> [OK] Loaded Plugin: "String Reverser Plugin"
  -> [RESULT]: redliuB ++C ytivargitnA !dlroW olleH

Summary: Successfully executed 2 dynamic plugin(s)!
```
