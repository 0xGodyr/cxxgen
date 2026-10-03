# 03 - Precompiled Headers (Intermediate)

This example demonstrates how to leverage **Precompiled Headers (PCH)** in `cxxgen` to significantly reduce C++ compilation times when working with large standard library headers or complex template libraries.

---

## What This Demonstrates
- Defining a precompiled header with `.pch_header = "include/pch.hpp"`.
- How `cxxgen` orchestrates precompilation across different compilers:
  - **MSVC**: Compiles PCH with `/Yc` companion object and injects via `/Yu` and `/Fp`.
  - **Clang**: Precompiles with `-emit-pch` and injects via `-include-pch`.
  - **GCC**: Compiles `.gch` header and automatically resolves during compilation.
- Preprocessor guard fallback (`#if !defined(CXXGEN_PCH_INCLUDED)`).
- Clean source files that do not redundantly re-parse heavy headers.

---

## Project Structure

```
03_precompiled_headers/
├── build.cpp                     # Build declaration with PCH enabled
├── include/
│   ├── pch.hpp                   # Heavy headers collected in single PCH
│   ├── data_processor.hpp        # Analytics engine
│   └── report_generator.hpp      # Formatter
├── src/
│   ├── data_processor.cpp
│   ├── report_generator.cpp
│   └── main.cpp
└── README.md
```

---

## Technical Details

### 1. The PCH Header (`include/pch.hpp`)
Gather all heavyweight standard library or third-party headers into a single header:
```cpp
#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
```

### 2. Enabling PCH in `build.cpp`
Simply provide the path to the PCH header in the target configuration:
```cpp
auto &app = b.add_executable({
    .name = "analytics_demo",
    .sources = {
        "src/data_processor.cpp",
        "src/report_generator.cpp",
        "src/main.cpp",
    },
    .include_dirs = {"include"},
    .pch_header = "include/pch.hpp",
});
```

When building, `cxxgen` will automatically:
1. Compile the precompiled header before compiling any dependent translation units.
2. Store the compiled binary artifact (`.pch` or `.gch`) inside `.cxxgen/obj/<target>/`.
3. Track hash changes to `pch.hpp` to ensure automatic recompilation on edits.

---

## How to Build and Run

### Build the Target
```bash
cxxgen build
```

### Run the Application
```bash
cxxgen run analytics_demo
```

Expected Output:
```
========================================
        ANALYTICS SUMMARY REPORT        
========================================
Total Sum : 835.20
Average   : 167.04

Breakdown by Category:
  - Compute        : 220.90
  - Database       : 310.00
  - Networking     : 215.10
  - Storage        : 89.20
========================================
[cxxgen PCH] Executed successfully using precompiled headers!
```
