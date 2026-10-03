# 04 - Unit Testing & Test Suites (Intermediate)

This example demonstrates how to integrate automated testing into your build workflow using `b.add_test` and `cxxgen test`.

---

## What This Demonstrates
- Defining unit test targets using `b.add_test`.
- Linking test suites against production libraries (`test.link(calc_lib)`).
- Executing all registered test suites with a single command (`cxxgen test`).
- Executing a specific targeted test suite (`cxxgen test test_basic`).
- Test result reporting and exit-code propagation.

---

## Project Structure

```
04_unit_testing/
├── build.cpp                     # Build and test declarations
├── include/
│   └── calculator.hpp            # Library API
├── src/
│   └── calculator.cpp            # Library implementation
├── tests/
│   ├── test_basic.cpp            # Basic assertions
│   └── test_advanced.cpp         # Complex algorithms & edge cases
└── README.md
```

---

## Technical Details

### Registering Test Targets
Test suites are registered using `b.add_test`:
```cpp
auto &test_basic = b.add_test({
    .name = "test_basic",
    .sources = {"tests/test_basic.cpp"},
    .include_dirs = {"include"},
    .cpp_version = cxxgen::CppVersion::Cpp20,
});
test_basic.link(calc_lib);
```

When `cxxgen test` is run:
1. `cxxgen` compiles all dependencies (in this case `calculator`).
2. `cxxgen` compiles each test suite executable.
3. `cxxgen` spawns the test executables sequentially or in parallel.
4. If any test exits with a non-zero code or assertions fail, `cxxgen test` reports the failure immediately and halts.

---

## How to Build and Run Tests

### Run All Test Suites
```bash
cxxgen test
```

Expected Output:
```
[RUNNING] test_basic suite...
[PASS] All test_basic checks passed!
[RUNNING] test_advanced suite...
[PASS] All test_advanced checks passed!
[cxxgen] All 2 test target(s) passed successfully.
```

### Run a Specific Test Target
```bash
cxxgen test test_basic
```

### Build Only Without Running Tests
```bash
cxxgen build
```

---

## Google Test (GTest) & Google Mock (GMock) Integration

While this example demonstrates simple zero-dependency test suites using standard assertions, `cxxgen` has first-class support for industry-standard test frameworks including **Google Test** and **Google Mock**.

When you scaffold a new project with `cxxgen init --type=exe`, `cxxgen` automatically configures Google Test and Google Mock (v1.18.0) by default:

```cpp
// 1. Declare Google Test / Google Mock dependency
auto &googletest = b.dependency("googletest", {
    .git = "https://github.com/google/googletest.git",
    .tag = "v1.18.0",
});

// 2. Build combined gmock_main static library
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

// 3. Register test target linking against gmock_main
auto &test_suite = b.add_test({
    .name = "test_math",
    .sources = {"tests/test_math.cpp"},
    .include_dirs = {
        "include",
        (googletest.path() / "googletest/include").string(),
        (googletest.path() / "googlemock/include").string(),
    },
    .cpp_version = cxxgen::CppVersion::Cpp20,
});
test_suite.link(gmock_lib);
#if !defined(_WIN32)
test_suite.add_system_library("pthread");
#endif
```

In your test files (`tests/test_math.cpp`):
* `#include <gtest/gtest.h>` for test fixtures and assertions (`EXPECT_EQ`, `EXPECT_TRUE`).
* `#include <gmock/gmock.h>` for mock classes (`MOCK_METHOD`) and expectations (`EXPECT_CALL`).
* `gmock_main` provides the entry point, eliminating manual `int main(...)` test runners.

