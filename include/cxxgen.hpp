#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// ============================================================================
// cxxgen.hpp - C++ Generator & Build System (Header-Only Core)
// Single-header build configuration and execution engine for modern C++.
// Standard: C++20 / C++23 | Zero External Dependencies
// ============================================================================

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <ostream>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef interface
#undef interface
#endif
#else
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace cxxgen {

// ============================================================================
// 1. Language Standards & Target Options
// ============================================================================

enum class CppVersion {
    Cpp11,
    Cpp14,
    Cpp17,
    Cpp20,
    Cpp23,
    Cpp26,
};

enum class CVersion {
    C89,
    C99,
    C11,
    C17,
    C23,
};

enum class Optimization {
    Debug,
    ReleaseFast,
    ReleaseSafe,
    ReleaseSmall,
};

enum class TargetType {
    Executable,
    StaticLibrary,
    SharedLibrary,
    Plugin,
    Interface,
    Test,
};

enum class WindowsSubsystem {
    Console,
    Windows,
};

enum class CompilerFamily {
    Auto,
    Msvc,
    Clang,
    Gcc,
    AppleClang,
    Emscripten,
};

// ============================================================================
// 2. String Conversion & Serialization Helpers
// ============================================================================

constexpr std::string_view to_string_view(CppVersion version) noexcept {
    switch (version) {
    case CppVersion::Cpp11:
        return "c++11";
    case CppVersion::Cpp14:
        return "c++14";
    case CppVersion::Cpp17:
        return "c++17";
    case CppVersion::Cpp20:
        return "c++20";
    case CppVersion::Cpp23:
        return "c++23";
    case CppVersion::Cpp26:
        return "c++26";
    }
    return "c++20";
}

constexpr std::string_view to_string_view(CVersion version) noexcept {
    switch (version) {
    case CVersion::C89:
        return "c89";
    case CVersion::C99:
        return "c99";
    case CVersion::C11:
        return "c11";
    case CVersion::C17:
        return "c17";
    case CVersion::C23:
        return "c23";
    }
    return "c17";
}

constexpr std::string_view to_string_view(Optimization opt) noexcept {
    switch (opt) {
    case Optimization::Debug:
        return "Debug";
    case Optimization::ReleaseFast:
        return "ReleaseFast";
    case Optimization::ReleaseSafe:
        return "ReleaseSafe";
    case Optimization::ReleaseSmall:
        return "ReleaseSmall";
    }
    return "Debug";
}

constexpr std::string_view to_string_view(TargetType type) noexcept {
    switch (type) {
    case TargetType::Executable:
        return "Executable";
    case TargetType::StaticLibrary:
        return "StaticLibrary";
    case TargetType::SharedLibrary:
        return "SharedLibrary";
    case TargetType::Plugin:
        return "Plugin";
    case TargetType::Interface:
        return "Interface";
    case TargetType::Test:
        return "Test";
    }
    return "Executable";
}

constexpr std::string_view to_string_view(WindowsSubsystem sub) noexcept {
    switch (sub) {
    case WindowsSubsystem::Console:
        return "Console";
    case WindowsSubsystem::Windows:
        return "Windows";
    }
    return "Console";
}

constexpr std::string_view to_string_view(CompilerFamily family) noexcept {
    switch (family) {
    case CompilerFamily::Auto:
        return "Auto";
    case CompilerFamily::Msvc:
        return "MSVC";
    case CompilerFamily::Clang:
        return "Clang";
    case CompilerFamily::Gcc:
        return "GCC";
    case CompilerFamily::AppleClang:
        return "AppleClang";
    case CompilerFamily::Emscripten:
        return "Emscripten";
    }
    return "Auto";
}

inline std::string to_string(CppVersion v) {
    return std::string(to_string_view(v));
}

inline std::string to_string(CVersion v) {
    return std::string(to_string_view(v));
}

inline std::string to_string(Optimization o) {
    return std::string(to_string_view(o));
}

inline std::string to_string(TargetType t) {
    return std::string(to_string_view(t));
}

inline std::string to_string(WindowsSubsystem s) {
    return std::string(to_string_view(s));
}

inline std::string to_string(CompilerFamily f) {
    return std::string(to_string_view(f));
}

inline std::ostream &operator<<(std::ostream &os, CppVersion v) {
    return os << to_string_view(v);
}

inline std::ostream &operator<<(std::ostream &os, CVersion v) {
    return os << to_string_view(v);
}

inline std::ostream &operator<<(std::ostream &os, Optimization o) {
    return os << to_string_view(o);
}

inline std::ostream &operator<<(std::ostream &os, TargetType t) {
    return os << to_string_view(t);
}

inline std::ostream &operator<<(std::ostream &os, WindowsSubsystem s) {
    return os << to_string_view(s);
}

inline std::ostream &operator<<(std::ostream &os, CompilerFamily f) {
    return os << to_string_view(f);
}

// ============================================================================
// 3. String Parsing Helpers
// ============================================================================

namespace detail {
inline std::string to_lower_copy(std::string_view sv) {
    std::string result(sv);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}
} // namespace detail

inline std::optional<CppVersion> parse_cpp_version(std::string_view sv) {
    std::string lower = detail::to_lower_copy(sv);
    if (lower == "c++11" || lower == "cpp11" || lower == "11" || lower == "cxx11")
        return CppVersion::Cpp11;
    if (lower == "c++14" || lower == "cpp14" || lower == "14" || lower == "cxx14")
        return CppVersion::Cpp14;
    if (lower == "c++17" || lower == "cpp17" || lower == "17" || lower == "cxx17")
        return CppVersion::Cpp17;
    if (lower == "c++20" || lower == "cpp20" || lower == "20" || lower == "cxx20")
        return CppVersion::Cpp20;
    if (lower == "c++23" || lower == "cpp23" || lower == "23" || lower == "cxx23" || lower == "c++2b")
        return CppVersion::Cpp23;
    if (lower == "c++26" || lower == "cpp26" || lower == "26" || lower == "cxx26" || lower == "c++2c")
        return CppVersion::Cpp26;
    return std::nullopt;
}

inline std::optional<Optimization> parse_optimization(std::string_view sv) {
    std::string lower = detail::to_lower_copy(sv);
    if (lower == "debug" || lower == "dbg")
        return Optimization::Debug;
    if (lower == "releasefast" || lower == "release" || lower == "fast" || lower == "o3")
        return Optimization::ReleaseFast;
    if (lower == "releasesafe" || lower == "safe")
        return Optimization::ReleaseSafe;
    if (lower == "releasesmall" || lower == "small" || lower == "minsize" || lower == "os")
        return Optimization::ReleaseSmall;
    return std::nullopt;
}

inline std::optional<TargetType> parse_target_type(std::string_view sv) {
    std::string lower = detail::to_lower_copy(sv);
    if (lower == "executable" || lower == "exe" || lower == "bin" || lower == "app")
        return TargetType::Executable;
    if (lower == "staticlibrary" || lower == "static" || lower == "lib" || lower == "a")
        return TargetType::StaticLibrary;
    if (lower == "sharedlibrary" || lower == "shared" || lower == "dll" || lower == "so" || lower == "dylib")
        return TargetType::SharedLibrary;
    if (lower == "plugin" || lower == "module")
        return TargetType::Plugin;
    if (lower == "interface" || lower == "headeronly" || lower == "header_only")
        return TargetType::Interface;
    if (lower == "test" || lower == "tests")
        return TargetType::Test;
    return std::nullopt;
}

inline std::optional<CompilerFamily> parse_compiler_family(std::string_view sv) {
    std::string lower = detail::to_lower_copy(sv);
    if (lower == "auto")
        return CompilerFamily::Auto;
    if (lower == "msvc" || lower == "cl")
        return CompilerFamily::Msvc;
    if (lower == "clang" || lower == "clang++" || lower == "llvm")
        return CompilerFamily::Clang;
    if (lower == "gcc" || lower == "g++" || lower == "gnu" || lower == "mingw")
        return CompilerFamily::Gcc;
    if (lower == "appleclang" || lower == "apple-clang")
        return CompilerFamily::AppleClang;
    if (lower == "emscripten" || lower == "emcc" || lower == "em++" || lower == "wasm" || lower == "wasm32")
        return CompilerFamily::Emscripten;
    return std::nullopt;
}

// ============================================================================
// 4. Target Configuration Struct
// ============================================================================

struct TargetOptions {
    std::string arch = "native";
    std::string os = "host";
    std::string abi = "";
    std::string triple = "";
    std::filesystem::path sysroot = "";
};

struct TargetProfile {
    std::string triple = "";
    std::string arch = "native";
    std::string vendor = "";
    std::string os = "host";
    std::string env = "";
    std::string environment = "";
    std::filesystem::path sysroot = "";

    static TargetProfile parse(std::string_view triple_str, const std::filesystem::path &sysroot_path = "") {
        TargetProfile prof;
        prof.triple = std::string(triple_str);
        prof.sysroot = sysroot_path;

        if (prof.triple.empty() || prof.triple == "native" || prof.triple == "host") {
            prof.arch = "native";
            prof.os = "host";
            return prof;
        }

        std::string lower = detail::to_lower_copy(prof.triple);
        if (lower == "wasm" || lower == "wasm32" || lower == "emscripten") {
            prof.arch = "wasm32";
            prof.vendor = "unknown";
            prof.os = "emscripten";
            prof.env = "";
            prof.environment = "";
            return prof;
        }
        if (lower == "wasm64") {
            prof.arch = "wasm64";
            prof.vendor = "unknown";
            prof.os = "emscripten";
            prof.env = "";
            prof.environment = "";
            return prof;
        }
        if (lower == "aarch64" || lower == "arm64") {
            prof.arch = "aarch64";
            prof.os = "linux";
            prof.env = "gnu";
            prof.environment = "gnu";
            return prof;
        }
        if (lower == "arm") {
            prof.arch = "arm";
            prof.os = "linux";
            prof.env = "gnueabihf";
            prof.environment = "gnueabihf";
            return prof;
        }
        if (lower == "x86_64" || lower == "x64") {
            prof.arch = "x86_64";
            return prof;
        }

        std::vector<std::string> tokens;
        size_t start = 0;
        while (start < lower.size()) {
            size_t dash = lower.find('-', start);
            if (dash == std::string::npos)
                dash = lower.size();
            tokens.push_back(lower.substr(start, dash - start));
            start = dash + 1;
        }

        if (tokens.size() == 1) {
            prof.arch = tokens[0];
        } else if (tokens.size() == 2) {
            prof.arch = tokens[0];
            prof.os = tokens[1];
        } else if (tokens.size() == 3) {
            prof.arch = tokens[0];
            if (tokens[2] == "emscripten") {
                prof.vendor = tokens[1];
                prof.os = "emscripten";
                prof.env = "emscripten";
            } else if (tokens[2] == "eabi" || tokens[2] == "elf") {
                prof.os = tokens[1];
                prof.env = tokens[2];
            } else if (tokens[1] == "unknown" || tokens[1] == "pc" || tokens[1] == "apple" || tokens[1] == "w64" ||
                       tokens[1] == "none") {
                prof.vendor = tokens[1];
                prof.os = tokens[2];
            } else {
                prof.os = tokens[1];
                prof.env = tokens[2];
            }
        } else if (tokens.size() >= 4) {
            prof.arch = tokens[0];
            prof.vendor = tokens[1];
            prof.os = tokens[2];
            prof.env = tokens[3];
        }
        prof.environment = prof.env;

        return prof;
    }

    [[nodiscard]] bool is_wasm() const noexcept {
        return arch == "wasm32" || arch == "wasm64" || os == "emscripten" || os == "wasi" ||
               triple.find("wasm") != std::string::npos || triple.find("emscripten") != std::string::npos;
    }

    [[nodiscard]] bool is_wasm32() const noexcept { return arch == "wasm32" || (is_wasm() && arch != "wasm64"); }

    [[nodiscard]] bool is_wasm64() const noexcept { return arch == "wasm64"; }

    [[nodiscard]] bool is_aarch64() const noexcept { return arch == "aarch64" || arch == "arm64"; }

    [[nodiscard]] bool is_arm() const noexcept { return arch == "arm" || is_aarch64(); }

    [[nodiscard]] bool is_riscv() const noexcept { return arch == "riscv32" || arch == "riscv64" || arch == "riscv"; }

    [[nodiscard]] bool is_x86_64() const noexcept { return arch == "x86_64" || arch == "x64" || arch == "amd64"; }

    [[nodiscard]] bool is_windows() const noexcept {
        return os == "windows" || os == "win32" || env == "msvc" || env == "mingw32" ||
               triple.find("windows") != std::string::npos || triple.find("mingw") != std::string::npos;
    }

    [[nodiscard]] bool is_linux() const noexcept {
        return os == "linux" || env == "gnu" || env == "musl" ||
               (triple.find("linux") != std::string::npos && !is_wasm());
    }

    [[nodiscard]] bool is_apple() const noexcept {
        return os == "darwin" || os == "macos" || os == "ios" || vendor == "apple" ||
               triple.find("apple") != std::string::npos || triple.find("darwin") != std::string::npos;
    }

    [[nodiscard]] bool is_darwin() const noexcept { return is_apple(); }

    [[nodiscard]] bool is_bare_metal() const noexcept {
        return os == "none" || os == "elf" || env == "eabi" || env == "elf" ||
               triple.find("none-eabi") != std::string::npos || triple.find("unknown-elf") != std::string::npos;
    }

    [[nodiscard]] std::vector<std::string> get_arch_defines() const {
        std::vector<std::string> defs;
        if (arch == "wasm64") {
            defs.push_back("__wasm__=1");
            defs.push_back("__wasm64__=1");
            if (os == "emscripten" || triple.find("emscripten") != std::string::npos) {
                defs.push_back("__EMSCRIPTEN__=1");
                defs.push_back("EMSCRIPTEN=1");
            }
        } else if (arch == "wasm32" || is_wasm()) {
            defs.push_back("__wasm__=1");
            defs.push_back("__wasm32__=1");
            if (os == "emscripten" || triple.find("emscripten") != std::string::npos) {
                defs.push_back("__EMSCRIPTEN__=1");
                defs.push_back("EMSCRIPTEN=1");
            }
        } else if (arch == "aarch64" || arch == "arm64") {
            defs.push_back("__aarch64__=1");
            defs.push_back("__ARM_ARCH_8A__=1");
        } else if (arch == "arm") {
            defs.push_back("__arm__=1");
        } else if (arch == "riscv64") {
            defs.push_back("__riscv=1");
            defs.push_back("__riscv_xlen=64");
        } else if (arch == "riscv32") {
            defs.push_back("__riscv=1");
            defs.push_back("__riscv_xlen=32");
        } else if (arch == "x86_64") {
            defs.push_back("__x86_64__=1");
        } else if (arch == "i386" || arch == "x86") {
            defs.push_back("__i386__=1");
        }
        return defs;
    }
};

struct OptimizeOptions {
    Optimization mode = Optimization::Debug;
    bool lto = false;
};

// ----------------------------------------------------------------------------
// Sanitizers & Coverage
// ----------------------------------------------------------------------------

enum class Sanitizer : uint32_t {
    None = 0,
    Address = 1 << 0,
    Undefined = 1 << 1,
    Leak = 1 << 2,
    Thread = 1 << 3,
};

inline constexpr Sanitizer operator|(Sanitizer a, Sanitizer b) noexcept {
    return static_cast<Sanitizer>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline constexpr Sanitizer operator&(Sanitizer a, Sanitizer b) noexcept {
    return static_cast<Sanitizer>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline constexpr Sanitizer operator~(Sanitizer a) noexcept {
    return static_cast<Sanitizer>(~static_cast<uint32_t>(a));
}

inline constexpr bool operator!(Sanitizer a) noexcept {
    return a == Sanitizer::None;
}

inline Sanitizer parse_sanitizers(std::string_view str) {
    Sanitizer result = Sanitizer::None;
    size_t start = 0;
    while (start < str.size()) {
        size_t comma = str.find(',', start);
        if (comma == std::string_view::npos)
            comma = str.size();
        std::string token = detail::to_lower_copy(str.substr(start, comma - start));
        size_t first = token.find_first_not_of(" \t\r\n");
        size_t last = token.find_last_not_of(" \t\r\n");
        if (first != std::string::npos && last != std::string::npos) {
            token = token.substr(first, last - first + 1);
        }
        if (token == "address" || token == "asan") {
            result = result | Sanitizer::Address;
        } else if (token == "undefined" || token == "ubsan") {
            result = result | Sanitizer::Undefined;
        } else if (token == "leak" || token == "lsan") {
            result = result | Sanitizer::Leak;
        } else if (token == "thread" || token == "tsan") {
            result = result | Sanitizer::Thread;
        } else if (token == "all") {
            result = result | Sanitizer::Address | Sanitizer::Undefined | Sanitizer::Leak;
        }
        start = comma + 1;
    }
    return result;
}

struct TargetConfig {
    std::string name = "";
    TargetType type = TargetType::Executable;
    std::vector<std::string> sources = {};
    std::vector<std::string> headers = {};
    std::vector<std::string> include_dirs = {};
    std::vector<std::string> defines = {};
    std::vector<std::string> compile_options = {};
    std::vector<std::string> extra_flags = {};
    std::vector<std::string> link_options = {};
    std::vector<std::string> link_libraries = {};
    std::vector<std::string> link_directories = {};
    CppVersion cpp_version = CppVersion::Cpp20;
    std::optional<CVersion> c_version = std::nullopt;
    TargetOptions target = {};
    Optimization optimize = Optimization::Debug;
    bool warnings_as_errors = false;
    bool exceptions = true;
    bool rtti = true;
    WindowsSubsystem subsystem = WindowsSubsystem::Console;
    std::string output_dir = "";
    std::string output_name = "";
    std::string pch_header = "";
    std::string pch_source = "";
    Sanitizer sanitizers = Sanitizer::None;
    bool coverage = false;
    bool enable_global_cache = true;
    bool enable_std_module = false;
};

struct DependencyConfig {
    std::string git = "";
    std::string tag = "";
    std::string branch = "";
    std::string commit = "";
    std::string url = "";
    std::string sha256 = "";
    std::filesystem::path path = "";
    std::filesystem::path cache_dir = "";
    bool shallow = true;
    bool force_fetch = false;
};

struct PackageConfig {
    std::string package_name = "";
    std::string version = "1.0.0";
    std::filesystem::path output_dir = "dist";
    std::vector<std::string> include_dirs = {};
    std::vector<std::string> artifacts = {};
    std::vector<std::string> extra_files = {};
    std::string format = "auto";
};

struct CMakeConfigOptions {
    std::string package_name = "";
    std::string version = "1.0.0";
    std::filesystem::path output_dir = "";
    std::string namespace_name = "";
    std::vector<std::string> target_names = {};
};

struct SystemPackageConfig {
    bool required = true;
    std::string version = "";
    std::vector<std::string> include_hints = {};
    std::vector<std::string> lib_hints = {};
    std::string install_hint = "";
};

struct SystemPackage {
    std::string name = "";
    bool found = false;
    std::string version = "";
    std::vector<std::string> include_dirs = {};
    std::vector<std::string> lib_dirs = {};
    std::vector<std::string> link_libraries = {};
    std::vector<std::string> compile_flags = {};
    std::vector<std::string> link_flags = {};
    std::vector<std::string> defines = {};
    std::string error_message = "";
    std::string install_instructions = "";

    [[nodiscard]] bool is_found() const noexcept { return found; }
    explicit operator bool() const noexcept { return found; }
};

class IToolchain;
struct BuildResult;
class Step;
class Builder;
class Dependency;

// ============================================================================
// 5. Target Abstraction
// ============================================================================

class Target {
public:
    explicit Target(TargetConfig config) : config_(std::move(config)) {
        if (!config_.extra_flags.empty()) {
            config_.compile_options.insert(config_.compile_options.end(), config_.extra_flags.begin(),
                                           config_.extra_flags.end());
        }
    }

    [[nodiscard]] const std::string &name() const noexcept { return config_.name; }
    [[nodiscard]] TargetType type() const noexcept { return config_.type; }

    [[nodiscard]] const TargetConfig &config() const noexcept { return config_; }
    TargetConfig &config() noexcept { return config_; }

    Target &link(const Target &dep);
    Target &link(std::string_view dep_name);
    Target &link(const SystemPackage &pkg);

    Target &depend_on(std::shared_ptr<Step> s);
    Target &depend_on(Step &s);
    Target &depend_on(const Target &dep);

    void set_build_step(std::shared_ptr<Step> s) { build_step_ = std::move(s); }
    [[nodiscard]] std::shared_ptr<Step> build_step() const noexcept { return build_step_; }

    Target &link_system_lib(std::string_view sys_lib) {
        if (std::find(linked_system_libs_.begin(), linked_system_libs_.end(), sys_lib) == linked_system_libs_.end()) {
            linked_system_libs_.emplace_back(sys_lib);
        }
        return *this;
    }

    Target &link_system_library(std::string_view sys_lib) { return link_system_lib(sys_lib); }
    Target &add_system_library(std::string_view sys_lib) { return link_system_lib(sys_lib); }

    Target &add_source(std::string_view src) {
        config_.sources.emplace_back(src);
        return *this;
    }

    Target &add_sources(std::initializer_list<std::string_view> sources) {
        for (auto src : sources) {
            config_.sources.emplace_back(src);
        }
        return *this;
    }

    Target &add_header(std::string_view hdr) {
        config_.headers.emplace_back(hdr);
        return *this;
    }

    Target &add_include_dir(std::string_view dir) {
        config_.include_dirs.emplace_back(dir);
        return *this;
    }

    Target &add_define(std::string_view def) {
        config_.defines.emplace_back(def);
        return *this;
    }

    Target &add_compile_option(std::string_view opt) {
        config_.compile_options.emplace_back(opt);
        return *this;
    }

    Target &add_link_option(std::string_view opt) {
        config_.link_options.emplace_back(opt);
        return *this;
    }

    Target &add_link_directory(std::string_view dir) {
        config_.link_directories.emplace_back(dir);
        return *this;
    }

    Target &set_output_dir(std::string_view dir) {
        config_.output_dir = std::string(dir);
        return *this;
    }

    Target &set_output_name(std::string_view name) {
        config_.output_name = std::string(name);
        return *this;
    }

    Target &set_subsystem(WindowsSubsystem sub) {
        config_.subsystem = sub;
        return *this;
    }

    Target &set_pch(std::string_view header, std::string_view source = "") {
        config_.pch_header = std::string(header);
        config_.pch_source = std::string(source);
        return *this;
    }

    [[nodiscard]] const std::string &pch_header() const noexcept { return config_.pch_header; }
    [[nodiscard]] const std::string &pch_source() const noexcept { return config_.pch_source; }

    Target &sanitize(Sanitizer s) noexcept {
        config_.sanitizers = config_.sanitizers | s;
        return *this;
    }

    Target &sanitize(std::string_view s_str) { return sanitize(parse_sanitizers(s_str)); }

    Target &coverage(bool enabled = true) noexcept {
        config_.coverage = enabled;
        return *this;
    }

    Target &set_coverage(bool enabled = true) noexcept { return coverage(enabled); }

    [[nodiscard]] Sanitizer sanitizers() const noexcept { return config_.sanitizers; }
    [[nodiscard]] bool has_coverage() const noexcept { return config_.coverage; }

    Target &set_target_triple(std::string_view triple) {
        config_.target.triple = std::string(triple);
        return *this;
    }

    Target &set_sysroot(const std::filesystem::path &sysroot) {
        config_.target.sysroot = sysroot;
        return *this;
    }

    [[nodiscard]] const std::string &target_triple() const noexcept { return config_.target.triple; }
    [[nodiscard]] const std::filesystem::path &sysroot() const noexcept { return config_.target.sysroot; }

    Target &enable_global_cache(bool enabled = true) noexcept {
        config_.enable_global_cache = enabled;
        return *this;
    }

    [[nodiscard]] bool is_global_cache_enabled() const noexcept { return config_.enable_global_cache; }

    Target &enable_std_module(bool enabled = true) noexcept {
        config_.enable_std_module = enabled;
        return *this;
    }

    [[nodiscard]] bool has_std_module() const noexcept { return config_.enable_std_module; }

    [[nodiscard]] const std::unordered_map<std::string, std::filesystem::path> &module_bmis() const noexcept {
        return module_bmis_;
    }

    void set_module_bmi(std::string module_name, std::filesystem::path bmi_path) {
        module_bmis_[std::move(module_name)] = std::move(bmi_path);
    }

    [[nodiscard]] std::filesystem::path artifact_path(const IToolchain &toolchain) const;

    [[nodiscard]] const std::vector<std::string> &linked_targets() const noexcept { return linked_targets_; }
    [[nodiscard]] const std::vector<std::string> &linked_system_libs() const noexcept { return linked_system_libs_; }

private:
    TargetConfig config_;
    std::vector<std::string> linked_targets_;
    std::vector<std::string> linked_system_libs_;
    std::unordered_map<std::string, std::filesystem::path> module_bmis_;
    std::shared_ptr<Step> build_step_ = nullptr;
};

// ============================================================================
// 6. Step Abstraction & DAG Dependency Model
// ============================================================================

enum class StepStatus { Success, Failed, Skipped, UpToDate };

enum class StepType { General, CompileSource, CompileBMI, LinkArtifact, EmbedAsset, Custom };

struct StepResult {
    StepStatus status = StepStatus::Success;
    int exit_code = 0;
    std::string error_message = "";
    std::chrono::milliseconds duration{0};

    [[nodiscard]] bool success() const noexcept {
        return status == StepStatus::Success || status == StepStatus::UpToDate;
    }

    static StepResult ok(std::chrono::milliseconds dur = std::chrono::milliseconds{0}) {
        return StepResult{.status = StepStatus::Success, .exit_code = 0, .duration = dur};
    }

    static StepResult fail(std::string_view msg, int code = 1,
                           std::chrono::milliseconds dur = std::chrono::milliseconds{0}) {
        return StepResult{.status = StepStatus::Failed,
                          .exit_code = code,
                          .error_message = std::string(msg),
                          .duration = dur};
    }

    static StepResult error(std::string_view msg, int code = 1,
                            std::chrono::milliseconds dur = std::chrono::milliseconds{0}) {
        return fail(msg, code, dur);
    }

    static StepResult up_to_date(std::chrono::milliseconds dur = std::chrono::milliseconds{0}) {
        return StepResult{.status = StepStatus::UpToDate, .exit_code = 0, .duration = dur};
    }
};

// ============================================================================
// Terminal UI, ANSI Color & Progress Reporting
// ============================================================================

namespace terminal {

enum class Color {
    Default,
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    Gray,
    BrightRed,
    BrightGreen,
    BrightYellow,
    BrightBlue,
    BrightMagenta,
    BrightCyan,
    BrightWhite,
};

enum class Style {
    None = 0,
    Bold = 1 << 0,
    Dim = 1 << 1,
    Italic = 1 << 2,
    Underline = 1 << 3,
};

inline constexpr Style operator|(Style a, Style b) noexcept {
    return static_cast<Style>(static_cast<int>(a) | static_cast<int>(b));
}

inline constexpr bool operator&(Style a, Style b) noexcept {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
}

inline std::mutex &get_terminal_mutex() {
    static std::mutex mtx;
    return mtx;
}

inline std::optional<bool> &color_override_storage() {
    static std::optional<bool> override_val = std::nullopt;
    return override_val;
}

inline void set_color_override(std::optional<bool> val) noexcept {
    color_override_storage() = val;
}

inline void init_console() noexcept {
#if defined(_WIN32)
    static std::once_flag flag;
    std::call_once(flag, []() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE && hOut != NULL) {
            DWORD mode = 0;
            if (GetConsoleMode(hOut, &mode)) {
                SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            }
        }
    });
#endif
}

inline bool is_unicode_supported() noexcept {
    init_console();
#if defined(_WIN32)
    return GetConsoleOutputCP() == CP_UTF8;
#else
    const char *term = std::getenv("TERM");
    if (term != nullptr && std::string_view(term) == "dumb") {
        return false;
    }
    return true;
#endif
}

inline bool is_color_supported() noexcept {
    init_console();
    const char *no_color = std::getenv("NO_COLOR");
    if (no_color != nullptr && no_color[0] != '\0') {
        return false;
    }
    const char *term = std::getenv("TERM");
    if (term != nullptr && std::string_view(term) == "dumb") {
        return false;
    }
#if defined(_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE || hOut == NULL) {
        return false;
    }
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) {
        return false;
    }
    if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
        if (!SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            return false;
        }
    }
    return true;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}

inline bool color_enabled() noexcept {
    auto &override_val = color_override_storage();
    if (override_val.has_value()) {
        return *override_val;
    }
    return is_color_supported();
}

inline std::string ansi_sequence(Color color, Style style = Style::None) {
    std::string seq = "\033[";
    bool first = true;

    auto append_code = [&](std::string_view code) {
        if (!first)
            seq += ";";
        seq += code;
        first = false;
    };

    if (style & Style::Bold)
        append_code("1");
    if (style & Style::Dim)
        append_code("2");
    if (style & Style::Italic)
        append_code("3");
    if (style & Style::Underline)
        append_code("4");

    switch (color) {
    case Color::Black:
        append_code("30");
        break;
    case Color::Red:
        append_code("31");
        break;
    case Color::Green:
        append_code("32");
        break;
    case Color::Yellow:
        append_code("33");
        break;
    case Color::Blue:
        append_code("34");
        break;
    case Color::Magenta:
        append_code("35");
        break;
    case Color::Cyan:
        append_code("36");
        break;
    case Color::White:
        append_code("37");
        break;
    case Color::Gray:
        append_code("90");
        break;
    case Color::BrightRed:
        append_code("91");
        break;
    case Color::BrightGreen:
        append_code("92");
        break;
    case Color::BrightYellow:
        append_code("93");
        break;
    case Color::BrightBlue:
        append_code("94");
        break;
    case Color::BrightMagenta:
        append_code("95");
        break;
    case Color::BrightCyan:
        append_code("96");
        break;
    case Color::BrightWhite:
        append_code("97");
        break;
    case Color::Default:
        if (first) {
            return "";
        }
        break;
    }

    if (first) {
        return "";
    }
    seq += "m";
    return seq;
}

inline constexpr std::string_view ansi_reset() noexcept {
    return "\033[0m";
}

inline std::string colorize(std::string_view text, Color color, Style style = Style::None, bool force_color = false) {
    if (!force_color && !color_enabled()) {
        return std::string(text);
    }
    std::string seq = ansi_sequence(color, style);
    if (seq.empty()) {
        return std::string(text);
    }
    return seq + std::string(text) + std::string(ansi_reset());
}

inline std::string strip_ansi(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\033' && i + 1 < text.size() && text[i + 1] == '[') {
            i += 2;
            while (i < text.size() && text[i] != 'm') {
                ++i;
            }
        } else {
            out.push_back(text[i]);
        }
    }
    return out;
}

inline std::string bold(std::string_view text, bool force_color = false) {
    return colorize(text, Color::Default, Style::Bold, force_color);
}
inline std::string dim(std::string_view text, bool force_color = false) {
    return colorize(text, Color::Default, Style::Dim, force_color);
}
inline std::string red(std::string_view text, bool force_color = false) {
    return colorize(text, Color::Red, Style::None, force_color);
}
inline std::string green(std::string_view text, bool force_color = false) {
    return colorize(text, Color::Green, Style::None, force_color);
}
inline std::string yellow(std::string_view text, bool force_color = false) {
    return colorize(text, Color::Yellow, Style::None, force_color);
}
inline std::string blue(std::string_view text, bool force_color = false) {
    return colorize(text, Color::Blue, Style::None, force_color);
}
inline std::string cyan(std::string_view text, bool force_color = false) {
    return colorize(text, Color::Cyan, Style::None, force_color);
}
inline std::string magenta(std::string_view text, bool force_color = false) {
    return colorize(text, Color::Magenta, Style::None, force_color);
}
inline std::string gray(std::string_view text, bool force_color = false) {
    return colorize(text, Color::Gray, Style::None, force_color);
}
inline std::string bright_red(std::string_view text, bool force_color = false) {
    return colorize(text, Color::BrightRed, Style::None, force_color);
}
inline std::string bright_green(std::string_view text, bool force_color = false) {
    return colorize(text, Color::BrightGreen, Style::None, force_color);
}
inline std::string bright_yellow(std::string_view text, bool force_color = false) {
    return colorize(text, Color::BrightYellow, Style::None, force_color);
}
inline std::string bright_cyan(std::string_view text, bool force_color = false) {
    return colorize(text, Color::BrightCyan, Style::None, force_color);
}

inline std::string format_step_status(size_t index, size_t total, std::string_view action, std::string_view target,
                                      double duration_seconds, bool cached = false, bool use_color = true) {
    (void)cached;
    size_t total_digits = total > 0 ? std::to_string(total).size() : 1;
    int pct = (total > 0) ? static_cast<int>((index * 100) / total) : 0;
    if (pct > 100)
        pct = 100;

    std::ostringstream ss_index;
    ss_index << std::setw(static_cast<int>(total_digits)) << index << "/" << total;

    std::ostringstream ss_pct;
    ss_pct << std::setw(3) << pct << "%";

    std::ostringstream ss_dur;
    ss_dur << std::fixed << std::setprecision(2) << duration_seconds << "s";

    if (use_color) {
        std::ostringstream ss;
        ss << gray("[", true) << " " << cyan(ss_index.str(), true) << " " << gray("]", true) << " " << gray("[", true)
           << green(ss_pct.str(), true) << gray("]", true) << " " << colorize(action, Color::Cyan, Style::Bold, true)
           << " " << bold(target, true) << " " << gray("(" + ss_dur.str() + ")", true);
        return ss.str();
    } else {
        std::ostringstream ss;
        ss << "[ " << ss_index.str() << " ] "
           << "[" << ss_pct.str() << "] " << action << " " << target << " "
           << "(" << ss_dur.str() << ")";
        return ss.str();
    }
}

inline std::string format_summary(bool success, double total_seconds, size_t compiled, size_t cached, size_t failed = 0,
                                  bool use_color = true) {
    std::ostringstream ss_dur;
    ss_dur << std::fixed << std::setprecision(2) << total_seconds << "s";

    std::ostringstream ss_counts;
    ss_counts << compiled << " compiled";
    if (failed > 0) {
        ss_counts << ", " << failed << " failed";
    }
    ss_counts << ", " << cached << " cached";

    if (use_color) {
        std::ostringstream ss;
        if (success) {
            ss << colorize("Build completed successfully", Color::Green, Style::Bold, true);
        } else {
            ss << colorize("Build failed", Color::Red, Style::Bold, true);
        }
        ss << " in " << cyan(ss_dur.str(), true) << " (" << ss_counts.str() << ")";
        return ss.str();
    } else {
        std::ostringstream ss;
        if (success) {
            ss << "Build completed successfully in " << ss_dur.str() << " (" << ss_counts.str() << ")";
        } else {
            ss << "Build failed in " << ss_dur.str() << " (" << ss_counts.str() << ")";
        }
        return ss.str();
    }
}

class ProgressReporter {
public:
    explicit ProgressReporter(size_t total_steps = 0, std::ostream &os = std::cout,
                              std::optional<bool> use_color = std::nullopt)
        : total_steps_(total_steps), os_(os), use_color_(use_color.value_or(color_enabled())),
          start_time_(std::chrono::steady_clock::now()) {}

    void reset(size_t total_steps = 0) {
        std::lock_guard<std::mutex> lock(get_terminal_mutex());
        total_steps_ = total_steps;
        current_step_.store(0);
        compiled_count_.store(0);
        cached_count_.store(0);
        failed_count_.store(0);
        start_time_ = std::chrono::steady_clock::now();
    }

    void set_total_steps(size_t total) noexcept { total_steps_ = total; }

    [[nodiscard]] size_t total_steps() const noexcept { return total_steps_; }

    [[nodiscard]] size_t current_step() const noexcept { return current_step_.load(); }

    [[nodiscard]] size_t compiled_count() const noexcept { return compiled_count_.load(); }

    [[nodiscard]] size_t cached_count() const noexcept { return cached_count_.load(); }

    [[nodiscard]] size_t failed_count() const noexcept { return failed_count_.load(); }

    [[nodiscard]] double elapsed_seconds() const noexcept {
        auto now = std::chrono::steady_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::duration<double>>(now - start_time_);
        return dur.count();
    }

    void report_step(std::string_view action, std::string_view target, double duration_seconds, bool cached = false) {
        size_t idx = ++current_step_;
        if (cached) {
            ++cached_count_;
        } else {
            ++compiled_count_;
        }
        std::string line = format_step_status(idx, total_steps_, action, target, duration_seconds, cached, use_color_);
        std::lock_guard<std::mutex> lock(get_terminal_mutex());
        os_ << line << "\n";
        os_.flush();
    }

    void report_step_at(size_t index, std::string_view action, std::string_view target, double duration_seconds,
                        bool cached = false) {
        if (cached) {
            ++cached_count_;
        } else {
            ++compiled_count_;
        }
        std::string line =
            format_step_status(index, total_steps_, action, target, duration_seconds, cached, use_color_);
        std::lock_guard<std::mutex> lock(get_terminal_mutex());
        os_ << line << "\n";
        os_.flush();
    }

    void report_failure(std::string_view action, std::string_view target, double duration_seconds,
                        std::string_view error_msg = "") {
        size_t idx = ++current_step_;
        ++failed_count_;
        std::string line = format_step_status(idx, total_steps_, action, target, duration_seconds, false, use_color_);
        std::lock_guard<std::mutex> lock(get_terminal_mutex());
        os_ << line << "\n";
        if (!error_msg.empty()) {
            os_ << error_msg << "\n";
        }
        os_.flush();
    }

    std::string summary() const {
        double elapsed = elapsed_seconds();
        bool success = (failed_count_.load() == 0);
        return format_summary(success, elapsed, compiled_count_.load(), cached_count_.load(), failed_count_.load(),
                              use_color_);
    }

    void print_summary() {
        std::string s = summary();
        std::lock_guard<std::mutex> lock(get_terminal_mutex());
        os_ << s << "\n";
        os_.flush();
    }

    void print(std::string_view message) {
        std::lock_guard<std::mutex> lock(get_terminal_mutex());
        os_ << message << "\n";
        os_.flush();
    }

    void print_error(std::string_view message) {
        std::lock_guard<std::mutex> lock(get_terminal_mutex());
        os_ << message << "\n";
        os_.flush();
    }

private:
    size_t total_steps_ = 0;
    std::ostream &os_;
    bool use_color_ = true;
    std::atomic<size_t> current_step_{0};
    std::atomic<size_t> compiled_count_{0};
    std::atomic<size_t> cached_count_{0};
    std::atomic<size_t> failed_count_{0};
    std::chrono::steady_clock::time_point start_time_;
};

class Terminal {
public:
    static bool is_color_supported() noexcept { return terminal::is_color_supported(); }
    static bool color_enabled() noexcept { return terminal::color_enabled(); }
    static void set_color_override(std::optional<bool> val) noexcept { terminal::set_color_override(val); }
    static std::string colorize(std::string_view text, Color color, Style style = Style::None, bool force = false) {
        return terminal::colorize(text, color, style, force);
    }
    static std::string bold(std::string_view text, bool force = false) { return terminal::bold(text, force); }
    static std::string dim(std::string_view text, bool force = false) { return terminal::dim(text, force); }
    static std::string red(std::string_view text, bool force = false) { return terminal::red(text, force); }
    static std::string green(std::string_view text, bool force = false) { return terminal::green(text, force); }
    static std::string yellow(std::string_view text, bool force = false) { return terminal::yellow(text, force); }
    static std::string blue(std::string_view text, bool force = false) { return terminal::blue(text, force); }
    static std::string magenta(std::string_view text, bool force = false) { return terminal::magenta(text, force); }
    static std::string cyan(std::string_view text, bool force = false) { return terminal::cyan(text, force); }
    static std::string gray(std::string_view text, bool force = false) { return terminal::gray(text, force); }
    static std::string bright_red(std::string_view text, bool force = false) {
        return terminal::bright_red(text, force);
    }
    static std::string bright_green(std::string_view text, bool force = false) {
        return terminal::bright_green(text, force);
    }
    static std::string bright_yellow(std::string_view text, bool force = false) {
        return terminal::bright_yellow(text, force);
    }
    static std::string bright_cyan(std::string_view text, bool force = false) {
        return terminal::bright_cyan(text, force);
    }
    static std::string strip_ansi(std::string_view text) { return terminal::strip_ansi(text); }
};

enum class DiagnosticSeverity {
    Unknown,
    Error,
    FatalError,
    Warning,
    Note,
};

struct DiagnosticMessage {
    std::string raw_line;
    std::string file;
    uint32_t line = 0;
    uint32_t column = 0;
    DiagnosticSeverity severity = DiagnosticSeverity::Unknown;
    std::string code;
    std::string message;
    bool is_msvc = false;
    bool is_parsed = false;
};

inline DiagnosticMessage parse_line(std::string_view raw) {
    DiagnosticMessage diag;
    diag.raw_line = std::string(raw);

    while (!raw.empty() && (raw.back() == '\r' || raw.back() == '\n' || raw.back() == ' ')) {
        raw.remove_suffix(1);
    }
    if (raw.empty()) {
        return diag;
    }

    // 1. Try MSVC format: <file>(<line>[,<col>]): <severity> [<code>:] <message>
    size_t paren_open = raw.find('(');
    if (paren_open != std::string_view::npos && paren_open > 0) {
        size_t paren_close = raw.find(')', paren_open + 1);
        if (paren_close != std::string_view::npos && paren_close + 1 < raw.size() && raw[paren_close + 1] == ':') {
            std::string_view in_parens = raw.substr(paren_open + 1, paren_close - (paren_open + 1));
            size_t comma = in_parens.find(',');
            uint32_t line_num = 0;
            uint32_t col_num = 0;
            bool valid_nums = false;

            auto parse_u32 = [](std::string_view s, uint32_t &val) -> bool {
                while (!s.empty() && s.front() == ' ')
                    s.remove_prefix(1);
                while (!s.empty() && s.back() == ' ')
                    s.remove_suffix(1);
                if (s.empty())
                    return false;
                uint32_t res = 0;
                for (char c : s) {
                    if (c < '0' || c > '9')
                        return false;
                    res = res * 10 + (c - '0');
                }
                val = res;
                return true;
            };

            if (comma != std::string_view::npos) {
                if (parse_u32(in_parens.substr(0, comma), line_num) &&
                    parse_u32(in_parens.substr(comma + 1), col_num)) {
                    valid_nums = true;
                }
            } else {
                if (parse_u32(in_parens, line_num)) {
                    valid_nums = true;
                }
            }

            if (valid_nums) {
                size_t after_colon = paren_close + 2;
                while (after_colon < raw.size() && raw[after_colon] == ' ') {
                    ++after_colon;
                }
                std::string_view remainder = raw.substr(after_colon);
                DiagnosticSeverity sev = DiagnosticSeverity::Unknown;
                size_t sev_len = 0;

                if (remainder.rfind("fatal error", 0) == 0) {
                    sev = DiagnosticSeverity::FatalError;
                    sev_len = 11;
                } else if (remainder.rfind("error", 0) == 0) {
                    sev = DiagnosticSeverity::Error;
                    sev_len = 5;
                } else if (remainder.rfind("warning", 0) == 0) {
                    sev = DiagnosticSeverity::Warning;
                    sev_len = 7;
                } else if (remainder.rfind("note:", 0) == 0) {
                    sev = DiagnosticSeverity::Note;
                    sev_len = 5;
                } else if (remainder.rfind("note", 0) == 0) {
                    sev = DiagnosticSeverity::Note;
                    sev_len = 4;
                }

                if (sev != DiagnosticSeverity::Unknown) {
                    diag.file = std::string(raw.substr(0, paren_open));
                    diag.line = line_num;
                    diag.column = col_num;
                    diag.severity = sev;
                    diag.is_msvc = true;

                    std::string_view after_sev = remainder.substr(sev_len);
                    while (!after_sev.empty() && after_sev.front() == ' ') {
                        after_sev.remove_prefix(1);
                    }

                    size_t code_colon = after_sev.find(':');
                    if (code_colon != std::string_view::npos && code_colon > 0 && code_colon < 12) {
                        std::string_view candidate_code = after_sev.substr(0, code_colon);
                        if (!candidate_code.empty() &&
                            (candidate_code[0] == 'C' || candidate_code[0] == 'D' || candidate_code[0] == 'L')) {
                            diag.code = std::string(candidate_code);
                            std::string_view msg_part = after_sev.substr(code_colon + 1);
                            while (!msg_part.empty() && msg_part.front() == ' ') {
                                msg_part.remove_prefix(1);
                            }
                            diag.message = std::string(msg_part);
                        } else {
                            diag.message = std::string(after_sev);
                        }
                    } else {
                        diag.message = std::string(after_sev);
                    }

                    diag.is_parsed = true;
                    return diag;
                }
            }
        }
    }

    // 2. Try Clang/GCC format: <file>:<line>[:<col>]: <severity>: <message>
    size_t scan_start = 0;
    if (raw.size() >= 3 && std::isalpha(static_cast<unsigned char>(raw[0])) && raw[1] == ':' &&
        (raw[2] == '/' || raw[2] == '\\')) {
        scan_start = 2; // skip Windows drive letter C:
    }

    size_t col1 = raw.find(':', scan_start);
    if (col1 != std::string_view::npos && col1 > 0) {
        size_t col2 = raw.find(':', col1 + 1);
        if (col2 != std::string_view::npos) {
            std::string_view line_str = raw.substr(col1 + 1, col2 - (col1 + 1));
            auto is_digits = [](std::string_view s) {
                if (s.empty())
                    return false;
                for (char c : s) {
                    if (c < '0' || c > '9')
                        return false;
                }
                return true;
            };

            if (is_digits(line_str)) {
                uint32_t line_num = static_cast<uint32_t>(std::stoul(std::string(line_str)));
                size_t col3 = raw.find(':', col2 + 1);
                uint32_t col_num = 0;
                size_t sev_start = col2 + 1;

                if (col3 != std::string_view::npos && is_digits(raw.substr(col2 + 1, col3 - (col2 + 1)))) {
                    col_num = static_cast<uint32_t>(std::stoul(std::string(raw.substr(col2 + 1, col3 - (col2 + 1)))));
                    sev_start = col3 + 1;
                }

                while (sev_start < raw.size() && raw[sev_start] == ' ') {
                    ++sev_start;
                }

                std::string_view remainder = raw.substr(sev_start);
                DiagnosticSeverity sev = DiagnosticSeverity::Unknown;
                size_t sev_len = 0;

                if (remainder.rfind("fatal error:", 0) == 0) {
                    sev = DiagnosticSeverity::FatalError;
                    sev_len = 12;
                } else if (remainder.rfind("error:", 0) == 0) {
                    sev = DiagnosticSeverity::Error;
                    sev_len = 6;
                } else if (remainder.rfind("warning:", 0) == 0) {
                    sev = DiagnosticSeverity::Warning;
                    sev_len = 8;
                } else if (remainder.rfind("note:", 0) == 0) {
                    sev = DiagnosticSeverity::Note;
                    sev_len = 5;
                }

                if (sev != DiagnosticSeverity::Unknown) {
                    diag.file = std::string(raw.substr(0, col1));
                    diag.line = line_num;
                    diag.column = col_num;
                    diag.severity = sev;
                    diag.is_msvc = false;

                    std::string_view msg_part = remainder.substr(sev_len);
                    while (!msg_part.empty() && msg_part.front() == ' ') {
                        msg_part.remove_prefix(1);
                    }

                    if (!msg_part.empty() && msg_part.back() == ']') {
                        size_t bracket = msg_part.rfind(" [");
                        if (bracket != std::string_view::npos) {
                            diag.code = std::string(msg_part.substr(bracket + 2, msg_part.size() - bracket - 3));
                            msg_part = msg_part.substr(0, bracket);
                        }
                    }

                    diag.message = std::string(msg_part);
                    diag.is_parsed = true;
                    return diag;
                }
            }
        }
    }

    return diag;
}

inline std::string format_diagnostic(const DiagnosticMessage &diag, bool use_color = true) {
    if (!diag.is_parsed) {
        if (!use_color) {
            return diag.raw_line;
        }
        bool has_caret = false;
        for (char c : diag.raw_line) {
            if (c == '^' || c == '~') {
                has_caret = true;
                break;
            }
        }
        if (has_caret) {
            std::string out;
            out.reserve(diag.raw_line.size() + 16);
            for (char c : diag.raw_line) {
                if (c == '^') {
                    out += bright_green("^", true);
                } else if (c == '~') {
                    out += green("~", true);
                } else {
                    out.push_back(c);
                }
            }
            return out;
        }
        return diag.raw_line;
    }

    if (!use_color) {
        return diag.raw_line;
    }

    std::ostringstream ss;
    std::string loc;
    if (diag.is_msvc) {
        loc = diag.file + "(" + std::to_string(diag.line);
        if (diag.column > 0) {
            loc += "," + std::to_string(diag.column);
        }
        loc += "):";
    } else {
        loc = diag.file + ":" + std::to_string(diag.line);
        if (diag.column > 0) {
            loc += ":" + std::to_string(diag.column);
        }
        loc += ":";
    }
    ss << colorize(loc, Color::Cyan, Style::Bold, true) << " ";

    switch (diag.severity) {
    case DiagnosticSeverity::FatalError:
        ss << colorize("fatal error", Color::BrightRed, Style::Bold, true);
        if (!diag.code.empty()) {
            ss << " " << colorize(diag.code, Color::BrightRed, Style::Bold, true);
        }
        ss << ": ";
        break;
    case DiagnosticSeverity::Error:
        ss << colorize("error", Color::Red, Style::Bold, true);
        if (!diag.code.empty()) {
            ss << " " << colorize(diag.code, Color::Red, Style::Bold, true);
        }
        ss << ": ";
        break;
    case DiagnosticSeverity::Warning:
        ss << colorize("warning", Color::Yellow, Style::Bold, true);
        if (!diag.code.empty()) {
            ss << " " << colorize(diag.code, Color::Yellow, Style::Bold, true);
        }
        ss << ": ";
        break;
    case DiagnosticSeverity::Note:
        ss << colorize("note", Color::BrightCyan, Style::Bold, true) << ": ";
        break;
    case DiagnosticSeverity::Unknown:
        break;
    }

    ss << bold(diag.message, true);

    if (!diag.is_msvc && !diag.code.empty()) {
        ss << " " << gray("[" + diag.code + "]", true);
    }

    return ss.str();
}

class DiagnosticDemangler {
public:
    static std::string strip_inline_namespaces(std::string text) {
        static const std::string patterns[] = {"std::__cxx11::", "std::__1::", "std::__cxx17::", "std::__cxx20::"};
        for (const auto &pat : patterns) {
            size_t pos = 0;
            while ((pos = text.find(pat, pos)) != std::string::npos) {
                text.replace(pos, pat.size(), "std::");
                pos += 5; // length of "std::"
            }
        }
        return text;
    }

    static std::vector<std::string> split_template_args(std::string_view args_str) {
        std::vector<std::string> result;
        int depth = 0;
        int paren_depth = 0;
        size_t start = 0;
        for (size_t i = 0; i < args_str.size(); ++i) {
            char c = args_str[i];
            if (c == '<') {
                depth++;
            } else if (c == '>') {
                if (depth > 0)
                    depth--;
            } else if (c == '(') {
                paren_depth++;
            } else if (c == ')') {
                if (paren_depth > 0)
                    paren_depth--;
            } else if (c == ',' && depth == 0 && paren_depth == 0) {
                std::string_view chunk = args_str.substr(start, i - start);
                while (!chunk.empty() && std::isspace(static_cast<unsigned char>(chunk.front())))
                    chunk.remove_prefix(1);
                while (!chunk.empty() && std::isspace(static_cast<unsigned char>(chunk.back())))
                    chunk.remove_suffix(1);
                result.emplace_back(chunk);
                start = i + 1;
            }
        }
        if (start < args_str.size()) {
            std::string_view chunk = args_str.substr(start);
            while (!chunk.empty() && std::isspace(static_cast<unsigned char>(chunk.front())))
                chunk.remove_prefix(1);
            while (!chunk.empty() && std::isspace(static_cast<unsigned char>(chunk.back())))
                chunk.remove_suffix(1);
            if (!chunk.empty()) {
                result.emplace_back(chunk);
            }
        }
        return result;
    }

    static size_t find_matching_angle_bracket(std::string_view s, size_t open_pos) {
        int depth = 0;
        for (size_t i = open_pos; i < s.size(); ++i) {
            if (s[i] == '<') {
                depth++;
            } else if (s[i] == '>') {
                depth--;
                if (depth == 0) {
                    return i;
                }
            }
        }
        return std::string_view::npos;
    }

    static std::string simplify_type(std::string_view raw_type) {
        std::string cleaned = strip_inline_namespaces(std::string(raw_type));
        std::string_view sv = cleaned;
        while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front())))
            sv.remove_prefix(1);
        while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back())))
            sv.remove_suffix(1);

        if (sv.empty())
            return "";

        std::string leading;
        if (sv.rfind("const ", 0) == 0) {
            leading = "const ";
            sv.remove_prefix(6);
        } else if (sv.rfind("volatile ", 0) == 0) {
            leading = "volatile ";
            sv.remove_prefix(9);
        }
        if (sv.rfind("class ", 0) == 0) {
            sv.remove_prefix(6);
        } else if (sv.rfind("struct ", 0) == 0) {
            sv.remove_prefix(7);
        }

        std::string trailing;
        while (!sv.empty() && (sv.back() == ' ' || sv.back() == '&' || sv.back() == '*')) {
            trailing = sv.back() + trailing;
            sv.remove_suffix(1);
        }
        if (sv.size() >= 5 && sv.substr(sv.size() - 5) == "const") {
            trailing = " const" + trailing;
            sv.remove_suffix(5);
            while (!sv.empty() && sv.back() == ' ')
                sv.remove_suffix(1);
        }

        size_t open_pos = sv.find('<');
        if (open_pos == std::string_view::npos) {
            return leading + std::string(sv) + trailing;
        }

        size_t close_pos = find_matching_angle_bracket(sv, open_pos);
        if (close_pos == std::string_view::npos) {
            return leading + std::string(sv) + trailing;
        }

        std::string_view base = sv.substr(0, open_pos);
        while (!base.empty() && std::isspace(static_cast<unsigned char>(base.back())))
            base.remove_suffix(1);
        std::string_view inner = sv.substr(open_pos + 1, close_pos - open_pos - 1);
        std::string_view extra_trailing = sv.substr(close_pos + 1);

        auto args = split_template_args(inner);
        for (auto &arg : args) {
            arg = simplify_type(arg);
        }

        // Simplify std::basic_string
        if (base == "std::basic_string" || base == "basic_string") {
            if (!args.empty()) {
                if (args[0] == "char")
                    return leading + "std::string" + std::string(extra_trailing) + trailing;
                if (args[0] == "wchar_t")
                    return leading + "std::wstring" + std::string(extra_trailing) + trailing;
                if (args[0] == "char8_t")
                    return leading + "std::u8string" + std::string(extra_trailing) + trailing;
                if (args[0] == "char16_t")
                    return leading + "std::u16string" + std::string(extra_trailing) + trailing;
                if (args[0] == "char32_t")
                    return leading + "std::u32string" + std::string(extra_trailing) + trailing;
            }
        }

        // Simplify std::basic_string_view
        if (base == "std::basic_string_view" || base == "basic_string_view") {
            if (!args.empty()) {
                if (args[0] == "char")
                    return leading + "std::string_view" + std::string(extra_trailing) + trailing;
                if (args[0] == "wchar_t")
                    return leading + "std::wstring_view" + std::string(extra_trailing) + trailing;
            }
        }

        // Simplify streams
        if (base == "std::basic_ostream" || base == "basic_ostream") {
            if (!args.empty() && args[0] == "char")
                return leading + "std::ostream" + std::string(extra_trailing) + trailing;
            if (!args.empty() && args[0] == "wchar_t")
                return leading + "std::wostream" + std::string(extra_trailing) + trailing;
        }
        if (base == "std::basic_istream" || base == "basic_istream") {
            if (!args.empty() && args[0] == "char")
                return leading + "std::istream" + std::string(extra_trailing) + trailing;
            if (!args.empty() && args[0] == "wchar_t")
                return leading + "std::wistream" + std::string(extra_trailing) + trailing;
        }
        if (base == "std::basic_iostream" || base == "basic_iostream") {
            if (!args.empty() && args[0] == "char")
                return leading + "std::iostream" + std::string(extra_trailing) + trailing;
        }
        if (base == "std::basic_stringstream" || base == "basic_stringstream") {
            if (!args.empty() && args[0] == "char")
                return leading + "std::stringstream" + std::string(extra_trailing) + trailing;
        }

        // Simplify std::vector
        if (base == "std::vector" || base == "vector") {
            if (args.size() >= 2) {
                if (args[1] == "std::allocator<" + args[0] + ">" || args[1] == "allocator<" + args[0] + ">" ||
                    args[1].find("allocator") != std::string::npos) {
                    args.resize(1);
                }
            }
            if (!args.empty()) {
                return leading + "std::vector<" + args[0] + ">" + std::string(extra_trailing) + trailing;
            }
        }

        // Simplify std::deque, std::list
        if (base == "std::deque" || base == "deque") {
            if (args.size() >= 2 && args[1].find("allocator") != std::string::npos)
                args.resize(1);
            if (!args.empty()) {
                return leading + "std::deque<" + args[0] + ">" + std::string(extra_trailing) + trailing;
            }
        }
        if (base == "std::list" || base == "list") {
            if (args.size() >= 2 && args[1].find("allocator") != std::string::npos)
                args.resize(1);
            if (!args.empty()) {
                return leading + "std::list<" + args[0] + ">" + std::string(extra_trailing) + trailing;
            }
        }

        // Simplify std::unique_ptr
        if (base == "std::unique_ptr" || base == "unique_ptr") {
            if (args.size() >= 2 && args[1].find("default_delete") != std::string::npos) {
                args.resize(1);
            }
            if (!args.empty()) {
                return leading + "std::unique_ptr<" + args[0] + ">" + std::string(extra_trailing) + trailing;
            }
        }

        // Simplify std::map
        if (base == "std::map" || base == "map") {
            if (args.size() >= 3 && args[2].find("less") != std::string::npos) {
                args.resize(2);
            }
            if (args.size() >= 2) {
                return leading + "std::map<" + args[0] + ", " + args[1] + ">" + std::string(extra_trailing) + trailing;
            }
        }

        // Simplify std::set
        if (base == "std::set" || base == "set") {
            if (args.size() >= 2 && args[1].find("less") != std::string::npos) {
                args.resize(1);
            }
            if (!args.empty()) {
                return leading + "std::set<" + args[0] + ">" + std::string(extra_trailing) + trailing;
            }
        }

        // Simplify std::unordered_map
        if (base == "std::unordered_map" || base == "unordered_map") {
            if (args.size() >= 3) {
                args.resize(2);
            }
            if (args.size() >= 2) {
                return leading + "std::unordered_map<" + args[0] + ", " + args[1] + ">" + std::string(extra_trailing) +
                       trailing;
            }
        }

        // Simplify std::unordered_set
        if (base == "std::unordered_set" || base == "unordered_set") {
            if (args.size() >= 2) {
                args.resize(1);
            }
            if (!args.empty()) {
                return leading + "std::unordered_set<" + args[0] + ">" + std::string(extra_trailing) + trailing;
            }
        }

        std::string rebuilt = leading + std::string(base) + "<";
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0)
                rebuilt += ", ";
            rebuilt += args[i];
        }
        rebuilt += ">" + std::string(extra_trailing) + trailing;
        return rebuilt;
    }

    static std::string demangle_text(std::string_view text) {
        std::string out = strip_inline_namespaces(std::string(text));

        // Simplify types enclosed in quotes: '...' or `...` or ‘...’
        auto simplify_enclosed = [&](char open_c, char close_c) {
            size_t pos = 0;
            while (pos < out.size()) {
                size_t o = out.find(open_c, pos);
                if (o == std::string::npos)
                    break;
                size_t c = out.find(close_c, o + 1);
                if (c == std::string::npos)
                    break;
                std::string inner = out.substr(o + 1, c - o - 1);
                if (inner.find('<') != std::string::npos && inner.find('>') != std::string::npos) {
                    std::string simp = simplify_type(inner);
                    out.replace(o + 1, inner.size(), simp);
                    pos = o + 1 + simp.size() + 1;
                } else {
                    pos = c + 1;
                }
            }
        };

        simplify_enclosed('\'', '\'');
        simplify_enclosed('`', '`');

        // Look for common long type patterns without quotes
        static const std::pair<std::string, std::string> replacements[] = {
            {"std::basic_string<char, std::char_traits<char>, std::allocator<char>>", "std::string"},
            {"basic_string<char, std::char_traits<char>, std::allocator<char>>", "std::string"},
            {"std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>", "std::wstring"},
            {"basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>", "std::wstring"},
            {"std::basic_string_view<char, std::char_traits<char>>", "std::string_view"},
            {"basic_string_view<char, std::char_traits<char>>", "std::string_view"},
            {"std::basic_ostream<char, std::char_traits<char>>", "std::ostream"},
            {"basic_ostream<char, std::char_traits<char>>", "std::ostream"},
            {"std::basic_istream<char, std::char_traits<char>>", "std::istream"},
            {"basic_istream<char, std::char_traits<char>>", "std::istream"},
            {"std::basic_iostream<char, std::char_traits<char>>", "std::iostream"},
            {"basic_iostream<char, std::char_traits<char>>", "std::iostream"},
        };

        for (const auto &[pat, rep] : replacements) {
            size_t pos = 0;
            while ((pos = out.find(pat, pos)) != std::string::npos) {
                out.replace(pos, pat.size(), rep);
                pos += rep.size();
            }
        }

        return out;
    }
};

inline size_t visible_length(std::string_view s) {
    size_t len = 0;
    bool in_escape = false;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\033') {
            in_escape = true;
        } else if (in_escape) {
            if (s[i] == 'm') {
                in_escape = false;
            }
        } else {
            unsigned char c = static_cast<unsigned char>(s[i]);
            if ((c & 0xC0) != 0x80) {
                len++;
            }
        }
    }
    return len;
}

struct BacktraceFrame {
    std::string location;
    std::string description;
    bool is_stdlib = false;
};

struct DiagnosticCard {
    std::string file;
    int line = 0;
    int column = 0;
    std::string severity = "error";
    std::string code;
    std::string message;
    std::vector<std::string> snippet_lines;
    std::string concept_name;
    std::vector<std::string> reasons;
    std::vector<BacktraceFrame> backtrace;
    std::vector<std::string> notes;
};

class DiagnosticCardFormatter {
public:
    static bool has_template_or_concept_error(std::string_view raw_output) {
        static const std::string_view indicators[] = {"constraints not satisfied",
                                                      "does not satisfy",
                                                      "the concept '",
                                                      "concept '",
                                                      "associated constraints are not satisfied",
                                                      "in instantiation of",
                                                      "template instantiation",
                                                      "required from here",
                                                      "required from '",
                                                      "basic_string<char",
                                                      "std::__cxx11",
                                                      "std::allocator",
                                                      "allocator<",
                                                      "default_delete<"};
        for (auto ind : indicators) {
            if (raw_output.find(ind) != std::string_view::npos) {
                return true;
            }
        }
        return false;
    }

    static std::vector<DiagnosticCard> parse_cards(std::string_view raw_output) {
        std::vector<DiagnosticCard> cards;
        DiagnosticCard *current_card = nullptr;

        size_t start = 0;
        while (start < raw_output.size()) {
            size_t end = raw_output.find('\n', start);
            std::string_view line_sv;
            if (end != std::string_view::npos) {
                line_sv = raw_output.substr(start, end - start);
                start = end + 1;
            } else {
                line_sv = raw_output.substr(start);
                start = raw_output.size();
            }

            // Remove trailing CR
            if (!line_sv.empty() && line_sv.back() == '\r') {
                line_sv.remove_suffix(1);
            }

            if (line_sv.empty())
                continue;

            DiagnosticMessage diag = parse_line(line_sv);
            bool is_primary_error =
                diag.is_parsed &&
                (diag.severity == DiagnosticSeverity::Error || diag.severity == DiagnosticSeverity::FatalError ||
                 (diag.severity == DiagnosticSeverity::Warning && current_card == nullptr));

            if (is_primary_error) {
                cards.emplace_back();
                current_card = &cards.back();
                current_card->file = diag.file;
                current_card->line = diag.line;
                current_card->column = diag.column;
                current_card->severity = (diag.severity == DiagnosticSeverity::Warning) ? "warning" : "error";
                current_card->code = diag.code;
                current_card->message = DiagnosticDemangler::demangle_text(diag.message);
                continue;
            }

            if (!current_card) {
                continue;
            }

            std::string demangled_line = DiagnosticDemangler::demangle_text(line_sv);

            // Check if this line is snippet or caret
            bool is_caret = false;
            for (char c : line_sv) {
                if (c == '^' || c == '~') {
                    is_caret = true;
                    break;
                }
            }
            bool is_numbered_code = false;
            size_t bar_pos = line_sv.find('|');
            if (bar_pos != std::string_view::npos && bar_pos < 10) {
                bool digits = false;
                for (size_t k = 0; k < bar_pos; ++k) {
                    if (std::isdigit(static_cast<unsigned char>(line_sv[k])))
                        digits = true;
                }
                if (digits || line_sv.substr(0, bar_pos).find_first_not_of(" \t") == std::string_view::npos) {
                    is_numbered_code = true;
                }
            }

            if (is_caret || is_numbered_code) {
                current_card->snippet_lines.push_back(std::string(line_sv));
                continue;
            }

            // Check for Concept Constraint notes
            if (demangled_line.find("constraints not satisfied") != std::string::npos ||
                demangled_line.find("does not satisfy") != std::string::npos ||
                demangled_line.find("concept '") != std::string::npos ||
                demangled_line.find("the concept ") != std::string::npos ||
                demangled_line.find("associated constraints are not satisfied") != std::string::npos ||
                demangled_line.find("required expression") != std::string::npos) {

                // Extract concept name if present
                size_t c_idx = demangled_line.find("does not satisfy '");
                if (c_idx != std::string::npos) {
                    size_t c_end = demangled_line.find('\'', c_idx + 18);
                    if (c_end != std::string::npos) {
                        current_card->concept_name = demangled_line.substr(c_idx + 18, c_end - (c_idx + 18));
                    }
                }
                if (current_card->concept_name.empty()) {
                    size_t c2 = demangled_line.find("concept '");
                    if (c2 != std::string::npos) {
                        size_t c2_end = demangled_line.find('\'', c2 + 9);
                        if (c2_end != std::string::npos) {
                            current_card->concept_name = demangled_line.substr(c2 + 9, c2_end - (c2 + 9));
                        }
                    }
                }

                // Clean note prefix (e.g. "note: ")
                std::string r = demangled_line;
                size_t n_pos = r.find("note: ");
                if (n_pos != std::string::npos) {
                    r = r.substr(n_pos + 6);
                }
                if (r.rfind("because ", 0) == 0) {
                    r = r.substr(8);
                }
                if (r.find("candidate template ignored") == std::string::npos) {
                    current_card->reasons.push_back(r);
                }
                continue;
            }

            // Check for Backtrace lines
            if (demangled_line.find("in instantiation of") != std::string::npos ||
                demangled_line.find("required from") != std::string::npos ||
                demangled_line.find("see reference to") != std::string::npos) {

                BacktraceFrame frame;
                size_t colon_pos = demangled_line.find(": ");
                if (colon_pos != std::string::npos && colon_pos < 60) {
                    frame.location = demangled_line.substr(0, colon_pos);
                    frame.description = demangled_line.substr(colon_pos + 2);
                } else {
                    frame.description = demangled_line;
                }

                // Check if internal stdlib frame
                std::string full_chk = frame.location + " " + frame.description;
                if (full_chk.find("include/c++") != std::string::npos || full_chk.find("/bits/") != std::string::npos ||
                    full_chk.find("msvc/include") != std::string::npos || full_chk.find("MSVC") != std::string::npos ||
                    full_chk.find("<xstring>") != std::string::npos ||
                    full_chk.find("<type_traits>") != std::string::npos ||
                    full_chk.find("<vector>") != std::string::npos || full_chk.find("<utility>") != std::string::npos ||
                    full_chk.find("<xutility>") != std::string::npos || full_chk.find("stl/inc") != std::string::npos ||
                    full_chk.find("libcxx") != std::string::npos) {
                    frame.is_stdlib = true;
                }

                current_card->backtrace.push_back(std::move(frame));
                continue;
            }

            // Other note line
            current_card->notes.push_back(demangled_line);
        }

        return cards;
    }

    static std::string format_card(const DiagnosticCard &card, bool use_color = true) {
        const size_t card_width = 78;
        std::ostringstream ss;

        auto box_color = [&](std::string_view s) {
            return use_color ? colorize(s, Color::BrightCyan, Style::None, true) : std::string(s);
        };

        auto add_box_line = [&](std::string text) {
            size_t vlen = visible_length(text);
            ss << box_color("│") << " ";
            ss << text;
            if (vlen + 3 < card_width) {
                size_t pad = card_width - (vlen + 3);
                ss << std::string(pad, ' ');
            }
            ss << box_color("│") << "\n";
        };

        // 1. Top border: ┌─ [Error] in <file>:<line> ────────────────┐
        std::string badge;
        if (card.severity == "warning") {
            badge = use_color ? colorize("[Warning]", Color::Yellow, Style::Bold, true) : "[Warning]";
        } else {
            badge = use_color ? colorize("[Error]", Color::BrightRed, Style::Bold, true) : "[Error]";
        }

        std::string loc_str;
        if (!card.file.empty()) {
            loc_str = " in " + card.file;
            if (card.line > 0) {
                loc_str += ":" + std::to_string(card.line);
                if (card.column > 0) {
                    loc_str += ":" + std::to_string(card.column);
                }
            }
            if (use_color) {
                loc_str = colorize(loc_str, Color::Cyan, Style::Bold, true);
            }
        }

        std::string code_str;
        if (!card.code.empty()) {
            code_str = " [" + card.code + "]";
            if (use_color) {
                code_str = colorize(code_str, Color::Yellow, Style::None, true);
            }
        }

        std::string header_left = " " + badge + loc_str + code_str + " ";
        size_t header_vlen = 3 + visible_length(header_left); // 3 for "┌─ "

        ss << box_color("┌─") << header_left;
        if (header_vlen + 1 < card_width) {
            size_t rem = card_width - header_vlen - 1;
            std::string dashes;
            for (size_t d = 0; d < rem; ++d)
                dashes += "─";
            ss << box_color(dashes);
        }
        ss << box_color("┐") << "\n";

        // 2. Message line
        std::string msg_formatted = use_color ? bold(card.message, true) : card.message;
        add_box_line(msg_formatted);

        // 3. Snippet & Carets
        if (!card.snippet_lines.empty()) {
            add_box_line("");
            for (const auto &sline : card.snippet_lines) {
                std::string formatted_sline;
                if (!use_color) {
                    formatted_sline = sline;
                } else {
                    for (char c : sline) {
                        if (c == '^') {
                            formatted_sline += bright_green("^", true);
                        } else if (c == '~') {
                            formatted_sline += green("~", true);
                        } else {
                            formatted_sline.push_back(c);
                        }
                    }
                }
                add_box_line("  " + formatted_sline);
            }
        }

        // 4. Concept & Reasons
        if (!card.concept_name.empty()) {
            add_box_line("");
            std::string c_label =
                use_color ? colorize("Concept: ", Color::BrightYellow, Style::Bold, true) : "Concept: ";
            std::string c_val =
                use_color ? colorize(card.concept_name, Color::BrightWhite, Style::Bold, true) : card.concept_name;
            add_box_line(c_label + c_val);
        }

        if (!card.reasons.empty()) {
            if (card.concept_name.empty()) {
                add_box_line("");
            }
            for (size_t i = 0; i < card.reasons.size(); ++i) {
                std::string r_prefix = (i == 0) ? "Reason:  " : "         ";
                if (use_color) {
                    r_prefix = colorize(r_prefix, Color::BrightRed, Style::Bold, true);
                }
                add_box_line(r_prefix + card.reasons[i]);
            }
        }

        // 5. Backtrace with collapsed stdlib frames
        if (!card.backtrace.empty()) {
            add_box_line("");
            std::string bt_title =
                use_color ? colorize("Backtrace:", Color::BrightMagenta, Style::Bold, true) : "Backtrace:";
            add_box_line(bt_title);

            size_t f_idx = 0;
            while (f_idx < card.backtrace.size()) {
                if (card.backtrace[f_idx].is_stdlib) {
                    size_t stdlib_start = f_idx;
                    while (f_idx < card.backtrace.size() && card.backtrace[f_idx].is_stdlib) {
                        f_idx++;
                    }
                    size_t count = f_idx - stdlib_start;
                    if (count >= 2) {
                        std::string hidden_str = "  ... [" + std::to_string(count) + " stdlib frames hidden] ...";
                        if (use_color) {
                            hidden_str = gray(hidden_str, true);
                        }
                        add_box_line(hidden_str);
                    } else {
                        for (size_t k = stdlib_start; k < f_idx; ++k) {
                            const auto &f = card.backtrace[k];
                            std::string arrow = use_color ? cyan("→ ", true) : "-> ";
                            std::string loc = f.location.empty()
                                                  ? ""
                                                  : ((use_color ? gray(f.location + ": ", true) : (f.location + ": ")));
                            add_box_line("  " + arrow + loc + f.description);
                        }
                    }
                } else {
                    const auto &f = card.backtrace[f_idx];
                    std::string arrow = use_color ? cyan("→ ", true) : "-> ";
                    std::string loc =
                        f.location.empty() ? "" : ((use_color ? gray(f.location + ": ", true) : (f.location + ": ")));
                    add_box_line("  " + arrow + loc + f.description);
                    f_idx++;
                }
            }
        }

        // 6. Bottom border: └────────────────────────┘
        std::string bottom_dashes;
        for (size_t d = 0; d < card_width - 2; ++d) {
            bottom_dashes += "─";
        }
        ss << box_color("└") << box_color(bottom_dashes) << box_color("┘") << "\n";

        return ss.str();
    }

    static std::string format_cards(std::string_view raw_output, bool use_color = true) {
        auto cards = parse_cards(raw_output);
        if (cards.empty()) {
            return std::string(raw_output);
        }
        std::string result;
        for (size_t i = 0; i < cards.size(); ++i) {
            result += format_card(cards[i], use_color);
            if (i + 1 < cards.size()) {
                result += "\n";
            }
        }
        return result;
    }
};

inline std::string format_compiler_output(std::string_view raw_output, bool use_color = true) {
    if (raw_output.empty()) {
        return "";
    }

    if (DiagnosticCardFormatter::has_template_or_concept_error(raw_output)) {
        return DiagnosticCardFormatter::format_cards(raw_output, use_color);
    }

    std::string result;
    result.reserve(raw_output.size() + 256);

    size_t start = 0;
    while (start < raw_output.size()) {
        size_t end = raw_output.find('\n', start);
        std::string_view line;
        if (end != std::string_view::npos) {
            line = raw_output.substr(start, end - start);
            start = end + 1;
        } else {
            line = raw_output.substr(start);
            start = raw_output.size();
        }

        DiagnosticMessage diag = parse_line(line);
        std::string formatted = format_diagnostic(diag, use_color);
        result += formatted;
        result += "\n";
    }

    return result;
}

inline void print_formatted(std::string_view raw_output, std::ostream &os = std::cerr, bool use_color = true) {
    std::string formatted = format_compiler_output(raw_output, use_color);
    std::lock_guard<std::mutex> lock(get_terminal_mutex());
    os << formatted;
    os.flush();
}

class DiagnosticsFormatter {
public:
    static DiagnosticMessage parse_line(std::string_view line) { return terminal::parse_line(line); }
    static std::string format_diagnostic(const DiagnosticMessage &diag, bool use_color = true) {
        return terminal::format_diagnostic(diag, use_color);
    }
    static std::string format_compiler_output(std::string_view raw_output, bool use_color = true) {
        return terminal::format_compiler_output(raw_output, use_color);
    }
    static void print_formatted(std::string_view raw_output, std::ostream &os = std::cerr, bool use_color = true) {
        terminal::print_formatted(raw_output, os, use_color);
    }
    static std::string simplify_type(std::string_view type_sv) { return DiagnosticDemangler::simplify_type(type_sv); }
    static std::string demangle_text(std::string_view text) { return DiagnosticDemangler::demangle_text(text); }
    static std::string format_cards(std::string_view raw_output, bool use_color = true) {
        return DiagnosticCardFormatter::format_cards(raw_output, use_color);
    }
};

} // namespace terminal

using Terminal = terminal::Terminal;
using DiagnosticsFormatter = terminal::DiagnosticsFormatter;
using DiagnosticDemangler = terminal::DiagnosticDemangler;
using DiagnosticCard = terminal::DiagnosticCard;
using DiagnosticCardFormatter = terminal::DiagnosticCardFormatter;
using DiagnosticMessage = terminal::DiagnosticMessage;
using DiagnosticSeverity = terminal::DiagnosticSeverity;

inline uint32_t get_current_thread_id() {
    static std::atomic<uint32_t> s_counter{1};
    thread_local uint32_t tid = s_counter++;
    return tid;
}

struct ProfileEvent {
    std::string name;
    std::string cat;      // "compile", "link", "step", "custom"
    std::string ph = "X"; // Complete event
    uint64_t ts = 0;      // Microseconds since start
    uint64_t dur = 0;     // Duration in microseconds
    uint32_t pid = 1;
    uint32_t tid = 1;
    std::string status = "Executed"; // "Executed", "UpToDate", "Failed"
    uint64_t file_size = 0;          // File size in bytes (for source files)
    std::string detail = "";         // Target name or description
};

class BuildProfiler {
public:
    BuildProfiler() : start_time_(std::chrono::steady_clock::now()) {}

    void record(std::string name, std::string cat, std::chrono::steady_clock::time_point start_tp,
                std::chrono::steady_clock::time_point end_tp, uint32_t tid, std::string status = "Executed",
                uint64_t file_size = 0, std::string detail = "") {
        uint64_t ts = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(start_tp - start_time_).count());
        uint64_t dur =
            static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(end_tp - start_tp).count());

        ProfileEvent evt{
            .name = std::move(name),
            .cat = std::move(cat),
            .ph = "X",
            .ts = ts,
            .dur = dur,
            .pid = 1,
            .tid = tid,
            .status = std::move(status),
            .file_size = file_size,
            .detail = std::move(detail),
        };

        std::lock_guard<std::mutex> lock(mutex_);
        events_.push_back(std::move(evt));
    }

    [[nodiscard]] std::vector<ProfileEvent> events() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return events_;
    }

    [[nodiscard]] std::string to_json() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream ss;
        ss << "{\n";
        ss << "  \"displayTimeUnit\": \"ms\",\n";
        ss << "  \"traceEvents\": [\n";

        for (size_t i = 0; i < events_.size(); ++i) {
            const auto &e = events_[i];
            ss << "    {\n";
            ss << "      \"name\": \"" << escape_json(e.name) << "\",\n";
            ss << "      \"cat\": \"" << escape_json(e.cat) << "\",\n";
            ss << "      \"ph\": \"X\",\n";
            ss << "      \"ts\": " << e.ts << ",\n";
            ss << "      \"dur\": " << e.dur << ",\n";
            ss << "      \"pid\": " << e.pid << ",\n";
            ss << "      \"tid\": " << e.tid << ",\n";
            ss << "      \"args\": {\n";
            ss << "        \"status\": \"" << escape_json(e.status) << "\"";
            if (e.file_size > 0) {
                ss << ",\n        \"file_size\": " << e.file_size;
            }
            if (!e.detail.empty()) {
                ss << ",\n        \"detail\": \"" << escape_json(e.detail) << "\"";
            }
            ss << "\n      }\n";
            ss << "    }" << (i + 1 < events_.size() ? "," : "") << "\n";
        }

        ss << "  ]\n";
        ss << "}\n";
        return ss.str();
    }

    bool write_trace_file(const std::filesystem::path &filepath) const {
        std::error_code ec;
        if (filepath.has_parent_path()) {
            std::filesystem::create_directories(filepath.parent_path(), ec);
        }
        std::ofstream out(filepath, std::ios::binary);
        if (!out.is_open()) {
            return false;
        }
        out << to_json();
        return out.good();
    }

    void print_summary(std::ostream &os = std::cout, bool use_color = true) const {
        std::vector<ProfileEvent> sorted_events;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            sorted_events = events_;
        }

        if (sorted_events.empty()) {
            return;
        }

        std::sort(sorted_events.begin(), sorted_events.end(),
                  [](const ProfileEvent &a, const ProfileEvent &b) { return a.dur > b.dur; });

        using namespace terminal;
        os << "\n"
           << colorize("================================================================================", Color::Cyan,
                       Style::Bold, use_color)
           << "\n";
        os << bold("cxxgen build --profile - Performance Summary", use_color) << "\n";
        os << colorize("================================================================================", Color::Cyan,
                       Style::Bold, use_color)
           << "\n";
        os << bold("Top Slowest Compilation Steps / Translation Units:", use_color) << "\n";

        size_t count = std::min<size_t>(5, sorted_events.size());
        for (size_t i = 0; i < count; ++i) {
            const auto &e = sorted_events[i];
            double dur_ms = static_cast<double>(e.dur) / 1000.0;
            std::ostringstream time_ss;
            time_ss << std::fixed << std::setprecision(1) << dur_ms << " ms";

            std::string time_str = "[" + time_ss.str() + "]";
            while (time_str.size() < 12)
                time_str += " ";

            os << "  " << (i + 1) << ". " << colorize(time_str, Color::Yellow, Style::Bold, use_color) << " "
               << bold(e.name, use_color);

            if (e.file_size > 0) {
                double size_kb = static_cast<double>(e.file_size) / 1024.0;
                std::ostringstream sz_ss;
                sz_ss << std::fixed << std::setprecision(1) << size_kb << " KB";
                os << " (" << sz_ss.str() << ")";
            }

            if (e.status == "UpToDate") {
                os << " " << colorize("[UpToDate]", Color::Gray, Style::None, use_color);
            } else if (e.status == "Failed") {
                os << " " << colorize("[Failed]", Color::Red, Style::Bold, use_color);
            } else {
                os << " " << colorize("[Executed]", Color::Green, Style::None, use_color);
            }
            os << "\n";
        }

        auto total_dur = std::chrono::steady_clock::now() - start_time_;
        double wall_s = std::chrono::duration_cast<std::chrono::duration<double>>(total_dur).count();
        os << "\n"
           << "Total profiled steps: " << sorted_events.size() << " | Wall clock time: " << std::fixed
           << std::setprecision(2) << wall_s << "s\n";
        os << "Trace written to: " << colorize("build_trace.json", Color::Cyan, Style::Bold, use_color)
           << " (view in chrome://tracing or https://ui.perfetto.dev)\n";
        os << colorize("================================================================================", Color::Cyan,
                       Style::Bold, use_color)
           << "\n\n";
    }

private:
    static std::string escape_json(std::string_view s) {
        std::string res;
        res.reserve(s.size() + 16);
        for (char c : s) {
            switch (c) {
            case '"':
                res += "\\\"";
                break;
            case '\\':
                res += "\\\\";
                break;
            case '\b':
                res += "\\b";
                break;
            case '\f':
                res += "\\f";
                break;
            case '\n':
                res += "\\n";
                break;
            case '\r':
                res += "\\r";
                break;
            case '\t':
                res += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    res += buf;
                } else {
                    res += c;
                }
                break;
            }
        }
        return res;
    }

    std::chrono::steady_clock::time_point start_time_;
    mutable std::mutex mutex_;
    std::vector<ProfileEvent> events_;
};

struct ExecutionContext {
    const IToolchain &toolchain;
    bool verbose = false;
    bool dry_run = false;
    size_t thread_count = 0;
    std::filesystem::path working_directory = {};
    bool no_color = false;
    bool coverage = false;
    std::shared_ptr<terminal::ProgressReporter> progress_reporter = nullptr;
    std::shared_ptr<BuildProfiler> profiler = nullptr;

    [[nodiscard]] size_t effective_thread_count() const noexcept {
        if (thread_count > 0) {
            return thread_count;
        }
        unsigned int hw = std::thread::hardware_concurrency();
        return hw > 0 ? static_cast<size_t>(hw) : 4;
    }
};

class CircularDependencyException : public std::runtime_error {
public:
    explicit CircularDependencyException(std::vector<std::string> cycle_path)
        : std::runtime_error(format_message(cycle_path)), cycle_path_(std::move(cycle_path)) {}

    [[nodiscard]] const std::vector<std::string> &cycle_path() const noexcept { return cycle_path_; }

private:
    static std::string format_message(const std::vector<std::string> &path) {
        std::string msg = "Circular dependency detected in build DAG: ";
        for (size_t i = 0; i < path.size(); ++i) {
            if (i > 0)
                msg += " -> ";
            msg += path[i];
        }
        return msg;
    }

    std::vector<std::string> cycle_path_;
};

using CyclicDependencyException = CircularDependencyException;

class Step : public std::enable_shared_from_this<Step> {
public:
    explicit Step(std::string name, std::string description = "", StepType step_type = StepType::General)
        : name_(std::move(name)), description_(std::move(description)), step_type_(step_type) {}
    virtual ~Step() = default;

    [[nodiscard]] const std::string &name() const noexcept { return name_; }
    [[nodiscard]] const std::string &description() const noexcept { return description_; }
    [[nodiscard]] StepType step_type() const noexcept { return step_type_; }
    void set_step_type(StepType type) noexcept { step_type_ = type; }

    virtual StepResult execute(ExecutionContext &ctx) = 0;

    Step &depend_on(std::shared_ptr<Step> dep) {
        if (dep) {
            if (std::find(dependencies_.begin(), dependencies_.end(), dep) == dependencies_.end()) {
                dependencies_.push_back(dep);
            }
        }
        return *this;
    }

    Step &depend_on(Step &step) { return depend_on(step.shared_from_this()); }

    Step &depend_on(const Target &target);
    Step &depend_on(std::string_view name);

    [[nodiscard]] const std::vector<std::shared_ptr<Step>> &dependencies() const noexcept { return dependencies_; }

    void set_builder(Builder *b) noexcept { builder_ = b; }
    [[nodiscard]] Builder *builder() const noexcept { return builder_; }

private:
    std::string name_;
    std::string description_;
    StepType step_type_ = StepType::General;
    std::vector<std::shared_ptr<Step>> dependencies_;
    Builder *builder_ = nullptr;
};

inline bool operator==(const std::shared_ptr<Step> &step, std::string_view name) {
    return step && step->name() == name;
}

inline bool operator==(std::string_view name, const std::shared_ptr<Step> &step) {
    return step && step->name() == name;
}

inline Target &Target::link(const Target &dep) {
    if (build_step_ && dep.build_step()) {
        build_step_->depend_on(dep.build_step());
    }
    return link(std::string_view(dep.name()));
}

inline Target &Target::link(std::string_view dep_name) {
    if (std::find(linked_targets_.begin(), linked_targets_.end(), dep_name) == linked_targets_.end()) {
        linked_targets_.emplace_back(dep_name);
    }
    return *this;
}

// ----------------------------------------------------------------------------
// DAG Topological Sorter & Cycle Detector (DFS Color-Marking)
// ----------------------------------------------------------------------------

class BuildGraph {
public:
    enum class Color { White, Gray, Black };

    static void detect_cycles_dfs(const std::shared_ptr<Step> &current, std::unordered_map<Step *, Color> &colors,
                                  std::vector<std::string> &call_stack, std::vector<std::shared_ptr<Step>> &order) {
        colors[current.get()] = Color::Gray;
        call_stack.push_back(current->name());

        for (const auto &dep : current->dependencies()) {
            if (!dep)
                continue;
            auto it = colors.find(dep.get());
            Color c = (it != colors.end()) ? it->second : Color::White;

            if (c == Color::Gray) {
                // Circular dependency detected! Reconstruct the exact loop
                std::vector<std::string> cycle_path;
                bool recording = false;
                for (const auto &step_name : call_stack) {
                    if (step_name == dep->name()) {
                        recording = true;
                    }
                    if (recording) {
                        cycle_path.push_back(step_name);
                    }
                }
                cycle_path.push_back(dep->name()); // Close the cycle loop
                throw CircularDependencyException(std::move(cycle_path));
            } else if (c == Color::White) {
                detect_cycles_dfs(dep, colors, call_stack, order);
            }
        }

        call_stack.pop_back();
        colors[current.get()] = Color::Black;
        order.push_back(current);
    }

    static std::vector<std::shared_ptr<Step>> topological_sort(const std::vector<std::shared_ptr<Step>> &roots) {
        std::unordered_map<Step *, Color> colors;
        std::vector<std::string> call_stack;
        std::vector<std::shared_ptr<Step>> order;

        for (const auto &root : roots) {
            if (!root)
                continue;
            auto it = colors.find(root.get());
            if (it == colors.end() || it->second == Color::White) {
                detect_cycles_dfs(root, colors, call_stack, order);
            }
        }

        return order;
    }

    static std::vector<std::shared_ptr<Step>> topological_sort(const std::shared_ptr<Step> &root) {
        return topological_sort(std::vector<std::shared_ptr<Step>>{root});
    }
};

// ----------------------------------------------------------------------------
// Concrete Step Types
// ----------------------------------------------------------------------------

class CustomStep : public Step {
public:
    using Action = std::function<StepResult(ExecutionContext &)>;

    CustomStep(std::string name, std::string description = "", Action action = nullptr)
        : Step(std::move(name), std::move(description)), action_(std::move(action)) {}

    StepResult execute(ExecutionContext &ctx) override {
        if (action_) {
            return action_(ctx);
        }
        return StepResult::ok();
    }

    void set_action(Action action) { action_ = std::move(action); }

private:
    Action action_;
};

class TargetBuildStep : public Step {
public:
    TargetBuildStep(const Builder &builder, const Target &target)
        : Step(target.name(), "Build target " + target.name()), builder_(builder), target_(target) {}

    StepResult execute(ExecutionContext &ctx) override;

    [[nodiscard]] const Target &target() const noexcept { return target_; }

private:
    const Builder &builder_;
    const Target &target_;
};

class RunArtifactStep : public Step {
public:
    RunArtifactStep(const Target &target, std::vector<std::string> args = {})
        : Step("run:" + target.name(), "Run target " + target.name()), target_(target), args_(std::move(args)) {}

    StepResult execute(ExecutionContext &ctx) override;

    [[nodiscard]] const Target &target() const noexcept { return target_; }
    void set_args(std::vector<std::string> args) { args_ = std::move(args); }
    void add_arg(std::string arg) { args_.push_back(std::move(arg)); }
    [[nodiscard]] const std::vector<std::string> &args() const noexcept { return args_; }

private:
    const Target &target_;
    std::vector<std::string> args_;
};

// ----------------------------------------------------------------------------
// Custom Command Step & Code Generation
// ----------------------------------------------------------------------------

struct CustomCommandConfig {
    std::string name = "";
    std::string description = "";
    std::vector<std::string> command = {};
    std::vector<std::string> inputs = {};
    std::vector<std::string> outputs = {};
    std::filesystem::path working_directory = {};
    std::unordered_map<std::string, std::string> env_vars = {};
};

class CustomCommandStep : public Step {
public:
    explicit CustomCommandStep(CustomCommandConfig config)
        : Step(config.name.empty() ? generate_name(config) : config.name,
               config.description.empty() ? "Custom command " + generate_name(config) : config.description),
          command_(std::move(config.command)), inputs_(std::move(config.inputs)), outputs_(std::move(config.outputs)),
          working_directory_(std::move(config.working_directory)), env_vars_(std::move(config.env_vars)) {}

    StepResult execute(ExecutionContext &ctx) override;

    [[nodiscard]] const std::vector<std::string> &command() const noexcept { return command_; }
    [[nodiscard]] const std::vector<std::string> &inputs() const noexcept { return inputs_; }
    [[nodiscard]] const std::vector<std::string> &outputs() const noexcept { return outputs_; }
    [[nodiscard]] const std::filesystem::path &working_directory() const noexcept { return working_directory_; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &env_vars() const noexcept { return env_vars_; }

    [[nodiscard]] bool is_up_to_date() const;

private:
    static std::string generate_name(const CustomCommandConfig &cfg) {
        if (!cfg.outputs.empty()) {
            return "gen:" + std::filesystem::path(cfg.outputs[0]).filename().string();
        }
        if (!cfg.command.empty()) {
            return "cmd:" + std::filesystem::path(cfg.command[0]).filename().string();
        }
        return "custom_command";
    }

    std::vector<std::string> command_;
    std::vector<std::string> inputs_;
    std::vector<std::string> outputs_;
    std::filesystem::path working_directory_;
    std::unordered_map<std::string, std::string> env_vars_;
};

// ----------------------------------------------------------------------------
// Binary Asset Embedder Step (Story 13.1)
// ----------------------------------------------------------------------------

struct EmbedFileOptions {
    std::string variable_name = "";
    bool as_constexpr = true;
    bool null_terminate = false;
    std::string namespace_name = "";
};

class EmbedFileStep : public Step {
public:
    EmbedFileStep(std::filesystem::path source_path, std::filesystem::path header_dest, EmbedFileOptions options = {})
        : Step("embed:" + source_path.filename().string(),
               "Embed binary asset " + source_path.string() + " -> " + header_dest.string()),
          source_path_(std::move(source_path)), header_dest_(std::move(header_dest)), options_(std::move(options)) {}

    StepResult execute(ExecutionContext &ctx) override;
    [[nodiscard]] bool is_up_to_date() const;

    static bool embed(const std::filesystem::path &source_path, const std::filesystem::path &header_dest,
                      const EmbedFileOptions &options, std::string &error_msg);

    [[nodiscard]] const std::filesystem::path &source_path() const noexcept { return source_path_; }
    [[nodiscard]] const std::filesystem::path &header_dest() const noexcept { return header_dest_; }
    [[nodiscard]] const EmbedFileOptions &options() const noexcept { return options_; }
    [[nodiscard]] std::vector<std::string> inputs() const { return {source_path_.string()}; }
    [[nodiscard]] std::vector<std::string> outputs() const { return {header_dest_.string()}; }

private:
    std::filesystem::path source_path_;
    std::filesystem::path header_dest_;
    EmbedFileOptions options_;
};

// ----------------------------------------------------------------------------
// Built Module Interface (BMI) Step (Epic 12)
// ----------------------------------------------------------------------------

class CompileBMIStep : public Step {
public:
    CompileBMIStep(std::string module_name, std::filesystem::path source_path, std::filesystem::path bmi_output,
                   std::filesystem::path obj_output, const Target &target, std::vector<std::string> compile_flags = {},
                   std::unordered_map<std::string, std::string> imported_bmis = {})
        : Step("bmi:" + module_name, "Compile BMI for module " + module_name, StepType::CompileBMI),
          module_name_(std::move(module_name)), source_path_(std::move(source_path)),
          bmi_output_(std::move(bmi_output)), obj_output_(std::move(obj_output)), target_(target),
          compile_flags_(std::move(compile_flags)), imported_bmis_(std::move(imported_bmis)) {}

    [[nodiscard]] const std::string &module_name() const noexcept { return module_name_; }
    [[nodiscard]] const std::filesystem::path &source_path() const noexcept { return source_path_; }
    [[nodiscard]] const std::filesystem::path &bmi_output() const noexcept { return bmi_output_; }
    [[nodiscard]] const std::filesystem::path &obj_output() const noexcept { return obj_output_; }
    [[nodiscard]] const Target &target() const noexcept { return target_; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &imported_bmis() const noexcept {
        return imported_bmis_;
    }

    void add_imported_bmi(std::string mod_name, std::string bmi_path) {
        imported_bmis_[std::move(mod_name)] = std::move(bmi_path);
    }

    [[nodiscard]] bool is_up_to_date() const;
    StepResult execute(ExecutionContext &ctx) override;

private:
    std::string module_name_;
    std::filesystem::path source_path_;
    std::filesystem::path bmi_output_;
    std::filesystem::path obj_output_;
    const Target &target_;
    std::vector<std::string> compile_flags_;
    std::unordered_map<std::string, std::string> imported_bmis_;
};

// ============================================================================
// Multi-Threaded Task Pool
// ============================================================================

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads = 0) {
        if (num_threads == 0) {
            unsigned int hw = std::thread::hardware_concurrency();
            num_threads = hw > 0 ? static_cast<size_t>(hw) : 4;
        }
        workers_.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        cv_task_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
                        if (stop_ && tasks_.empty()) {
                            return;
                        }
                        task = std::move(tasks_.front());
                        tasks_.pop();
                        ++active_tasks_;
                    }
                    try {
                        task();
                    } catch (...) {}
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex_);
                        --active_tasks_;
                    }
                    cv_done_.notify_all();
                }
            });
        }
    }

    ~ThreadPool() { shutdown(); }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    [[nodiscard]] size_t thread_count() const noexcept { return workers_.size(); }

    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (stop_) {
                return;
            }
            tasks_.push(std::move(task));
        }
        cv_task_.notify_one();
    }

    void wait_all() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        cv_done_.wait(lock, [this]() { return tasks_.empty() && active_tasks_ == 0; });
    }

    void cancel_pending() {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        std::queue<std::function<void()>> empty;
        std::swap(tasks_, empty);
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (stop_) {
                return;
            }
            stop_ = true;
        }
        cv_task_.notify_all();
        for (auto &w : workers_) {
            if (w.joinable()) {
                w.join();
            }
        }
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable cv_task_;
    std::condition_variable cv_done_;
    size_t active_tasks_{0};
    bool stop_{false};
};

// ============================================================================
// Parallel Step Scheduler with In-Degree Topological Tracking
// ============================================================================

class ParallelScheduler {
public:
    static StepResult execute(const std::vector<std::shared_ptr<Step>> &steps, ExecutionContext &ctx) {
        if (steps.empty()) {
            return StepResult::ok();
        }

        if (ctx.progress_reporter && ctx.progress_reporter->total_steps() == 0) {
            ctx.progress_reporter->set_total_steps(steps.size());
        }

        size_t threads = ctx.effective_thread_count();
        if (threads <= 1 || steps.size() <= 1) {
            // Sequential execution
            bool all_up_to_date = true;
            StepResult last_res = StepResult::ok();
            for (const auto &s : steps) {
                if (!s)
                    continue;
                if (ctx.verbose) {
                    std::cout << "[cxxgen step] Executing: " << s->name() << "\n";
                }
                auto t0 = std::chrono::steady_clock::now();
                uint32_t tid = get_current_thread_id();
                StepResult res = s->execute(ctx);
                auto t1 = std::chrono::steady_clock::now();
                if (ctx.profiler) {
                    ctx.profiler->record(s->name(), "step", t0, t1, tid,
                                         (res.status == StepStatus::UpToDate) ? "UpToDate"
                                                                              : (res.success() ? "Executed" : "Failed"),
                                         0, s->description());
                }
                if (ctx.progress_reporter) {
                    double dur_s = static_cast<double>(res.duration.count()) / 1000.0;
                    if (res.success()) {
                        ctx.progress_reporter->report_step((res.status == StepStatus::UpToDate) ? "Up-to-date"
                                                                                                : "Executed",
                                                           s->name(), dur_s, res.status == StepStatus::UpToDate);
                    } else {
                        ctx.progress_reporter->report_failure("Failed", s->name(), dur_s, res.error_message);
                    }
                }
                if (!res.success()) {
                    return res;
                }
                if (res.status != StepStatus::UpToDate) {
                    all_up_to_date = false;
                }
                last_res = res;
            }
            if (steps.size() == 1) {
                return last_res;
            }
            return all_up_to_date ? StepResult::up_to_date() : StepResult::ok();
        }

        // Parallel in-degree dependency tracking
        std::unordered_set<Step *> step_set;
        for (const auto &s : steps) {
            if (s) {
                step_set.insert(s.get());
            }
        }

        std::unordered_map<Step *, size_t> remaining_deps;
        std::unordered_map<Step *, std::vector<std::shared_ptr<Step>>> dependents;

        for (const auto &s : steps) {
            if (!s)
                continue;
            size_t count = 0;
            for (const auto &dep : s->dependencies()) {
                if (dep && step_set.find(dep.get()) != step_set.end()) {
                    ++count;
                    dependents[dep.get()].push_back(s);
                }
            }
            remaining_deps[s.get()] = count;
        }

        ThreadPool pool(threads);
        std::atomic<bool> failed{false};
        std::atomic<bool> any_executed{false};
        std::mutex mtx;
        std::condition_variable cv_done;
        StepResult first_failure = StepResult::ok();
        size_t remaining_to_complete = steps.size();
        size_t in_flight = 0;

        std::function<void(std::shared_ptr<Step>)> schedule_step;
        schedule_step = [&](std::shared_ptr<Step> current_step) {
            pool.enqueue([&, current_step]() {
                if (failed.load()) {
                    std::lock_guard<std::mutex> lock(mtx);
                    --remaining_to_complete;
                    --in_flight;
                    cv_done.notify_all();
                    return;
                }

                if (ctx.verbose) {
                    std::lock_guard<std::mutex> lock(mtx);
                    std::cout << "[cxxgen step] Executing: " << current_step->name() << "\n";
                }

                auto t0 = std::chrono::steady_clock::now();
                uint32_t tid = get_current_thread_id();
                StepResult res = current_step->execute(ctx);
                auto t1 = std::chrono::steady_clock::now();
                if (ctx.profiler) {
                    ctx.profiler->record(current_step->name(), "step", t0, t1, tid,
                                         (res.status == StepStatus::UpToDate) ? "UpToDate"
                                                                              : (res.success() ? "Executed" : "Failed"),
                                         0, current_step->description());
                }

                if (!res.success()) {
                    bool expected = false;
                    if (failed.compare_exchange_strong(expected, true)) {
                        std::lock_guard<std::mutex> lock(mtx);
                        first_failure = res;
                    }
                    if (ctx.progress_reporter) {
                        double dur_s = static_cast<double>(res.duration.count()) / 1000.0;
                        ctx.progress_reporter->report_failure("Failed", current_step->name(), dur_s, res.error_message);
                    }
                    pool.cancel_pending();
                    std::lock_guard<std::mutex> lock(mtx);
                    --remaining_to_complete;
                    --in_flight;
                    cv_done.notify_all();
                    return;
                }

                if (ctx.progress_reporter) {
                    double dur_s = static_cast<double>(res.duration.count()) / 1000.0;
                    ctx.progress_reporter->report_step((res.status == StepStatus::UpToDate) ? "Up-to-date" : "Executed",
                                                       current_step->name(), dur_s, res.status == StepStatus::UpToDate);
                }

                if (res.status != StepStatus::UpToDate) {
                    any_executed.store(true);
                }

                std::vector<std::shared_ptr<Step>> ready_dependents;
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    --remaining_to_complete;
                    --in_flight;
                    if (!failed.load()) {
                        auto it = dependents.find(current_step.get());
                        if (it != dependents.end()) {
                            for (const auto &dep_step : it->second) {
                                auto dep_it = remaining_deps.find(dep_step.get());
                                if (dep_it != remaining_deps.end()) {
                                    if (--(dep_it->second) == 0) {
                                        ready_dependents.push_back(dep_step);
                                        ++in_flight;
                                    }
                                }
                            }
                        }
                    }
                    if (in_flight == 0 && remaining_to_complete > 0 && !failed.load()) {
                        failed.store(true);
                        first_failure = StepResult::error(
                            "Cycle or unresolvable dependency detected during parallel step execution.");
                    }
                    cv_done.notify_all();
                }

                for (const auto &next : ready_dependents) {
                    schedule_step(next);
                }
            });
        };

        std::vector<std::shared_ptr<Step>> initial_ready;
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (const auto &s : steps) {
                if (s && remaining_deps[s.get()] == 0) {
                    initial_ready.push_back(s);
                }
            }
            if (initial_ready.empty() && !steps.empty()) {
                return StepResult::error(
                    "Cycle or unresolvable dependency in steps: no initial steps can be scheduled.");
            }
            in_flight = initial_ready.size();
        }

        for (const auto &s : initial_ready) {
            schedule_step(s);
        }

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv_done.wait(lock, [&]() { return remaining_to_complete == 0 || failed.load(); });
        }

        pool.wait_all();

        if (failed.load()) {
            return first_failure;
        }
        return any_executed.load() ? StepResult::ok() : StepResult::up_to_date();
    }
};

// ============================================================================
// 7. Builder Context
// ============================================================================

class Builder {
public:
    Builder() = default;

    // Factory methods for targets
    Target &add_executable(TargetConfig config) {
        config.type = TargetType::Executable;
        return register_target(std::move(config));
    }

    Target &add_static_library(TargetConfig config) {
        config.type = TargetType::StaticLibrary;
        return register_target(std::move(config));
    }

    Target &add_shared_library(TargetConfig config) {
        config.type = TargetType::SharedLibrary;
        return register_target(std::move(config));
    }

    Target &add_plugin(TargetConfig config) {
        config.type = TargetType::Plugin;
        return register_target(std::move(config));
    }

    Target &add_interface_library(TargetConfig config) {
        config.type = TargetType::Interface;
        return register_target(std::move(config));
    }

    Target &add_test(TargetConfig config) {
        config.type = TargetType::Test;
        return register_target(std::move(config));
    }

    // Step management
    std::shared_ptr<Step> step_ptr(std::string_view name, std::string_view description = "") {
        auto it = step_lookup_.find(std::string(name));
        if (it != step_lookup_.end()) {
            return steps_[it->second];
        }
        std::string sname(name);
        auto step_obj = std::make_shared<CustomStep>(sname, std::string(description));
        step_obj->set_builder(this);
        size_t idx = steps_.size();
        steps_.push_back(step_obj);
        step_lookup_[sname] = idx;
        return step_obj;
    }

    Step &step(std::string_view name, std::string_view description = "") { return *step_ptr(name, description); }

    std::shared_ptr<Step> add_step(std::shared_ptr<Step> s) {
        if (!s)
            return nullptr;
        s->set_builder(this);
        auto it = step_lookup_.find(s->name());
        if (it != step_lookup_.end()) {
            steps_[it->second] = s;
            return s;
        }
        size_t idx = steps_.size();
        step_lookup_[s->name()] = idx;
        steps_.push_back(s);
        return s;
    }

    std::shared_ptr<CustomStep> add_step(std::string name, std::string description = "",
                                         std::function<StepResult(ExecutionContext &)> action = nullptr) {
        auto s = std::make_shared<CustomStep>(std::move(name), std::move(description), std::move(action));
        add_step(s);
        return s;
    }

    [[nodiscard]] Step *find_step(std::string_view name) noexcept {
        auto it = step_lookup_.find(std::string(name));
        if (it != step_lookup_.end()) {
            return steps_[it->second].get();
        }
        return nullptr;
    }

    [[nodiscard]] const Step *find_step(std::string_view name) const noexcept {
        auto it = step_lookup_.find(std::string(name));
        if (it != step_lookup_.end()) {
            return steps_[it->second].get();
        }
        return nullptr;
    }

    [[nodiscard]] std::shared_ptr<Step> find_step_ptr(std::string_view name) const noexcept {
        auto it = step_lookup_.find(std::string(name));
        if (it != step_lookup_.end()) {
            return steps_[it->second];
        }
        return nullptr;
    }

    std::shared_ptr<RunArtifactStep> add_run_artifact(const Target &target, std::vector<std::string> args = {}) {
        auto run_s = std::make_shared<RunArtifactStep>(target, std::move(args));
        if (auto bs = target.build_step()) {
            run_s->depend_on(bs);
        }
        add_step(run_s);
        return run_s;
    }

    std::shared_ptr<CustomCommandStep> add_custom_command(CustomCommandConfig config) {
        auto step = std::make_shared<CustomCommandStep>(std::move(config));
        add_step(step);
        return step;
    }

    std::shared_ptr<CustomCommandStep> add_custom_command(std::vector<std::string> command,
                                                          std::vector<std::string> inputs = {},
                                                          std::vector<std::string> outputs = {},
                                                          std::string description = "") {
        return add_custom_command(CustomCommandConfig{
            .description = std::move(description),
            .command = std::move(command),
            .inputs = std::move(inputs),
            .outputs = std::move(outputs),
        });
    }

    std::shared_ptr<EmbedFileStep> embed_file(std::filesystem::path source_path, std::filesystem::path header_dest,
                                              EmbedFileOptions options = {}) {
        auto step = std::make_shared<EmbedFileStep>(std::move(source_path), std::move(header_dest), std::move(options));
        step->set_builder(this);
        add_step(step);
        return step;
    }

    // Target query
    [[nodiscard]] Target *find_target(std::string_view name) noexcept {
        auto it = target_lookup_.find(std::string(name));
        if (it != target_lookup_.end()) {
            return targets_[it->second].get();
        }
        return nullptr;
    }

    [[nodiscard]] const Target *find_target(std::string_view name) const noexcept {
        auto it = target_lookup_.find(std::string(name));
        if (it != target_lookup_.end()) {
            return targets_[it->second].get();
        }
        return nullptr;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Target>> &targets() const noexcept { return targets_; }

    [[nodiscard]] const std::vector<std::shared_ptr<Step>> &steps() const noexcept { return steps_; }

    // Installation
    void install_artifact(const Target &target) {
        if (std::find(installed_artifacts_.begin(), installed_artifacts_.end(), target.name()) ==
            installed_artifacts_.end()) {
            installed_artifacts_.push_back(target.name());
        }
        step("install", "Install build artifacts").depend_on(target);
    }

    [[nodiscard]] const std::vector<std::string> &installed_artifacts() const noexcept { return installed_artifacts_; }

    // Standard options (mirroring Zig b.standardTargetOptions and b.standardOptimizeOption)
    [[nodiscard]] TargetOptions standard_target_options() const { return default_target_options_; }

    void set_standard_target_options(TargetOptions options) { default_target_options_ = std::move(options); }

    void set_target_triple(std::string_view triple) { default_target_options_.triple = std::string(triple); }
    void set_sysroot(const std::filesystem::path &sysroot) { default_target_options_.sysroot = sysroot; }
    [[nodiscard]] const std::string &target_triple() const noexcept { return default_target_options_.triple; }
    [[nodiscard]] const std::filesystem::path &sysroot() const noexcept { return default_target_options_.sysroot; }

    [[nodiscard]] OptimizeOptions standard_optimize_options() const { return default_optimize_options_; }

    void set_standard_optimize_options(OptimizeOptions options) { default_optimize_options_ = std::move(options); }

    [[nodiscard]] Sanitizer sanitizers() const noexcept { return default_sanitizers_; }
    void set_sanitizers(Sanitizer s) noexcept { default_sanitizers_ = s; }

    [[nodiscard]] bool coverage() const noexcept { return default_coverage_; }
    void set_coverage(bool c = true) noexcept { default_coverage_ = c; }

    [[nodiscard]] bool is_global_cache_enabled() const noexcept { return default_global_cache_; }
    void enable_global_cache(bool enabled = true) noexcept { default_global_cache_ = enabled; }

    [[nodiscard]] bool is_std_module_enabled() const noexcept { return default_std_module_; }
    void enable_std_module(bool enabled = true) noexcept { default_std_module_ = enabled; }

    void set_project_name(std::string name) { project_name_ = std::move(name); }
    [[nodiscard]] const std::string &project_name() const noexcept { return project_name_; }

    // Artifact and build pipeline
    [[nodiscard]] std::filesystem::path get_artifact_path(const Target &target, const IToolchain &toolchain) const;
    [[nodiscard]] std::filesystem::path get_import_library_path(const Target &target,
                                                                const IToolchain &toolchain) const;
    BuildResult build_target(const Target &target, const IToolchain &toolchain, bool verbose = false,
                             size_t thread_count = 0);
    BuildResult build_target(std::string_view target_name, const IToolchain &toolchain, bool verbose = false,
                             size_t thread_count = 0);

    // DAG Topological sort & execution
    void resolve_target_dependencies();
    std::vector<std::shared_ptr<Step>> topological_sort(std::string_view root_step = "");
    StepResult execute_step(std::string_view step_name, ExecutionContext &ctx);
    StepResult execute_all(ExecutionContext &ctx);

    // Target registration
    Target &register_target(TargetConfig config) {
        if (config.name.empty()) {
            throw std::invalid_argument("cxxgen error: target name cannot be empty");
        }
        if (target_lookup_.find(config.name) != target_lookup_.end()) {
            throw std::invalid_argument("cxxgen error: target with name '" + config.name + "' already exists");
        }
        std::string name = config.name;
        if (default_sanitizers_ != Sanitizer::None) {
            config.sanitizers = config.sanitizers | default_sanitizers_;
        }
        if (default_coverage_) {
            config.coverage = true;
        }
        if (!default_global_cache_) {
            config.enable_global_cache = false;
        }
        if (default_std_module_) {
            config.enable_std_module = true;
        }
        auto target_ptr = std::make_unique<Target>(std::move(config));
        Target &ref = *target_ptr;

        auto target_step = std::make_shared<TargetBuildStep>(*this, ref);
        target_step->set_builder(this);
        ref.set_build_step(target_step);
        add_step(target_step);

        size_t idx = targets_.size();
        targets_.push_back(std::move(target_ptr));
        target_lookup_[name] = idx;
        return ref;
    }

    // Dependency management (b.dependency)
    Dependency &dependency(std::string name, DependencyConfig config);
    [[nodiscard]] Dependency *find_dependency(std::string_view name) noexcept;
    [[nodiscard]] const Dependency *find_dependency(std::string_view name) const noexcept;
    [[nodiscard]] const std::vector<std::unique_ptr<Dependency>> &dependencies() const noexcept {
        return dependencies_;
    }

    // Target manifest export and import
    [[nodiscard]] std::string dump_manifest_json() const;
    bool dump_manifest_to_file(const std::filesystem::path &filepath) const;
    bool import_manifest(const std::filesystem::path &filepath, const std::filesystem::path &pkg_root = {},
                         std::vector<std::string> *imported_names = nullptr);
    bool import_manifest_from_string(std::string_view json_str, const std::filesystem::path &pkg_root = {},
                                     std::vector<std::string> *imported_names = nullptr);

    // System package discovery (b.find_system_package)
    SystemPackage find_system_package(std::string name, SystemPackageConfig config = {});
    Target &add_system_package(std::string name, SystemPackageConfig config = {});

    // Packaging & Distribution (Story 14.3)
    void set_package(PackageConfig config) { package_config_ = std::move(config); }
    [[nodiscard]] const PackageConfig &package_config() const noexcept { return package_config_; }
    [[nodiscard]] PackageConfig &package_config() noexcept { return package_config_; }

    // Lockfile & Reproducibility (Story 14.1)
    void set_locked(bool locked) noexcept { locked_ = locked; }
    [[nodiscard]] bool is_locked() const noexcept { return locked_; }
    void set_update_packages(std::vector<std::string> pkgs) { update_packages_ = std::move(pkgs); }
    [[nodiscard]] const std::vector<std::string> &update_packages() const noexcept { return update_packages_; }
    [[nodiscard]] bool should_update_package(const std::string &name) const noexcept {
        if (update_packages_.empty())
            return false;
        for (const auto &p : update_packages_) {
            if (p == "*" || p == "all" || p == name)
                return true;
        }
        return false;
    }

    void set_working_directory(std::filesystem::path dir) { working_directory_ = std::move(dir); }
    [[nodiscard]] const std::filesystem::path &working_directory() const noexcept { return working_directory_; }

    // CMake Config Exporter (Story 14.4)
    bool export_cmake_config(const CMakeConfigOptions &options, std::string *error = nullptr) const;

private:
    std::filesystem::path working_directory_{};
    std::vector<std::unique_ptr<Target>> targets_;
    std::unordered_map<std::string, size_t> target_lookup_;
    std::vector<std::shared_ptr<Step>> steps_;
    std::unordered_map<std::string, size_t> step_lookup_;
    std::vector<std::string> installed_artifacts_;
    std::vector<std::unique_ptr<Dependency>> dependencies_;
    std::unordered_map<std::string, size_t> dependency_lookup_;
    TargetOptions default_target_options_{};
    OptimizeOptions default_optimize_options_{};
    Sanitizer default_sanitizers_ = Sanitizer::None;
    bool default_coverage_ = false;
    std::string project_name_ = "";
    PackageConfig package_config_{};
    bool locked_ = false;
    std::vector<std::string> update_packages_{};
    bool default_global_cache_ = true;
    bool default_std_module_ = false;
};

inline Target &Target::depend_on(std::shared_ptr<Step> s) {
    if (build_step_) {
        build_step_->depend_on(std::move(s));
    }
    return *this;
}

inline Target &Target::depend_on(Step &s) {
    return depend_on(s.shared_from_this());
}

inline Target &Target::depend_on(const Target &dep) {
    return link(dep);
}

inline Step &Step::depend_on(const Target &target) {
    if (auto s = target.build_step()) {
        return depend_on(s);
    }
    return depend_on(target.name());
}

inline Step &Step::depend_on(std::string_view name) {
    if (builder_) {
        if (const Target *t = builder_->find_target(name)) {
            if (auto s = t->build_step()) {
                return depend_on(s);
            }
        }
        return depend_on(builder_->step_ptr(name));
    }
    auto placeholder = std::make_shared<CustomStep>(std::string(name));
    return depend_on(placeholder);
}

// ============================================================================
// 8. Toolchain Abstraction & Parameter Structs
// ============================================================================

namespace util {

inline std::string normalize_path(std::string_view path, bool use_native_separators = true) {
    if (path.empty())
        return "";
    std::string res;
    res.reserve(path.size());
    char sep = use_native_separators ?
#if defined(_WIN32)
                                     '\\'
#else
                                     '/'
#endif
                                     : '/';

    for (size_t i = 0; i < path.size(); ++i) {
        char c = path[i];
        if (c == '/' || c == '\\') {
            if (res.empty() || res.back() != sep) {
                res.push_back(sep);
            }
        } else {
            res.push_back(c);
        }
    }
    return res;
}

inline std::string trim(std::string_view str) {
    size_t first = 0;
    while (first < str.size() &&
           (str[first] == ' ' || str[first] == '\t' || str[first] == '\r' || str[first] == '\n')) {
        first++;
    }
    if (first == str.size())
        return {};
    size_t last = str.size() - 1;
    while (last > first && (str[last] == ' ' || str[last] == '\t' || str[last] == '\r' || str[last] == '\n')) {
        last--;
    }
    return std::string(str.substr(first, last - first + 1));
}

inline std::string to_lower(std::string_view str) {
    std::string res;
    res.reserve(str.size());
    for (char c : str) {
        res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return res;
}

inline bool iequals(std::string_view a, std::string_view b) {
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

inline std::string quote_argument(std::string_view arg, bool windows_style = true) {
    if (arg.empty())
        return "\"\"";

    bool needs_quotes = false;
    for (char c : arg) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\"' ||
            (!windows_style && (c == '\'' || c == '\\' || c == '$' || c == '`'))) {
            needs_quotes = true;
            break;
        }
    }

    if (!needs_quotes)
        return std::string(arg);

    std::string result;
    result.push_back('\"');
    for (size_t i = 0; i < arg.size(); ++i) {
        if (arg[i] == '\"') {
            result.append("\\\"");
        } else if (arg[i] == '\\' && windows_style) {
            // In Windows CommandLineToArgvW, backslashes preceding quotes must be escaped
            size_t backslash_count = 1;
            while (i + 1 < arg.size() && arg[i + 1] == '\\') {
                ++backslash_count;
                ++i;
            }
            if (i + 1 == arg.size() || arg[i + 1] == '\"') {
                result.append(backslash_count * 2, '\\');
            } else {
                result.append(backslash_count, '\\');
            }
        } else {
            result.push_back(arg[i]);
        }
    }
    result.push_back('\"');
    return result;
}

inline std::string flatten_command(const std::vector<std::string> &cmd, bool windows_style = true) {
    std::string result;
    for (size_t i = 0; i < cmd.size(); ++i) {
        if (i > 0)
            result.push_back(' ');
        result.append(quote_argument(cmd[i], windows_style));
    }
    return result;
}

inline std::string join(const std::vector<std::string> &items, std::string_view delimiter) {
    if (items.empty())
        return {};
    std::string res;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) {
            res += delimiter;
        }
        res += items[i];
    }
    return res;
}

inline std::vector<std::string> split(std::string_view str, std::string_view delimiter) {
    std::vector<std::string> result;
    if (str.empty())
        return result;
    if (delimiter.empty()) {
        result.emplace_back(str);
        return result;
    }
    size_t start = 0;
    size_t pos = 0;
    while ((pos = str.find(delimiter, start)) != std::string_view::npos) {
        result.emplace_back(str.substr(start, pos - start));
        start = pos + delimiter.size();
    }
    result.emplace_back(str.substr(start));
    return result;
}

inline std::string sanitize_identifier(std::string_view name) {
    std::string res;
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
            res.push_back(c);
        } else {
            res.push_back('_');
        }
    }
    if (res.empty() || std::isdigit(static_cast<unsigned char>(res[0]))) {
        res = "asset_" + res;
    }
    return res;
}

#if defined(_WIN32)
inline std::wstring utf8_to_wstring(std::string_view str) {
    if (str.empty())
        return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
    if (len <= 0)
        return {};
    std::wstring wstr(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &wstr[0], len);
    return wstr;
}

inline std::string wstring_to_utf8(std::wstring_view wstr) {
    if (wstr.empty())
        return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    if (len <= 0)
        return {};
    std::string str(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), &str[0], len, NULL, NULL);
    return str;
}
#endif

inline std::string escape_json(std::string_view str) {
    std::string res;
    res.reserve(str.size() + 16);
    for (char c : str) {
        switch (c) {
        case '"':
            res += "\\\"";
            break;
        case '\\':
            res += "\\\\";
            break;
        case '\b':
            res += "\\b";
            break;
        case '\f':
            res += "\\f";
            break;
        case '\n':
            res += "\\n";
            break;
        case '\r':
            res += "\\r";
            break;
        case '\t':
            res += "\\t";
            break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned int>(static_cast<unsigned char>(c)));
                res += buf;
            } else {
                res += c;
            }
            break;
        }
    }
    return res;
}

inline std::string escape_c_string(std::string_view str) {
    std::string res;
    res.reserve(str.size() + 16);
    for (char c : str) {
        switch (c) {
        case '\\':
            res += "\\\\";
            break;
        case '"':
            res += "\\\"";
            break;
        case '\n':
            res += "\\n";
            break;
        case '\r':
            res += "\\r";
            break;
        case '\t':
            res += "\\t";
            break;
        default:
            if (static_cast<unsigned char>(c) < 32 || static_cast<unsigned char>(c) >= 127) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\x%02x", static_cast<unsigned char>(c));
                res += buf;
            } else {
                res += c;
            }
            break;
        }
    }
    return res;
}

inline std::string escape_cmake_string(std::string_view str) {
    std::string res;
    res.reserve(str.size() + 8);
    for (char c : str) {
        if (c == '\\') {
            res += "\\\\";
        } else if (c == '"') {
            res += "\\\"";
        } else {
            res += c;
        }
    }
    return res;
}

inline bool is_safe_hash_key(std::string_view key) {
    if (key.empty() || key.size() > 128)
        return false;
    for (char c : key) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-')
            return false;
    }
    return true;
}

inline bool is_safe_extension(std::string_view ext) {
    if (ext.size() < 2 || ext.size() > 16)
        return false;
    if (ext[0] != '.')
        return false;
    for (size_t i = 1; i < ext.size(); ++i) {
        char c = ext[i];
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-')
            return false;
    }
    return true;
}

inline bool is_safe_package_name(std::string_view name) {
    if (name.empty() || name.size() > 128)
        return false;
    if (name[0] == '-' || name[0] == '.')
        return false;
    for (char c : name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-' && c != '+' && c != '.')
            return false;
    }
    return name.find("..") == std::string_view::npos;
}

inline bool is_system_protected_dir(const std::filesystem::path &dir) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path p = fs::absolute(dir, ec).lexically_normal();
    if (p == p.root_path())
        return true;

#if defined(_WIN32)
    std::string p_str = to_lower(p.string());
    const char *windir = std::getenv("WINDIR");
    if (windir && *windir) {
        std::string w_str = to_lower(fs::absolute(windir, ec).lexically_normal().string());
        if (p_str == w_str || p_str.rfind(w_str + "\\", 0) == 0 || p_str.rfind(w_str + "/", 0) == 0)
            return true;
    }
    const char *sysroot = std::getenv("SystemRoot");
    if (sysroot && *sysroot) {
        std::string s_str = to_lower(fs::absolute(sysroot, ec).lexically_normal().string());
        if (p_str == s_str || p_str.rfind(s_str + "\\", 0) == 0 || p_str.rfind(s_str + "/", 0) == 0)
            return true;
    }
    const char *prog = std::getenv("ProgramFiles");
    if (prog && *prog) {
        std::string pr_str = to_lower(fs::absolute(prog, ec).lexically_normal().string());
        if (p_str == pr_str || p_str.rfind(pr_str + "\\", 0) == 0 || p_str.rfind(pr_str + "/", 0) == 0)
            return true;
    }
    const char *progx86 = std::getenv("ProgramFiles(x86)");
    if (progx86 && *progx86) {
        std::string pr_str = to_lower(fs::absolute(progx86, ec).lexically_normal().string());
        if (p_str == pr_str || p_str.rfind(pr_str + "\\", 0) == 0 || p_str.rfind(pr_str + "/", 0) == 0)
            return true;
    }
    const char *progdata = std::getenv("ProgramData");
    if (progdata && *progdata) {
        std::string pd_str = to_lower(fs::absolute(progdata, ec).lexically_normal().string());
        if (p_str == pd_str || p_str.rfind(pd_str + "\\", 0) == 0 || p_str.rfind(pd_str + "/", 0) == 0)
            return true;
    }
#else
    std::string s = p.string();
    if (s == "/" || s == "/bin" || s == "/sbin" || s == "/etc" || s == "/usr" || s == "/usr/bin" || s == "/usr/sbin" ||
        s == "/usr/lib" || s == "/var" || s == "/lib" || s == "/lib64" || s == "/boot" || s == "/dev" || s == "/sys" ||
        s == "/proc" || s == "/root") {
        return true;
    }
#endif
    return false;
}

enum class JsonType { Null, Boolean, Number, String, Array, Object };

class JsonValue {
public:
    JsonType type = JsonType::Null;
    bool bool_val = false;
    double num_val = 0.0;
    int64_t int_val = 0;
    bool is_int = false;
    std::string str_val;
    std::vector<JsonValue> arr_val;
    std::map<std::string, JsonValue> obj_val;

    JsonValue() : type(JsonType::Null) {}
    JsonValue(bool b) : type(JsonType::Boolean), bool_val(b) {}
    JsonValue(int n) : type(JsonType::Number), num_val(n), int_val(n), is_int(true) {}
    JsonValue(int64_t n) : type(JsonType::Number), num_val(static_cast<double>(n)), int_val(n), is_int(true) {}
    JsonValue(uint64_t n)
        : type(JsonType::Number), num_val(static_cast<double>(n)), int_val(static_cast<int64_t>(n)), is_int(true) {}
    JsonValue(double n) : type(JsonType::Number), num_val(n), int_val(static_cast<int64_t>(n)), is_int(false) {}
    JsonValue(const char *s) : type(JsonType::String), str_val(s ? s : "") {}
    JsonValue(std::string s) : type(JsonType::String), str_val(std::move(s)) {}
    JsonValue(std::vector<JsonValue> arr) : type(JsonType::Array), arr_val(std::move(arr)) {}
    JsonValue(std::map<std::string, JsonValue> obj) : type(JsonType::Object), obj_val(std::move(obj)) {}

    bool is_null() const noexcept { return type == JsonType::Null; }
    bool is_bool() const noexcept { return type == JsonType::Boolean; }
    bool is_number() const noexcept { return type == JsonType::Number; }
    bool is_string() const noexcept { return type == JsonType::String; }
    bool is_array() const noexcept { return type == JsonType::Array; }
    bool is_object() const noexcept { return type == JsonType::Object; }

    bool as_bool(bool def = false) const noexcept { return is_bool() ? bool_val : def; }
    int64_t as_int64(int64_t def = 0) const noexcept { return is_number() ? int_val : def; }
    double as_double(double def = 0.0) const noexcept { return is_number() ? num_val : def; }
    const std::string &as_string() const noexcept {
        static const std::string empty;
        return is_string() ? str_val : empty;
    }
    const std::vector<JsonValue> &as_array() const noexcept {
        static const std::vector<JsonValue> empty;
        return is_array() ? arr_val : empty;
    }
    const std::map<std::string, JsonValue> &as_object() const noexcept {
        static const std::map<std::string, JsonValue> empty;
        return is_object() ? obj_val : empty;
    }

    [[nodiscard]] size_t size() const noexcept {
        if (is_array())
            return arr_val.size();
        if (is_object())
            return obj_val.size();
        return 0;
    }

    bool contains(const std::string &key) const {
        if (!is_object())
            return false;
        return obj_val.find(key) != obj_val.end();
    }

    const JsonValue &operator[](const std::string &key) const {
        static const JsonValue null_val;
        if (!is_object())
            return null_val;
        auto it = obj_val.find(key);
        if (it != obj_val.end())
            return it->second;
        return null_val;
    }

    JsonValue &operator[](const std::string &key) {
        if (!is_object()) {
            type = JsonType::Object;
            obj_val.clear();
        }
        return obj_val[key];
    }

    const JsonValue &operator[](size_t index) const {
        static const JsonValue null_val;
        if (!is_array() || index >= arr_val.size())
            return null_val;
        return arr_val[index];
    }

    std::string serialize(int indent = 0) const {
        switch (type) {
        case JsonType::Null:
            return "null";
        case JsonType::Boolean:
            return bool_val ? "true" : "false";
        case JsonType::Number:
            if (is_int)
                return std::to_string(int_val);
            else {
                std::ostringstream ss;
                ss << num_val;
                return ss.str();
            }
        case JsonType::String:
            return "\"" + escape_json(str_val) + "\"";
        case JsonType::Array: {
            if (arr_val.empty())
                return "[]";
            std::string s = "[\n";
            std::string ind(static_cast<size_t>((indent + 1) * 2), ' ');
            for (size_t i = 0; i < arr_val.size(); ++i) {
                s += ind + arr_val[i].serialize(indent + 1);
                if (i + 1 < arr_val.size())
                    s += ",";
                s += "\n";
            }
            s += std::string(static_cast<size_t>(indent * 2), ' ') + "]";
            return s;
        }
        case JsonType::Object: {
            if (obj_val.empty())
                return "{}";
            std::string s = "{\n";
            std::string ind(static_cast<size_t>((indent + 1) * 2), ' ');
            size_t count = 0;
            for (const auto &[k, v] : obj_val) {
                s += ind + "\"" + escape_json(k) + "\": " + v.serialize(indent + 1);
                if (++count < obj_val.size())
                    s += ",";
                s += "\n";
            }
            s += std::string(static_cast<size_t>(indent * 2), ' ') + "}";
            return s;
        }
        }
        return "null";
    }

    static std::optional<JsonValue> parse(std::string_view sv);
};

class JsonParser {
public:
    explicit JsonParser(std::string_view sv, size_t max_depth = 128) : src_(sv), pos_(0), max_depth_(max_depth) {}

    std::optional<JsonValue> parse() {
        skip_whitespace();
        if (pos_ >= src_.size())
            return std::nullopt;
        auto val = parse_value(0);
        skip_whitespace();
        return val;
    }

private:
    std::string_view src_;
    size_t pos_ = 0;
    size_t max_depth_ = 128;

    void skip_whitespace() {
        while (pos_ < src_.size()) {
            char c = src_[pos_];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                pos_++;
            } else {
                break;
            }
        }
    }

    std::optional<JsonValue> parse_value(size_t depth = 0) {
        if (depth > max_depth_)
            return std::nullopt;
        skip_whitespace();
        if (pos_ >= src_.size())
            return std::nullopt;
        char c = src_[pos_];
        if (c == 'n')
            return parse_null();
        if (c == 't' || c == 'f')
            return parse_bool();
        if (c == '"')
            return parse_string();
        if (c == '[')
            return parse_array(depth);
        if (c == '{')
            return parse_object(depth);
        if (c == '-' || (c >= '0' && c <= '9'))
            return parse_number();
        return std::nullopt;
    }

    std::optional<JsonValue> parse_null() {
        if (src_.substr(pos_, 4) == "null") {
            pos_ += 4;
            return JsonValue();
        }
        return std::nullopt;
    }

    std::optional<JsonValue> parse_bool() {
        if (src_.substr(pos_, 4) == "true") {
            pos_ += 4;
            return JsonValue(true);
        }
        if (src_.substr(pos_, 5) == "false") {
            pos_ += 5;
            return JsonValue(false);
        }
        return std::nullopt;
    }

    std::optional<JsonValue> parse_string() {
        if (pos_ >= src_.size() || src_[pos_] != '"')
            return std::nullopt;
        pos_++;
        std::string s;
        while (pos_ < src_.size()) {
            char c = src_[pos_++];
            if (c == '"') {
                return JsonValue(std::move(s));
            }
            if (c == '\\') {
                if (pos_ >= src_.size())
                    return std::nullopt;
                char esc = src_[pos_++];
                switch (esc) {
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                case '/':
                    s.push_back('/');
                    break;
                case 'b':
                    s.push_back('\b');
                    break;
                case 'f':
                    s.push_back('\f');
                    break;
                case 'n':
                    s.push_back('\n');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'u': {
                    if (pos_ + 4 > src_.size())
                        return std::nullopt;
                    unsigned int cp = 0;
                    for (int i = 0; i < 4; ++i) {
                        char h = src_[pos_++];
                        cp <<= 4;
                        if (h >= '0' && h <= '9')
                            cp |= static_cast<unsigned int>(h - '0');
                        else if (h >= 'a' && h <= 'f')
                            cp |= static_cast<unsigned int>(h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F')
                            cp |= static_cast<unsigned int>(h - 'A' + 10);
                        else
                            return std::nullopt;
                    }
                    if (cp < 0x80) {
                        s.push_back(static_cast<char>(cp));
                    } else if (cp < 0x800) {
                        s.push_back(static_cast<char>(0xC0 | (cp >> 6)));
                        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                    } else {
                        s.push_back(static_cast<char>(0xE0 | (cp >> 12)));
                        s.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                    }
                    break;
                }
                default:
                    s.push_back(esc);
                    break;
                }
            } else {
                s.push_back(c);
            }
        }
        return std::nullopt;
    }

    std::optional<JsonValue> parse_number() {
        size_t start = pos_;
        if (pos_ < src_.size() && src_[pos_] == '-')
            pos_++;
        bool has_digits = false;
        while (pos_ < src_.size() && (src_[pos_] >= '0' && src_[pos_] <= '9')) {
            has_digits = true;
            pos_++;
        }
        if (!has_digits)
            return std::nullopt;
        bool is_floating = false;
        if (pos_ < src_.size() && src_[pos_] == '.') {
            is_floating = true;
            pos_++;
            while (pos_ < src_.size() && (src_[pos_] >= '0' && src_[pos_] <= '9')) {
                pos_++;
            }
        }
        if (pos_ < src_.size() && (src_[pos_] == 'e' || src_[pos_] == 'E')) {
            is_floating = true;
            pos_++;
            if (pos_ < src_.size() && (src_[pos_] == '+' || src_[pos_] == '-'))
                pos_++;
            while (pos_ < src_.size() && (src_[pos_] >= '0' && src_[pos_] <= '9')) {
                pos_++;
            }
        }
        std::string num_str(src_.substr(start, pos_ - start));
        char *endp = nullptr;
        if (is_floating) {
            double d = std::strtod(num_str.c_str(), &endp);
            return JsonValue(d);
        } else {
            int64_t i = std::strtoll(num_str.c_str(), &endp, 10);
            return JsonValue(i);
        }
    }

    std::optional<JsonValue> parse_array(size_t depth) {
        if (depth > max_depth_)
            return std::nullopt;
        if (pos_ >= src_.size() || src_[pos_] != '[')
            return std::nullopt;
        pos_++;
        std::vector<JsonValue> arr;
        skip_whitespace();
        if (pos_ < src_.size() && src_[pos_] == ']') {
            pos_++;
            return JsonValue(std::move(arr));
        }
        while (pos_ < src_.size()) {
            auto val = parse_value(depth + 1);
            if (!val)
                return std::nullopt;
            arr.push_back(std::move(*val));
            skip_whitespace();
            if (pos_ >= src_.size())
                return std::nullopt;
            if (src_[pos_] == ',') {
                pos_++;
                skip_whitespace();
            } else if (src_[pos_] == ']') {
                pos_++;
                return JsonValue(std::move(arr));
            } else {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }

    std::optional<JsonValue> parse_object(size_t depth) {
        if (depth > max_depth_)
            return std::nullopt;
        if (pos_ >= src_.size() || src_[pos_] != '{')
            return std::nullopt;
        pos_++;
        std::map<std::string, JsonValue> obj;
        skip_whitespace();
        if (pos_ < src_.size() && src_[pos_] == '}') {
            pos_++;
            return JsonValue(std::move(obj));
        }
        while (pos_ < src_.size()) {
            skip_whitespace();
            if (pos_ >= src_.size() || src_[pos_] != '"')
                return std::nullopt;
            auto key_val = parse_string();
            if (!key_val || !key_val->is_string())
                return std::nullopt;
            std::string key = key_val->as_string();
            skip_whitespace();
            if (pos_ >= src_.size() || src_[pos_] != ':')
                return std::nullopt;
            pos_++;
            auto val = parse_value(depth + 1);
            if (!val)
                return std::nullopt;
            obj[std::move(key)] = std::move(*val);
            skip_whitespace();
            if (pos_ >= src_.size())
                return std::nullopt;
            if (src_[pos_] == ',') {
                pos_++;
                skip_whitespace();
            } else if (src_[pos_] == '}') {
                pos_++;
                return JsonValue(std::move(obj));
            } else {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
};

inline std::optional<JsonValue> JsonValue::parse(std::string_view sv) {
    JsonParser parser(sv);
    return parser.parse();
}

inline std::string escape_xml(std::string_view text) {
    std::string res;
    res.reserve(text.size());
    for (char c : text) {
        switch (c) {
        case '&':
            res += "&amp;";
            break;
        case '<':
            res += "&lt;";
            break;
        case '>':
            res += "&gt;";
            break;
        case '"':
            res += "&quot;";
            break;
        case '\'':
            res += "&apos;";
            break;
        default:
            res += c;
            break;
        }
    }
    return res;
}

inline std::string read_file_text(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        return "";
    file.seekg(0, std::ios::end);
    auto sz = file.tellg();
    if (sz <= 0)
        return "";
    file.seekg(0, std::ios::beg);
    std::string buffer;
    buffer.resize(static_cast<size_t>(sz));
    file.read(buffer.data(), sz);
    return buffer;
}

} // namespace util

struct CompileParams {
    std::string source_file = "";
    std::string object_file = "";
    CppVersion cpp_version = CppVersion::Cpp20;
    std::optional<CVersion> c_version = std::nullopt;
    Optimization optimize = Optimization::Debug;
    bool warnings_as_errors = false;
    bool exceptions = true;
    bool rtti = true;
    bool position_independent_code = false;
    std::vector<std::string> include_dirs = {};
    std::vector<std::string> defines = {};
    std::vector<std::string> extra_flags = {};
    std::string depfile = "";
    std::string pch_header = "";
    std::string pch_file = "";
    bool is_c_source = false;
    Sanitizer sanitizers = Sanitizer::None;
    bool coverage = false;
    std::string target_triple = "";
    std::filesystem::path sysroot = "";
    std::unordered_map<std::string, std::string> module_references = {};
    std::string module_interface_output = "";
    bool is_module_interface = false;
};

struct PchParams {
    std::string header_file = "";
    std::string output_pch = "";
    std::string pch_source = "";
    std::string pch_object = "";
    CppVersion cpp_version = CppVersion::Cpp20;
    std::optional<CVersion> c_version = std::nullopt;
    Optimization optimize = Optimization::Debug;
    bool warnings_as_errors = false;
    bool exceptions = true;
    bool rtti = true;
    bool position_independent_code = false;
    std::vector<std::string> include_dirs = {};
    std::vector<std::string> defines = {};
    std::vector<std::string> extra_flags = {};
    std::string target_triple = "";
    std::filesystem::path sysroot = "";
};

struct ArchiveParams {
    std::string output_lib = "";
    std::vector<std::string> object_files = {};
    std::vector<std::string> extra_flags = {};
};

struct LinkParams {
    std::string output_binary = "";
    TargetType target_type = TargetType::Executable;
    WindowsSubsystem subsystem = WindowsSubsystem::Console;
    std::vector<std::string> object_files = {};
    std::vector<std::string> library_dirs = {};
    std::vector<std::string> libraries = {};
    std::vector<std::string> extra_flags = {};
    std::string import_library = "";
    Sanitizer sanitizers = Sanitizer::None;
    bool coverage = false;
    std::string target_triple = "";
    std::filesystem::path sysroot = "";
};

class IToolchain {
public:
    virtual ~IToolchain() = default;

    [[nodiscard]] virtual CompilerFamily family() const noexcept = 0;
    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual std::string compiler_path() const = 0;
    [[nodiscard]] virtual std::string archiver_path() const = 0;
    [[nodiscard]] virtual std::string linker_path() const = 0;

    [[nodiscard]] virtual std::vector<std::string> build_compile_command(const CompileParams &params) const = 0;
    [[nodiscard]] virtual std::vector<std::string> build_pch_command(const PchParams &params) const {
        (void)params;
        return {};
    }
    [[nodiscard]] virtual std::vector<std::string> build_archive_command(const ArchiveParams &params) const = 0;
    [[nodiscard]] virtual std::vector<std::string> build_link_command(const LinkParams &params) const = 0;
    [[nodiscard]] virtual std::vector<std::string> get_depfile_flags(std::string_view depfile) const = 0;
    [[nodiscard]] virtual std::string format_import_library_name(std::string_view base_name) const {
#if defined(_WIN32)
        return std::string(base_name) + ".lib";
#else
        (void)base_name;
        return "";
#endif
    }
    [[nodiscard]] virtual const std::unordered_map<std::string, std::string> &env_vars() const noexcept {
        static const std::unordered_map<std::string, std::string> empty_env{};
        return empty_env;
    }
};

// ============================================================================
// Process Runner (Cross-Platform Non-Blocking Streaming)
// ============================================================================

namespace detail {
inline std::mutex &s_terminal_mutex = terminal::get_terminal_mutex();

#if defined(_WIN32)
inline bool ends_with_ci(std::string_view str, std::string_view suffix) {
    if (str.size() < suffix.size())
        return false;
    size_t offset = str.size() - suffix.size();
    for (size_t i = 0; i < suffix.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(str[offset + i])) !=
            std::tolower(static_cast<unsigned char>(suffix[i]))) {
            return false;
        }
    }
    return true;
}
#endif
} // namespace detail

struct ProcessResult {
    int exit_code = 0;
    std::string stdout_output = "";
    std::string stderr_output = "";
    std::chrono::milliseconds duration{0};
    bool timed_out = false;

    [[nodiscard]] bool success() const noexcept { return exit_code == 0 && !timed_out; }

    [[nodiscard]] std::string combined_output() const {
        if (stdout_output.empty())
            return stderr_output;
        if (stderr_output.empty())
            return stdout_output;
        return stdout_output + "\n" + stderr_output;
    }
};

struct ProcessOptions {
    std::vector<std::string> command = {};
    std::string raw_command = "";
    std::filesystem::path working_directory = {};
    std::unordered_map<std::string, std::string> extra_env = {};
    std::chrono::milliseconds timeout = std::chrono::milliseconds::zero();
    bool stream_to_terminal = false;
    std::function<void(std::string_view)> on_stdout = nullptr;
    std::function<void(std::string_view)> on_stderr = nullptr;
};

class ProcessRunner {
public:
    static ProcessResult run(const ProcessOptions &options) {
        auto start_time = std::chrono::steady_clock::now();
        ProcessResult result;

        std::string cmd_str = options.raw_command;
        if (cmd_str.empty()) {
            if (options.command.empty()) {
                result.exit_code = -1;
                result.stderr_output = "ProcessRunner: No command provided.\n";
                return result;
            }
#if defined(_WIN32)
            if (detail::ends_with_ci(options.command[0], ".bat") || detail::ends_with_ci(options.command[0], ".cmd")) {
                std::vector<std::string> cmd_wrapped = {"cmd.exe", "/c"};
                cmd_wrapped.insert(cmd_wrapped.end(), options.command.begin(), options.command.end());
                cmd_str = util::flatten_command(cmd_wrapped, true);
            } else {
                cmd_str = util::flatten_command(options.command, true);
            }
#else
            cmd_str = util::flatten_command(options.command, false);
#endif
        }

#if defined(_WIN32)
        // 1. Prepare pipes for standard output and standard error
        SECURITY_ATTRIBUTES sa;
        ZeroMemory(&sa, sizeof(sa));
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        HANDLE hStdoutRead = NULL;
        HANDLE hStdoutWrite = NULL;
        if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0)) {
            result.exit_code = -1;
            result.stderr_output = "ProcessRunner: Failed to create stdout pipe.\n";
            return result;
        }
        SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);

        HANDLE hStderrRead = NULL;
        HANDLE hStderrWrite = NULL;
        if (!CreatePipe(&hStderrRead, &hStderrWrite, &sa, 0)) {
            CloseHandle(hStdoutRead);
            CloseHandle(hStdoutWrite);
            result.exit_code = -1;
            result.stderr_output = "ProcessRunner: Failed to create stderr pipe.\n";
            return result;
        }
        SetHandleInformation(hStderrRead, HANDLE_FLAG_INHERIT, 0);

        // 2. Setup STARTUPINFOW
        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdOutput = hStdoutWrite;
        si.hStdError = hStderrWrite;
        si.hStdInput = INVALID_HANDLE_VALUE;

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        // 3. Command line buffer (mutable wchar_t array required by CreateProcessW)
        std::wstring wcmd = util::utf8_to_wstring(cmd_str);
        std::vector<wchar_t> cmd_buf(wcmd.begin(), wcmd.end());
        cmd_buf.push_back(L'\0');

        // 4. Working directory
        std::wstring wworkdir;
        const wchar_t *pWorkDir = NULL;
        if (!options.working_directory.empty()) {
            wworkdir = options.working_directory.wstring();
            pWorkDir = wworkdir.c_str();
        }

        // 5. Environment block
        std::vector<wchar_t> env_block;
        LPVOID pEnv = NULL;
        if (!options.extra_env.empty()) {
            std::map<std::wstring, std::wstring> env_map;
            wchar_t *cur_env = GetEnvironmentStringsW();
            if (cur_env) {
                const wchar_t *p = cur_env;
                while (*p) {
                    std::wstring entry(p);
                    size_t eq = entry.find(L'=');
                    if (eq != std::wstring::npos && eq > 0) {
                        env_map[entry.substr(0, eq)] = entry.substr(eq + 1);
                    } else if (eq == 0) {
                        size_t second_eq = entry.find(L'=', 1);
                        if (second_eq != std::wstring::npos) {
                            env_map[entry.substr(0, second_eq)] = entry.substr(second_eq + 1);
                        }
                    }
                    p += entry.size() + 1;
                }
                FreeEnvironmentStringsW(cur_env);
            }
            for (const auto &[k, v] : options.extra_env) {
                env_map[util::utf8_to_wstring(k)] = util::utf8_to_wstring(v);
            }
            for (const auto &[k, v] : env_map) {
                std::wstring entry = k + L"=" + v;
                for (wchar_t ch : entry) {
                    env_block.push_back(ch);
                }
                env_block.push_back(L'\0');
            }
            env_block.push_back(L'\0');
            pEnv = env_block.data();
        }

        DWORD creation_flags = CREATE_UNICODE_ENVIRONMENT;

        BOOL created = CreateProcessW(NULL, cmd_buf.data(), NULL, NULL, TRUE, creation_flags, pEnv, pWorkDir, &si, &pi);

        // Crucial: Parent must immediately close write handles so reader threads detect EOF when child exits
        CloseHandle(hStdoutWrite);
        CloseHandle(hStderrWrite);

        if (!created) {
            CloseHandle(hStdoutRead);
            CloseHandle(hStderrRead);
            DWORD err = GetLastError();
            result.exit_code = static_cast<int>(err);
            result.stderr_output = "ProcessRunner: CreateProcess failed with error " + std::to_string(err) +
                                   " for command: " + cmd_str + "\n";
            auto end_time = std::chrono::steady_clock::now();
            result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            return result;
        }

        // 6. Asynchronously read stdout and stderr concurrently to prevent pipe buffer deadlocks
        std::thread stdout_thread([&]() {
            char buffer[4096];
            DWORD bytesRead = 0;
            while (ReadFile(hStdoutRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
                std::string_view chunk(buffer, bytesRead);
                if (options.on_stdout) {
                    options.on_stdout(chunk);
                }
                if (options.stream_to_terminal) {
                    std::lock_guard<std::mutex> lock(detail::s_terminal_mutex);
                    std::cout.write(buffer, bytesRead);
                    std::cout.flush();
                }
                result.stdout_output.append(buffer, bytesRead);
            }
        });

        std::thread stderr_thread([&]() {
            char buffer[4096];
            DWORD bytesRead = 0;
            while (ReadFile(hStderrRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
                std::string_view chunk(buffer, bytesRead);
                if (options.on_stderr) {
                    options.on_stderr(chunk);
                }
                if (options.stream_to_terminal) {
                    std::lock_guard<std::mutex> lock(detail::s_terminal_mutex);
                    std::cerr.write(buffer, bytesRead);
                    std::cerr.flush();
                }
                result.stderr_output.append(buffer, bytesRead);
            }
        });

        // 7. Wait for child process (or timeout)
        DWORD timeout_ms = (options.timeout > std::chrono::milliseconds::zero())
                               ? static_cast<DWORD>(options.timeout.count())
                               : INFINITE;

        DWORD wait_res = WaitForSingleObject(pi.hProcess, timeout_ms);
        if (wait_res == WAIT_TIMEOUT) {
            result.timed_out = true;
            TerminateProcess(pi.hProcess, 124);
        }

        // 8. Wait for reader threads to finish reading remaining buffer contents
        stdout_thread.join();
        stderr_thread.join();

        DWORD exit_code = 0;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        result.exit_code = result.timed_out ? 124 : static_cast<int>(exit_code);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hStdoutRead);
        CloseHandle(hStderrRead);
#else
        // POSIX implementation
        int out_pipe[2];
        int err_pipe[2];
        if (pipe(out_pipe) != 0 || pipe(err_pipe) != 0) {
            result.exit_code = -1;
            result.stderr_output = "ProcessRunner: Failed to create pipes.\n";
            return result;
        }

        pid_t pid = fork();
        if (pid < 0) {
            close(out_pipe[0]);
            close(out_pipe[1]);
            close(err_pipe[0]);
            close(err_pipe[1]);
            result.exit_code = -1;
            result.stderr_output = "ProcessRunner: fork failed.\n";
            return result;
        }

        if (pid == 0) {
            // Child process
            close(out_pipe[0]);
            close(err_pipe[0]);
            dup2(out_pipe[1], STDOUT_FILENO);
            dup2(err_pipe[1], STDERR_FILENO);
            close(out_pipe[1]);
            close(err_pipe[1]);

            if (!options.working_directory.empty()) {
                if (chdir(options.working_directory.c_str()) != 0) {
                    _exit(127);
                }
            }

            for (const auto &[k, v] : options.extra_env) {
                setenv(k.c_str(), v.c_str(), 1);
            }

            if (!options.command.empty()) {
                std::vector<char *> argv;
                for (const auto &arg : options.command) {
                    argv.push_back(const_cast<char *>(arg.c_str()));
                }
                argv.push_back(nullptr);
                execvp(argv[0], argv.data());
            } else {
                execl("/bin/sh", "sh", "-c", cmd_str.c_str(), nullptr);
            }
            _exit(127);
        }

        // Parent process: close write ends immediately
        close(out_pipe[1]);
        close(err_pipe[1]);

        std::thread stdout_thread([&]() {
            char buffer[4096];
            ssize_t bytesRead = 0;
            while ((bytesRead = read(out_pipe[0], buffer, sizeof(buffer))) > 0) {
                std::string_view chunk(buffer, static_cast<size_t>(bytesRead));
                if (options.on_stdout) {
                    options.on_stdout(chunk);
                }
                if (options.stream_to_terminal) {
                    std::lock_guard<std::mutex> lock(detail::s_terminal_mutex);
                    std::cout.write(buffer, bytesRead);
                    std::cout.flush();
                }
                result.stdout_output.append(buffer, static_cast<size_t>(bytesRead));
            }
        });

        std::thread stderr_thread([&]() {
            char buffer[4096];
            ssize_t bytesRead = 0;
            while ((bytesRead = read(err_pipe[0], buffer, sizeof(buffer))) > 0) {
                std::string_view chunk(buffer, static_cast<size_t>(bytesRead));
                if (options.on_stderr) {
                    options.on_stderr(chunk);
                }
                if (options.stream_to_terminal) {
                    std::lock_guard<std::mutex> lock(detail::s_terminal_mutex);
                    std::cerr.write(buffer, bytesRead);
                    std::cerr.flush();
                }
                result.stderr_output.append(buffer, static_cast<size_t>(bytesRead));
            }
        });

        if (options.timeout > std::chrono::milliseconds::zero()) {
            auto deadline = std::chrono::steady_clock::now() + options.timeout;
            bool done = false;
            int status = 0;
            while (std::chrono::steady_clock::now() < deadline) {
                pid_t res = waitpid(pid, &status, WNOHANG);
                if (res == pid) {
                    done = true;
                    if (WIFEXITED(status)) {
                        result.exit_code = WEXITSTATUS(status);
                    } else if (WIFSIGNALED(status)) {
                        result.exit_code = 128 + WTERMSIG(status);
                    }
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            if (!done) {
                result.timed_out = true;
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                result.exit_code = 124;
            }
            stdout_thread.join();
            stderr_thread.join();
        } else {
            stdout_thread.join();
            stderr_thread.join();
            int status = 0;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                result.exit_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                result.exit_code = 128 + WTERMSIG(status);
            }
        }

        close(out_pipe[0]);
        close(err_pipe[0]);
#endif

        auto end_time = std::chrono::steady_clock::now();
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        return result;
    }

    static ProcessResult run(const std::vector<std::string> &command,
                             const std::filesystem::path &working_directory = {},
                             const std::unordered_map<std::string, std::string> &extra_env = {}) {
        ProcessOptions opts;
        opts.command = command;
        opts.working_directory = working_directory;
        opts.extra_env = extra_env;
        return run(opts);
    }

    static ProcessResult execute(std::string_view raw_command, const std::filesystem::path &working_directory = {},
                                 const std::unordered_map<std::string, std::string> &extra_env = {}) {
        ProcessOptions opts;
        opts.raw_command = std::string(raw_command);
        opts.working_directory = working_directory;
        opts.extra_env = extra_env;
        return run(opts);
    }

    static ProcessResult run_compiler(const std::vector<std::string> &command,
                                      const std::filesystem::path &working_directory = {},
                                      const std::unordered_map<std::string, std::string> &extra_env = {},
                                      bool stream_to_terminal = false) {
        ProcessOptions opts;
        opts.command = command;
        opts.working_directory = working_directory;
        opts.extra_env = extra_env;
        opts.stream_to_terminal = stream_to_terminal;
        return run(opts);
    }
};

// ============================================================================
// MSVC Toolchain & Discovery
// ============================================================================

struct MsvcEnvironment {
    bool found = false;
    std::string installation_path = "";
    std::string version = "";
    std::string cl_path = "cl.exe";
    std::string link_path = "link.exe";
    std::string lib_path = "lib.exe";
    std::unordered_map<std::string, std::string> env_vars = {};
};

inline MsvcEnvironment find_msvc_environment(bool force_refresh = false) {
    MsvcEnvironment result;
#if defined(_WIN32)
    namespace fs = std::filesystem;
    fs::path cache_dir = ".cxxgen/cache";
    fs::path cache_file = cache_dir / "msvc_env.txt";

    // 1. Check cached environment if available and not forced
    if (!force_refresh && fs::exists(cache_file)) {
        std::ifstream f(cache_file);
        if (f.is_open()) {
            std::string line;
            while (std::getline(f, line)) {
                size_t eq = line.find('=');
                if (eq != std::string::npos) {
                    std::string key = line.substr(0, eq);
                    std::string val = line.substr(eq + 1);
                    if (key == "__CXXGEN_INSTALL_PATH")
                        result.installation_path = val;
                    else if (key == "__CXXGEN_VERSION")
                        result.version = val;
                    else if (key == "__CXXGEN_CL")
                        result.cl_path = val;
                    else if (key == "__CXXGEN_LINK")
                        result.link_path = val;
                    else if (key == "__CXXGEN_LIB")
                        result.lib_path = val;
                    else
                        result.env_vars[key] = val;
                }
            }
            if (!result.cl_path.empty() && fs::exists(result.cl_path)) {
                result.found = true;
                return result;
            }
        }
    }

    // 2. Discover vswhere.exe
    std::vector<fs::path> vswhere_candidates;
    const char *pfx86 = std::getenv("ProgramFiles(x86)");
    if (pfx86) {
        vswhere_candidates.push_back(fs::path(pfx86) / "Microsoft Visual Studio/Installer/vswhere.exe");
    }
    const char *pf = std::getenv("ProgramFiles");
    if (pf) {
        vswhere_candidates.push_back(fs::path(pf) / "Microsoft Visual Studio/Installer/vswhere.exe");
    }

    fs::path vswhere_path;
    for (const auto &p : vswhere_candidates) {
        if (fs::exists(p)) {
            vswhere_path = p;
            break;
        }
    }

    if (vswhere_path.empty()) {
        return result;
    }

    // 3. Run vswhere to query installationPath
    std::string vswhere_cmd = "\"" + vswhere_path.string() +
                              "\" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 "
                              "-property installationPath";
    FILE *pipe = _popen(vswhere_cmd.c_str(), "r");
    if (!pipe) {
        return result;
    }

    char buffer[1024];
    std::string install_path;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        install_path.append(buffer);
    }
    _pclose(pipe);

    while (!install_path.empty() &&
           (install_path.back() == '\r' || install_path.back() == '\n' || install_path.back() == ' ')) {
        install_path.pop_back();
    }

    if (install_path.empty() || !fs::exists(install_path)) {
        return result;
    }

    result.installation_path = install_path;

    // 4. Locate cl.exe, link.exe, lib.exe under VC/Tools/MSVC/<version>/bin/Hostx64/x64
    fs::path msvc_tools_dir = fs::path(install_path) / "VC/Tools/MSVC";
    if (fs::exists(msvc_tools_dir)) {
        std::string latest_ver;
        for (const auto &entry : fs::directory_iterator(msvc_tools_dir)) {
            if (entry.is_directory()) {
                std::string dirname = entry.path().filename().string();
                if (latest_ver.empty() || dirname > latest_ver) {
                    latest_ver = dirname;
                }
            }
        }
        if (!latest_ver.empty()) {
            result.version = latest_ver;
            fs::path bin_dir = msvc_tools_dir / latest_ver / "bin/Hostx64/x64";
            if (fs::exists(bin_dir / "cl.exe")) {
                result.cl_path = (bin_dir / "cl.exe").string();
                result.link_path = (bin_dir / "link.exe").string();
                result.lib_path = (bin_dir / "lib.exe").string();
                result.found = true;
            }
        }
    }

    // 5. Run vcvars64.bat to capture environment
    fs::path vcvars = fs::path(install_path) / "VC/Auxiliary/Build/vcvars64.bat";
    if (fs::exists(vcvars)) {
        std::string cmd = "cmd.exe /c \"call \"" + vcvars.string() + "\" >nul 2>&1 && set\"";
        FILE *vpipe = _popen(cmd.c_str(), "r");
        if (vpipe) {
            char env_buf[2048];
            while (fgets(env_buf, sizeof(env_buf), vpipe) != nullptr) {
                std::string line(env_buf);
                while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
                    line.pop_back();
                }
                size_t eq = line.find('=');
                if (eq != std::string::npos) {
                    std::string key = line.substr(0, eq);
                    std::string val = line.substr(eq + 1);
                    result.env_vars[key] = val;
                }
            }
            _pclose(vpipe);
        }
    }

    // 6. Write cache file
    std::error_code ec;
    fs::create_directories(cache_dir, ec);
    std::ofstream out(cache_file);
    if (out.is_open()) {
        out << "__CXXGEN_INSTALL_PATH=" << result.installation_path << "\n";
        out << "__CXXGEN_VERSION=" << result.version << "\n";
        out << "__CXXGEN_CL=" << result.cl_path << "\n";
        out << "__CXXGEN_LINK=" << result.link_path << "\n";
        out << "__CXXGEN_LIB=" << result.lib_path << "\n";
        for (const auto &[k, v] : result.env_vars) {
            out << k << "=" << v << "\n";
        }
    }
#endif
    return result;
}

class MsvcToolchain : public IToolchain {
public:
    MsvcToolchain(std::string cl_path = "cl.exe", std::string link_path = "link.exe", std::string lib_path = "lib.exe",
                  std::string version = "19.x", std::unordered_map<std::string, std::string> env_vars = {})
        : cl_path_(std::move(cl_path)), link_path_(std::move(link_path)), lib_path_(std::move(lib_path)),
          version_(std::move(version)), env_vars_(std::move(env_vars)) {}

    [[nodiscard]] CompilerFamily family() const noexcept override { return CompilerFamily::Msvc; }
    [[nodiscard]] std::string name() const override { return "MSVC (" + version_ + ")"; }
    [[nodiscard]] std::string compiler_path() const override { return cl_path_; }
    [[nodiscard]] std::string archiver_path() const override { return lib_path_; }
    [[nodiscard]] std::string linker_path() const override { return link_path_; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &env_vars() const noexcept override {
        return env_vars_;
    }

    [[nodiscard]] std::vector<std::string> build_compile_command(const CompileParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(cl_path_);
        cmd.push_back("/nologo");
        cmd.push_back("/showIncludes");
        cmd.push_back("/FS");

        // Language standard
        if (params.is_c_source) {
            if (params.c_version.has_value()) {
                if (*params.c_version == CVersion::C11) {
                    cmd.push_back("/std:c11");
                } else if (*params.c_version == CVersion::C17) {
                    cmd.push_back("/std:c17");
                }
            }
        } else {
            switch (params.cpp_version) {
            case CppVersion::Cpp11:
            case CppVersion::Cpp14:
                cmd.push_back("/std:c++14");
                break;
            case CppVersion::Cpp17:
                cmd.push_back("/std:c++17");
                break;
            case CppVersion::Cpp20:
                cmd.push_back("/std:c++20");
                break;
            case CppVersion::Cpp23:
            case CppVersion::Cpp26:
                cmd.push_back("/std:c++latest");
                break;
            }
        }

        // Optimization
        switch (params.optimize) {
        case Optimization::Debug:
            cmd.push_back("/Od");
            cmd.push_back("/MDd");
            cmd.push_back("/Zi");
            cmd.push_back("/RTC1");
            break;
        case Optimization::ReleaseFast:
            cmd.push_back("/O2");
            cmd.push_back("/MD");
            cmd.push_back("/DNDEBUG");
            break;
        case Optimization::ReleaseSmall:
            cmd.push_back("/O1");
            cmd.push_back("/MD");
            cmd.push_back("/Os");
            cmd.push_back("/DNDEBUG");
            break;
        case Optimization::ReleaseSafe:
            cmd.push_back("/O2");
            cmd.push_back("/MD");
            cmd.push_back("/GS");
            cmd.push_back("/DNDEBUG");
            break;
        }

        // Exceptions and RTTI
        if (params.exceptions) {
            cmd.push_back("/EHsc");
        }
        if (!params.rtti) {
            cmd.push_back("/GR-");
        }

        // Warnings
        cmd.push_back("/W4");
        if (params.warnings_as_errors) {
            cmd.push_back("/WX");
        }

        // Defines
        for (const auto &def : params.defines) {
            cmd.push_back("/D" + def);
        }

        // Include directories
        for (const auto &inc : params.include_dirs) {
            cmd.push_back("/I" + util::normalize_path(inc));
        }

        // Extra compiler options
        for (const auto &opt : params.extra_flags) {
            cmd.push_back(opt);
        }

        // Sanitizers
        if (params.sanitizers != Sanitizer::None) {
            if ((params.sanitizers & Sanitizer::Address) != Sanitizer::None) {
                cmd.push_back("/fsanitize=address");
                auto rtc_it = std::find(cmd.begin(), cmd.end(), "/RTC1");
                if (rtc_it != cmd.end()) {
                    cmd.erase(rtc_it);
                }
                if (std::find(cmd.begin(), cmd.end(), "/Zi") == cmd.end() &&
                    std::find(cmd.begin(), cmd.end(), "/Z7") == cmd.end()) {
                    cmd.push_back("/Zi");
                }
            }
            if ((params.sanitizers & (Sanitizer::Undefined | Sanitizer::Leak | Sanitizer::Thread)) != Sanitizer::None) {
                static bool warned_msvc_san = false;
                if (!warned_msvc_san) {
                    std::cerr << "cxxgen warning: MSVC only supports Address Sanitizer on Windows; ignoring other "
                                 "requested sanitizers.\n";
                    warned_msvc_san = true;
                }
            }
        }

        // Coverage instrumentation
        if (params.coverage) {
            if (std::find(cmd.begin(), cmd.end(), "/Zi") == cmd.end() &&
                std::find(cmd.begin(), cmd.end(), "/Z7") == cmd.end()) {
                cmd.push_back("/Zi");
            }
        }

        // Precompiled headers
        if (!params.pch_header.empty()) {
            cmd.push_back("/Yu" + params.pch_header);
            if (!params.pch_file.empty()) {
                cmd.push_back("/Fp" + util::normalize_path(params.pch_file));
            }
            cmd.push_back("/FI" + params.pch_header);
        }

        // Modules (Epic 12)
        if (params.is_module_interface) {
            cmd.push_back("/interface");
            if (!params.module_interface_output.empty()) {
                cmd.push_back("/ifcOutput");
                cmd.push_back(util::normalize_path(params.module_interface_output));
            }
            std::string ext = std::filesystem::path(params.source_file).extension().string();
            if (ext != ".ixx") {
                cmd.push_back("/TP");
            }
        }
        for (const auto &[mod_name, bmi_p] : params.module_references) {
            cmd.push_back("/reference");
            cmd.push_back(mod_name + "=" + util::normalize_path(bmi_p));
        }

        // Output object and compile-only flag
        cmd.push_back("/c");
        cmd.push_back(util::normalize_path(params.source_file));
        cmd.push_back("/Fo:" + util::normalize_path(params.object_file));

        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_pch_command(const PchParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(cl_path_);
        cmd.push_back("/nologo");
        cmd.push_back("/FS");

        // Standard
        switch (params.cpp_version) {
        case CppVersion::Cpp11:
        case CppVersion::Cpp14:
            cmd.push_back("/std:c++14");
            break;
        case CppVersion::Cpp17:
            cmd.push_back("/std:c++17");
            break;
        case CppVersion::Cpp20:
            cmd.push_back("/std:c++20");
            break;
        case CppVersion::Cpp23:
        case CppVersion::Cpp26:
            cmd.push_back("/std:c++latest");
            break;
        }

        // Optimization
        switch (params.optimize) {
        case Optimization::Debug:
            cmd.push_back("/Od");
            cmd.push_back("/MDd");
            cmd.push_back("/Zi");
            cmd.push_back("/RTC1");
            break;
        case Optimization::ReleaseFast:
            cmd.push_back("/O2");
            cmd.push_back("/MD");
            cmd.push_back("/DNDEBUG");
            break;
        case Optimization::ReleaseSmall:
            cmd.push_back("/O1");
            cmd.push_back("/MD");
            cmd.push_back("/Os");
            cmd.push_back("/DNDEBUG");
            break;
        case Optimization::ReleaseSafe:
            cmd.push_back("/O2");
            cmd.push_back("/MD");
            cmd.push_back("/GS");
            cmd.push_back("/DNDEBUG");
            break;
        }

        // Exceptions and RTTI
        if (params.exceptions) {
            cmd.push_back("/EHsc");
        }
        if (!params.rtti) {
            cmd.push_back("/GR-");
        }

        // Warnings
        cmd.push_back("/W4");
        if (params.warnings_as_errors) {
            cmd.push_back("/WX");
        }

        // Defines
        for (const auto &def : params.defines) {
            cmd.push_back("/D" + def);
        }

        // Include directories
        for (const auto &inc : params.include_dirs) {
            cmd.push_back("/I" + util::normalize_path(inc));
        }

        // Extra compiler options
        for (const auto &opt : params.extra_flags) {
            cmd.push_back(opt);
        }

        // Output and PCH flags
        cmd.push_back("/Yc" + params.header_file);
        if (!params.output_pch.empty()) {
            cmd.push_back("/Fp" + util::normalize_path(params.output_pch));
        }
        cmd.push_back("/c");
        cmd.push_back(util::normalize_path(params.pch_source));
        if (!params.pch_object.empty()) {
            cmd.push_back("/Fo:" + util::normalize_path(params.pch_object));
        }

        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_archive_command(const ArchiveParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(lib_path_);
        cmd.push_back("/nologo");
        cmd.push_back("/OUT:" + util::normalize_path(params.output_lib));
        for (const auto &obj : params.object_files) {
            cmd.push_back(util::normalize_path(obj));
        }
        for (const auto &flag : params.extra_flags) {
            cmd.push_back(flag);
        }
        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_link_command(const LinkParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(link_path_);
        cmd.push_back("/nologo");
        cmd.push_back("/OUT:" + util::normalize_path(params.output_binary));

        // Subsystem
        if (params.subsystem == WindowsSubsystem::Windows) {
            cmd.push_back("/SUBSYSTEM:WINDOWS");
        } else {
            cmd.push_back("/SUBSYSTEM:CONSOLE");
        }

        // DLL / Plugin handling
        if (params.target_type == TargetType::SharedLibrary || params.target_type == TargetType::Plugin) {
            cmd.push_back("/DLL");
            if (!params.import_library.empty()) {
                cmd.push_back("/IMPLIB:" + util::normalize_path(params.import_library));
            }
        }

        // Library search paths
        for (const auto &dir : params.library_dirs) {
            cmd.push_back("/LIBPATH:" + util::normalize_path(dir));
        }

        // Input object files
        for (const auto &obj : params.object_files) {
            cmd.push_back(util::normalize_path(obj));
        }

        // Dependent libraries (.lib extension added if missing)
        for (const auto &lib : params.libraries) {
            std::string lib_name = lib;
            if (lib_name.find('.') == std::string::npos) {
                lib_name += ".lib";
            }
            cmd.push_back(util::normalize_path(lib_name));
        }

        for (const auto &flag : params.extra_flags) {
            cmd.push_back(flag);
        }

        // Sanitizers & Coverage debug symbols for MSVC link
        if ((params.sanitizers & Sanitizer::Address) != Sanitizer::None || params.coverage) {
            if (std::find(cmd.begin(), cmd.end(), "/DEBUG") == cmd.end() &&
                std::find(cmd.begin(), cmd.end(), "/debug") == cmd.end()) {
                cmd.push_back("/DEBUG");
            }
        }

        return cmd;
    }

    [[nodiscard]] std::vector<std::string> get_depfile_flags(std::string_view /*depfile*/) const override {
        return {"/showIncludes"};
    }

    [[nodiscard]] std::string format_import_library_name(std::string_view base_name) const override {
        return std::string(base_name) + ".lib";
    }

private:
    std::string cl_path_;
    std::string link_path_;
    std::string lib_path_;
    std::string version_;
    std::unordered_map<std::string, std::string> env_vars_;
};

inline std::optional<MsvcToolchain> find_msvc_toolchain(bool force_refresh = false) {
    auto env = find_msvc_environment(force_refresh);
    if (!env.found) {
        return std::nullopt;
    }
    return MsvcToolchain(env.cl_path, env.link_path, env.lib_path, env.version, std::move(env.env_vars));
}

// ============================================================================
// Clang Toolchain & Discovery
// ============================================================================

class ClangToolchain : public IToolchain {
public:
    ClangToolchain(std::string compiler_path = "clang++", std::string archiver_path = "llvm-ar",
                   std::string linker_path = "clang++", std::string version = "LLVM Clang", bool is_msvc_target = false)
        : compiler_path_(std::move(compiler_path)), archiver_path_(std::move(archiver_path)),
          linker_path_(std::move(linker_path)), version_(std::move(version)), is_msvc_target_(is_msvc_target) {}

    [[nodiscard]] CompilerFamily family() const noexcept override { return CompilerFamily::Clang; }
    [[nodiscard]] std::string name() const override { return "Clang (" + version_ + ")"; }
    [[nodiscard]] std::string compiler_path() const override { return compiler_path_; }
    [[nodiscard]] std::string archiver_path() const override { return archiver_path_; }
    [[nodiscard]] std::string linker_path() const override { return linker_path_; }
    [[nodiscard]] bool is_msvc_target() const noexcept { return is_msvc_target_; }

    [[nodiscard]] std::string format_import_library_name(std::string_view base_name) const override {
#if defined(_WIN32)
        if (is_msvc_target_) {
            return std::string(base_name) + ".lib";
        } else {
            return "lib" + std::string(base_name) + ".dll.a";
        }
#else
        (void)base_name;
        return "";
#endif
    }

    [[nodiscard]] std::vector<std::string> build_compile_command(const CompileParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(compiler_path_);

        // Cross-target triple & sysroot
        if (!params.target_triple.empty()) {
            cmd.push_back("--target=" + params.target_triple);
            auto arch_defs = TargetProfile::parse(params.target_triple).get_arch_defines();
            for (const auto &ad : arch_defs) {
                if (std::find(cmd.begin(), cmd.end(), "-D" + ad) == cmd.end()) {
                    cmd.push_back("-D" + ad);
                }
            }
        }
        if (!params.sysroot.empty()) {
            cmd.push_back("--sysroot=" + util::normalize_path(params.sysroot.string(), false));
        }

        // Language standard
        if (params.is_c_source) {
            if (params.c_version.has_value()) {
                switch (*params.c_version) {
                case CVersion::C89:
                    cmd.push_back("-std=c89");
                    break;
                case CVersion::C99:
                    cmd.push_back("-std=c99");
                    break;
                case CVersion::C11:
                    cmd.push_back("-std=c11");
                    break;
                case CVersion::C17:
                    cmd.push_back("-std=c17");
                    break;
                case CVersion::C23:
                    cmd.push_back("-std=c2x");
                    break;
                }
            }
        } else {
            switch (params.cpp_version) {
            case CppVersion::Cpp11:
                cmd.push_back("-std=c++11");
                break;
            case CppVersion::Cpp14:
                cmd.push_back("-std=c++14");
                break;
            case CppVersion::Cpp17:
                cmd.push_back("-std=c++17");
                break;
            case CppVersion::Cpp20:
                cmd.push_back("-std=c++20");
                break;
            case CppVersion::Cpp23:
                cmd.push_back("-std=c++23");
                break;
            case CppVersion::Cpp26:
                cmd.push_back("-std=c++26");
                break;
            }
        }

        // Optimization
        switch (params.optimize) {
        case Optimization::Debug:
            cmd.push_back("-O0");
            cmd.push_back("-g");
            break;
        case Optimization::ReleaseFast:
            cmd.push_back("-O3");
            cmd.push_back("-DNDEBUG");
            break;
        case Optimization::ReleaseSmall:
            cmd.push_back("-Oz");
            cmd.push_back("-DNDEBUG");
            break;
        case Optimization::ReleaseSafe:
            cmd.push_back("-O2");
            cmd.push_back("-DNDEBUG");
            break;
        }

        // PIC
        if (params.position_independent_code && !is_msvc_target_) {
            cmd.push_back("-fPIC");
        }

        // Exceptions and RTTI
        if (!params.exceptions) {
            cmd.push_back("-fno-exceptions");
        }
        if (!params.rtti) {
            cmd.push_back("-fno-rtti");
        }

        // Warnings
        cmd.push_back("-Wall");
        cmd.push_back("-Wextra");
        if (params.warnings_as_errors) {
            cmd.push_back("-Werror");
        }

        // Defines
        for (const auto &def : params.defines) {
            cmd.push_back("-D" + def);
        }

        // Include directories
        for (const auto &inc : params.include_dirs) {
            cmd.push_back("-I" + util::normalize_path(inc, false));
        }

        // Depfile
        if (!params.depfile.empty()) {
            cmd.push_back("-MMD");
            cmd.push_back("-MF");
            cmd.push_back(util::normalize_path(params.depfile, false));
        }

        // PCH
        if (!params.pch_file.empty()) {
            cmd.push_back("-include-pch");
            cmd.push_back(util::normalize_path(params.pch_file, false));
        } else if (!params.pch_header.empty()) {
            cmd.push_back("-include");
            cmd.push_back(util::normalize_path(params.pch_header, false));
        }

        // Extra compiler options
        for (const auto &opt : params.extra_flags) {
            cmd.push_back(opt);
        }

        // Sanitizers
        if (params.sanitizers != Sanitizer::None) {
            std::vector<std::string> san_list;
            if ((params.sanitizers & Sanitizer::Address) != Sanitizer::None)
                san_list.push_back("address");
            if ((params.sanitizers & Sanitizer::Undefined) != Sanitizer::None)
                san_list.push_back("undefined");
            if ((params.sanitizers & Sanitizer::Leak) != Sanitizer::None)
                san_list.push_back("leak");
            if ((params.sanitizers & Sanitizer::Thread) != Sanitizer::None)
                san_list.push_back("thread");
            if (!san_list.empty()) {
                std::string san_arg = "-fsanitize=";
                for (size_t i = 0; i < san_list.size(); ++i) {
                    if (i > 0)
                        san_arg += ",";
                    san_arg += san_list[i];
                }
                cmd.push_back(san_arg);
                cmd.push_back("-fno-omit-frame-pointer");
                cmd.push_back("-g");
            }
        }

        // Coverage
        if (params.coverage) {
            cmd.push_back("-fprofile-instr-generate");
            cmd.push_back("-fcoverage-mapping");
        }

        // Modules (Epic 12)
        if (params.is_module_interface && !params.module_interface_output.empty()) {
            cmd.push_back("-x");
            cmd.push_back("c++-module");
            cmd.push_back("--precompile");
        }
        for (const auto &[mod_name, bmi_p] : params.module_references) {
            cmd.push_back("-fmodule-file=" + mod_name + "=" + util::normalize_path(bmi_p, false));
        }

        // Compile-only & input/output
        if (!params.is_module_interface || params.module_interface_output.empty()) {
            cmd.push_back("-c");
        }
        cmd.push_back(util::normalize_path(params.source_file, false));
        cmd.push_back("-o");
        if (params.is_module_interface && !params.module_interface_output.empty()) {
            cmd.push_back(util::normalize_path(params.module_interface_output, false));
        } else {
            cmd.push_back(util::normalize_path(params.object_file, false));
        }

        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_pch_command(const PchParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(compiler_path_);

        // Language standard
        switch (params.cpp_version) {
        case CppVersion::Cpp11:
            cmd.push_back("-std=c++11");
            break;
        case CppVersion::Cpp14:
            cmd.push_back("-std=c++14");
            break;
        case CppVersion::Cpp17:
            cmd.push_back("-std=c++17");
            break;
        case CppVersion::Cpp20:
            cmd.push_back("-std=c++20");
            break;
        case CppVersion::Cpp23:
            cmd.push_back("-std=c++23");
            break;
        case CppVersion::Cpp26:
            cmd.push_back("-std=c++26");
            break;
        }

        // Optimization
        switch (params.optimize) {
        case Optimization::Debug:
            cmd.push_back("-O0");
            cmd.push_back("-g");
            break;
        case Optimization::ReleaseFast:
            cmd.push_back("-O3");
            cmd.push_back("-DNDEBUG");
            break;
        case Optimization::ReleaseSmall:
            cmd.push_back("-Oz");
            cmd.push_back("-DNDEBUG");
            break;
        case Optimization::ReleaseSafe:
            cmd.push_back("-O2");
            cmd.push_back("-DNDEBUG");
            break;
        }

        // PIC
        if (params.position_independent_code && !is_msvc_target_) {
            cmd.push_back("-fPIC");
        }

        // Exceptions and RTTI
        if (!params.exceptions) {
            cmd.push_back("-fno-exceptions");
        }
        if (!params.rtti) {
            cmd.push_back("-fno-rtti");
        }

        // Warnings
        cmd.push_back("-Wall");
        cmd.push_back("-Wextra");
        if (params.warnings_as_errors) {
            cmd.push_back("-Werror");
        }

        // Defines
        for (const auto &def : params.defines) {
            cmd.push_back("-D" + def);
        }

        // Include directories
        for (const auto &inc : params.include_dirs) {
            cmd.push_back("-I" + util::normalize_path(inc, false));
        }

        // Extra compiler options
        for (const auto &opt : params.extra_flags) {
            cmd.push_back(opt);
        }

        // PCH flags
        cmd.push_back("-x");
        cmd.push_back("c++-header");
        cmd.push_back("-c");
        cmd.push_back(util::normalize_path(params.header_file, false));
        cmd.push_back("-o");
        cmd.push_back(util::normalize_path(params.output_pch, false));

        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_archive_command(const ArchiveParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(archiver_path_);
        cmd.push_back("rcs");
        cmd.push_back(util::normalize_path(params.output_lib, false));
        for (const auto &obj : params.object_files) {
            cmd.push_back(util::normalize_path(obj, false));
        }
        for (const auto &flag : params.extra_flags) {
            cmd.push_back(flag);
        }
        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_link_command(const LinkParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(linker_path_);

        // Cross-target triple & sysroot
        if (!params.target_triple.empty()) {
            cmd.push_back("--target=" + params.target_triple);
        }
        if (!params.sysroot.empty()) {
            cmd.push_back("--sysroot=" + util::normalize_path(params.sysroot.string(), false));
            cmd.push_back("-L" + util::normalize_path((params.sysroot / "lib").string(), false));
            cmd.push_back("-L" + util::normalize_path((params.sysroot / "usr/lib").string(), false));
        }

        // Subsystem (only for Windows targets)
        bool is_win_target =
            params.target_triple.empty() ? is_msvc_target_ : TargetProfile::parse(params.target_triple).is_windows();
        if (is_win_target) {
            if (params.subsystem == WindowsSubsystem::Windows) {
                if (is_msvc_target_) {
                    cmd.push_back("-Wl,-subsystem:windows");
                } else {
                    cmd.push_back("-mwindows");
                }
            } else {
#if defined(_WIN32)
                if (is_msvc_target_) {
                    cmd.push_back("-Wl,-subsystem:console");
                }
#endif
            }
        }

        // Shared / Plugin
        if (params.target_type == TargetType::SharedLibrary || params.target_type == TargetType::Plugin) {
            cmd.push_back("-shared");
#if defined(_WIN32)
            if (!params.import_library.empty()) {
                if (is_msvc_target_) {
                    cmd.push_back("-Wl,-implib:" + util::normalize_path(params.import_library, false));
                } else {
                    cmd.push_back("-Wl,--out-implib," + util::normalize_path(params.import_library, false));
                }
            }
#endif
        }

        // Output binary
        cmd.push_back("-o");
        cmd.push_back(util::normalize_path(params.output_binary, false));

        // Object files
        for (const auto &obj : params.object_files) {
            cmd.push_back(util::normalize_path(obj, false));
        }

        // Library search paths
        for (const auto &dir : params.library_dirs) {
            cmd.push_back("-L" + util::normalize_path(dir, false));
        }

        // Linked libraries
        for (const auto &lib : params.libraries) {
            if (lib.find('/') != std::string::npos || lib.find('\\') != std::string::npos ||
                lib.rfind(".a") != std::string::npos || lib.rfind(".lib") != std::string::npos) {
                cmd.push_back(util::normalize_path(lib, false));
            } else {
                cmd.push_back("-l" + lib);
            }
        }

        for (const auto &flag : params.extra_flags) {
            cmd.push_back(flag);
        }

        // Sanitizers
        if (params.sanitizers != Sanitizer::None) {
            std::vector<std::string> san_list;
            if ((params.sanitizers & Sanitizer::Address) != Sanitizer::None)
                san_list.push_back("address");
            if ((params.sanitizers & Sanitizer::Undefined) != Sanitizer::None)
                san_list.push_back("undefined");
            if ((params.sanitizers & Sanitizer::Leak) != Sanitizer::None)
                san_list.push_back("leak");
            if ((params.sanitizers & Sanitizer::Thread) != Sanitizer::None)
                san_list.push_back("thread");
            if (!san_list.empty()) {
                std::string san_arg = "-fsanitize=";
                for (size_t i = 0; i < san_list.size(); ++i) {
                    if (i > 0)
                        san_arg += ",";
                    san_arg += san_list[i];
                }
                cmd.push_back(san_arg);
            }
        }

        // Coverage
        if (params.coverage) {
            cmd.push_back("-fprofile-instr-generate");
            cmd.push_back("-fcoverage-mapping");
        }

        return cmd;
    }

    [[nodiscard]] std::vector<std::string> get_depfile_flags(std::string_view depfile) const override {
        return {"-MMD", "-MF", std::string(depfile)};
    }

private:
    std::string compiler_path_;
    std::string archiver_path_;
    std::string linker_path_;
    std::string version_;
    bool is_msvc_target_ = false;
};

inline std::optional<ClangToolchain> find_clang_toolchain() {
    namespace fs = std::filesystem;
    std::vector<std::string> candidates = {"clang++"};
#if defined(_WIN32)
    candidates.push_back("clang++.exe");
    const char *userprofile = std::getenv("USERPROFILE");
    if (userprofile) {
        fs::path scoop_clang = fs::path(userprofile) / "scoop/apps/llvm/current/bin/clang++.exe";
        if (fs::exists(scoop_clang)) {
            candidates.insert(candidates.begin(), scoop_clang.string());
        }
    }
    const char *pf = std::getenv("ProgramFiles");
    if (pf) {
        fs::path pf_clang = fs::path(pf) / "LLVM/bin/clang++.exe";
        if (fs::exists(pf_clang)) {
            candidates.push_back(pf_clang.string());
        }
    }
#endif

    for (const auto &cand : candidates) {
        std::string cmd = "\"" + cand + "\" --version";
#if defined(_WIN32)
        FILE *pipe = _popen(cmd.c_str(), "r");
#else
        FILE *pipe = popen(cmd.c_str(), "r");
#endif
        if (pipe) {
            char buf[512];
            std::string ver;
            if (fgets(buf, sizeof(buf), pipe) != nullptr) {
                ver = buf;
            }
#if defined(_WIN32)
            int ret = _pclose(pipe);
#else
            int ret = pclose(pipe);
#endif
            if (ret == 0 && !ver.empty()) {
                while (!ver.empty() && (ver.back() == '\r' || ver.back() == '\n')) {
                    ver.pop_back();
                }
                std::string archiver = "llvm-ar";
                std::string linker = cand;
                bool is_msvc = false;
#if defined(_WIN32)
                std::string dump_cmd = "\"" + cand + "\" -dumpmachine";
                FILE *dpipe = _popen(dump_cmd.c_str(), "r");
                if (dpipe) {
                    char dbuf[256];
                    std::string dm;
                    if (fgets(dbuf, sizeof(dbuf), dpipe) != nullptr) {
                        dm = dbuf;
                    }
                    _pclose(dpipe);
                    if (dm.find("msvc") != std::string::npos) {
                        is_msvc = true;
                    }
                } else {
                    is_msvc = true;
                }
#endif
                return ClangToolchain(cand, archiver, linker, ver, is_msvc);
            }
        }
    }
    return std::nullopt;
}

// ============================================================================
// GCC Toolchain & Discovery
// ============================================================================

class GccToolchain : public IToolchain {
public:
    GccToolchain(std::string compiler_path = "g++", std::string archiver_path = "ar", std::string linker_path = "g++",
                 std::string version = "GNU GCC")
        : compiler_path_(std::move(compiler_path)), archiver_path_(std::move(archiver_path)),
          linker_path_(std::move(linker_path)), version_(std::move(version)) {}

    [[nodiscard]] CompilerFamily family() const noexcept override { return CompilerFamily::Gcc; }
    [[nodiscard]] std::string name() const override { return "GCC (" + version_ + ")"; }
    [[nodiscard]] std::string compiler_path() const override { return compiler_path_; }
    [[nodiscard]] std::string archiver_path() const override { return archiver_path_; }
    [[nodiscard]] std::string linker_path() const override { return linker_path_; }

    [[nodiscard]] std::string format_import_library_name(std::string_view base_name) const override {
#if defined(_WIN32)
        return "lib" + std::string(base_name) + ".dll.a";
#else
        (void)base_name;
        return "";
#endif
    }

    [[nodiscard]] std::vector<std::string> build_compile_command(const CompileParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(compiler_path_);

        // Cross-target architecture defines & sysroot
        if (!params.target_triple.empty()) {
            auto arch_defs = TargetProfile::parse(params.target_triple).get_arch_defines();
            for (const auto &ad : arch_defs) {
                if (std::find(cmd.begin(), cmd.end(), "-D" + ad) == cmd.end()) {
                    cmd.push_back("-D" + ad);
                }
            }
        }
        if (!params.sysroot.empty()) {
            cmd.push_back("--sysroot=" + util::normalize_path(params.sysroot.string(), false));
        }

        // Language standard
        if (params.is_c_source) {
            if (params.c_version.has_value()) {
                switch (*params.c_version) {
                case CVersion::C89:
                    cmd.push_back("-std=c89");
                    break;
                case CVersion::C99:
                    cmd.push_back("-std=c99");
                    break;
                case CVersion::C11:
                    cmd.push_back("-std=c11");
                    break;
                case CVersion::C17:
                    cmd.push_back("-std=c17");
                    break;
                case CVersion::C23:
                    cmd.push_back("-std=c2x");
                    break;
                }
            }
        } else {
            switch (params.cpp_version) {
            case CppVersion::Cpp11:
                cmd.push_back("-std=c++11");
                break;
            case CppVersion::Cpp14:
                cmd.push_back("-std=c++14");
                break;
            case CppVersion::Cpp17:
                cmd.push_back("-std=c++17");
                break;
            case CppVersion::Cpp20:
                cmd.push_back("-std=c++20");
                break;
            case CppVersion::Cpp23:
                cmd.push_back("-std=c++23");
                break;
            case CppVersion::Cpp26:
                cmd.push_back("-std=c++26");
                break;
            }
        }

        // Optimization
        switch (params.optimize) {
        case Optimization::Debug:
            cmd.push_back("-O0");
            cmd.push_back("-g");
            break;
        case Optimization::ReleaseFast:
            cmd.push_back("-O3");
            cmd.push_back("-DNDEBUG");
            break;
        case Optimization::ReleaseSmall:
            cmd.push_back("-Os");
            cmd.push_back("-DNDEBUG");
            break;
        case Optimization::ReleaseSafe:
            cmd.push_back("-O2");
            cmd.push_back("-DNDEBUG");
            break;
        }

        // PIC
        if (params.position_independent_code) {
            cmd.push_back("-fPIC");
        }

        // Exceptions and RTTI
        if (!params.exceptions) {
            cmd.push_back("-fno-exceptions");
        }
        if (!params.rtti) {
            cmd.push_back("-fno-rtti");
        }

        // Warnings
        cmd.push_back("-Wall");
        cmd.push_back("-Wextra");
        if (params.warnings_as_errors) {
            cmd.push_back("-Werror");
        }

        // Defines
        for (const auto &def : params.defines) {
            cmd.push_back("-D" + def);
        }

        // Include directories
        for (const auto &inc : params.include_dirs) {
            cmd.push_back("-I" + util::normalize_path(inc, false));
        }

        // Depfile
        if (!params.depfile.empty()) {
            cmd.push_back("-MMD");
            cmd.push_back("-MF");
            cmd.push_back(util::normalize_path(params.depfile, false));
        }

        // PCH
        if (!params.pch_file.empty()) {
            std::string inc_target = params.pch_file;
            if (inc_target.size() >= 4 && inc_target.rfind(".gch") == inc_target.size() - 4) {
                inc_target = inc_target.substr(0, inc_target.size() - 4);
            }
            cmd.push_back("-include");
            cmd.push_back(util::normalize_path(inc_target, false));
        } else if (!params.pch_header.empty()) {
            cmd.push_back("-include");
            cmd.push_back(util::normalize_path(params.pch_header, false));
        }

        // Extra compiler options
        for (const auto &opt : params.extra_flags) {
            cmd.push_back(opt);
        }

        // Sanitizers
        if (params.sanitizers != Sanitizer::None) {
            std::vector<std::string> san_list;
            if ((params.sanitizers & Sanitizer::Address) != Sanitizer::None)
                san_list.push_back("address");
            if ((params.sanitizers & Sanitizer::Undefined) != Sanitizer::None)
                san_list.push_back("undefined");
            if ((params.sanitizers & Sanitizer::Leak) != Sanitizer::None)
                san_list.push_back("leak");
            if ((params.sanitizers & Sanitizer::Thread) != Sanitizer::None)
                san_list.push_back("thread");
            if (!san_list.empty()) {
                std::string san_arg = "-fsanitize=";
                for (size_t i = 0; i < san_list.size(); ++i) {
                    if (i > 0)
                        san_arg += ",";
                    san_arg += san_list[i];
                }
                cmd.push_back(san_arg);
                cmd.push_back("-fno-omit-frame-pointer");
                cmd.push_back("-g");
            }
        }

        // Coverage
        if (params.coverage) {
            cmd.push_back("--coverage");
            cmd.push_back("-fprofile-arcs");
            cmd.push_back("-ftest-coverage");
        }

        // Modules (Epic 12)
        if (params.is_module_interface || !params.module_references.empty()) {
            cmd.push_back("-fmodules-ts");
        }

        // Compile-only & input/output
        cmd.push_back("-c");
        cmd.push_back(util::normalize_path(params.source_file, false));
        cmd.push_back("-o");
        cmd.push_back(util::normalize_path(params.object_file, false));

        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_pch_command(const PchParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(compiler_path_);

        // Language standard
        switch (params.cpp_version) {
        case CppVersion::Cpp11:
            cmd.push_back("-std=c++11");
            break;
        case CppVersion::Cpp14:
            cmd.push_back("-std=c++14");
            break;
        case CppVersion::Cpp17:
            cmd.push_back("-std=c++17");
            break;
        case CppVersion::Cpp20:
            cmd.push_back("-std=c++20");
            break;
        case CppVersion::Cpp23:
            cmd.push_back("-std=c++23");
            break;
        case CppVersion::Cpp26:
            cmd.push_back("-std=c++26");
            break;
        }

        // Optimization
        switch (params.optimize) {
        case Optimization::Debug:
            cmd.push_back("-O0");
            cmd.push_back("-g");
            break;
        case Optimization::ReleaseFast:
            cmd.push_back("-O3");
            cmd.push_back("-DNDEBUG");
            break;
        case Optimization::ReleaseSmall:
            cmd.push_back("-Os");
            cmd.push_back("-DNDEBUG");
            break;
        case Optimization::ReleaseSafe:
            cmd.push_back("-O2");
            cmd.push_back("-DNDEBUG");
            break;
        }

        // PIC
        if (params.position_independent_code) {
            cmd.push_back("-fPIC");
        }

        // Exceptions and RTTI
        if (!params.exceptions) {
            cmd.push_back("-fno-exceptions");
        }
        if (!params.rtti) {
            cmd.push_back("-fno-rtti");
        }

        // Warnings
        cmd.push_back("-Wall");
        cmd.push_back("-Wextra");
        if (params.warnings_as_errors) {
            cmd.push_back("-Werror");
        }

        // Defines
        for (const auto &def : params.defines) {
            cmd.push_back("-D" + def);
        }

        // Include directories
        for (const auto &inc : params.include_dirs) {
            cmd.push_back("-I" + util::normalize_path(inc, false));
        }

        // Extra compiler options
        for (const auto &opt : params.extra_flags) {
            cmd.push_back(opt);
        }

        // PCH flags
        cmd.push_back("-x");
        cmd.push_back("c++-header");
        cmd.push_back("-c");
        cmd.push_back(util::normalize_path(params.header_file, false));
        cmd.push_back("-o");
        cmd.push_back(util::normalize_path(params.output_pch, false));

        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_archive_command(const ArchiveParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(archiver_path_);
        cmd.push_back("rcs");
        cmd.push_back(util::normalize_path(params.output_lib, false));
        for (const auto &obj : params.object_files) {
            cmd.push_back(util::normalize_path(obj, false));
        }
        for (const auto &flag : params.extra_flags) {
            cmd.push_back(flag);
        }
        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_link_command(const LinkParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(linker_path_);

        // Sysroot
        if (!params.sysroot.empty()) {
            cmd.push_back("--sysroot=" + util::normalize_path(params.sysroot.string(), false));
            cmd.push_back("-L" + util::normalize_path((params.sysroot / "lib").string(), false));
            cmd.push_back("-L" + util::normalize_path((params.sysroot / "usr/lib").string(), false));
        }

        // Subsystem (Windows GUI)
        if (params.subsystem == WindowsSubsystem::Windows) {
            cmd.push_back("-mwindows");
        }

        // Shared / Plugin
        if (params.target_type == TargetType::SharedLibrary || params.target_type == TargetType::Plugin) {
            cmd.push_back("-shared");
#if defined(_WIN32)
            if (!params.import_library.empty()) {
                cmd.push_back("-Wl,--out-implib," + util::normalize_path(params.import_library, false));
            }
#endif
        }

        // Output binary
        cmd.push_back("-o");
        cmd.push_back(util::normalize_path(params.output_binary, false));

        // Object files
        for (const auto &obj : params.object_files) {
            cmd.push_back(util::normalize_path(obj, false));
        }

        // Library search paths
        for (const auto &dir : params.library_dirs) {
            cmd.push_back("-L" + util::normalize_path(dir, false));
        }

        // Linked libraries
        for (const auto &lib : params.libraries) {
            if (lib.find('/') != std::string::npos || lib.find('\\') != std::string::npos ||
                lib.rfind(".a") != std::string::npos || lib.rfind(".lib") != std::string::npos) {
                cmd.push_back(util::normalize_path(lib, false));
            } else {
                cmd.push_back("-l" + lib);
            }
        }

        for (const auto &flag : params.extra_flags) {
            cmd.push_back(flag);
        }

        // Sanitizers
        if (params.sanitizers != Sanitizer::None) {
            std::vector<std::string> san_list;
            if ((params.sanitizers & Sanitizer::Address) != Sanitizer::None)
                san_list.push_back("address");
            if ((params.sanitizers & Sanitizer::Undefined) != Sanitizer::None)
                san_list.push_back("undefined");
            if ((params.sanitizers & Sanitizer::Leak) != Sanitizer::None)
                san_list.push_back("leak");
            if ((params.sanitizers & Sanitizer::Thread) != Sanitizer::None)
                san_list.push_back("thread");
            if (!san_list.empty()) {
                std::string san_arg = "-fsanitize=";
                for (size_t i = 0; i < san_list.size(); ++i) {
                    if (i > 0)
                        san_arg += ",";
                    san_arg += san_list[i];
                }
                cmd.push_back(san_arg);
            }
        }

        // Coverage
        if (params.coverage) {
            cmd.push_back("--coverage");
        }

        return cmd;
    }

    [[nodiscard]] std::vector<std::string> get_depfile_flags(std::string_view depfile) const override {
        return {"-MMD", "-MF", std::string(depfile)};
    }

private:
    std::string compiler_path_;
    std::string archiver_path_;
    std::string linker_path_;
    std::string version_;
};

inline std::optional<GccToolchain> find_gcc_toolchain() {
    namespace fs = std::filesystem;
    std::vector<std::string> candidates = {"g++"};
#if defined(_WIN32)
    candidates.push_back("g++.exe");
    const char *userprofile = std::getenv("USERPROFILE");
    if (userprofile) {
        fs::path scoop_gcc = fs::path(userprofile) / "scoop/apps/gcc/current/bin/g++.exe";
        if (fs::exists(scoop_gcc)) {
            candidates.insert(candidates.begin(), scoop_gcc.string());
        }
        fs::path msys_gcc = fs::path(userprofile) / "scoop/apps/msys2/current/ucrt64/bin/g++.exe";
        if (fs::exists(msys_gcc)) {
            candidates.push_back(msys_gcc.string());
        }
    }
#endif

    for (const auto &cand : candidates) {
        std::string cmd = "\"" + cand + "\" --version";
#if defined(_WIN32)
        FILE *pipe = _popen(cmd.c_str(), "r");
#else
        FILE *pipe = popen(cmd.c_str(), "r");
#endif
        if (pipe) {
            char buf[512];
            std::string ver;
            if (fgets(buf, sizeof(buf), pipe) != nullptr) {
                ver = buf;
            }
#if defined(_WIN32)
            int ret = _pclose(pipe);
#else
            int ret = pclose(pipe);
#endif
            if (ret == 0 && !ver.empty()) {
                while (!ver.empty() && (ver.back() == '\r' || ver.back() == '\n')) {
                    ver.pop_back();
                }
                std::string archiver = "ar";
                std::string linker = cand;
                return GccToolchain(cand, archiver, linker, ver);
            }
        }
    }
    return std::nullopt;
}

// ============================================================================
// Emscripten Toolchain & Discovery (Story 15.1)
// ============================================================================

class EmscriptenToolchain : public IToolchain {
public:
    EmscriptenToolchain(std::string compiler_path = "em++", std::string archiver_path = "emar",
                        std::string linker_path = "em++", std::string version = "emcc",
                        std::filesystem::path emsdk_root = {},
                        std::unordered_map<std::string, std::string> env_vars = {})
        : compiler_path_(std::move(compiler_path)), archiver_path_(std::move(archiver_path)),
          linker_path_(std::move(linker_path)), version_(std::move(version)), emsdk_root_(std::move(emsdk_root)),
          env_vars_(std::move(env_vars)) {}

    [[nodiscard]] CompilerFamily family() const noexcept override { return CompilerFamily::Emscripten; }
    [[nodiscard]] std::string name() const override { return "Emscripten (" + version_ + ")"; }
    [[nodiscard]] std::string compiler_path() const override { return compiler_path_; }
    [[nodiscard]] std::string archiver_path() const override { return archiver_path_; }
    [[nodiscard]] std::string linker_path() const override { return linker_path_; }
    [[nodiscard]] const std::filesystem::path &emsdk_root() const noexcept { return emsdk_root_; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &env_vars() const noexcept override {
        return env_vars_;
    }

    [[nodiscard]] std::string format_import_library_name(std::string_view base_name) const override {
        return "lib" + std::string(base_name) + ".a";
    }

    [[nodiscard]] std::vector<std::string> get_depfile_flags(std::string_view depfile) const override {
        return {"-MMD", "-MF", std::string(depfile)};
    }

    [[nodiscard]] std::vector<std::string> build_compile_command(const CompileParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(compiler_path_);

        // Language standard
        if (params.is_c_source) {
            if (params.c_version.has_value()) {
                switch (*params.c_version) {
                case CVersion::C89:
                    cmd.push_back("-std=c89");
                    break;
                case CVersion::C99:
                    cmd.push_back("-std=c99");
                    break;
                case CVersion::C11:
                    cmd.push_back("-std=c11");
                    break;
                case CVersion::C17:
                    cmd.push_back("-std=c17");
                    break;
                case CVersion::C23:
                    cmd.push_back("-std=c2x");
                    break;
                }
            }
        } else {
            switch (params.cpp_version) {
            case CppVersion::Cpp11:
                cmd.push_back("-std=c++11");
                break;
            case CppVersion::Cpp14:
                cmd.push_back("-std=c++14");
                break;
            case CppVersion::Cpp17:
                cmd.push_back("-std=c++17");
                break;
            case CppVersion::Cpp20:
                cmd.push_back("-std=c++20");
                break;
            case CppVersion::Cpp23:
                cmd.push_back("-std=c++23");
                break;
            case CppVersion::Cpp26:
                cmd.push_back("-std=c++26");
                break;
            }
        }

        // Optimization
        switch (params.optimize) {
        case Optimization::Debug:
            cmd.push_back("-O0");
            cmd.push_back("-g");
            break;
        case Optimization::ReleaseFast:
            cmd.push_back("-O3");
            cmd.push_back("-DNDEBUG");
            break;
        case Optimization::ReleaseSmall:
            cmd.push_back("-Oz");
            cmd.push_back("-DNDEBUG");
            break;
        case Optimization::ReleaseSafe:
            cmd.push_back("-O2");
            cmd.push_back("-DNDEBUG");
            break;
        }

        // Exceptions and RTTI
        if (params.exceptions) {
            cmd.push_back("-fexceptions");
        } else {
            cmd.push_back("-fno-exceptions");
        }
        if (!params.rtti) {
            cmd.push_back("-fno-rtti");
        }

        // Warnings
        cmd.push_back("-Wall");
        cmd.push_back("-Wextra");
        if (params.warnings_as_errors) {
            cmd.push_back("-Werror");
        }

        // Standard defines
        cmd.push_back("-D__EMSCRIPTEN__=1");
        cmd.push_back("-DEMSCRIPTEN=1");
        cmd.push_back("-D__wasm__=1");
        cmd.push_back("-D__wasm32__=1");
        for (const auto &def : params.defines) {
            cmd.push_back("-D" + def);
        }

        // Include directories
        for (const auto &inc : params.include_dirs) {
            cmd.push_back("-I" + util::normalize_path(inc, false));
        }

        // Sysroot
        if (!params.sysroot.empty()) {
            cmd.push_back("--sysroot=" + util::normalize_path(params.sysroot.string(), false));
        }

        // Depfile
        if (!params.depfile.empty()) {
            cmd.push_back("-MMD");
            cmd.push_back("-MF");
            cmd.push_back(util::normalize_path(params.depfile, false));
        }

        // PCH
        if (!params.pch_file.empty()) {
            cmd.push_back("-include-pch");
            cmd.push_back(util::normalize_path(params.pch_file, false));
        } else if (!params.pch_header.empty()) {
            cmd.push_back("-include");
            cmd.push_back(util::normalize_path(params.pch_header, false));
        }

        // Extra compiler options (forwarded -s flags etc.)
        for (const auto &opt : params.extra_flags) {
            cmd.push_back(opt);
        }

        // Compile-only & input/output
        cmd.push_back("-c");
        cmd.push_back(util::normalize_path(params.source_file, false));
        cmd.push_back("-o");
        cmd.push_back(util::normalize_path(params.object_file, false));

        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_pch_command(const PchParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(compiler_path_);
        cmd.push_back("-x");
        cmd.push_back("c++-header");
        cmd.push_back("-c");
        cmd.push_back(util::normalize_path(params.header_file, false));
        cmd.push_back("-o");
        cmd.push_back(util::normalize_path(params.output_pch, false));
        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_archive_command(const ArchiveParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(archiver_path_);
        cmd.push_back("rcs");
        cmd.push_back(util::normalize_path(params.output_lib, false));
        for (const auto &obj : params.object_files) {
            cmd.push_back(util::normalize_path(obj, false));
        }
        for (const auto &flag : params.extra_flags) {
            cmd.push_back(flag);
        }
        return cmd;
    }

    [[nodiscard]] std::vector<std::string> build_link_command(const LinkParams &params) const override {
        std::vector<std::string> cmd;
        cmd.push_back(linker_path_);

        // Output binary (can be .html, .js, .wasm)
        cmd.push_back("-o");
        cmd.push_back(util::normalize_path(params.output_binary, false));

        // Object files
        for (const auto &obj : params.object_files) {
            cmd.push_back(util::normalize_path(obj, false));
        }

        // Library search paths
        for (const auto &dir : params.library_dirs) {
            cmd.push_back("-L" + util::normalize_path(dir, false));
        }

        // Linked libraries
        for (const auto &lib : params.libraries) {
            if (lib.find('/') != std::string::npos || lib.find('\\') != std::string::npos ||
                lib.rfind(".a") != std::string::npos) {
                cmd.push_back(util::normalize_path(lib, false));
            } else {
                cmd.push_back("-l" + lib);
            }
        }

        // Sysroot
        if (!params.sysroot.empty()) {
            cmd.push_back("--sysroot=" + util::normalize_path(params.sysroot.string(), false));
            cmd.push_back("-L" + util::normalize_path((params.sysroot / "lib").string(), false));
        }

        // Extra link flags (e.g. -sUSE_WEBGL2=1, -sALLOW_MEMORY_GROWTH=1, etc.)
        for (const auto &flag : params.extra_flags) {
            cmd.push_back(flag);
        }

        return cmd;
    }

private:
    std::string compiler_path_;
    std::string archiver_path_;
    std::string linker_path_;
    std::string version_;
    std::filesystem::path emsdk_root_;
    std::unordered_map<std::string, std::string> env_vars_;
};

inline std::optional<GccToolchain> find_gcc_cross_toolchain(const std::string &triple) {
    if (triple.empty() || triple == "native" || triple == "host") {
        return find_gcc_toolchain();
    }
    std::string prefix = triple + "-";
    std::string compiler = prefix + "g++";
    std::string archiver = prefix + "ar";
    std::string linker = compiler;

    ProcessResult res = ProcessRunner::run({compiler, "--version"});
    if (res.success()) {
        std::string ver = res.stdout_output;
        size_t nl = ver.find('\n');
        if (nl != std::string::npos)
            ver = ver.substr(0, nl);
        return GccToolchain(compiler, archiver, linker, ver);
    }
    return std::nullopt;
}

inline std::optional<EmscriptenToolchain> find_emscripten_toolchain(bool force_refresh = false) {
    (void)force_refresh;
    namespace fs = std::filesystem;
    std::error_code ec;

    std::vector<std::string> compiler_candidates;
    std::filesystem::path emsdk_found_root;

    // 1. Check EMSDK environment variable
    if (const char *emsdk_env = std::getenv("EMSDK")) {
        fs::path emsdk_path(emsdk_env);
        if (fs::exists(emsdk_path, ec)) {
            emsdk_found_root = emsdk_path;
            fs::path em_dir = emsdk_path / "upstream/emscripten";
#if defined(_WIN32)
            compiler_candidates.push_back((em_dir / "em++.bat").string());
            compiler_candidates.push_back((em_dir / "em++.cmd").string());
#endif
            compiler_candidates.push_back((em_dir / "em++").string());
        }
    }

    // 2. Check PATH candidates
#if defined(_WIN32)
    compiler_candidates.push_back("em++.bat");
    compiler_candidates.push_back("em++.cmd");
#endif
    compiler_candidates.push_back("em++");
#if defined(_WIN32)
    compiler_candidates.push_back("emcc.bat");
#endif
    compiler_candidates.push_back("emcc");

    // 3. Check standard installation directories
    std::vector<fs::path> standard_roots = {
#if defined(_WIN32)
        "C:/emsdk",
        "C:/Program Files/emsdk",
#else
        "/opt/emsdk",
        "/usr/local/emsdk",
#endif
    };
    if (const char *home = std::getenv(
#if defined(_WIN32)
            "USERPROFILE"
#else
            "HOME"
#endif
            )) {
        standard_roots.push_back(fs::path(home) / "emsdk");
        standard_roots.push_back(fs::path(home) / ".emsdk");
    }

    for (const auto &sr : standard_roots) {
        if (fs::exists(sr, ec)) {
            fs::path em_dir = sr / "upstream/emscripten";
#if defined(_WIN32)
            compiler_candidates.push_back((em_dir / "em++.bat").string());
#endif
            compiler_candidates.push_back((em_dir / "em++").string());
        }
    }

    for (const auto &cand : compiler_candidates) {
        ProcessResult pres = ProcessRunner::run({cand, "--version"});
        if (pres.success()) {
            std::string ver = pres.stdout_output;
            size_t nl = ver.find('\n');
            if (nl != std::string::npos)
                ver = ver.substr(0, nl);
            while (!ver.empty() && (ver.back() == '\r' || ver.back() == ' '))
                ver.pop_back();

            fs::path cand_path(cand);
            fs::path cand_dir = cand_path.parent_path();
            std::string archiver = "emar";
            std::string linker = cand;
            if (!cand_dir.empty()) {
#if defined(_WIN32)
                if (fs::exists(cand_dir / "emar.bat", ec))
                    archiver = (cand_dir / "emar.bat").string();
                else if (fs::exists(cand_dir / "emar", ec))
                    archiver = (cand_dir / "emar").string();
#else
                if (fs::exists(cand_dir / "emar", ec))
                    archiver = (cand_dir / "emar").string();
#endif
            }
            return EmscriptenToolchain(cand, archiver, linker, ver, emsdk_found_root);
        }
    }

    return std::nullopt;
}

// ============================================================================
// Unified Toolchain Finder
// ============================================================================

inline std::shared_ptr<IToolchain> find_toolchain(CompilerFamily family = CompilerFamily::Auto,
                                                  const TargetOptions &options = {}) {
    if (family == CompilerFamily::Emscripten || options.triple == "wasm32" || options.triple == "wasm64" ||
        options.triple.find("emscripten") != std::string::npos) {
        auto em = find_emscripten_toolchain();
        if (em)
            return std::make_shared<EmscriptenToolchain>(std::move(*em));
        if (options.triple.find("wasm") != std::string::npos) {
            auto clang = find_clang_toolchain();
            if (clang)
                return std::make_shared<ClangToolchain>(std::move(*clang));
        }
    }
    if (family == CompilerFamily::Clang) {
        auto clang = find_clang_toolchain();
        if (clang)
            return std::make_shared<ClangToolchain>(std::move(*clang));
        return nullptr;
    }
    if (family == CompilerFamily::Gcc) {
        if (!options.triple.empty() && options.triple != "native" && options.triple != "host") {
            auto cross_gcc = find_gcc_cross_toolchain(options.triple);
            if (cross_gcc)
                return std::make_shared<GccToolchain>(std::move(*cross_gcc));
        }
        auto gcc = find_gcc_toolchain();
        if (gcc)
            return std::make_shared<GccToolchain>(std::move(*gcc));
        return nullptr;
    }
    if (family == CompilerFamily::Msvc) {
        auto msvc = find_msvc_toolchain();
        if (msvc)
            return std::make_shared<MsvcToolchain>(std::move(*msvc));
        return nullptr;
    }
    // Auto:
    if (!options.triple.empty() && options.triple != "native" && options.triple != "host") {
        auto prof = TargetProfile::parse(options.triple);
        if (prof.is_wasm()) {
            auto em = find_emscripten_toolchain();
            if (em)
                return std::make_shared<EmscriptenToolchain>(std::move(*em));
        } else {
            auto cross_gcc = find_gcc_cross_toolchain(options.triple);
            if (cross_gcc)
                return std::make_shared<GccToolchain>(std::move(*cross_gcc));
        }
    }
    auto clang = find_clang_toolchain();
    if (clang)
        return std::make_shared<ClangToolchain>(std::move(*clang));
    auto gcc = find_gcc_toolchain();
    if (gcc)
        return std::make_shared<GccToolchain>(std::move(*gcc));
    auto msvc = find_msvc_toolchain();
    if (msvc)
        return std::make_shared<MsvcToolchain>(std::move(*msvc));
    return nullptr;
}

// ============================================================================
// 8.6. Target Compilation Pipelines (Executable, Static, Shared, Plugin)
// ============================================================================

struct BuildResult {
    bool success = false;
    int exit_code = 0;
    std::string artifact_path = "";
    std::vector<std::string> object_files = {};
    std::string error_message = "";
    std::chrono::milliseconds duration{0};
    bool up_to_date = false;
};

// ============================================================================
// 8.5. Makefile Depfile Scanner (GCC & Clang)
// ============================================================================

class DepfileScanner {
public:
    static std::vector<std::string> parse_dependencies(std::string_view content) {
        std::vector<std::string> deps;
        size_t n = content.size();
        size_t i = 0;

        bool in_prerequisites = false;
        std::string cur_token;

        auto flush_token = [&]() {
            if (!cur_token.empty()) {
                if (in_prerequisites) {
                    deps.push_back(cur_token);
                }
                cur_token.clear();
            }
        };

        while (i < n) {
            char c = content[i];

            // 1. Line continuation: \ followed by \r\n or \n
            if (c == '\\' && i + 1 < n && (content[i + 1] == '\r' || content[i + 1] == '\n')) {
                i += (content[i + 1] == '\r' && i + 2 < n && content[i + 2] == '\n') ? 3 : 2;
                flush_token();
                continue;
            }

            // 2. Escaped characters: \ followed by space, tab, backslash, hash, dollar
            if (c == '\\' && i + 1 < n) {
                char next = content[i + 1];
                if (next == ' ' || next == '\t' || next == '\\' || next == '#' || next == '$') {
                    cur_token.push_back(next);
                    i += 2;
                    continue;
                }
            }

            // 3. Newline: ends current rule
            if (c == '\n' || c == '\r') {
                flush_token();
                in_prerequisites = false;
                ++i;
                continue;
            }

            // 4. Colon: check if Windows drive letter (e.g. C:/ or C:\)
            if (c == ':') {
                bool is_drive_letter = false;
                if (!cur_token.empty() && cur_token.size() == 1 &&
                    std::isalpha(static_cast<unsigned char>(cur_token[0]))) {
                    if (i + 1 < n && (content[i + 1] == '/' || content[i + 1] == '\\')) {
                        is_drive_letter = true;
                    }
                }
                if (is_drive_letter) {
                    cur_token.push_back(':');
                    ++i;
                    continue;
                } else {
                    cur_token.clear();
                    in_prerequisites = true;
                    ++i;
                    continue;
                }
            }

            // 5. Whitespace separates tokens
            if (c == ' ' || c == '\t') {
                flush_token();
                ++i;
                continue;
            }

            cur_token.push_back(c);
            ++i;
        }

        flush_token();
        return deps;
    }

    static std::vector<std::string> parse_file(const std::filesystem::path &depfile_path) {
        std::ifstream file(depfile_path, std::ios::binary);
        if (!file.is_open()) {
            return {};
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        auto deps = parse_dependencies(content);
        record_dependencies(depfile_path.string(), deps);
        return deps;
    }

    static bool is_object_up_to_date(const std::filesystem::path &obj_path, const std::filesystem::path &depfile_path) {
        namespace fs = std::filesystem;
        std::error_code ec;

        if (!fs::exists(obj_path, ec)) {
            return false;
        }
        auto obj_time = fs::last_write_time(obj_path, ec);
        if (ec) {
            return false;
        }

        if (!fs::exists(depfile_path, ec)) {
            return false;
        }

        auto deps = parse_file(depfile_path);
        if (deps.empty()) {
            return false;
        }

        for (const auto &dep : deps) {
            fs::path p(dep);
            if (!fs::exists(p, ec)) {
                return false;
            }
            auto dep_time = fs::last_write_time(p, ec);
            if (ec) {
                return false;
            }
            if (dep_time > obj_time) {
                return false;
            }
        }

        return true;
    }

    static void record_dependencies(const std::string &depfile_key, const std::vector<std::string> &deps) {
        std::lock_guard<std::mutex> lock(cache_mutex());
        cache()[depfile_key] = deps;
    }

    static std::vector<std::string> get_cached_dependencies(const std::string &depfile_key) {
        std::lock_guard<std::mutex> lock(cache_mutex());
        auto it = cache().find(depfile_key);
        if (it != cache().end()) {
            return it->second;
        }
        return {};
    }

    static void clear_cache() {
        std::lock_guard<std::mutex> lock(cache_mutex());
        cache().clear();
    }

private:
    static std::unordered_map<std::string, std::vector<std::string>> &cache() {
        static std::unordered_map<std::string, std::vector<std::string>> s_cache;
        return s_cache;
    }

    static std::mutex &cache_mutex() {
        static std::mutex s_mtx;
        return s_mtx;
    }
};

// ============================================================================
// 8.5.1. MSVC /showIncludes Dependency Scanner
// ============================================================================

struct ShowIncludesResult {
    std::vector<std::string> dependencies;
    std::string clean_output;
};

class ShowIncludesScanner {
public:
    static bool is_show_includes_line(std::string_view line, std::string_view prefix = "Note: including file:") {
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) {
            line.remove_prefix(1);
        }
        if (line.size() < prefix.size()) {
            return false;
        }
        for (size_t i = 0; i < prefix.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(line[i])) !=
                std::tolower(static_cast<unsigned char>(prefix[i]))) {
                return false;
            }
        }
        return true;
    }

    static std::string extract_header_path(std::string_view line, std::string_view prefix = "Note: including file:") {
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) {
            line.remove_prefix(1);
        }
        if (line.size() < prefix.size()) {
            return "";
        }
        line.remove_prefix(prefix.size());
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) {
            line.remove_prefix(1);
        }
        while (!line.empty() &&
               (line.back() == '\r' || line.back() == '\n' || line.back() == ' ' || line.back() == '\t')) {
            line.remove_suffix(1);
        }
        return std::string(line);
    }

    static ShowIncludesResult parse(std::string_view output, std::string_view prefix = "Note: including file:") {
        ShowIncludesResult res;
        std::istringstream iss{std::string(output)};
        std::string line;
        std::ostringstream clean_oss;
        bool first_clean_line = true;

        while (std::getline(iss, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (is_show_includes_line(line, prefix)) {
                std::string hdr = extract_header_path(line, prefix);
                if (!hdr.empty()) {
                    res.dependencies.push_back(hdr);
                }
            } else {
                if (!first_clean_line) {
                    clean_oss << "\n";
                }
                clean_oss << line;
                first_clean_line = false;
            }
        }

        res.clean_output = clean_oss.str();
        return res;
    }

    static std::string escape_depfile_path(std::string_view path) {
        std::string res;
        for (char c : path) {
            if (c == ' ') {
                res += "\\ ";
            } else if (c == '\\') {
                res += '/';
            } else {
                res.push_back(c);
            }
        }
        return res;
    }

    static bool write_depfile(const std::filesystem::path &depfile_path, const std::string &object_path,
                              const std::string &source_path, const std::vector<std::string> &dependencies) {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (depfile_path.has_parent_path()) {
            fs::create_directories(depfile_path.parent_path(), ec);
        }
        std::ofstream out(depfile_path, std::ios::trunc);
        if (!out.is_open()) {
            return false;
        }

        out << escape_depfile_path(util::normalize_path(object_path, false)) << ": "
            << escape_depfile_path(util::normalize_path(source_path, false));
        for (const auto &dep : dependencies) {
            out << " \\\n  " << escape_depfile_path(util::normalize_path(dep, false));
        }
        out << "\n";
        return true;
    }
};

namespace hash {

inline constexpr uint64_t PRIME64_1 = 0x9E3779B185EBCA87ULL;
inline constexpr uint64_t PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
inline constexpr uint64_t PRIME64_3 = 0x165667B19E3779F9ULL;
inline constexpr uint64_t PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
inline constexpr uint64_t PRIME64_5 = 0x27D4EB2F165667C5ULL;

inline constexpr uint64_t rotl64(uint64_t x, int r) noexcept {
    return (x << r) | (x >> (64 - r));
}

inline uint64_t round64(uint64_t acc, uint64_t input) noexcept {
    acc += input * PRIME64_2;
    acc = rotl64(acc, 31);
    acc *= PRIME64_1;
    return acc;
}

inline uint64_t merge_round64(uint64_t acc, uint64_t val) noexcept {
    val = round64(0, val);
    acc ^= val;
    acc = acc * PRIME64_1 + PRIME64_4;
    return acc;
}

inline uint64_t read64_le(const uint8_t *p) noexcept {
    return static_cast<uint64_t>(p[0]) | (static_cast<uint64_t>(p[1]) << 8) | (static_cast<uint64_t>(p[2]) << 16) |
           (static_cast<uint64_t>(p[3]) << 24) | (static_cast<uint64_t>(p[4]) << 32) |
           (static_cast<uint64_t>(p[5]) << 40) | (static_cast<uint64_t>(p[6]) << 48) |
           (static_cast<uint64_t>(p[7]) << 56);
}

inline uint32_t read32_le(const uint8_t *p) noexcept {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

inline uint64_t xxhash64(const void *input, size_t len, uint64_t seed = 0) noexcept {
    const uint8_t *p = static_cast<const uint8_t *>(input);
    const uint8_t *const bEnd = p + len;
    uint64_t h64;

    if (len >= 32) {
        const uint8_t *const limit = bEnd - 32;
        uint64_t v1 = seed + PRIME64_1 + PRIME64_2;
        uint64_t v2 = seed + PRIME64_2;
        uint64_t v3 = seed + 0;
        uint64_t v4 = seed - PRIME64_1;

        do {
            v1 = round64(v1, read64_le(p));
            p += 8;
            v2 = round64(v2, read64_le(p));
            p += 8;
            v3 = round64(v3, read64_le(p));
            p += 8;
            v4 = round64(v4, read64_le(p));
            p += 8;
        } while (p <= limit);

        h64 = rotl64(v1, 1) + rotl64(v2, 7) + rotl64(v3, 12) + rotl64(v4, 18);
        h64 = merge_round64(h64, v1);
        h64 = merge_round64(h64, v2);
        h64 = merge_round64(h64, v3);
        h64 = merge_round64(h64, v4);
    } else {
        h64 = seed + PRIME64_5;
    }

    h64 += static_cast<uint64_t>(len);

    while (p + 8 <= bEnd) {
        uint64_t k1 = round64(0, read64_le(p));
        h64 ^= k1;
        h64 = rotl64(h64, 27) * PRIME64_1 + PRIME64_4;
        p += 8;
    }

    if (p + 4 <= bEnd) {
        h64 ^= static_cast<uint64_t>(read32_le(p)) * PRIME64_1;
        h64 = rotl64(h64, 23) * PRIME64_2 + PRIME64_3;
        p += 4;
    }

    while (p < bEnd) {
        h64 ^= static_cast<uint64_t>(*p) * PRIME64_5;
        h64 = rotl64(h64, 11) * PRIME64_1;
        p++;
    }

    h64 ^= h64 >> 33;
    h64 *= PRIME64_2;
    h64 ^= h64 >> 29;
    h64 *= PRIME64_3;
    h64 ^= h64 >> 32;

    return h64;
}

inline std::string to_hex(uint64_t val) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << val;
    return oss.str();
}

inline uint64_t from_hex(std::string_view hex) {
    if (hex.empty())
        return 0;
    uint64_t val = 0;
    for (char c : hex) {
        val <<= 4;
        if (c >= '0' && c <= '9')
            val |= static_cast<uint64_t>(c - '0');
        else if (c >= 'a' && c <= 'f')
            val |= static_cast<uint64_t>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
            val |= static_cast<uint64_t>(c - 'A' + 10);
    }
    return val;
}

inline std::optional<uint64_t> hash_file(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return std::nullopt;
    }
    file.seekg(0, std::ios::end);
    auto sz = file.tellg();
    if (sz < 0)
        return std::nullopt;
    file.seekg(0, std::ios::beg);
    std::string buffer;
    buffer.resize(static_cast<size_t>(sz));
    if (sz > 0) {
        file.read(buffer.data(), sz);
        if (!file && !file.eof()) {
            return std::nullopt;
        }
    }
    return xxhash64(buffer.data(), buffer.size(), 0);
}

class Sha256 {
public:
    Sha256() { reset(); }

    void reset() {
        state_[0] = 0x6a09e667;
        state_[1] = 0xbb67ae85;
        state_[2] = 0x3c6ef372;
        state_[3] = 0xa54ff53a;
        state_[4] = 0x510e527f;
        state_[5] = 0x9b05688c;
        state_[6] = 0x1f83d9ab;
        state_[7] = 0x5be0cd19;
        count_ = 0;
        buffer_len_ = 0;
    }

    void update(const void *data, size_t len) {
        const uint8_t *p = static_cast<const uint8_t *>(data);
        count_ += len;
        while (len > 0) {
            size_t take = std::min(len, static_cast<size_t>(64 - buffer_len_));
            std::memcpy(buffer_ + buffer_len_, p, take);
            buffer_len_ += take;
            p += take;
            len -= take;
            if (buffer_len_ == 64) {
                transform(buffer_);
                buffer_len_ = 0;
            }
        }
    }

    void update(std::string_view sv) { update(sv.data(), sv.size()); }

    std::string finalize() {
        uint64_t total_bits = count_ * 8;
        uint8_t pad = 0x80;
        update(&pad, 1);
        uint8_t zero = 0;
        while (buffer_len_ != 56) {
            update(&zero, 1);
        }
        uint8_t len_bytes[8];
        for (int i = 7; i >= 0; --i) {
            len_bytes[i] = static_cast<uint8_t>(total_bits & 0xFF);
            total_bits >>= 8;
        }
        update(len_bytes, 8);

        std::ostringstream oss;
        for (int i = 0; i < 8; ++i) {
            oss << std::hex << std::setfill('0') << std::setw(8) << state_[i];
        }
        return oss.str();
    }

    static std::string hash(std::string_view sv) {
        Sha256 s;
        s.update(sv);
        return s.finalize();
    }

    static std::string hash_file(const std::filesystem::path &file_path) {
        std::ifstream is(file_path, std::ios::binary);
        if (!is.is_open()) {
            return "";
        }
        Sha256 s;
        char buffer[16384];
        while (is.read(buffer, sizeof(buffer))) {
            s.update(buffer, static_cast<size_t>(is.gcount()));
        }
        if (is.gcount() > 0) {
            s.update(buffer, static_cast<size_t>(is.gcount()));
        }
        return s.finalize();
    }

private:
    static constexpr uint32_t rotr32(uint32_t x, uint32_t n) noexcept { return (x >> n) | (x << (32 - n)); }

    static constexpr uint32_t ch(uint32_t x, uint32_t y, uint32_t z) noexcept { return (x & y) ^ (~x & z); }

    static constexpr uint32_t maj(uint32_t x, uint32_t y, uint32_t z) noexcept { return (x & y) ^ (x & z) ^ (y & z); }

    static constexpr uint32_t ep0(uint32_t x) noexcept { return rotr32(x, 2) ^ rotr32(x, 13) ^ rotr32(x, 22); }

    static constexpr uint32_t ep1(uint32_t x) noexcept { return rotr32(x, 6) ^ rotr32(x, 11) ^ rotr32(x, 25); }

    static constexpr uint32_t sig0(uint32_t x) noexcept { return rotr32(x, 7) ^ rotr32(x, 18) ^ (x >> 3); }

    static constexpr uint32_t sig1(uint32_t x) noexcept { return rotr32(x, 17) ^ rotr32(x, 19) ^ (x >> 10); }

    void transform(const uint8_t block[64]) {
        static constexpr uint32_t K[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) | (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(block[i * 4 + 2]) << 8) | (static_cast<uint32_t>(block[i * 4 + 3]));
        }
        for (int i = 16; i < 64; ++i) {
            w[i] = sig1(w[i - 2]) + w[i - 7] + sig0(w[i - 15]) + w[i - 16];
        }

        uint32_t a = state_[0];
        uint32_t b = state_[1];
        uint32_t c = state_[2];
        uint32_t d = state_[3];
        uint32_t e = state_[4];
        uint32_t f = state_[5];
        uint32_t g = state_[6];
        uint32_t h = state_[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t t1 = h + ep1(e) + ch(e, f, g) + K[i] + w[i];
            uint32_t t2 = ep0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        state_[0] += a;
        state_[1] += b;
        state_[2] += c;
        state_[3] += d;
        state_[4] += e;
        state_[5] += f;
        state_[6] += g;
        state_[7] += h;
    }

    uint32_t state_[8];
    uint64_t count_{0};
    uint8_t buffer_[64];
    size_t buffer_len_{0};
};

inline std::optional<std::string> sha256_file(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return std::nullopt;
    }
    Sha256 hasher;
    char buffer[8192];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        hasher.update(buffer, static_cast<size_t>(file.gcount()));
    }
    return hasher.finalize();
}

} // namespace hash

namespace util {

inline std::string generate_deterministic_guid(std::string_view name) {
    uint64_t h1 = hash::xxhash64(name.data(), name.size(), 0x123456789ABCDEF0ULL);
    uint64_t h2 = hash::xxhash64(name.data(), name.size(), 0xFEDCBA9876543210ULL);
    uint32_t p1 = static_cast<uint32_t>(h1 >> 32);
    uint16_t p2 = static_cast<uint16_t>((h1 >> 16) & 0xFFFF);
    uint16_t p3 = static_cast<uint16_t>((h1 & 0x0FFF) | 0x5000);         // Version 5
    uint16_t p4 = static_cast<uint16_t>(((h2 >> 48) & 0x3FFF) | 0x8000); // RFC 4122 variant
    uint64_t p5 = h2 & 0x0000FFFFFFFFFFFFULL;

    char buf[40];
    std::snprintf(buf, sizeof(buf), "{%08X-%04X-%04X-%04X-%012llX}", p1, p2, p3, p4,
                  static_cast<unsigned long long>(p5));
    return std::string(buf);
}

} // namespace util

struct HeaderState {
    std::string path;
    uint64_t hash = 0;
    uint64_t mtime = 0;
};

struct ObjectBuildState {
    std::string source_file;
    std::string object_file;
    uint64_t source_hash = 0;
    uint64_t source_mtime = 0;
    uint64_t source_size = 0;
    std::string compiler_signature;
    std::string compile_flags;
    std::string fingerprint;
    std::vector<HeaderState> headers;
};

class BuildStateDatabase {
public:
    static BuildStateDatabase &instance() {
        static BuildStateDatabase inst;
        return inst;
    }

    void set_db_path(const std::filesystem::path &path) {
        std::lock_guard<std::mutex> lk(mtx_);
        db_path_ = path;
    }

    std::filesystem::path db_path() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return db_path_.empty() ? default_db_path() : db_path_;
    }

    void clear() {
        std::lock_guard<std::mutex> lk(mtx_);
        entries_.clear();
        file_hash_cache_.clear();
        loaded_ = false;
        is_dirty_ = false;
        db_path_.clear();
    }

    bool is_loaded() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return loaded_;
    }

    bool load(const std::filesystem::path &path = "") {
        std::lock_guard<std::mutex> lk(mtx_);
        std::filesystem::path p = path.empty() ? (db_path_.empty() ? default_db_path() : db_path_) : path;
        db_path_ = p;
        entries_.clear();
        is_dirty_ = false;
        loaded_ = true;

        std::error_code ec;
        if (!std::filesystem::exists(p, ec)) {
            return false;
        }

        std::ifstream f(p);
        if (!f.is_open()) {
            return false;
        }
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.close();

        auto root = util::JsonValue::parse(content);
        if (!root || !root->is_object()) {
            return false;
        }

        auto objs = (*root)["objects"];
        if (objs.is_array()) {
            for (const auto &item : objs.as_array()) {
                if (!item.is_object())
                    continue;
                std::string obj_file = util::normalize_path(item["object"].as_string(), false);
                ObjectBuildState state;
                state.object_file = obj_file;
                state.source_file = util::normalize_path(item["source"].as_string(), false);
                state.source_hash = hash::from_hex(item["source_hash"].as_string());
                state.source_mtime = static_cast<uint64_t>(item["source_mtime"].as_int64());
                state.source_size = static_cast<uint64_t>(item["source_size"].as_int64());
                state.compiler_signature = item["compiler"].as_string();
                state.compile_flags = item["flags"].as_string();
                state.fingerprint = item["fingerprint"].as_string();

                auto hdrs = item["headers"];
                if (hdrs.is_array()) {
                    for (const auto &h : hdrs.as_array()) {
                        if (!h.is_object())
                            continue;
                        HeaderState hs;
                        hs.path = util::normalize_path(h["path"].as_string(), false);
                        hs.hash = hash::from_hex(h["hash"].as_string());
                        hs.mtime = static_cast<uint64_t>(h["mtime"].as_int64());
                        state.headers.push_back(hs);
                    }
                }
                entries_[obj_file] = state;
            }
        }
        return true;
    }

    bool save(const std::filesystem::path &path = "") {
        std::lock_guard<std::mutex> lk(mtx_);
        std::filesystem::path p = path.empty() ? (db_path_.empty() ? default_db_path() : db_path_) : path;
        if (util::is_system_protected_dir(p)) {
            return false;
        }
        std::error_code ec;
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path(), ec);
        }

        std::ofstream out(p, std::ios::trunc);
        if (!out.is_open()) {
            return false;
        }

        out << "{\n";
        out << "  \"version\": 1,\n";
        out << "  \"objects\": [\n";
        size_t idx = 0;
        for (const auto &[k, state] : entries_) {
            (void)k;
            out << "    {\n";
            out << "      \"object\": \"" << util::escape_json(state.object_file) << "\",\n";
            out << "      \"source\": \"" << util::escape_json(state.source_file) << "\",\n";
            out << "      \"source_hash\": \"" << hash::to_hex(state.source_hash) << "\",\n";
            out << "      \"source_mtime\": " << state.source_mtime << ",\n";
            out << "      \"source_size\": " << state.source_size << ",\n";
            out << "      \"compiler\": \"" << util::escape_json(state.compiler_signature) << "\",\n";
            out << "      \"flags\": \"" << util::escape_json(state.compile_flags) << "\",\n";
            out << "      \"fingerprint\": \"" << util::escape_json(state.fingerprint) << "\",\n";
            out << "      \"headers\": [\n";
            for (size_t h_idx = 0; h_idx < state.headers.size(); ++h_idx) {
                const auto &h = state.headers[h_idx];
                out << "        {\"path\": \"" << util::escape_json(h.path) << "\", \"hash\": \""
                    << hash::to_hex(h.hash) << "\", \"mtime\": " << h.mtime << "}"
                    << (h_idx + 1 < state.headers.size() ? "," : "") << "\n";
            }
            out << "      ]\n";
            out << "    }" << (++idx < entries_.size() ? "," : "") << "\n";
        }
        out << "  ]\n";
        out << "}\n";
        is_dirty_ = false;
        return true;
    }

    bool has_object_state(const std::string &obj_path) const {
        std::lock_guard<std::mutex> lk(mtx_);
        std::string norm = util::normalize_path(obj_path, false);
        return entries_.find(norm) != entries_.end();
    }

    std::optional<ObjectBuildState> get_object_state(const std::string &obj_path) const {
        std::lock_guard<std::mutex> lk(mtx_);
        std::string norm = util::normalize_path(obj_path, false);
        auto it = entries_.find(norm);
        if (it != entries_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void set_object_state(const std::string &obj_path, const ObjectBuildState &state) {
        std::lock_guard<std::mutex> lk(mtx_);
        std::string norm = util::normalize_path(obj_path, false);
        entries_[norm] = state;
        is_dirty_ = true;
    }

    bool is_dirty() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return is_dirty_;
    }

    struct FileHashEntry {
        uint64_t hash = 0;
        uint64_t mtime = 0;
        uint64_t size = 0;
    };

    uint64_t get_file_content_hash(const std::filesystem::path &path) {
        namespace fs = std::filesystem;
        std::error_code ec;
        auto fsize = fs::file_size(path, ec);
        if (ec)
            return 0;
        auto ftime = fs::last_write_time(path, ec);
        if (ec)
            return 0;
        uint64_t mtime_epoch =
            static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count());

        std::string key = util::normalize_path(path.string(), false);
        {
            std::lock_guard<std::mutex> lk(mtx_);
            auto it = file_hash_cache_.find(key);
            if (it != file_hash_cache_.end()) {
                if (it->second.mtime == mtime_epoch && it->second.size == fsize) {
                    return it->second.hash;
                }
            }
        }
        auto h = hash::hash_file(path);
        uint64_t val = h.value_or(0);
        {
            std::lock_guard<std::mutex> lk(mtx_);
            file_hash_cache_[key] = FileHashEntry{val, mtime_epoch, fsize};
        }
        return val;
    }

    static std::filesystem::path default_db_path() {
        return std::filesystem::path(".cxxgen") / "cache" / "build_state.json";
    }

    static std::string compute_fingerprint(uint64_t src_hash, const std::vector<HeaderState> &headers,
                                           const std::string &compiler_sig, const std::string &compile_flags) {
        std::vector<HeaderState> sorted_hdrs = headers;
        std::sort(sorted_hdrs.begin(), sorted_hdrs.end(),
                  [](const HeaderState &a, const HeaderState &b) { return a.path < b.path; });

        std::string combined;
        combined.reserve(compiler_sig.size() + compile_flags.size() + sorted_hdrs.size() * 64 + 64);
        combined += hash::to_hex(src_hash);
        combined += "|";
        combined += compiler_sig;
        combined += "|";
        combined += compile_flags;
        combined += "|";
        for (const auto &h : sorted_hdrs) {
            combined += h.path;
            combined += "=";
            combined += hash::to_hex(h.hash);
            combined += ";";
        }
        uint64_t fp = hash::xxhash64(combined.data(), combined.size(), 0);
        return hash::to_hex(fp);
    }

    bool is_object_up_to_date(const std::filesystem::path &obj_path, const std::string &src_path,
                              const std::string &compiler_sig, const std::string &compile_flags) {
        namespace fs = std::filesystem;
        std::error_code ec;

        if (!fs::exists(obj_path, ec)) {
            return false;
        }
        if (!fs::exists(src_path, ec)) {
            return false;
        }

        std::string norm_obj = util::normalize_path(obj_path.string(), false);
        std::optional<ObjectBuildState> opt_state;
        {
            std::lock_guard<std::mutex> lk(mtx_);
            auto it = entries_.find(norm_obj);
            if (it != entries_.end()) {
                opt_state = it->second;
            }
        }

        if (!opt_state.has_value()) {
            return false;
        }

        const auto &state = *opt_state;

        // 1. Compiler signature and compile flags check
        if (state.compiler_signature != compiler_sig) {
            return false;
        }
        if (state.compile_flags != compile_flags) {
            return false;
        }

        // 2. Source file check
        auto src_fsize = fs::file_size(src_path, ec);
        if (ec)
            return false;
        auto src_ftime = fs::last_write_time(src_path, ec);
        if (ec)
            return false;
        uint64_t src_mtime_epoch = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(src_ftime.time_since_epoch()).count());

        uint64_t current_src_hash = 0;
        if (src_mtime_epoch == state.source_mtime && src_fsize == state.source_size) {
            current_src_hash = state.source_hash;
        } else {
            current_src_hash = get_file_content_hash(src_path);
            if (current_src_hash != state.source_hash) {
                return false;
            }
        }

        // 3. Dependent headers check
        std::vector<HeaderState> current_headers;
        current_headers.reserve(state.headers.size());

        for (const auto &h : state.headers) {
            if (!fs::exists(h.path, ec)) {
                return false;
            }
            auto h_ftime = fs::last_write_time(h.path, ec);
            if (ec)
                return false;
            uint64_t h_mtime_epoch = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(h_ftime.time_since_epoch()).count());

            uint64_t h_hash = 0;
            if (h_mtime_epoch == h.mtime) {
                h_hash = h.hash;
            } else {
                h_hash = get_file_content_hash(h.path);
                if (h_hash != h.hash) {
                    return false;
                }
            }
            current_headers.push_back({h.path, h_hash, h_mtime_epoch});
        }

        // 4. Overall fingerprint verification
        std::string current_fp = compute_fingerprint(current_src_hash, current_headers, compiler_sig, compile_flags);
        return current_fp == state.fingerprint;
    }

private:
    BuildStateDatabase() = default;
    mutable std::mutex mtx_;
    std::filesystem::path db_path_;
    std::unordered_map<std::string, ObjectBuildState> entries_;
    std::unordered_map<std::string, FileHashEntry> file_hash_cache_;
    bool loaded_ = false;
    bool is_dirty_ = false;
};

struct CliArgs;

struct CacheStats {
    uint64_t hits = 0;
    uint64_t direct_hits = 0;
    uint64_t misses = 0;
    uint64_t files_stored = 0;
    uint64_t total_files = 0;
    uint64_t total_bytes = 0;
    uint64_t max_bytes = 5ULL * 1024 * 1024 * 1024; // 5 GB
    uint64_t reclaimed_bytes = 0;
};

class GlobalObjectCache {
public:
    static GlobalObjectCache &instance() {
        static GlobalObjectCache inst;
        return inst;
    }

    static std::filesystem::path default_cache_dir() {
        const char *env_dir = std::getenv("CXXGEN_CACHE_DIR");
        if (env_dir && *env_dir) {
            return std::filesystem::path(env_dir);
        }
#if defined(_WIN32)
        const char *user_profile = std::getenv("USERPROFILE");
        if (user_profile && *user_profile) {
            return std::filesystem::path(user_profile) / ".cxxgen" / "cache";
        }
        const char *app_data = std::getenv("LOCALAPPDATA");
        if (app_data && *app_data) {
            return std::filesystem::path(app_data) / "cxxgen" / "cache";
        }
#else
        const char *home = std::getenv("HOME");
        if (home && *home) {
            return std::filesystem::path(home) / ".cxxgen" / "cache";
        }
#endif
        return std::filesystem::path(".cxxgen") / "global_cache";
    }

    void set_cache_dir(const std::filesystem::path &dir) {
        std::lock_guard<std::recursive_mutex> lk(mtx_);
        cache_dir_ = dir;
        loaded_config_ = false;
    }

    std::filesystem::path get_cache_dir() const {
        std::lock_guard<std::recursive_mutex> lk(mtx_);
        return cache_dir_.empty() ? default_cache_dir() : cache_dir_;
    }

    std::filesystem::path get_obj_dir() const { return get_cache_dir() / "obj"; }

    std::filesystem::path get_manifest_dir() const { return get_cache_dir() / "manifests"; }

    std::filesystem::path get_config_path() const { return get_cache_dir() / "cache_config.json"; }

    std::filesystem::path get_stats_path() const { return get_cache_dir() / "cache_stats.json"; }

    void set_enabled(bool enabled) noexcept { enabled_.store(enabled); }

    bool is_enabled() const noexcept { return enabled_.load(); }

    void set_max_size(uint64_t bytes) {
        std::lock_guard<std::recursive_mutex> lk(mtx_);
        max_size_bytes_ = bytes;
        save_config_locked();
    }

    uint64_t get_max_size() const {
        std::lock_guard<std::recursive_mutex> lk(mtx_);
        ensure_config_loaded_locked();
        return max_size_bytes_;
    }

    static uint64_t parse_size(std::string_view str) {
        while (!str.empty() && std::isspace(static_cast<unsigned char>(str.front())))
            str.remove_prefix(1);
        while (!str.empty() && std::isspace(static_cast<unsigned char>(str.back())))
            str.remove_suffix(1);
        if (str.empty())
            return 0;

        uint64_t multiplier = 1;
        std::string num_str;
        for (size_t i = 0; i < str.size(); ++i) {
            char c = str[i];
            if (std::isdigit(static_cast<unsigned char>(c))) {
                num_str.push_back(c);
            } else {
                std::string_view suffix = str.substr(i);
                if (suffix == "K" || suffix == "KB" || suffix == "k" || suffix == "kb") {
                    multiplier = 1024ULL;
                } else if (suffix == "M" || suffix == "MB" || suffix == "m" || suffix == "mb") {
                    multiplier = 1024ULL * 1024ULL;
                } else if (suffix == "G" || suffix == "GB" || suffix == "g" || suffix == "gb") {
                    multiplier = 1024ULL * 1024ULL * 1024ULL;
                } else if (suffix == "T" || suffix == "TB" || suffix == "t" || suffix == "tb") {
                    multiplier = 1024ULL * 1024ULL * 1024ULL * 1024ULL;
                }
                break;
            }
        }
        if (num_str.empty())
            return 0;
        try {
            return static_cast<uint64_t>(std::stoull(num_str)) * multiplier;
        } catch (...) {
            return 0;
        }
    }

    static std::string format_bytes(uint64_t bytes) {
        std::ostringstream oss;
        if (bytes >= 1024ULL * 1024ULL * 1024ULL) {
            double gb = static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
            oss << std::fixed << std::setprecision(1) << gb << " GB";
        } else if (bytes >= 1024ULL * 1024ULL) {
            double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
            oss << std::fixed << std::setprecision(1) << mb << " MB";
        } else if (bytes >= 1024ULL) {
            double kb = static_cast<double>(bytes) / 1024.0;
            oss << std::fixed << std::setprecision(1) << kb << " KB";
        } else {
            oss << bytes << " B";
        }
        return oss.str();
    }

    static std::string make_portable_path(const std::string &path_str, const std::string &src_dir,
                                          const std::vector<std::string> &include_dirs = {}) {
        std::string norm = util::normalize_path(path_str, false);
        for (const auto &inc : include_dirs) {
            std::string inc_norm = util::normalize_path(inc, false);
            if (!inc_norm.empty() && norm.rfind(inc_norm, 0) == 0) {
                std::string rel = norm.substr(inc_norm.size());
                while (!rel.empty() && (rel[0] == '/' || rel[0] == '\\')) {
                    rel = rel.substr(1);
                }
                return rel;
            }
        }
        if (!src_dir.empty()) {
            std::string src_norm = util::normalize_path(src_dir, false);
            if (norm.rfind(src_norm, 0) == 0) {
                std::string rel = norm.substr(src_norm.size());
                while (!rel.empty() && (rel[0] == '/' || rel[0] == '\\')) {
                    rel = rel.substr(1);
                }
                return rel;
            }
        }
        return norm;
    }

    static std::filesystem::path resolve_portable_path(const std::string &portable_path, const std::string &src_dir,
                                                       const std::vector<std::string> &include_dirs = {}) {
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path p(portable_path);
        if (fs::exists(p, ec)) {
            return p;
        }
        if (!src_dir.empty()) {
            fs::path p_src = fs::path(src_dir) / portable_path;
            if (fs::exists(p_src, ec)) {
                return p_src;
            }
        }
        for (const auto &inc : include_dirs) {
            fs::path p_inc = fs::path(inc) / portable_path;
            if (fs::exists(p_inc, ec)) {
                return p_inc;
            }
        }
        return p;
    }

    static std::string compute_cache_key(const std::string &src_sha256, const std::string &compiler_sig,
                                         const std::string &compile_flags, const std::string &target_triple,
                                         const std::vector<std::pair<std::string, std::string>> &sorted_headers) {
        hash::Sha256 hasher;
        hasher.update("src:" + src_sha256 + "\n");
        hasher.update("compiler:" + compiler_sig + "\n");
        hasher.update("flags:" + compile_flags + "\n");
        if (!target_triple.empty()) {
            hasher.update("triple:" + target_triple + "\n");
        }
        for (const auto &[hdr_path, hdr_hash] : sorted_headers) {
            hasher.update("hdr:" + hdr_path + "=" + hdr_hash + "\n");
        }
        return hasher.finalize();
    }

    bool try_direct_restore(const std::string &src_path, const std::string &compiler_sig,
                            const std::string &compile_flags, const std::string &target_triple,
                            const std::filesystem::path &dest_obj_path,
                            const std::vector<std::string> &include_dirs = {}) {
        if (!enabled_.load())
            return false;
        namespace fs = std::filesystem;
        std::error_code ec;

        if (!fs::exists(src_path, ec))
            return false;

        std::string src_sha256 = hash::Sha256::hash_file(src_path);
        if (src_sha256.empty())
            return false;

        fs::path manifest_dir = get_manifest_dir();
        fs::path prefix_dir = manifest_dir / src_sha256.substr(0, 2);
        fs::path manifest_file = prefix_dir / (src_sha256 + ".json");

        if (!fs::exists(manifest_file, ec)) {
            return false;
        }

        std::string content;
        {
            std::ifstream f(manifest_file);
            if (!f.is_open())
                return false;
            content.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        }

        auto root = util::JsonValue::parse(content);
        if (!root || !root->is_object())
            return false;

        auto entries = (*root)["entries"];
        if (!entries.is_array())
            return false;

        std::string matched_key;
        std::string src_dir = fs::path(src_path).parent_path().string();

        for (const auto &item : entries.as_array()) {
            if (!item.is_object())
                continue;
            if (item["compiler"].as_string() != compiler_sig)
                continue;
            if (item["flags"].as_string() != compile_flags)
                continue;
            if (item.contains("triple") && item["triple"].as_string() != target_triple)
                continue;

            // Verify header dependencies
            bool all_headers_valid = true;
            auto hdrs = item["headers"];
            if (hdrs.is_array()) {
                for (const auto &h : hdrs.as_array()) {
                    if (!h.is_object())
                        continue;
                    std::string port_path = h["path"].as_string();
                    std::string expected_hash = h["hash"].as_string();

                    fs::path resolved = resolve_portable_path(port_path, src_dir, include_dirs);
                    if (!fs::exists(resolved, ec)) {
                        all_headers_valid = false;
                        break;
                    }
                    std::string actual_hash = hash::Sha256::hash_file(resolved);
                    if (actual_hash != expected_hash) {
                        all_headers_valid = false;
                        break;
                    }
                }
            }

            if (all_headers_valid) {
                matched_key = item["key"].as_string();
                break;
            }
        }

        if (matched_key.empty() || !util::is_safe_hash_key(matched_key)) {
            return false;
        }

        fs::path obj_dir = get_obj_dir();
        fs::path key_prefix_dir = obj_dir / matched_key.substr(0, 2);
        std::string ext = dest_obj_path.extension().string();
        if (!util::is_safe_extension(ext)) {
            ext = ".obj";
        }
        fs::path cached_obj = key_prefix_dir / (matched_key + ext);

        if (!fs::exists(cached_obj, ec)) {
            return false;
        }

        // Restore object file
        if (dest_obj_path.has_parent_path()) {
            fs::create_directories(dest_obj_path.parent_path(), ec);
        }

        fs::copy_file(cached_obj, dest_obj_path, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            return false;
        }

        // Touch timestamp on cached object for LRU
        fs::last_write_time(cached_obj, fs::file_time_type::clock::now(), ec);

        {
            std::lock_guard<std::recursive_mutex> lk(mtx_);
            hits_++;
            direct_hits_++;
            save_stats_locked();
        }

        return true;
    }

    bool store(const std::string &src_path, const std::string &compiler_sig, const std::string &compile_flags,
               const std::string &target_triple, const std::vector<std::string> &dep_headers,
               const std::filesystem::path &compiled_obj_path, const std::vector<std::string> &include_dirs = {}) {
        if (!enabled_.load())
            return false;
        namespace fs = std::filesystem;
        std::error_code ec;

        if (!fs::exists(compiled_obj_path, ec))
            return false;
        if (!fs::exists(src_path, ec))
            return false;

        std::string src_sha256 = hash::Sha256::hash_file(src_path);
        if (src_sha256.empty())
            return false;

        std::string src_dir = fs::path(src_path).parent_path().string();

        std::vector<std::pair<std::string, std::string>> headers;
        headers.reserve(dep_headers.size());

        for (const auto &dep : dep_headers) {
            if (fs::exists(dep, ec)) {
                std::string h_hash = hash::Sha256::hash_file(dep);
                std::string port_path = make_portable_path(dep, src_dir, include_dirs);
                headers.emplace_back(std::move(port_path), std::move(h_hash));
            }
        }

        std::sort(headers.begin(), headers.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

        std::string key = compute_cache_key(src_sha256, compiler_sig, compile_flags, target_triple, headers);

        fs::path obj_dir = get_obj_dir();
        fs::path key_prefix_dir = obj_dir / key.substr(0, 2);
        fs::create_directories(key_prefix_dir, ec);

        std::string ext = compiled_obj_path.extension().string();
        if (!util::is_safe_extension(ext)) {
            ext = ".obj";
        }
        fs::path cached_obj = key_prefix_dir / (key + ext);

        fs::copy_file(compiled_obj_path, cached_obj, fs::copy_options::overwrite_existing, ec);
        if (ec)
            return false;

        fs::last_write_time(cached_obj, fs::file_time_type::clock::now(), ec);

        // Update manifest
        fs::path manifest_dir = get_manifest_dir();
        fs::path manifest_prefix_dir = manifest_dir / src_sha256.substr(0, 2);
        fs::create_directories(manifest_prefix_dir, ec);
        fs::path manifest_file = manifest_prefix_dir / (src_sha256 + ".json");

        std::vector<util::JsonValue> existing_entries;
        if (fs::exists(manifest_file, ec)) {
            std::ifstream mf(manifest_file);
            if (mf.is_open()) {
                std::string mfc((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
                auto mroot = util::JsonValue::parse(mfc);
                if (mroot && mroot->is_object()) {
                    auto arr = (*mroot)["entries"];
                    if (arr.is_array()) {
                        for (const auto &item : arr.as_array()) {
                            if (!item.is_object())
                                continue;
                            if (item["compiler"].as_string() == compiler_sig &&
                                item["flags"].as_string() == compile_flags && item.contains("triple") &&
                                item["triple"].as_string() == target_triple) {
                                continue; // Replace matching configuration
                            }
                            existing_entries.push_back(item);
                        }
                    }
                }
            }
        }

        // Build new entry
        std::ostringstream ss;
        ss << "{\n";
        ss << "  \"version\": 1,\n";
        ss << "  \"entries\": [\n";

        for (const auto &item : existing_entries) {
            ss << "    " << item.serialize(4) << ",\n";
        }

        ss << "    {\n";
        ss << "      \"key\": \"" << util::escape_json(key) << "\",\n";
        ss << "      \"compiler\": \"" << util::escape_json(compiler_sig) << "\",\n";
        ss << "      \"flags\": \"" << util::escape_json(compile_flags) << "\",\n";
        ss << "      \"triple\": \"" << util::escape_json(target_triple) << "\",\n";
        ss << "      \"headers\": [\n";
        for (size_t i = 0; i < headers.size(); ++i) {
            ss << "        {\"path\": \"" << util::escape_json(headers[i].first) << "\", \"hash\": \""
               << headers[i].second << "\"}" << (i + 1 < headers.size() ? "," : "") << "\n";
        }
        ss << "      ]\n";
        ss << "    }\n";
        ss << "  ]\n";
        ss << "}\n";

        std::ofstream out(manifest_file, std::ios::trunc);
        if (out.is_open()) {
            out << ss.str();
        }

        {
            std::lock_guard<std::recursive_mutex> lk(mtx_);
            misses_++;
            files_stored_++;
            save_stats_locked();
        }

        prune_to_max_size();
        return true;
    }

    uint64_t prune_to_max_size() {
        namespace fs = std::filesystem;
        std::lock_guard<std::recursive_mutex> lk(mtx_);
        ensure_config_loaded_locked();
        std::error_code ec;

        fs::path odir = get_obj_dir();
        if (!fs::exists(odir, ec)) {
            return 0;
        }

        struct ObjEntry {
            fs::path path;
            uint64_t size = 0;
            fs::file_time_type mtime;
        };
        std::vector<ObjEntry> entries;
        uint64_t total_bytes = 0;

        for (const auto &sub : fs::directory_iterator(odir, ec)) {
            if (!sub.is_directory())
                continue;
            for (const auto &file : fs::directory_iterator(sub.path(), ec)) {
                if (file.is_regular_file()) {
                    auto sz = file.file_size(ec);
                    auto mt = file.last_write_time(ec);
                    entries.push_back({file.path(), sz, mt});
                    total_bytes += sz;
                }
            }
        }

        if (total_bytes <= max_size_bytes_) {
            return 0;
        }

        // Sort by mtime ascending (oldest first)
        std::sort(entries.begin(), entries.end(),
                  [](const ObjEntry &a, const ObjEntry &b) { return a.mtime < b.mtime; });

        uint64_t target_bytes = static_cast<uint64_t>(max_size_bytes_ * 0.9);
        uint64_t bytes_pruned = 0;

        for (const auto &e : entries) {
            if (total_bytes <= target_bytes) {
                break;
            }
            if (fs::remove(e.path, ec)) {
                total_bytes -= e.size;
                bytes_pruned += e.size;
            }
        }
        reclaimed_bytes_ += bytes_pruned;
        save_stats_locked();
        return bytes_pruned;
    }

    struct CleanResult {
        uint64_t files_removed = 0;
        uint64_t bytes_reclaimed = 0;
    };

    CleanResult clean() {
        namespace fs = std::filesystem;
        std::lock_guard<std::recursive_mutex> lk(mtx_);
        std::error_code ec;
        CleanResult res;

        auto remove_dir_contents = [&](const fs::path &dir) {
            if (!fs::exists(dir, ec))
                return;
            for (const auto &sub : fs::directory_iterator(dir, ec)) {
                if (sub.is_directory()) {
                    for (const auto &f : fs::directory_iterator(sub.path(), ec)) {
                        if (f.is_regular_file()) {
                            res.bytes_reclaimed += f.file_size(ec);
                            res.files_removed++;
                        }
                    }
                    fs::remove_all(sub.path(), ec);
                } else if (sub.is_regular_file()) {
                    res.bytes_reclaimed += sub.file_size(ec);
                    res.files_removed++;
                    fs::remove(sub.path(), ec);
                }
            }
        };

        remove_dir_contents(get_obj_dir());
        remove_dir_contents(get_manifest_dir());

        reclaimed_bytes_ += res.bytes_reclaimed;
        save_stats_locked();
        return res;
    }

    CacheStats get_stats() const {
        namespace fs = std::filesystem;
        std::lock_guard<std::recursive_mutex> lk(mtx_);
        ensure_config_loaded_locked();
        ensure_stats_loaded_locked();

        CacheStats s;
        s.hits = hits_;
        s.direct_hits = direct_hits_;
        s.misses = misses_;
        s.files_stored = files_stored_;
        s.max_bytes = max_size_bytes_;
        s.reclaimed_bytes = reclaimed_bytes_;

        std::error_code ec;
        fs::path odir = get_obj_dir();
        if (fs::exists(odir, ec)) {
            for (const auto &sub : fs::directory_iterator(odir, ec)) {
                if (!sub.is_directory())
                    continue;
                for (const auto &file : fs::directory_iterator(sub.path(), ec)) {
                    if (file.is_regular_file()) {
                        s.total_files++;
                        s.total_bytes += file.file_size(ec);
                    }
                }
            }
        }
        return s;
    }

    void reset_session_stats() {
        std::lock_guard<std::recursive_mutex> lk(mtx_);
        hits_ = 0;
        direct_hits_ = 0;
        misses_ = 0;
        files_stored_ = 0;
        reclaimed_bytes_ = 0;
    }

    int run_cli_cache(const CliArgs &args);

private:
    GlobalObjectCache() {
        const char *env_max = std::getenv("CXXGEN_CACHE_MAX_SIZE");
        if (env_max && *env_max) {
            uint64_t sz = parse_size(env_max);
            if (sz > 0)
                max_size_bytes_ = sz;
        }
    }

    void ensure_config_loaded_locked() const {
        if (loaded_config_)
            return;
        loaded_config_ = true;
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path cfg_file = get_config_path();
        if (!fs::exists(cfg_file, ec))
            return;

        std::ifstream f(cfg_file);
        if (!f.is_open())
            return;
        std::string s((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        auto root = util::JsonValue::parse(s);
        if (root && root->is_object()) {
            if (root->contains("max_size_bytes") && (*root)["max_size_bytes"].is_number()) {
                max_size_bytes_ = static_cast<uint64_t>((*root)["max_size_bytes"].as_int64());
            }
        }
    }

    void save_config_locked() const {
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path p = get_config_path();
        if (p.has_parent_path()) {
            fs::create_directories(p.parent_path(), ec);
        }
        std::ofstream f(p, std::ios::trunc);
        if (f.is_open()) {
            f << "{\n  \"max_size_bytes\": " << max_size_bytes_ << "\n}\n";
        }
    }

    void ensure_stats_loaded_locked() const {
        if (loaded_stats_)
            return;
        loaded_stats_ = true;
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path s_file = get_stats_path();
        if (!fs::exists(s_file, ec))
            return;

        std::ifstream f(s_file);
        if (!f.is_open())
            return;
        std::string s((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        auto root = util::JsonValue::parse(s);
        if (root && root->is_object()) {
            if (root->contains("hits"))
                hits_ = static_cast<uint64_t>((*root)["hits"].as_int64());
            if (root->contains("direct_hits"))
                direct_hits_ = static_cast<uint64_t>((*root)["direct_hits"].as_int64());
            if (root->contains("misses"))
                misses_ = static_cast<uint64_t>((*root)["misses"].as_int64());
            if (root->contains("files_stored"))
                files_stored_ = static_cast<uint64_t>((*root)["files_stored"].as_int64());
            if (root->contains("reclaimed_bytes"))
                reclaimed_bytes_ = static_cast<uint64_t>((*root)["reclaimed_bytes"].as_int64());
        }
    }

    void save_stats_locked() const {
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path p = get_stats_path();
        if (p.has_parent_path()) {
            fs::create_directories(p.parent_path(), ec);
        }
        std::ofstream f(p, std::ios::trunc);
        if (f.is_open()) {
            f << "{\n"
              << "  \"hits\": " << hits_ << ",\n"
              << "  \"direct_hits\": " << direct_hits_ << ",\n"
              << "  \"misses\": " << misses_ << ",\n"
              << "  \"files_stored\": " << files_stored_ << ",\n"
              << "  \"reclaimed_bytes\": " << reclaimed_bytes_ << "\n"
              << "}\n";
        }
    }

    mutable std::recursive_mutex mtx_;
    std::filesystem::path cache_dir_;
    std::atomic<bool> enabled_{true};
    mutable uint64_t max_size_bytes_ = 5ULL * 1024 * 1024 * 1024; // 5 GB
    mutable uint64_t hits_ = 0;
    mutable uint64_t direct_hits_ = 0;
    mutable uint64_t misses_ = 0;
    mutable uint64_t files_stored_ = 0;
    mutable uint64_t reclaimed_bytes_ = 0;
    mutable bool loaded_config_ = false;
    mutable bool loaded_stats_ = false;
};

// ============================================================================
// C++20 / C++23 Modern Modules Architecture (Epic 12)
// ============================================================================

namespace modules {

struct ModuleProvide {
    std::string logical_name;
    std::string source_path;
    bool is_interface = true;
};

struct ModuleRequire {
    std::string logical_name;
    std::string source_path;
};

struct P1689Rule {
    std::string primary_output;
    std::vector<std::string> outputs;
    std::vector<ModuleProvide> provides;
    std::vector<ModuleRequire> required_modules;
};

struct P1689Data {
    int version = 1;
    int revision = 0;
    std::vector<P1689Rule> rules;

    [[nodiscard]] std::string to_json() const {
        std::map<std::string, util::JsonValue> root;
        root["version"] = util::JsonValue(static_cast<int64_t>(version));
        root["revision"] = util::JsonValue(static_cast<int64_t>(revision));
        std::vector<util::JsonValue> rule_arr;
        for (const auto &r : rules) {
            std::map<std::string, util::JsonValue> rm;
            if (!r.primary_output.empty())
                rm["primary-output"] = util::JsonValue(r.primary_output);
            if (!r.outputs.empty()) {
                std::vector<util::JsonValue> outs;
                for (const auto &o : r.outputs)
                    outs.emplace_back(o);
                rm["outputs"] = util::JsonValue(std::move(outs));
            }
            if (!r.provides.empty()) {
                std::vector<util::JsonValue> provs;
                for (const auto &p : r.provides) {
                    std::map<std::string, util::JsonValue> pm;
                    pm["logical-name"] = util::JsonValue(p.logical_name);
                    if (!p.source_path.empty())
                        pm["source-path"] = util::JsonValue(p.source_path);
                    pm["is-interface"] = util::JsonValue(p.is_interface);
                    provs.emplace_back(std::move(pm));
                }
                rm["provides"] = util::JsonValue(std::move(provs));
            }
            if (!r.required_modules.empty()) {
                std::vector<util::JsonValue> reqs;
                for (const auto &rq : r.required_modules) {
                    std::map<std::string, util::JsonValue> rqm;
                    rqm["logical-name"] = util::JsonValue(rq.logical_name);
                    if (!rq.source_path.empty())
                        rqm["source-path"] = util::JsonValue(rq.source_path);
                    reqs.emplace_back(std::move(rqm));
                }
                rm["requires"] = util::JsonValue(std::move(reqs));
            }
            rule_arr.emplace_back(std::move(rm));
        }
        root["rules"] = util::JsonValue(std::move(rule_arr));
        return util::JsonValue(std::move(root)).serialize(2);
    }

    static P1689Data from_json(std::string_view json_str) {
        P1689Data data;
        auto opt = util::JsonValue::parse(json_str);
        if (!opt || !opt->is_object())
            return data;
        const auto &root = *opt;
        if (root.contains("version"))
            data.version = static_cast<int>(root["version"].as_int64());
        if (root.contains("revision"))
            data.revision = static_cast<int>(root["revision"].as_int64());
        if (root.contains("rules") && root["rules"].is_array()) {
            for (const auto &rv : root["rules"].as_array()) {
                if (!rv.is_object())
                    continue;
                P1689Rule rule;
                if (rv.contains("primary-output"))
                    rule.primary_output = rv["primary-output"].as_string();
                if (rv.contains("outputs") && rv["outputs"].is_array()) {
                    for (const auto &ov : rv["outputs"].as_array()) {
                        if (ov.is_string())
                            rule.outputs.push_back(ov.as_string());
                    }
                }
                if (rv.contains("provides") && rv["provides"].is_array()) {
                    for (const auto &pv : rv["provides"].as_array()) {
                        if (!pv.is_object())
                            continue;
                        ModuleProvide mp;
                        if (pv.contains("logical-name"))
                            mp.logical_name = pv["logical-name"].as_string();
                        if (pv.contains("source-path"))
                            mp.source_path = pv["source-path"].as_string();
                        if (pv.contains("is-interface"))
                            mp.is_interface = pv["is-interface"].as_bool();
                        rule.provides.push_back(std::move(mp));
                    }
                }
                if (rv.contains("requires") && rv["requires"].is_array()) {
                    for (const auto &rqv : rv["requires"].as_array()) {
                        if (!rqv.is_object())
                            continue;
                        ModuleRequire mr;
                        if (rqv.contains("logical-name"))
                            mr.logical_name = rqv["logical-name"].as_string();
                        if (rqv.contains("source-path"))
                            mr.source_path = rqv["source-path"].as_string();
                        rule.required_modules.push_back(std::move(mr));
                    }
                }
                data.rules.push_back(std::move(rule));
            }
        }
        return data;
    }
};

struct ModuleDependencyGraph {
    bool has_modules = false;
    bool requires_std = false;
    bool has_cycle = false;
    std::vector<std::string> cycle_path;
    std::vector<std::string> sorted_interfaces;
    std::vector<std::string> non_interface_sources;
    std::unordered_map<std::string, std::string> source_to_module;
    std::unordered_map<std::string, std::string> module_to_source;
    std::unordered_map<std::string, std::vector<std::string>> module_dependencies;
    P1689Data p1689;
};

class ModuleScanner {
public:
    static std::string sanitize_module_name(std::string_view name) {
        std::string res;
        res.reserve(name.size());
        for (char c : name) {
            if (c == '.' || c == ':' || c == '/' || c == '\\' || c == ' ') {
                res.push_back('_');
            } else {
                res.push_back(c);
            }
        }
        return res;
    }

    static std::string strip_comments_and_strings(std::string_view code) {
        std::string result;
        result.reserve(code.size());
        size_t i = 0;
        const size_t n = code.size();
        while (i < n) {
            if (code[i] == '/' && i + 1 < n && code[i + 1] == '/') {
                result.push_back(' ');
                result.push_back(' ');
                i += 2;
                while (i < n && code[i] != '\n') {
                    result.push_back(' ');
                    ++i;
                }
                if (i < n && code[i] == '\n') {
                    result.push_back('\n');
                    ++i;
                }
                continue;
            }
            if (code[i] == '/' && i + 1 < n && code[i + 1] == '*') {
                result.push_back(' ');
                result.push_back(' ');
                i += 2;
                while (i + 1 < n && !(code[i] == '*' && code[i + 1] == '/')) {
                    result.push_back(code[i] == '\n' ? '\n' : ' ');
                    ++i;
                }
                if (i + 1 < n) {
                    result.push_back(' ');
                    result.push_back(' ');
                    i += 2;
                }
                continue;
            }
            if (code[i] == 'R' && i + 1 < n && code[i + 1] == '"') {
                size_t start = i;
                i += 2;
                std::string delim;
                while (i < n && code[i] != '(' && code[i] != '"') {
                    delim.push_back(code[i]);
                    ++i;
                }
                if (i < n && code[i] == '(') {
                    ++i;
                    std::string closing = ")" + delim + "\"";
                    size_t close_pos = code.find(closing, i);
                    if (close_pos != std::string_view::npos) {
                        i = close_pos + closing.size();
                    }
                }
                for (size_t k = start; k < i; ++k) {
                    result.push_back(code[k] == '\n' ? '\n' : ' ');
                }
                continue;
            }
            if (code[i] == '"') {
                // Check if preceded by 'import' keyword (ignoring whitespace)
                bool after_import = false;
                if (!result.empty()) {
                    size_t p = result.size();
                    while (p > 0 && (result[p - 1] == ' ' || result[p - 1] == '\t' || result[p - 1] == '\r' ||
                                     result[p - 1] == '\n')) {
                        --p;
                    }
                    if (p >= 6 && result.substr(p - 6, 6) == "import") {
                        if (p == 6 ||
                            (!std::isalnum(static_cast<unsigned char>(result[p - 7])) && result[p - 7] != '_')) {
                            after_import = true;
                        }
                    }
                }
                if (after_import) {
                    // Preserve header string literal: "..."
                    result.push_back(code[i]);
                    ++i;
                    while (i < n && code[i] != '"') {
                        result.push_back(code[i]);
                        if (code[i] == '\\' && i + 1 < n) {
                            ++i;
                            result.push_back(code[i]);
                        }
                        ++i;
                    }
                    if (i < n && code[i] == '"') {
                        result.push_back(code[i]);
                        ++i;
                    }
                    continue;
                }
                result.push_back(' ');
                ++i;
                while (i < n && code[i] != '"') {
                    if (code[i] == '\\' && i + 1 < n) {
                        result.push_back(' ');
                        result.push_back(' ');
                        i += 2;
                    } else {
                        result.push_back(code[i] == '\n' ? '\n' : ' ');
                        ++i;
                    }
                }
                if (i < n && code[i] == '"') {
                    result.push_back(' ');
                    ++i;
                }
                continue;
            }
            if (code[i] == '\'') {
                result.push_back(' ');
                ++i;
                while (i < n && code[i] != '\'') {
                    if (code[i] == '\\' && i + 1 < n) {
                        result.push_back(' ');
                        result.push_back(' ');
                        i += 2;
                    } else {
                        result.push_back(' ');
                        ++i;
                    }
                }
                if (i < n && code[i] == '\'') {
                    result.push_back(' ');
                    ++i;
                }
                continue;
            }
            result.push_back(code[i]);
            ++i;
        }
        return result;
    }

    static P1689Rule scan_source_text(std::string_view text, const std::string &source_path = "") {
        P1689Rule rule;
        rule.primary_output = source_path.empty() ? "" : (source_path + ".o");
        std::string clean = strip_comments_and_strings(text);

        // 1. Export module (module interface)
        std::regex re_export_mod(R"(\bexport\s+module\s+([a-zA-Z0-9_\.:]+)\s*;)");
        std::smatch m;
        if (std::regex_search(clean, m, re_export_mod)) {
            ModuleProvide mp;
            mp.logical_name = m[1].str();
            mp.source_path = source_path;
            mp.is_interface = true;
            rule.provides.push_back(std::move(mp));
        } else {
            // Module implementation unit (module <name>; without export)
            std::regex re_impl_mod(R"(^\s*module\s+([a-zA-Z0-9_\.:]+)\s*;)");
            size_t start = 0;
            while (start < clean.size()) {
                size_t nl = clean.find('\n', start);
                if (nl == std::string::npos)
                    nl = clean.size();
                std::string line = clean.substr(start, nl - start);
                if (std::regex_search(line, m, re_impl_mod)) {
                    ModuleProvide mp;
                    mp.logical_name = m[1].str();
                    mp.source_path = source_path;
                    mp.is_interface = false;
                    rule.provides.push_back(std::move(mp));
                    break;
                }
                start = nl + 1;
            }
        }

        // 2. Export import <name>;
        std::regex re_exp_imp(R"(\bexport\s+import\s+([a-zA-Z0-9_\.:]+)\s*;)");
        auto b_exp_imp = std::sregex_iterator(clean.begin(), clean.end(), re_exp_imp);
        auto e_exp_imp = std::sregex_iterator();
        std::unordered_set<std::string> seen_requires;
        for (auto it = b_exp_imp; it != e_exp_imp; ++it) {
            std::string req = (*it)[1].str();
            if (seen_requires.insert(req).second) {
                rule.required_modules.push_back({.logical_name = req, .source_path = ""});
            }
        }

        // 3. Regular import <name>;
        std::regex re_imp(R"(\bimport\s+([a-zA-Z0-9_\.:]+)\s*;)");
        auto b_imp = std::sregex_iterator(clean.begin(), clean.end(), re_imp);
        auto e_imp = std::sregex_iterator();
        for (auto it = b_imp; it != e_imp; ++it) {
            std::string req = (*it)[1].str();
            if (seen_requires.insert(req).second) {
                rule.required_modules.push_back({.logical_name = req, .source_path = ""});
            }
        }

        // 4. Header unit import: import <header>; or import "header";
        std::regex re_hdr(R"(\bimport\s*([<"][^>"]+[>"])\s*;)");
        auto b_hdr = std::sregex_iterator(clean.begin(), clean.end(), re_hdr);
        auto e_hdr = std::sregex_iterator();
        for (auto it = b_hdr; it != e_hdr; ++it) {
            std::string req = (*it)[1].str();
            if (seen_requires.insert(req).second) {
                rule.required_modules.push_back({.logical_name = req, .source_path = ""});
            }
        }

        // 5. Partition import: import :partition;
        std::regex re_part(R"(\bimport\s*:([a-zA-Z0-9_]+)\s*;)");
        auto b_part = std::sregex_iterator(clean.begin(), clean.end(), re_part);
        auto e_part = std::sregex_iterator();
        for (auto it = b_part; it != e_part; ++it) {
            std::string req = ":" + (*it)[1].str();
            if (seen_requires.insert(req).second) {
                rule.required_modules.push_back({.logical_name = req, .source_path = ""});
            }
        }

        return rule;
    }

    static std::optional<P1689Rule> scan_vendor(const std::filesystem::path &source_path, const IToolchain &toolchain,
                                                const std::vector<std::string> &extra_flags = {}) {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (!fs::exists(source_path, ec))
            return std::nullopt;

        if (toolchain.family() == CompilerFamily::Clang) {
            fs::path scan_deps = "clang-scan-deps";
            fs::path cl_path = toolchain.compiler_path();
            if (fs::exists(cl_path, ec)) {
                fs::path alongside = cl_path.parent_path() / "clang-scan-deps.exe";
                if (fs::exists(alongside, ec))
                    scan_deps = alongside;
                else {
                    alongside = cl_path.parent_path() / "clang-scan-deps";
                    if (fs::exists(alongside, ec))
                        scan_deps = alongside;
                }
            }
            std::vector<std::string> cmd;
            cmd.push_back(scan_deps.string());
            cmd.push_back("-format=p1689");
            cmd.push_back("--");
            bool has_std = false;
            for (const auto &f : extra_flags) {
                if (f.rfind("-std=", 0) == 0) {
                    has_std = true;
                    break;
                }
            }
            if (!has_std) {
                cmd.push_back("-std=c++20");
            }
            for (const auto &f : extra_flags)
                cmd.push_back(f);
            cmd.push_back("-c");
            cmd.push_back(util::normalize_path(source_path.string(), false));

            auto res = ProcessRunner::run(cmd, {}, toolchain.env_vars());
            if (res.success() && !res.stdout_output.empty()) {
                auto p1689 = P1689Data::from_json(res.stdout_output);
                if (!p1689.rules.empty()) {
                    return p1689.rules.front();
                }
            }
        } else if (toolchain.family() == CompilerFamily::Msvc) {
            fs::path temp_json =
                fs::temp_directory_path() /
                ("cxxgen_scan_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) +
                 ".json");
            std::vector<std::string> cmd;
            cmd.push_back(toolchain.compiler_path());
            cmd.push_back("/nologo");
            bool has_std = false;
            for (const auto &f : extra_flags) {
                if (f.rfind("/std:", 0) == 0) {
                    has_std = true;
                    break;
                }
            }
            if (!has_std) {
                cmd.push_back("/std:c++20");
            }
            for (const auto &f : extra_flags)
                cmd.push_back(f);
            cmd.push_back("/scanDependencies");
            cmd.push_back(util::normalize_path(temp_json.string()));
            cmd.push_back("/c");
            cmd.push_back(util::normalize_path(source_path.string()));

            auto res = ProcessRunner::run(cmd, {}, toolchain.env_vars());
            if (fs::exists(temp_json, ec)) {
                std::string json_str = util::read_file_text(temp_json);
                fs::remove(temp_json, ec);
                auto p1689 = P1689Data::from_json(json_str);
                if (!p1689.rules.empty()) {
                    return p1689.rules.front();
                }
            }
        }
        return std::nullopt;
    }

    static P1689Rule scan_file(const std::filesystem::path &source_path, const IToolchain *toolchain = nullptr,
                               const std::vector<std::string> &extra_flags = {}, bool force_builtin = false) {
        if (!force_builtin && toolchain != nullptr) {
            auto vendor_opt = scan_vendor(source_path, *toolchain, extra_flags);
            if (vendor_opt)
                return *vendor_opt;
        }
        std::string content = util::read_file_text(source_path);
        return scan_source_text(content, source_path.string());
    }

    static ModuleDependencyGraph scan_target(const Target &target, const IToolchain *toolchain = nullptr) {
        namespace fs = std::filesystem;
        ModuleDependencyGraph graph;
        std::unordered_map<std::string, std::vector<std::string>> adj;
        std::unordered_map<std::string, int> in_degree;
        std::vector<std::string> interface_modules;

        std::vector<std::string> scan_flags;
        if (target.config().cpp_version == CppVersion::Cpp23) {
            scan_flags.push_back("-std=c++23");
        } else if (target.config().cpp_version == CppVersion::Cpp26) {
            scan_flags.push_back("-std=c++26");
        }
        for (const auto &inc : target.config().include_dirs) {
            scan_flags.push_back("-I" + util::normalize_path(inc, false));
        }
        for (const auto &def : target.config().defines) {
            scan_flags.push_back("-D" + def);
        }

        for (const auto &src : target.config().sources) {
            P1689Rule rule = scan_file(src, toolchain, scan_flags);
            graph.p1689.rules.push_back(rule);

            bool is_interface = false;
            std::string mod_name;
            for (const auto &p : rule.provides) {
                graph.has_modules = true;
                graph.source_to_module[src] = p.logical_name;
                graph.module_to_source[p.logical_name] = src;
                mod_name = p.logical_name;
                if (p.is_interface) {
                    is_interface = true;
                    interface_modules.push_back(p.logical_name);
                    in_degree[p.logical_name] = 0;
                }
            }

            if (!rule.required_modules.empty()) {
                graph.has_modules = true;
            }

            for (const auto &req : rule.required_modules) {
                if (req.logical_name == "std" || req.logical_name == "std.compat") {
                    graph.requires_std = true;
                }
                if (is_interface) {
                    std::string req_name = req.logical_name;
                    if (!req_name.empty() && req_name.front() == ':') {
                        size_t cpos = mod_name.find(':');
                        std::string primary = (cpos != std::string::npos) ? mod_name.substr(0, cpos) : mod_name;
                        req_name = primary + req_name;
                    }
                    graph.module_dependencies[mod_name].push_back(req_name);
                }
            }

            if (!is_interface) {
                graph.non_interface_sources.push_back(src);
            }
        }

        // Build adjacency and in-degree for interface modules
        for (const auto &m : interface_modules) {
            for (const auto &req_m : graph.module_dependencies[m]) {
                if (graph.module_to_source.find(req_m) != graph.module_to_source.end()) {
                    adj[req_m].push_back(m);
                    in_degree[m]++;
                }
            }
        }

        // Topological Sort (Kahn's Algorithm)
        std::queue<std::string> q;
        for (const auto &m : interface_modules) {
            if (in_degree[m] == 0) {
                q.push(m);
            }
        }

        std::vector<std::string> sorted_mods;
        while (!q.empty()) {
            std::string u = q.front();
            q.pop();
            sorted_mods.push_back(u);
            graph.sorted_interfaces.push_back(graph.module_to_source[u]);

            for (const auto &v : adj[u]) {
                in_degree[v]--;
                if (in_degree[v] == 0) {
                    q.push(v);
                }
            }
        }

        if (sorted_mods.size() < interface_modules.size()) {
            graph.has_cycle = true;
            for (const auto &m : interface_modules) {
                if (in_degree[m] > 0) {
                    graph.cycle_path.push_back(m);
                }
            }
            if (!graph.cycle_path.empty()) {
                graph.cycle_path.push_back(graph.cycle_path.front());
            }
        }

        return graph;
    }
};

class BmiCache {
public:
    static BmiCache &instance() {
        static BmiCache s_inst;
        return s_inst;
    }

    static std::string compute_key(const std::filesystem::path &source_path, std::string_view compiler_sig,
                                   const std::vector<std::string> &compile_flags,
                                   const std::vector<std::string> &dep_hashes = {}) {
        std::string content = util::read_file_text(source_path);
        std::string raw = content;
        raw += "|sig=";
        raw += compiler_sig;
        raw += "|flags=";
        raw += util::join(compile_flags, " ");
        for (const auto &dh : dep_hashes) {
            raw += "|dep=";
            raw += dh;
        }
        return hash::Sha256::hash(raw);
    }

    std::filesystem::path get_cache_dir() const {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        if (!custom_cache_dir_.empty())
            return custom_cache_dir_;
        const char *env_p = std::getenv("CXXGEN_CACHE_DIR");
        if (env_p && *env_p) {
            return std::filesystem::path(env_p) / "bmi";
        }
        const char *user_prof = std::getenv("USERPROFILE");
        if (user_prof && *user_prof) {
            return std::filesystem::path(user_prof) / ".cxxgen/cache/bmi";
        }
        const char *home = std::getenv("HOME");
        if (home && *home) {
            return std::filesystem::path(home) / ".cxxgen/cache/bmi";
        }
        return std::filesystem::current_path() / ".cxxgen/cache/bmi";
    }

    void set_cache_dir(std::filesystem::path dir) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        custom_cache_dir_ = std::move(dir);
    }

    bool try_restore(const std::string &cache_key, const std::filesystem::path &out_bmi,
                     const std::filesystem::path &out_obj) {
        if (!util::is_safe_hash_key(cache_key))
            return false;
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path entry_dir = get_cache_dir() / cache_key;
        if (!fs::exists(entry_dir, ec)) {
            ++misses_;
            return false;
        }

        std::string bmi_ext = out_bmi.extension().string();
        if (!util::is_safe_extension(bmi_ext))
            bmi_ext = ".bmi";
        std::string obj_ext = out_obj.extension().string();
        if (!util::is_safe_extension(obj_ext))
            obj_ext = ".obj";

        fs::path cached_bmi = entry_dir / ("bmi" + bmi_ext);
        fs::path cached_obj = entry_dir / ("obj" + obj_ext);

        if (fs::exists(cached_bmi, ec) && fs::exists(cached_obj, ec)) {
            if (out_bmi.has_parent_path())
                fs::create_directories(out_bmi.parent_path(), ec);
            if (out_obj.has_parent_path())
                fs::create_directories(out_obj.parent_path(), ec);
            fs::copy_file(cached_bmi, out_bmi, fs::copy_options::overwrite_existing, ec);
            fs::copy_file(cached_obj, out_obj, fs::copy_options::overwrite_existing, ec);
            if (!ec) {
                ++hits_;
                return true;
            }
        }
        ++misses_;
        return false;
    }

    void store(const std::string &cache_key, const std::filesystem::path &in_bmi, const std::filesystem::path &in_obj) {
        if (!util::is_safe_hash_key(cache_key))
            return;
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path entry_dir = get_cache_dir() / cache_key;
        fs::create_directories(entry_dir, ec);

        std::string bmi_ext = in_bmi.extension().string();
        if (!util::is_safe_extension(bmi_ext))
            bmi_ext = ".bmi";
        std::string obj_ext = in_obj.extension().string();
        if (!util::is_safe_extension(obj_ext))
            obj_ext = ".obj";

        fs::path cached_bmi = entry_dir / ("bmi" + bmi_ext);
        fs::path cached_obj = entry_dir / ("obj" + obj_ext);

        if (fs::exists(in_bmi, ec)) {
            fs::copy_file(in_bmi, cached_bmi, fs::copy_options::overwrite_existing, ec);
        }
        if (fs::exists(in_obj, ec)) {
            fs::copy_file(in_obj, cached_obj, fs::copy_options::overwrite_existing, ec);
        }
    }

    void clean() {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        std::error_code ec;
        std::filesystem::remove_all(get_cache_dir(), ec);
        hits_ = 0;
        misses_ = 0;
    }

    uint64_t hits() const noexcept { return hits_.load(); }
    uint64_t misses() const noexcept { return misses_.load(); }

private:
    mutable std::recursive_mutex mtx_;
    std::filesystem::path custom_cache_dir_;
    std::atomic<uint64_t> hits_{0};
    std::atomic<uint64_t> misses_{0};
};

class StdModuleHelper {
public:
    static std::filesystem::path find_std_module_source(const IToolchain &toolchain) {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (toolchain.family() == CompilerFamily::Msvc) {
            const char *vc_tools = std::getenv("VCToolsInstallDir");
            if (vc_tools && *vc_tools) {
                fs::path p = fs::path(vc_tools) / "modules" / "std.ixx";
                if (fs::exists(p, ec))
                    return p;
            }
            fs::path cl_p = toolchain.compiler_path();
            if (fs::exists(cl_p, ec)) {
                fs::path cur = cl_p;
                for (int i = 0; i < 4; ++i) {
                    if (cur.has_parent_path())
                        cur = cur.parent_path();
                }
                fs::path candidate = cur / "modules" / "std.ixx";
                if (fs::exists(candidate, ec))
                    return candidate;
            }
            std::vector<fs::path> candidates = {
                "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC",
                "C:/Program Files/Microsoft Visual Studio/2022/Professional/VC/Tools/MSVC",
                "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC",
                "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC"};
            for (const auto &base : candidates) {
                if (fs::exists(base, ec)) {
                    for (const auto &entry : fs::directory_iterator(base, ec)) {
                        if (entry.is_directory()) {
                            fs::path mod_p = entry.path() / "modules" / "std.ixx";
                            if (fs::exists(mod_p, ec))
                                return mod_p;
                        }
                    }
                }
            }
        } else if (toolchain.family() == CompilerFamily::Clang) {
#if defined(_WIN32)
            const char *vc_tools = std::getenv("VCToolsInstallDir");
            if (vc_tools && *vc_tools) {
                fs::path p = fs::path(vc_tools) / "modules" / "std.ixx";
                if (fs::exists(p, ec))
                    return p;
            }
            fs::path msvc_def = "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC";
            if (fs::exists(msvc_def, ec)) {
                for (const auto &entry : fs::directory_iterator(msvc_def, ec)) {
                    if (entry.is_directory()) {
                        fs::path mod_p = entry.path() / "modules" / "std.ixx";
                        if (fs::exists(mod_p, ec))
                            return mod_p;
                    }
                }
            }
#endif
            fs::path clang_p = toolchain.compiler_path();
            if (fs::exists(clang_p, ec)) {
                fs::path cand = clang_p.parent_path().parent_path() / "include/c++/v1/std.cppm";
                if (fs::exists(cand, ec))
                    return cand;
            }
        }
        return {};
    }

    static bool ensure_std_module(const Builder &builder, const Target &target, const IToolchain &toolchain,
                                  std::filesystem::path &out_ifc, std::filesystem::path &out_obj,
                                  std::string &error_msg) {
        (void)builder;
        (void)target;
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path std_src = find_std_module_source(toolchain);
        if (std_src.empty()) {
            error_msg = "Could not locate standard library module source (std.ixx or std.cppm) for toolchain " +
                        toolchain.name();
            return false;
        }

        std::string comp_hash = hash::Sha256::hash(toolchain.compiler_path() + "_" + toolchain.name());
        fs::path cache_std_dir = BmiCache::instance().get_cache_dir() / "std" / comp_hash;
        fs::create_directories(cache_std_dir, ec);

        std::string ifc_ext = (toolchain.family() == CompilerFamily::Msvc) ? ".ifc" : ".pcm";
        std::string obj_ext = (toolchain.family() == CompilerFamily::Msvc) ? ".obj" : ".o";
        fs::path cached_ifc = cache_std_dir / ("std" + ifc_ext);
        fs::path cached_obj = cache_std_dir / ("std" + obj_ext);

        if (fs::exists(cached_ifc, ec) && fs::exists(cached_obj, ec)) {
            auto src_time = fs::last_write_time(std_src, ec);
            auto ifc_time = fs::last_write_time(cached_ifc, ec);
            auto obj_time = fs::last_write_time(cached_obj, ec);
            if (!ec && ifc_time >= src_time && obj_time >= src_time && fs::file_size(cached_ifc, ec) > 0) {
                out_ifc = cached_ifc;
                out_obj = cached_obj;
                return true;
            }
        }

        if (toolchain.family() == CompilerFamily::Msvc) {
            std::vector<std::string> cmd;
            cmd.push_back(toolchain.compiler_path());
            cmd.push_back("/nologo");
            cmd.push_back("/std:c++latest");
            cmd.push_back("/EHsc");
            cmd.push_back("/utf-8");
            cmd.push_back("/interface");
            cmd.push_back("/c");
            cmd.push_back(util::normalize_path(std_src.string()));
            cmd.push_back("/ifcOutput");
            cmd.push_back(util::normalize_path(cached_ifc.string()));
            cmd.push_back("/Fo" + util::normalize_path(cached_obj.string()));

            auto res = ProcessRunner::run(cmd, {}, toolchain.env_vars());
            if (!res.success()) {
                error_msg = "Compilation of MSVC std.ixx failed:\n" + res.combined_output();
                return false;
            }
        } else if (toolchain.family() == CompilerFamily::Clang) {
            std::vector<std::string> pre_cmd = {toolchain.compiler_path(),
                                                "-std=c++23",
                                                "-x",
                                                "c++-module",
                                                "--precompile",
                                                util::normalize_path(std_src.string(), false),
                                                "-o",
                                                util::normalize_path(cached_ifc.string(), false)};
            auto res1 = ProcessRunner::run(pre_cmd, {}, toolchain.env_vars());
            if (!res1.success()) {
                error_msg = "Precompilation of Clang std module failed:\n" + res1.combined_output();
                return false;
            }
            std::vector<std::string> obj_cmd = {toolchain.compiler_path(),
                                                "-c",
                                                "-x",
                                                "pcm",
                                                util::normalize_path(cached_ifc.string(), false),
                                                "-o",
                                                util::normalize_path(cached_obj.string(), false)};
            auto res2 = ProcessRunner::run(obj_cmd, {}, toolchain.env_vars());
            if (!res2.success()) {
                error_msg = "Object compilation of Clang std module failed:\n" + res2.combined_output();
                return false;
            }
        }

        out_ifc = cached_ifc;
        out_obj = cached_obj;
        return true;
    }
};

} // namespace modules

class CompilationPipeline {
public:
    static std::string sanitize_source_for_object(std::string_view src) {
        std::string s;
        s.reserve(src.size());
        for (char c : src) {
            if (c == '/' || c == '\\' || c == ':' || c == '.' || c == ' ') {
                s.push_back('_');
            } else {
                s.push_back(c);
            }
        }
        return s;
    }

    static std::filesystem::path get_artifact_path(const Target &target, const IToolchain &toolchain) {
        namespace fs = std::filesystem;
        std::string dir_str = target.config().output_dir;
        if (dir_str.empty()) {
            switch (target.type()) {
            case TargetType::Executable:
                dir_str = "bin";
                break;
            case TargetType::Test:
                dir_str = "bin/tests";
                break;
            case TargetType::StaticLibrary:
                dir_str = "lib";
                break;
            case TargetType::SharedLibrary:
                dir_str = "bin";
                break;
            case TargetType::Plugin:
                dir_str = "bin/plugins";
                break;
            case TargetType::Interface:
                return {};
            }
        }

        std::string base_name = target.config().output_name.empty() ? target.name() : target.config().output_name;

        // Check if base_name already contains an extension
        if (base_name.find('.') != std::string::npos) {
            return fs::path(dir_str) / base_name;
        }

        TargetProfile profile = TargetProfile::parse(target.config().target.triple);
        bool is_wasm_target = (toolchain.family() == CompilerFamily::Emscripten) || profile.is_wasm();

        std::string filename;
        switch (target.type()) {
        case TargetType::Executable:
        case TargetType::Test:
            if (is_wasm_target) {
                if (toolchain.family() == CompilerFamily::Emscripten) {
                    filename = base_name + ".html";
                } else {
                    filename = base_name + ".wasm";
                }
            } else if (profile.is_windows()) {
                filename = base_name + ".exe";
            } else if (profile.is_linux() || profile.is_apple() || profile.is_bare_metal()) {
                filename = base_name;
            } else {
#if defined(_WIN32)
                filename = base_name + ".exe";
#else
                filename = base_name;
#endif
            }
            break;
        case TargetType::StaticLibrary:
            if (toolchain.family() == CompilerFamily::Msvc && !profile.is_linux() && !profile.is_wasm()) {
                filename = base_name + ".lib";
            } else {
                if (base_name.rfind("lib", 0) == 0) {
                    filename = base_name + ".a";
                } else {
                    filename = "lib" + base_name + ".a";
                }
            }
            break;
        case TargetType::SharedLibrary:
            if (profile.is_windows()) {
                filename = base_name + ".dll";
            } else if (profile.is_apple()) {
                filename = "lib" + base_name + ".dylib";
            } else if (profile.is_linux() || is_wasm_target) {
                filename = "lib" + base_name + ".so";
            } else {
#if defined(_WIN32)
                filename = base_name + ".dll";
#elif defined(__APPLE__)
                filename = "lib" + base_name + ".dylib";
#else
                filename = "lib" + base_name + ".so";
#endif
            }
            break;
        case TargetType::Plugin:
            if (profile.is_windows()) {
                filename = base_name + ".dll";
            } else if (profile.is_apple()) {
                filename = base_name + ".dylib";
            } else if (profile.is_linux() || is_wasm_target) {
                filename = base_name + ".so";
            } else {
#if defined(_WIN32)
                filename = base_name + ".dll";
#elif defined(__APPLE__)
                filename = base_name + ".dylib";
#else
                filename = base_name + ".so";
#endif
            }
            break;
        case TargetType::Interface:
            return {};
        }

        return fs::path(dir_str) / filename;
    }

    static std::vector<std::string> get_effective_include_dirs(const Builder &builder, const Target &target) {
        std::vector<std::string> result = target.config().include_dirs;
        std::vector<std::string> visited;
        visited.push_back(target.name());

        std::vector<std::string> to_visit = target.linked_targets();
        while (!to_visit.empty()) {
            std::string cur = to_visit.back();
            to_visit.pop_back();
            if (std::find(visited.begin(), visited.end(), cur) != visited.end()) {
                continue;
            }
            visited.push_back(cur);
            const Target *dep = builder.find_target(cur);
            if (dep) {
                for (const auto &inc : dep->config().include_dirs) {
                    if (std::find(result.begin(), result.end(), inc) == result.end()) {
                        result.push_back(inc);
                    }
                }
                for (const auto &next : dep->linked_targets()) {
                    to_visit.push_back(next);
                }
            }
        }
        return result;
    }

    static std::vector<std::string> get_effective_defines(const Builder &builder, const Target &target) {
        std::vector<std::string> result = target.config().defines;
        std::vector<std::string> visited;
        visited.push_back(target.name());

        std::vector<std::string> to_visit = target.linked_targets();
        while (!to_visit.empty()) {
            std::string cur = to_visit.back();
            to_visit.pop_back();
            if (std::find(visited.begin(), visited.end(), cur) != visited.end()) {
                continue;
            }
            visited.push_back(cur);
            const Target *dep = builder.find_target(cur);
            if (dep) {
                for (const auto &def : dep->config().defines) {
                    if (std::find(result.begin(), result.end(), def) == result.end()) {
                        result.push_back(def);
                    }
                }
                for (const auto &next : dep->linked_targets()) {
                    to_visit.push_back(next);
                }
            }
        }
        return result;
    }

    static std::vector<std::string> get_effective_compile_options(const Builder &builder, const Target &target) {
        std::vector<std::string> result = target.config().compile_options;
        std::vector<std::string> visited;
        visited.push_back(target.name());

        std::vector<std::string> to_visit = target.linked_targets();
        while (!to_visit.empty()) {
            std::string cur = to_visit.back();
            to_visit.pop_back();
            if (std::find(visited.begin(), visited.end(), cur) != visited.end()) {
                continue;
            }
            visited.push_back(cur);
            const Target *dep = builder.find_target(cur);
            if (dep) {
                for (const auto &opt : dep->config().compile_options) {
                    if (std::find(result.begin(), result.end(), opt) == result.end()) {
                        result.push_back(opt);
                    }
                }
                for (const auto &next : dep->linked_targets()) {
                    to_visit.push_back(next);
                }
            }
        }
        return result;
    }

    static std::vector<std::string> get_effective_link_options(const Builder &builder, const Target &target) {
        std::vector<std::string> result = target.config().link_options;
        std::vector<std::string> visited;
        visited.push_back(target.name());

        std::vector<std::string> to_visit = target.linked_targets();
        while (!to_visit.empty()) {
            std::string cur = to_visit.back();
            to_visit.pop_back();
            if (std::find(visited.begin(), visited.end(), cur) != visited.end()) {
                continue;
            }
            visited.push_back(cur);
            const Target *dep = builder.find_target(cur);
            if (dep) {
                for (const auto &opt : dep->config().link_options) {
                    if (std::find(result.begin(), result.end(), opt) == result.end()) {
                        result.push_back(opt);
                    }
                }
                for (const auto &next : dep->linked_targets()) {
                    to_visit.push_back(next);
                }
            }
        }
        return result;
    }

    static std::filesystem::path get_import_library_path(const Target &target, const IToolchain &toolchain) {
        namespace fs = std::filesystem;
        if (target.type() != TargetType::SharedLibrary) {
            return {};
        }
        std::string base_name = target.config().output_name.empty() ? target.name() : target.config().output_name;
        std::string implib_name = toolchain.format_import_library_name(base_name);
        if (implib_name.empty()) {
            return {};
        }
        std::string dir_str = target.config().output_dir.empty() ? "lib" : target.config().output_dir;
        return fs::path(dir_str) / implib_name;
    }

    static std::pair<std::vector<std::string>, std::vector<std::string>>
    get_effective_link_dependencies(const Builder &builder, const Target &target, const IToolchain &toolchain) {
        std::vector<std::string> lib_dirs = target.config().link_directories;
        std::vector<std::string> libs = target.config().link_libraries;

        // System libraries
        for (const auto &sys : target.linked_system_libs()) {
            if (std::find(libs.begin(), libs.end(), sys) == libs.end()) {
                libs.push_back(sys);
            }
        }

        // Linked targets (transitive traversal supporting interface libraries)
        std::vector<std::string> to_visit = target.linked_targets();
        std::vector<std::string> visited;
        visited.push_back(target.name());

        while (!to_visit.empty()) {
            std::string cur = to_visit.back();
            to_visit.pop_back();
            if (std::find(visited.begin(), visited.end(), cur) != visited.end()) {
                continue;
            }
            visited.push_back(cur);
            const Target *dep = builder.find_target(cur);
            if (dep) {
                if (dep->type() == TargetType::StaticLibrary) {
                    std::filesystem::path dep_artifact = get_artifact_path(*dep, toolchain);
                    libs.push_back(dep_artifact.string());
                } else if (dep->type() == TargetType::SharedLibrary) {
#if defined(_WIN32)
                    std::filesystem::path implib = get_import_library_path(*dep, toolchain);
                    libs.push_back(implib.string());
#else
                    std::filesystem::path dep_artifact = get_artifact_path(*dep, toolchain);
                    lib_dirs.push_back(dep_artifact.parent_path().string());
                    std::string out_name = dep->config().output_name.empty() ? dep->name() : dep->config().output_name;
                    libs.push_back(out_name);
#endif
                } else if (dep->type() == TargetType::Interface) {
                    for (const auto &ld : dep->config().link_directories) {
                        if (std::find(lib_dirs.begin(), lib_dirs.end(), ld) == lib_dirs.end()) {
                            lib_dirs.push_back(ld);
                        }
                    }
                    for (const auto &l : dep->config().link_libraries) {
                        if (std::find(libs.begin(), libs.end(), l) == libs.end()) {
                            libs.push_back(l);
                        }
                    }
                    for (const auto &sys : dep->linked_system_libs()) {
                        if (std::find(libs.begin(), libs.end(), sys) == libs.end()) {
                            libs.push_back(sys);
                        }
                    }
                    for (const auto &next : dep->linked_targets()) {
                        to_visit.push_back(next);
                    }
                }
            }
        }

        return {lib_dirs, libs};
    }

    static std::optional<std::vector<std::string>>
    compile_target_sources(const Builder &builder, const Target &target, const IToolchain &toolchain,
                           const std::filesystem::path &obj_dir, bool pic, bool verbose, BuildResult &result,
                           size_t thread_count, bool &any_recompiled, BuildProfiler *profiler = nullptr) {
        namespace fs = std::filesystem;
        std::error_code ec;
        std::vector<std::string> compiled_objects;
        const char *obj_ext = (toolchain.family() == CompilerFamily::Msvc) ? ".obj" : ".o";
        any_recompiled = false;

        auto effective_includes = get_effective_include_dirs(builder, target);
        auto effective_defines = get_effective_defines(builder, target);
        auto effective_compile_opts = get_effective_compile_options(builder, target);

        // For shared library or plugin, auto-define <TARGET>_EXPORTS macro
        if (target.type() == TargetType::SharedLibrary || target.type() == TargetType::Plugin) {
            std::string upper_name;
            for (char c : target.name()) {
                upper_name.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
            }
            std::string export_macro = upper_name + "_EXPORTS";
            if (std::find(effective_defines.begin(), effective_defines.end(), export_macro) ==
                effective_defines.end()) {
                effective_defines.push_back(export_macro);
            }
        }

        std::string eff_triple = !target.target_triple().empty() ? target.target_triple() : builder.target_triple();
        std::filesystem::path eff_sysroot = !target.sysroot().empty() ? target.sysroot() : builder.sysroot();

        fs::path pch_output_file;
        fs::path pch_companion_source;
        fs::path pch_companion_obj;
        std::string pch_hdr_path;

        if (!target.pch_header().empty()) {
            pch_hdr_path = target.pch_header();
            if (!fs::exists(pch_hdr_path)) {
                for (const auto &inc : effective_includes) {
                    fs::path candidate = fs::path(inc) / pch_hdr_path;
                    if (fs::exists(candidate)) {
                        pch_hdr_path = candidate.string();
                        break;
                    }
                }
            }

            if (fs::exists(pch_hdr_path)) {
                pch_hdr_path = fs::absolute(pch_hdr_path).lexically_normal().generic_string();
            } else {
                pch_hdr_path = util::normalize_path(pch_hdr_path, false);
            }

            std::string hdr_filename = fs::path(target.pch_header()).filename().string();

            if (toolchain.family() == CompilerFamily::Msvc) {
                pch_output_file = obj_dir / (target.name() + "_" + hdr_filename + ".pch");
                pch_companion_obj = obj_dir / (target.name() + "_" + hdr_filename + ".obj");

                if (!target.pch_source().empty()) {
                    pch_companion_source = target.pch_source();
                } else {
                    pch_companion_source = obj_dir / (target.name() + "_" + hdr_filename + ".cpp");
                    std::ofstream f(pch_companion_source);
                    f << "#include \"" << pch_hdr_path << "\"\n";
                    f.close();
                }
            } else if (toolchain.family() == CompilerFamily::Gcc) {
                pch_output_file = obj_dir / (hdr_filename + ".gch");
            } else {
                pch_output_file = obj_dir / (hdr_filename + ".pch");
            }

            PchParams pch_params{
                .header_file = pch_hdr_path,
                .output_pch = pch_output_file.string(),
                .pch_source = pch_companion_source.string(),
                .pch_object = pch_companion_obj.string(),
                .cpp_version = target.config().cpp_version,
                .c_version = target.config().c_version,
                .optimize = target.config().optimize,
                .warnings_as_errors = target.config().warnings_as_errors,
                .exceptions = target.config().exceptions,
                .rtti = target.config().rtti,
                .position_independent_code = pic,
                .include_dirs = effective_includes,
                .defines = effective_defines,
                .extra_flags = effective_compile_opts,
                .target_triple = eff_triple,
                .sysroot = eff_sysroot,
            };

            auto pch_cmd = toolchain.build_pch_command(pch_params);
            if (verbose) {
                std::cout << "[cxxgen pch] " << util::flatten_command(pch_cmd) << "\n";
            }

            auto pch_start = std::chrono::steady_clock::now();
            ProcessResult pch_res = ProcessRunner::run(pch_cmd, "", toolchain.env_vars());
            if (profiler) {
                uint64_t pch_sz = 0;
                std::error_code fec;
                if (fs::exists(target.pch_header(), fec)) {
                    pch_sz = fs::file_size(target.pch_header(), fec);
                }
                profiler->record("pch: " + target.name(), "compile", pch_start, std::chrono::steady_clock::now(),
                                 get_current_thread_id(), pch_res.success() ? "Executed" : "Failed", pch_sz,
                                 "Target: " + target.name());
            }
            if (!pch_res.success()) {
                result.success = false;
                result.exit_code = pch_res.exit_code != 0 ? pch_res.exit_code : 1;
                result.error_message =
                    "PCH compilation failed for " + target.pch_header() + ":\n" + pch_res.combined_output();
                return std::nullopt;
            }

            if (toolchain.family() == CompilerFamily::Msvc && !pch_companion_obj.empty()) {
                compiled_objects.push_back(pch_companion_obj.string());
            }
        }

        if (thread_count == 0) {
            unsigned int hw = std::thread::hardware_concurrency();
            thread_count = hw > 0 ? static_cast<size_t>(hw) : 4;
        }

        if (!BuildStateDatabase::instance().is_loaded()) {
            BuildStateDatabase::instance().load();
        }

        std::string compiler_sig = toolchain.name() + " " + toolchain.compiler_path();

        // ====================================================================
        // C++20 / C++23 Modules Pipeline (Epic 12)
        // ====================================================================
        std::unordered_map<std::string, std::string> active_module_references;

        // Inherit BMIs from linked targets
        for (const auto &dep_name : target.linked_targets()) {
            if (const Target *dep_t = builder.find_target(dep_name)) {
                for (const auto &[m_name, m_path] : dep_t->module_bmis()) {
                    active_module_references[m_name] = m_path.string();
                }
            }
        }
        for (const auto &[m_name, m_path] : target.module_bmis()) {
            active_module_references[m_name] = m_path.string();
        }

        // Scan target sources for modules
        auto mod_graph = modules::ModuleScanner::scan_target(target, &toolchain);
        if (mod_graph.has_cycle) {
            result.success = false;
            result.exit_code = 1;
            result.error_message = "Cyclic module dependency detected in target '" + target.name() +
                                   "': " + util::join(mod_graph.cycle_path, " -> ");
            return std::nullopt;
        }

        // Story 12.3: Standard Library Module Support (import std;)
        if (target.has_std_module() || mod_graph.requires_std) {
            fs::path std_ifc, std_obj;
            std::string std_err;
            if (!modules::StdModuleHelper::ensure_std_module(builder, target, toolchain, std_ifc, std_obj, std_err)) {
                result.success = false;
                result.exit_code = 1;
                result.error_message = "Standard library module (import std;) compilation failed:\n" + std_err;
                return std::nullopt;
            }
            active_module_references["std"] = std_ifc.string();
            active_module_references["std.compat"] = std_ifc.string();
            if (!std_obj.empty()) {
                compiled_objects.push_back(std_obj.string());
            }
        }

        // Phase 1: Compile Module Interfaces to BMIs in topological order
        fs::path bmi_dir = obj_dir / "bmi";
        fs::create_directories(bmi_dir, ec);

        std::unordered_set<std::string> interface_source_set;
        for (const auto &iface_src : mod_graph.sorted_interfaces) {
            interface_source_set.insert(fs::path(iface_src).lexically_normal().string());
            std::string mod_name = mod_graph.source_to_module[iface_src];
            std::string safe_mod_name = modules::ModuleScanner::sanitize_module_name(mod_name);

            fs::path bmi_path;
            if (toolchain.family() == CompilerFamily::Msvc) {
                bmi_path = bmi_dir / (safe_mod_name + ".ifc");
            } else if (toolchain.family() == CompilerFamily::Clang) {
                bmi_path = bmi_dir / (safe_mod_name + ".pcm");
            } else {
                bmi_path = bmi_dir / (safe_mod_name + ".gcm");
            }

            fs::path bmi_obj = obj_dir / (safe_mod_name + obj_ext);

            // Check BMI cache or incremental up-to-date
            std::string bmi_cache_key = modules::BmiCache::compute_key(iface_src, compiler_sig, effective_compile_opts);
            bool restored_from_cache = false;
            if (target.is_global_cache_enabled()) {
                restored_from_cache = modules::BmiCache::instance().try_restore(bmi_cache_key, bmi_path, bmi_obj);
            }

            if (!restored_from_cache) {
                bool bmi_up_to_date = false;
                if (fs::exists(bmi_path, ec) && fs::exists(bmi_obj, ec)) {
                    auto src_time = fs::last_write_time(iface_src, ec);
                    auto bmi_time = fs::last_write_time(bmi_path, ec);
                    auto obj_time = fs::last_write_time(bmi_obj, ec);
                    if (!ec && bmi_time >= src_time && obj_time >= src_time) {
                        bmi_up_to_date = true;
                    }
                }

                if (!bmi_up_to_date) {
                    any_recompiled = true;
                    bool comp_ok = false;
                    std::string comp_err;
                    if (toolchain.family() == CompilerFamily::Msvc) {
                        CompileParams cp;
                        cp.source_file = iface_src;
                        cp.object_file = bmi_obj.string();
                        cp.module_interface_output = bmi_path.string();
                        cp.is_module_interface = true;
                        cp.cpp_version = (target.config().cpp_version < CppVersion::Cpp20)
                                             ? CppVersion::Cpp20
                                             : target.config().cpp_version;
                        if (active_module_references.find("std") != active_module_references.end()) {
                            cp.cpp_version = CppVersion::Cpp23;
                        }
                        cp.optimize = target.config().optimize;
                        cp.exceptions = target.config().exceptions;
                        cp.rtti = target.config().rtti;
                        cp.include_dirs = effective_includes;
                        cp.defines = effective_defines;
                        cp.extra_flags = effective_compile_opts;
                        cp.module_references = active_module_references;
                        auto cmd = toolchain.build_compile_command(cp);
                        if (verbose) {
                            std::cout << "[cxxgen bmi] " << util::flatten_command(cmd) << "\n";
                        }
                        auto res = ProcessRunner::run(cmd, "", toolchain.env_vars());
                        comp_ok = res.success();
                        comp_err = res.combined_output();
                    } else if (toolchain.family() == CompilerFamily::Clang) {
                        std::vector<std::string> pre_cmd;
                        pre_cmd.push_back(toolchain.compiler_path());
                        std::string cpp_std = "-std=c++20";
                        if (target.config().cpp_version == CppVersion::Cpp23)
                            cpp_std = "-std=c++23";
                        else if (target.config().cpp_version == CppVersion::Cpp26)
                            cpp_std = "-std=c++26";
                        pre_cmd.push_back(cpp_std);
                        for (const auto &inc : effective_includes)
                            pre_cmd.push_back("-I" + util::normalize_path(inc, false));
                        for (const auto &d : effective_defines)
                            pre_cmd.push_back("-D" + d);
                        for (const auto &[m, p] : active_module_references)
                            pre_cmd.push_back("-fmodule-file=" + m + "=" + util::normalize_path(p, false));
                        for (const auto &opt : effective_compile_opts)
                            pre_cmd.push_back(opt);
                        pre_cmd.push_back("-x");
                        pre_cmd.push_back("c++-module");
                        pre_cmd.push_back("--precompile");
                        pre_cmd.push_back(util::normalize_path(iface_src, false));
                        pre_cmd.push_back("-o");
                        pre_cmd.push_back(util::normalize_path(bmi_path.string(), false));

                        if (verbose) {
                            std::cout << "[cxxgen precompile] " << util::flatten_command(pre_cmd) << "\n";
                        }
                        auto res1 = ProcessRunner::run(pre_cmd, "", toolchain.env_vars());
                        if (res1.success()) {
                            std::vector<std::string> obj_cmd;
                            obj_cmd.push_back(toolchain.compiler_path());
                            obj_cmd.push_back("-c");
                            obj_cmd.push_back("-x");
                            obj_cmd.push_back("pcm");
                            for (const auto &[m, p] : active_module_references) {
                                obj_cmd.push_back("-fmodule-file=" + m + "=" + util::normalize_path(p, false));
                            }
                            obj_cmd.push_back(util::normalize_path(bmi_path.string(), false));
                            obj_cmd.push_back("-o");
                            obj_cmd.push_back(util::normalize_path(bmi_obj.string(), false));
                            auto res2 = ProcessRunner::run(obj_cmd, "", toolchain.env_vars());
                            comp_ok = res2.success();
                            comp_err = res2.combined_output();
                        } else {
                            comp_ok = false;
                            comp_err = res1.combined_output();
                        }
                    } else {
                        CompileParams cp;
                        cp.source_file = iface_src;
                        cp.object_file = bmi_obj.string();
                        cp.is_module_interface = true;
                        cp.cpp_version = CppVersion::Cpp20;
                        cp.include_dirs = effective_includes;
                        cp.defines = effective_defines;
                        cp.extra_flags = effective_compile_opts;
                        auto cmd = toolchain.build_compile_command(cp);
                        auto res = ProcessRunner::run(cmd, "", toolchain.env_vars());
                        comp_ok = res.success();
                        comp_err = res.combined_output();
                    }

                    if (!comp_ok) {
                        result.success = false;
                        result.exit_code = 1;
                        result.error_message =
                            "BMI compilation failed for module '" + mod_name + "' (" + iface_src + "):\n" + comp_err;
                        return std::nullopt;
                    }

                    if (target.is_global_cache_enabled()) {
                        modules::BmiCache::instance().store(bmi_cache_key, bmi_path, bmi_obj);
                    }
                }
            }

            active_module_references[mod_name] = bmi_path.string();
            const_cast<Target &>(target).set_module_bmi(mod_name, bmi_path);
            compiled_objects.push_back(bmi_obj.string());
        }

        std::vector<std::string> valid_sources;
        for (const auto &src : target.config().sources) {
            if (!pch_companion_source.empty()) {
                if (fs::path(src).lexically_normal() == pch_companion_source.lexically_normal()) {
                    continue;
                }
            }
            if (interface_source_set.find(fs::path(src).lexically_normal().string()) != interface_source_set.end()) {
                continue;
            }
            valid_sources.push_back(src);
        }

        auto make_flags_str = [](const CompileParams &cp) {
            std::string s;
            s.reserve(512);
            s += "opt=" + std::to_string(static_cast<int>(cp.optimize));
            s += ";std=" + std::to_string(static_cast<int>(cp.cpp_version));
            s += ";c_std=" + (cp.c_version.has_value() ? std::to_string(static_cast<int>(*cp.c_version)) : "none");
            s += ";werr=" + std::string(cp.warnings_as_errors ? "1" : "0");
            s += ";exc=" + std::string(cp.exceptions ? "1" : "0");
            s += ";rtti=" + std::string(cp.rtti ? "1" : "0");
            s += ";pic=" + std::string(cp.position_independent_code ? "1" : "0");
            s += ";is_c=" + std::string(cp.is_c_source ? "1" : "0");
            s += ";inc=" + util::join(cp.include_dirs, ",");
            s += ";def=" + util::join(cp.defines, ",");
            s += ";extra=" + util::join(cp.extra_flags, ",");
            s += ";pch=" + cp.pch_header;
            s += ";pch_f=" + cp.pch_file;
            s += ";san=" + std::to_string(static_cast<uint32_t>(cp.sanitizers));
            s += ";cov=" + std::string(cp.coverage ? "1" : "0");
            for (const auto &[m, p] : cp.module_references) {
                s += ";mod=" + m + ":" + p;
            }
            if (!cp.target_triple.empty()) {
                s += ";triple=" + cp.target_triple;
            }
            if (!cp.sysroot.empty()) {
                s += ";sysroot=" + cp.sysroot.generic_string();
            }
            return s;
        };

        auto record_compiled_state = [&](const std::string &src_file, const fs::path &o_path, const std::string &d_path,
                                         const std::string &c_flags) {
            std::error_code l_ec;
            std::vector<std::string> deps = DepfileScanner::get_cached_dependencies(d_path);
            if (deps.empty() && fs::exists(d_path, l_ec)) {
                deps = DepfileScanner::parse_file(d_path);
            }

            uint64_t src_hash = BuildStateDatabase::instance().get_file_content_hash(src_file);
            auto src_fsize = fs::file_size(src_file, l_ec);
            auto src_ftime = fs::last_write_time(src_file, l_ec);
            uint64_t src_mtime_epoch = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(src_ftime.time_since_epoch()).count());

            std::vector<HeaderState> header_states;
            header_states.reserve(deps.size());
            for (const auto &dep : deps) {
                if (fs::exists(dep, l_ec)) {
                    uint64_t h_hash = BuildStateDatabase::instance().get_file_content_hash(dep);
                    auto h_ftime = fs::last_write_time(dep, l_ec);
                    uint64_t h_mtime_epoch = static_cast<uint64_t>(
                        std::chrono::duration_cast<std::chrono::seconds>(h_ftime.time_since_epoch()).count());
                    header_states.push_back({util::normalize_path(dep, false), h_hash, h_mtime_epoch});
                }
            }

            std::string fp = BuildStateDatabase::compute_fingerprint(src_hash, header_states, compiler_sig, c_flags);
            ObjectBuildState new_state{
                .source_file = util::normalize_path(src_file, false),
                .object_file = util::normalize_path(o_path.string(), false),
                .source_hash = src_hash,
                .source_mtime = src_mtime_epoch,
                .source_size = src_fsize,
                .compiler_signature = compiler_sig,
                .compile_flags = c_flags,
                .fingerprint = fp,
                .headers = std::move(header_states),
            };
            BuildStateDatabase::instance().set_object_state(o_path.string(), new_state);
        };

        if (thread_count <= 1 || valid_sources.size() <= 1) {
            for (const auto &src : valid_sources) {
                auto t_tu_start = std::chrono::steady_clock::now();
                std::string safe_name = sanitize_source_for_object(src);
                fs::path obj_path = obj_dir / (safe_name + obj_ext);
                std::string dep_path = obj_path.string() + ".d";

                if (obj_path.has_parent_path()) {
                    fs::create_directories(obj_path.parent_path(), ec);
                }

                bool is_c = detail::ends_with_ci(src, ".c");

                CompileParams cparams{
                    .source_file = src,
                    .object_file = obj_path.string(),
                    .cpp_version = (active_module_references.find("std") != active_module_references.end() &&
                                    toolchain.family() == CompilerFamily::Msvc)
                                       ? CppVersion::Cpp23
                                       : target.config().cpp_version,
                    .c_version = target.config().c_version,
                    .optimize = target.config().optimize,
                    .warnings_as_errors = target.config().warnings_as_errors,
                    .exceptions = target.config().exceptions,
                    .rtti = target.config().rtti,
                    .position_independent_code = pic,
                    .include_dirs = effective_includes,
                    .defines = effective_defines,
                    .extra_flags = effective_compile_opts,
                    .depfile =
                        (toolchain.family() == CompilerFamily::Clang || toolchain.family() == CompilerFamily::Gcc ||
                         toolchain.family() == CompilerFamily::Emscripten)
                            ? dep_path
                            : "",
                    .pch_header = (toolchain.family() == CompilerFamily::Msvc) ? pch_hdr_path : target.pch_header(),
                    .pch_file = pch_output_file.string(),
                    .is_c_source = is_c,
                    .sanitizers = target.config().sanitizers,
                    .coverage = target.config().coverage,
                    .target_triple = eff_triple,
                    .sysroot = eff_sysroot,
                    .module_references = active_module_references,
                };

                std::string flags_str = make_flags_str(cparams);

                bool is_up_to_date = false;
                if (fs::exists(obj_path, ec)) {
                    if (BuildStateDatabase::instance().has_object_state(obj_path.string())) {
                        is_up_to_date =
                            BuildStateDatabase::instance().is_object_up_to_date(obj_path, src, compiler_sig, flags_str);
                    } else if (fs::exists(dep_path, ec)) {
                        is_up_to_date = DepfileScanner::is_object_up_to_date(obj_path, dep_path);
                        if (is_up_to_date) {
                            record_compiled_state(src, obj_path, dep_path, flags_str);
                        }
                    }
                }

                if (is_up_to_date) {
                    if (verbose) {
                        std::cout << "[cxxgen compile] " << src << " is up to date.\n";
                    }
                    if (profiler) {
                        uint64_t fsz = 0;
                        std::error_code fec;
                        if (fs::exists(src, fec)) {
                            fsz = fs::file_size(src, fec);
                        }
                        profiler->record("compile: " + src, "compile", t_tu_start, std::chrono::steady_clock::now(),
                                         get_current_thread_id(), "UpToDate", fsz, "Target: " + target.name());
                    }
                    compiled_objects.push_back(obj_path.string());
                    continue;
                }

                if (target.is_global_cache_enabled() && GlobalObjectCache::instance().is_enabled()) {
                    bool restored = GlobalObjectCache::instance().try_direct_restore(
                        src, compiler_sig, flags_str, eff_triple, obj_path, effective_includes);
                    if (restored) {
                        if (verbose) {
                            std::cout << "[cxxgen cache] Global cache hit: " << src << "\n";
                        }
                        if (profiler) {
                            uint64_t fsz = 0;
                            std::error_code fec;
                            if (fs::exists(src, fec)) {
                                fsz = fs::file_size(src, fec);
                            }
                            profiler->record("cache: " + src, "compile", t_tu_start, std::chrono::steady_clock::now(),
                                             get_current_thread_id(), "Cached", fsz, "Target: " + target.name());
                        }
                        record_compiled_state(src, obj_path, dep_path, flags_str);
                        compiled_objects.push_back(obj_path.string());
                        continue;
                    }
                }

                any_recompiled = true;

                auto compile_cmd = toolchain.build_compile_command(cparams);
                if (verbose) {
                    std::cout << "[cxxgen compile] " << util::flatten_command(compile_cmd) << "\n";
                }

                ProcessResult c_res = ProcessRunner::run(compile_cmd, "", toolchain.env_vars());
                if (profiler) {
                    uint64_t fsz = 0;
                    std::error_code fec;
                    if (fs::exists(src, fec)) {
                        fsz = fs::file_size(src, fec);
                    }
                    profiler->record("compile: " + src, "compile", t_tu_start, std::chrono::steady_clock::now(),
                                     get_current_thread_id(), c_res.success() ? "Executed" : "Failed", fsz,
                                     "Target: " + target.name());
                }
                if (!c_res.success()) {
                    result.success = false;
                    result.exit_code = c_res.exit_code != 0 ? c_res.exit_code : 1;
                    result.error_message = "Compilation failed for " + src + ":\n" +
                                           terminal::DiagnosticsFormatter::format_compiler_output(
                                               c_res.combined_output(), terminal::color_enabled());
                    return std::nullopt;
                }

                if (toolchain.family() == CompilerFamily::Msvc) {
                    auto show_res = ShowIncludesScanner::parse(c_res.combined_output());
                    ShowIncludesScanner::write_depfile(dep_path, obj_path.string(), src, show_res.dependencies);
                    DepfileScanner::record_dependencies(dep_path, show_res.dependencies);
                    c_res.stdout_output = show_res.clean_output;
                    c_res.stderr_output.clear();
                } else if (fs::exists(dep_path, ec)) {
                    DepfileScanner::parse_file(dep_path);
                }

                record_compiled_state(src, obj_path, dep_path, flags_str);
                if (target.is_global_cache_enabled() && GlobalObjectCache::instance().is_enabled()) {
                    std::vector<std::string> deps = DepfileScanner::get_cached_dependencies(dep_path);
                    if (deps.empty() && fs::exists(dep_path, ec)) {
                        deps = DepfileScanner::parse_file(dep_path);
                    }
                    GlobalObjectCache::instance().store(src, compiler_sig, flags_str, eff_triple, deps, obj_path,
                                                        effective_includes);
                }
                compiled_objects.push_back(obj_path.string());
            }
        } else {
            size_t workers = std::min(thread_count, valid_sources.size());
            ThreadPool pool(workers);
            std::atomic<bool> compile_failed{false};
            std::atomic<bool> any_recompiled_atomic{false};
            std::mutex mtx;
            std::vector<std::string> parallel_objects(valid_sources.size());

            for (const auto &src : valid_sources) {
                std::string safe_name = sanitize_source_for_object(src);
                fs::path obj_path = obj_dir / (safe_name + obj_ext);
                if (obj_path.has_parent_path()) {
                    fs::create_directories(obj_path.parent_path(), ec);
                }
            }

            for (size_t i = 0; i < valid_sources.size(); ++i) {
                pool.enqueue([&, i]() {
                    if (compile_failed.load()) {
                        return;
                    }

                    auto t_tu_start = std::chrono::steady_clock::now();
                    const auto &src = valid_sources[i];
                    std::string safe_name = sanitize_source_for_object(src);
                    fs::path obj_path = obj_dir / (safe_name + obj_ext);
                    std::string dep_path = obj_path.string() + ".d";

                    bool is_c = detail::ends_with_ci(src, ".c");

                    CompileParams cparams{
                        .source_file = src,
                        .object_file = obj_path.string(),
                        .cpp_version = (active_module_references.find("std") != active_module_references.end() &&
                                        toolchain.family() == CompilerFamily::Msvc)
                                           ? CppVersion::Cpp23
                                           : target.config().cpp_version,
                        .c_version = target.config().c_version,
                        .optimize = target.config().optimize,
                        .warnings_as_errors = target.config().warnings_as_errors,
                        .exceptions = target.config().exceptions,
                        .rtti = target.config().rtti,
                        .position_independent_code = pic,
                        .include_dirs = effective_includes,
                        .defines = effective_defines,
                        .extra_flags = effective_compile_opts,
                        .depfile =
                            (toolchain.family() == CompilerFamily::Clang || toolchain.family() == CompilerFamily::Gcc ||
                             toolchain.family() == CompilerFamily::Emscripten)
                                ? dep_path
                                : "",
                        .pch_header = (toolchain.family() == CompilerFamily::Msvc) ? pch_hdr_path : target.pch_header(),
                        .pch_file = pch_output_file.string(),
                        .is_c_source = is_c,
                        .sanitizers = target.config().sanitizers,
                        .coverage = target.config().coverage,
                        .target_triple = eff_triple,
                        .sysroot = eff_sysroot,
                        .module_references = active_module_references,
                    };

                    std::string flags_str = make_flags_str(cparams);

                    std::error_code err;
                    bool is_up_to_date = false;
                    if (fs::exists(obj_path, err)) {
                        if (BuildStateDatabase::instance().has_object_state(obj_path.string())) {
                            is_up_to_date = BuildStateDatabase::instance().is_object_up_to_date(
                                obj_path, src, compiler_sig, flags_str);
                        } else if (fs::exists(dep_path, err)) {
                            is_up_to_date = DepfileScanner::is_object_up_to_date(obj_path, dep_path);
                            if (is_up_to_date) {
                                record_compiled_state(src, obj_path, dep_path, flags_str);
                            }
                        }
                    }

                    if (is_up_to_date) {
                        if (verbose) {
                            std::lock_guard<std::mutex> lk(mtx);
                            std::cout << "[cxxgen compile] " << src << " is up to date.\n";
                        }
                        if (profiler) {
                            uint64_t fsz = 0;
                            std::error_code fec;
                            if (fs::exists(src, fec)) {
                                fsz = fs::file_size(src, fec);
                            }
                            profiler->record("compile: " + src, "compile", t_tu_start, std::chrono::steady_clock::now(),
                                             get_current_thread_id(), "UpToDate", fsz, "Target: " + target.name());
                        }
                        parallel_objects[i] = obj_path.string();
                        return;
                    }

                    if (target.is_global_cache_enabled() && GlobalObjectCache::instance().is_enabled()) {
                        bool restored = GlobalObjectCache::instance().try_direct_restore(
                            src, compiler_sig, flags_str, eff_triple, obj_path, effective_includes);
                        if (restored) {
                            if (verbose) {
                                std::lock_guard<std::mutex> lk(mtx);
                                std::cout << "[cxxgen cache] Global cache hit: " << src << "\n";
                            }
                            if (profiler) {
                                uint64_t fsz = 0;
                                std::error_code fec;
                                if (fs::exists(src, fec)) {
                                    fsz = fs::file_size(src, fec);
                                }
                                profiler->record("cache: " + src, "compile", t_tu_start,
                                                 std::chrono::steady_clock::now(), get_current_thread_id(), "Cached",
                                                 fsz, "Target: " + target.name());
                            }
                            record_compiled_state(src, obj_path, dep_path, flags_str);
                            parallel_objects[i] = obj_path.string();
                            return;
                        }
                    }

                    any_recompiled_atomic.store(true);

                    auto compile_cmd = toolchain.build_compile_command(cparams);
                    if (verbose) {
                        std::lock_guard<std::mutex> lk(mtx);
                        std::cout << "[cxxgen compile] " << util::flatten_command(compile_cmd) << "\n";
                    }

                    ProcessResult c_res = ProcessRunner::run(compile_cmd, "", toolchain.env_vars());
                    if (profiler) {
                        uint64_t fsz = 0;
                        std::error_code fec;
                        if (fs::exists(src, fec)) {
                            fsz = fs::file_size(src, fec);
                        }
                        profiler->record("compile: " + src, "compile", t_tu_start, std::chrono::steady_clock::now(),
                                         get_current_thread_id(), c_res.success() ? "Executed" : "Failed", fsz,
                                         "Target: " + target.name());
                    }
                    if (!c_res.success()) {
                        bool expected = false;
                        if (compile_failed.compare_exchange_strong(expected, true)) {
                            std::lock_guard<std::mutex> lk(mtx);
                            result.success = false;
                            result.exit_code = c_res.exit_code != 0 ? c_res.exit_code : 1;
                            result.error_message = "Compilation failed for " + src + ":\n" +
                                                   terminal::DiagnosticsFormatter::format_compiler_output(
                                                       c_res.combined_output(), terminal::color_enabled());
                        }
                        pool.cancel_pending();
                        return;
                    }

                    if (toolchain.family() == CompilerFamily::Msvc) {
                        auto show_res = ShowIncludesScanner::parse(c_res.combined_output());
                        ShowIncludesScanner::write_depfile(dep_path, obj_path.string(), src, show_res.dependencies);
                        DepfileScanner::record_dependencies(dep_path, show_res.dependencies);
                        c_res.stdout_output = show_res.clean_output;
                        c_res.stderr_output.clear();
                    } else if (fs::exists(dep_path, err)) {
                        DepfileScanner::parse_file(dep_path);
                    }

                    record_compiled_state(src, obj_path, dep_path, flags_str);
                    if (target.is_global_cache_enabled() && GlobalObjectCache::instance().is_enabled()) {
                        std::vector<std::string> deps = DepfileScanner::get_cached_dependencies(dep_path);
                        if (deps.empty() && fs::exists(dep_path, err)) {
                            deps = DepfileScanner::parse_file(dep_path);
                        }
                        GlobalObjectCache::instance().store(src, compiler_sig, flags_str, eff_triple, deps, obj_path,
                                                            effective_includes);
                    }
                    parallel_objects[i] = obj_path.string();
                });
            }

            pool.wait_all();

            if (compile_failed.load()) {
                return std::nullopt;
            }

            if (any_recompiled_atomic.load()) {
                any_recompiled = true;
            }

            for (const auto &obj : parallel_objects) {
                if (!obj.empty()) {
                    compiled_objects.push_back(obj);
                }
            }
        }

        if (any_recompiled && BuildStateDatabase::instance().is_dirty()) {
            BuildStateDatabase::instance().save();
        }

        return compiled_objects;
    }

    static BuildResult build_static_library(const Builder &builder, const Target &target, const IToolchain &toolchain,
                                            bool verbose = false, size_t thread_count = 0,
                                            BuildProfiler *profiler = nullptr) {
        auto start_time = std::chrono::steady_clock::now();
        BuildResult result;

        if (target.type() != TargetType::StaticLibrary) {
            result.success = false;
            result.exit_code = -1;
            result.error_message = "Target '" + target.name() + "' is not a static library target";
            return result;
        }

        if (target.config().sources.empty()) {
            result.success = false;
            result.exit_code = -1;
            result.error_message = "Static library target '" + target.name() + "' has no source files to compile";
            return result;
        }

        namespace fs = std::filesystem;
        std::error_code ec;

        // Resolve output artifact path
        fs::path artifact = get_artifact_path(target, toolchain);
        if (artifact.has_parent_path()) {
            fs::create_directories(artifact.parent_path(), ec);
        }

        // Object directory
        fs::path obj_dir = fs::path(".cxxgen/obj") / target.name();
        fs::create_directories(obj_dir, ec);

        // 1. Compilation Stage
        bool any_recompiled = false;
        auto compiled_objects_opt = compile_target_sources(builder, target, toolchain, obj_dir, false, verbose, result,
                                                           thread_count, any_recompiled, profiler);
        if (!compiled_objects_opt) {
            return result;
        }
        std::vector<std::string> compiled_objects = std::move(*compiled_objects_opt);

        // Check if static library is up-to-date
        if (!any_recompiled && fs::exists(artifact, ec)) {
            auto art_time = fs::last_write_time(artifact, ec);
            if (!ec) {
                bool all_older = true;
                for (const auto &obj : compiled_objects) {
                    auto obj_time = fs::last_write_time(obj, ec);
                    if (ec || obj_time > art_time) {
                        all_older = false;
                        break;
                    }
                }
                if (all_older) {
                    if (verbose) {
                        std::cout << "[cxxgen archive] " << target.name() << " is up to date.\n";
                    }
                    auto end_time = std::chrono::steady_clock::now();
                    result.success = true;
                    result.exit_code = 0;
                    result.up_to_date = true;
                    result.artifact_path = artifact.string();
                    result.object_files = compiled_objects;
                    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                    return result;
                }
            }
        }

        // 2. Archive Stage
        ArchiveParams aparams{
            .output_lib = artifact.string(),
            .object_files = compiled_objects,
            .extra_flags = get_effective_link_options(builder, target),
        };

        auto archive_cmd = toolchain.build_archive_command(aparams);
        if (verbose) {
            std::cout << "[cxxgen archive] " << util::flatten_command(archive_cmd) << "\n";
        }

        auto archive_start = std::chrono::steady_clock::now();
        ProcessResult a_res = ProcessRunner::run(archive_cmd, "", toolchain.env_vars());
        if (profiler) {
            profiler->record("archive: " + target.name(), "link", archive_start, std::chrono::steady_clock::now(),
                             get_current_thread_id(), a_res.success() ? "Executed" : "Failed", 0,
                             "Static library artifact");
        }
        if (!a_res.success()) {
            result.success = false;
            result.exit_code = a_res.exit_code != 0 ? a_res.exit_code : 1;
            result.error_message = "Archiving failed for " + artifact.string() + ":\n" + a_res.combined_output();
            return result;
        }

        auto end_time = std::chrono::steady_clock::now();
        result.success = true;
        result.exit_code = 0;
        result.artifact_path = artifact.string();
        result.object_files = compiled_objects;
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        return result;
    }

    static bool build_dependent_libraries(const Builder &builder, const Target &target, const IToolchain &toolchain,
                                          const std::filesystem::path &artifact, BuildResult &result, bool verbose,
                                          std::error_code &ec) {
        namespace fs = std::filesystem;
        std::vector<std::string> to_visit = target.linked_targets();
        std::vector<std::string> visited;
        visited.push_back(target.name());

        while (!to_visit.empty()) {
            std::string dep_name = to_visit.back();
            to_visit.pop_back();
            if (std::find(visited.begin(), visited.end(), dep_name) != visited.end()) {
                continue;
            }
            visited.push_back(dep_name);

            const Target *dep = builder.find_target(dep_name);
            if (!dep) {
                continue;
            }

            if (dep->type() == TargetType::StaticLibrary) {
                fs::path dep_artifact = get_artifact_path(*dep, toolchain);
                if (!fs::exists(dep_artifact)) {
                    auto dep_res = build_static_library(builder, *dep, toolchain, verbose);
                    if (!dep_res.success) {
                        result.success = false;
                        result.exit_code = dep_res.exit_code != 0 ? dep_res.exit_code : 1;
                        result.error_message =
                            "Failed to build dependent static library '" + dep_name + "':\n" + dep_res.error_message;
                        return false;
                    }
                }
            } else if (dep->type() == TargetType::SharedLibrary) {
                fs::path dep_artifact = get_artifact_path(*dep, toolchain);
                if (!fs::exists(dep_artifact)) {
                    auto dep_res = build_shared_library(builder, *dep, toolchain, verbose);
                    if (!dep_res.success) {
                        result.success = false;
                        result.exit_code = dep_res.exit_code != 0 ? dep_res.exit_code : 1;
                        result.error_message =
                            "Failed to build dependent shared library '" + dep_name + "':\n" + dep_res.error_message;
                        return false;
                    }
                }
#if defined(_WIN32)
                if (artifact.parent_path() != dep_artifact.parent_path()) {
                    fs::copy_file(dep_artifact, artifact.parent_path() / dep_artifact.filename(),
                                  fs::copy_options::overwrite_existing, ec);
                }
#endif
            } else if (dep->type() == TargetType::Interface) {
                for (const auto &next : dep->linked_targets()) {
                    to_visit.push_back(next);
                }
            }
        }
        return true;
    }

    static BuildResult build_shared_library(const Builder &builder, const Target &target, const IToolchain &toolchain,
                                            bool verbose = false, size_t thread_count = 0,
                                            BuildProfiler *profiler = nullptr) {
        auto start_time = std::chrono::steady_clock::now();
        BuildResult result;

        if (target.type() != TargetType::SharedLibrary) {
            result.success = false;
            result.exit_code = -1;
            result.error_message = "Target '" + target.name() + "' is not a shared library target";
            return result;
        }

        if (target.config().sources.empty()) {
            result.success = false;
            result.exit_code = -1;
            result.error_message = "Shared library target '" + target.name() + "' has no source files to compile";
            return result;
        }

        namespace fs = std::filesystem;
        std::error_code ec;

        // Resolve output artifact path (.dll on Windows, .so on Linux, .dylib on macOS)
        fs::path artifact = get_artifact_path(target, toolchain);
        if (artifact.has_parent_path()) {
            fs::create_directories(artifact.parent_path(), ec);
        }

        // Import library path on Windows
        fs::path implib = get_import_library_path(target, toolchain);
        if (!implib.empty() && implib.has_parent_path()) {
            fs::create_directories(implib.parent_path(), ec);
        }

        // Object directory
        fs::path obj_dir = fs::path(".cxxgen/obj") / target.name();
        fs::create_directories(obj_dir, ec);

        // 1. Compilation Stage (PIC enabled for shared library)
        bool any_recompiled = false;
        auto compiled_objects_opt = compile_target_sources(builder, target, toolchain, obj_dir, true, verbose, result,
                                                           thread_count, any_recompiled, profiler);
        if (!compiled_objects_opt) {
            return result;
        }
        std::vector<std::string> compiled_objects = std::move(*compiled_objects_opt);

        // Build any dependent static or shared libraries
        if (!build_dependent_libraries(builder, target, toolchain, artifact, result, verbose, ec)) {
            return result;
        }

        // Check if shared library is up-to-date
        if (!any_recompiled && fs::exists(artifact, ec)) {
            auto art_time = fs::last_write_time(artifact, ec);
            if (!ec) {
                bool all_older = true;
                for (const auto &obj : compiled_objects) {
                    auto obj_time = fs::last_write_time(obj, ec);
                    if (ec || obj_time > art_time) {
                        all_older = false;
                        break;
                    }
                }
                if (all_older) {
                    for (const auto &dep_name : target.linked_targets()) {
                        if (const Target *dep_t = builder.find_target(dep_name)) {
                            fs::path dep_art = get_artifact_path(*dep_t, toolchain);
                            if (fs::exists(dep_art, ec)) {
                                auto dep_time = fs::last_write_time(dep_art, ec);
                                if (ec || dep_time > art_time) {
                                    all_older = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (all_older) {
                    if (verbose) {
                        std::cout << "[cxxgen link shared] " << target.name() << " is up to date.\n";
                    }
                    auto end_time = std::chrono::steady_clock::now();
                    result.success = true;
                    result.exit_code = 0;
                    result.up_to_date = true;
                    result.artifact_path = artifact.string();
                    result.object_files = compiled_objects;
                    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                    return result;
                }
            }
        }

        // 2. Link Stage (producing DLL / SO / Dylib and Windows import library)
        auto [lib_dirs, libs] = get_effective_link_dependencies(builder, target, toolchain);
        std::string eff_triple = !target.target_triple().empty() ? target.target_triple() : builder.target_triple();
        std::filesystem::path eff_sysroot = !target.sysroot().empty() ? target.sysroot() : builder.sysroot();

        LinkParams lparams{
            .output_binary = artifact.string(),
            .target_type = TargetType::SharedLibrary,
            .subsystem = target.config().subsystem,
            .object_files = compiled_objects,
            .library_dirs = lib_dirs,
            .libraries = libs,
            .extra_flags = get_effective_link_options(builder, target),
            .import_library = implib.string(),
            .sanitizers = target.config().sanitizers,
            .coverage = target.config().coverage,
            .target_triple = eff_triple,
            .sysroot = eff_sysroot,
        };

        auto link_cmd = toolchain.build_link_command(lparams);
        if (verbose) {
            std::cout << "[cxxgen link shared] " << util::flatten_command(link_cmd) << "\n";
        }

        auto link_start = std::chrono::steady_clock::now();
        ProcessResult l_res = ProcessRunner::run(link_cmd, "", toolchain.env_vars());
        if (profiler) {
            profiler->record("link: " + target.name(), "link", link_start, std::chrono::steady_clock::now(),
                             get_current_thread_id(), l_res.success() ? "Executed" : "Failed", 0,
                             "Shared library artifact");
        }
        if (!l_res.success()) {
            result.success = false;
            result.exit_code = l_res.exit_code != 0 ? l_res.exit_code : 1;
            result.error_message =
                "Linking shared library failed for " + artifact.string() + ":\n" + l_res.combined_output();
            return result;
        }

        auto end_time = std::chrono::steady_clock::now();
        result.success = true;
        result.exit_code = 0;
        result.artifact_path = artifact.string();
        result.object_files = compiled_objects;
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        return result;
    }

    static BuildResult build_plugin(const Builder &builder, const Target &target, const IToolchain &toolchain,
                                    bool verbose = false, size_t thread_count = 0, BuildProfiler *profiler = nullptr) {
        auto start_time = std::chrono::steady_clock::now();
        BuildResult result;

        if (target.type() != TargetType::Plugin) {
            result.success = false;
            result.exit_code = -1;
            result.error_message = "Target '" + target.name() + "' is not a plugin target";
            return result;
        }

        if (target.config().sources.empty()) {
            result.success = false;
            result.exit_code = -1;
            result.error_message = "Plugin target '" + target.name() + "' has no source files to compile";
            return result;
        }

        namespace fs = std::filesystem;
        std::error_code ec;

        // Resolve output artifact path (.dll on Windows, .so on Linux, .dylib on macOS)
        fs::path artifact = get_artifact_path(target, toolchain);
        if (artifact.has_parent_path()) {
            fs::create_directories(artifact.parent_path(), ec);
        }

        // Object directory
        fs::path obj_dir = fs::path(".cxxgen/obj") / target.name();
        fs::create_directories(obj_dir, ec);

        // 1. Compilation Stage (PIC enabled for dynamic modules)
        bool any_recompiled = false;
        auto compiled_objects_opt = compile_target_sources(builder, target, toolchain, obj_dir, true, verbose, result,
                                                           thread_count, any_recompiled, profiler);
        if (!compiled_objects_opt) {
            return result;
        }
        std::vector<std::string> compiled_objects = std::move(*compiled_objects_opt);

        // Build any dependent static or shared libraries
        if (!build_dependent_libraries(builder, target, toolchain, artifact, result, verbose, ec)) {
            return result;
        }

        // Check if plugin is up-to-date
        if (!any_recompiled && fs::exists(artifact, ec)) {
            auto art_time = fs::last_write_time(artifact, ec);
            if (!ec) {
                bool all_older = true;
                for (const auto &obj : compiled_objects) {
                    auto obj_time = fs::last_write_time(obj, ec);
                    if (ec || obj_time > art_time) {
                        all_older = false;
                        break;
                    }
                }
                if (all_older) {
                    for (const auto &dep_name : target.linked_targets()) {
                        if (const Target *dep_t = builder.find_target(dep_name)) {
                            fs::path dep_art = get_artifact_path(*dep_t, toolchain);
                            if (fs::exists(dep_art, ec)) {
                                auto dep_time = fs::last_write_time(dep_art, ec);
                                if (ec || dep_time > art_time) {
                                    all_older = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (all_older) {
                    if (verbose) {
                        std::cout << "[cxxgen link plugin] " << target.name() << " is up to date.\n";
                    }
                    auto end_time = std::chrono::steady_clock::now();
                    result.success = true;
                    result.exit_code = 0;
                    result.up_to_date = true;
                    result.artifact_path = artifact.string();
                    result.object_files = compiled_objects;
                    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                    return result;
                }
            }
        }

        // 2. Link Stage (producing standalone loadable module; no import library required)
        auto [lib_dirs, libs] = get_effective_link_dependencies(builder, target, toolchain);
        std::string eff_triple = !target.target_triple().empty() ? target.target_triple() : builder.target_triple();
        std::filesystem::path eff_sysroot = !target.sysroot().empty() ? target.sysroot() : builder.sysroot();

        LinkParams lparams{
            .output_binary = artifact.string(),
            .target_type = TargetType::Plugin,
            .subsystem = target.config().subsystem,
            .object_files = compiled_objects,
            .library_dirs = lib_dirs,
            .libraries = libs,
            .extra_flags = get_effective_link_options(builder, target),
            .import_library = "",
            .sanitizers = target.config().sanitizers,
            .coverage = target.config().coverage,
            .target_triple = eff_triple,
            .sysroot = eff_sysroot,
        };

        auto link_cmd = toolchain.build_link_command(lparams);
        if (verbose) {
            std::cout << "[cxxgen link plugin] " << util::flatten_command(link_cmd) << "\n";
        }

        auto link_start = std::chrono::steady_clock::now();
        ProcessResult l_res = ProcessRunner::run(link_cmd, "", toolchain.env_vars());
        if (profiler) {
            profiler->record("link: " + target.name(), "link", link_start, std::chrono::steady_clock::now(),
                             get_current_thread_id(), l_res.success() ? "Executed" : "Failed", 0, "Plugin artifact");
        }
        if (!l_res.success()) {
            result.success = false;
            result.exit_code = l_res.exit_code != 0 ? l_res.exit_code : 1;
            result.error_message = "Linking plugin failed for " + artifact.string() + ":\n" + l_res.combined_output();
            return result;
        }

        auto end_time = std::chrono::steady_clock::now();
        result.success = true;
        result.exit_code = 0;
        result.artifact_path = artifact.string();
        result.object_files = compiled_objects;
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        return result;
    }

    static BuildResult build_executable(const Builder &builder, const Target &target, const IToolchain &toolchain,
                                        bool verbose = false, size_t thread_count = 0,
                                        BuildProfiler *profiler = nullptr) {
        auto start_time = std::chrono::steady_clock::now();
        BuildResult result;

        if (target.type() != TargetType::Executable && target.type() != TargetType::Test) {
            result.success = false;
            result.exit_code = -1;
            result.error_message = "Target '" + target.name() + "' is not an executable or test target";
            return result;
        }

        if (target.config().sources.empty()) {
            result.success = false;
            result.exit_code = -1;
            result.error_message = "Target '" + target.name() + "' has no source files to compile";
            return result;
        }

        namespace fs = std::filesystem;
        std::error_code ec;

        // Resolve output artifact path
        fs::path artifact = get_artifact_path(target, toolchain);
        if (artifact.has_parent_path()) {
            fs::create_directories(artifact.parent_path(), ec);
        }

        // Object directory
        fs::path obj_dir = fs::path(".cxxgen/obj") / target.name();
        fs::create_directories(obj_dir, ec);

        // 1. Compilation Stage
        bool any_recompiled = false;
        auto compiled_objects_opt = compile_target_sources(builder, target, toolchain, obj_dir, false, verbose, result,
                                                           thread_count, any_recompiled, profiler);
        if (!compiled_objects_opt) {
            return result;
        }
        std::vector<std::string> compiled_objects = std::move(*compiled_objects_opt);

        // Build any dependent static or shared libraries
        if (!build_dependent_libraries(builder, target, toolchain, artifact, result, verbose, ec)) {
            return result;
        }

        // Check if executable binary is up-to-date
        if (!any_recompiled && fs::exists(artifact, ec)) {
            auto art_time = fs::last_write_time(artifact, ec);
            if (!ec) {
                bool all_older = true;
                for (const auto &obj : compiled_objects) {
                    auto obj_time = fs::last_write_time(obj, ec);
                    if (ec || obj_time > art_time) {
                        all_older = false;
                        break;
                    }
                }
                if (all_older) {
                    for (const auto &dep_name : target.linked_targets()) {
                        if (const Target *dep_t = builder.find_target(dep_name)) {
                            fs::path dep_art = get_artifact_path(*dep_t, toolchain);
                            if (fs::exists(dep_art, ec)) {
                                auto dep_time = fs::last_write_time(dep_art, ec);
                                if (ec || dep_time > art_time) {
                                    all_older = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (all_older) {
                    if (verbose) {
                        std::cout << "[cxxgen link] " << target.name() << " is up to date.\n";
                    }
                    auto end_time = std::chrono::steady_clock::now();
                    result.success = true;
                    result.exit_code = 0;
                    result.up_to_date = true;
                    result.artifact_path = artifact.string();
                    result.object_files = compiled_objects;
                    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                    return result;
                }
            }
        }

#if defined(_WIN32)
        // Check if destination artifact file is currently locked/opened by another process (e.g. self-hosting CLI
        // runner)
        bool is_locked = false;
        if (fs::exists(artifact, ec)) {
            HANDLE h = CreateFileW(artifact.wstring().c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL, nullptr);
            if (h == INVALID_HANDLE_VALUE) {
                DWORD err = GetLastError();
                if (err == ERROR_SHARING_VIOLATION || err == ERROR_LOCK_VIOLATION || err == ERROR_ACCESS_DENIED) {
                    // Attempt Windows atomic hot-swap by renaming locked file to .old
                    fs::path old_file = artifact.string() + ".old";
                    fs::remove(old_file, ec);
                    fs::rename(artifact, old_file, ec);
                    if (ec) {
                        is_locked = true;
                    }
                }
            } else {
                CloseHandle(h);
            }
        }
        if (is_locked) {
            if (verbose) {
                std::cout << "[cxxgen link] " << target.name() << " (" << artifact.string()
                          << ") is currently locked by a running process; skipping re-link.\n";
            }
            auto end_time = std::chrono::steady_clock::now();
            result.success = true;
            result.exit_code = 0;
            result.up_to_date = true;
            result.artifact_path = artifact.string();
            result.object_files = compiled_objects;
            result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            return result;
        }
#endif

        // 2. Link Stage
        auto [lib_dirs, libs] = get_effective_link_dependencies(builder, target, toolchain);
        std::string eff_triple = !target.target_triple().empty() ? target.target_triple() : builder.target_triple();
        std::filesystem::path eff_sysroot = !target.sysroot().empty() ? target.sysroot() : builder.sysroot();

        LinkParams lparams{
            .output_binary = artifact.string(),
            .target_type = target.type(),
            .subsystem = target.config().subsystem,
            .object_files = compiled_objects,
            .library_dirs = lib_dirs,
            .libraries = libs,
            .extra_flags = get_effective_link_options(builder, target),
            .sanitizers = target.config().sanitizers,
            .coverage = target.config().coverage,
            .target_triple = eff_triple,
            .sysroot = eff_sysroot,
        };

        auto link_cmd = toolchain.build_link_command(lparams);
        if (verbose) {
            std::cout << "[cxxgen link] " << util::flatten_command(link_cmd) << "\n";
        }

        auto link_start = std::chrono::steady_clock::now();
        ProcessResult l_res = ProcessRunner::run(link_cmd, "", toolchain.env_vars());
        if (profiler) {
            profiler->record("link: " + target.name(), "link", link_start, std::chrono::steady_clock::now(),
                             get_current_thread_id(), l_res.success() ? "Executed" : "Failed", 0,
                             "Executable artifact");
        }
        if (!l_res.success()) {
            result.success = false;
            result.exit_code = l_res.exit_code != 0 ? l_res.exit_code : 1;
            result.error_message = "Linking failed for " + artifact.string() + ":\n" + l_res.combined_output();
            return result;
        }

        auto end_time = std::chrono::steady_clock::now();
        result.success = true;
        result.exit_code = 0;
        result.artifact_path = artifact.string();
        result.object_files = compiled_objects;
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        return result;
    }

    static BuildResult build_interface_library(const Builder &builder, const Target &target,
                                               const IToolchain &toolchain, bool verbose = false,
                                               size_t thread_count = 0) {
        (void)builder;
        (void)toolchain;
        (void)verbose;
        (void)thread_count;
        BuildResult result;
        if (target.type() != TargetType::Interface) {
            result.success = false;
            result.exit_code = -1;
            result.error_message = "Target '" + target.name() + "' is not an interface library target";
            return result;
        }

        // Interface targets are header-only: emit no compilation or link commands
        result.success = true;
        result.exit_code = 0;
        result.artifact_path = "";
        result.duration = std::chrono::milliseconds(0);
        return result;
    }

    static BuildResult build_target(const Builder &builder, const Target &target, const IToolchain &toolchain,
                                    bool verbose = false, size_t thread_count = 0, BuildProfiler *profiler = nullptr) {
        switch (target.type()) {
        case TargetType::Executable:
        case TargetType::Test:
            return build_executable(builder, target, toolchain, verbose, thread_count, profiler);
        case TargetType::StaticLibrary:
            return build_static_library(builder, target, toolchain, verbose, thread_count, profiler);
        case TargetType::SharedLibrary:
            return build_shared_library(builder, target, toolchain, verbose, thread_count, profiler);
        case TargetType::Plugin:
            return build_plugin(builder, target, toolchain, verbose, thread_count, profiler);
        case TargetType::Interface:
            return build_interface_library(builder, target, toolchain, verbose, thread_count);
        default:
            BuildResult res;
            res.success = false;
            res.exit_code = -1;
            res.error_message = "Pipeline for target type not yet implemented";
            return res;
        }
    }
};

inline std::filesystem::path Builder::get_artifact_path(const Target &target, const IToolchain &toolchain) const {
    return CompilationPipeline::get_artifact_path(target, toolchain);
}

inline std::filesystem::path Builder::get_import_library_path(const Target &target, const IToolchain &toolchain) const {
    return CompilationPipeline::get_import_library_path(target, toolchain);
}

inline BuildResult Builder::build_target(const Target &target, const IToolchain &toolchain, bool verbose,
                                         size_t thread_count) {
    return CompilationPipeline::build_target(*this, target, toolchain, verbose, thread_count);
}

inline BuildResult Builder::build_target(std::string_view target_name, const IToolchain &toolchain, bool verbose,
                                         size_t thread_count) {
    const Target *t = find_target(target_name);
    if (!t) {
        BuildResult res;
        res.success = false;
        res.exit_code = -1;
        res.error_message = "Target '" + std::string(target_name) + "' not found in builder";
        return res;
    }
    return build_target(*t, toolchain, verbose, thread_count);
}

inline std::filesystem::path Target::artifact_path(const IToolchain &toolchain) const {
    return CompilationPipeline::get_artifact_path(*this, toolchain);
}

inline StepResult TargetBuildStep::execute(ExecutionContext &ctx) {
    auto start = std::chrono::steady_clock::now();
    auto res = CompilationPipeline::build_target(builder_, target_, ctx.toolchain, ctx.verbose,
                                                 ctx.effective_thread_count(), ctx.profiler.get());
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    if (!res.success) {
        return StepResult::fail(res.error_message, res.exit_code != 0 ? res.exit_code : 1, dur);
    }
    if (res.up_to_date) {
        return StepResult::up_to_date(dur);
    }
    return StepResult::ok(dur);
}

inline bool CompileBMIStep::is_up_to_date() const {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::exists(bmi_output_, ec) || !fs::exists(obj_output_, ec)) {
        return false;
    }
    if (!fs::exists(source_path_, ec)) {
        return false;
    }
    auto src_time = fs::last_write_time(source_path_, ec);
    auto bmi_time = fs::last_write_time(bmi_output_, ec);
    auto obj_time = fs::last_write_time(obj_output_, ec);
    if (ec || bmi_time < src_time || obj_time < src_time) {
        return false;
    }
    for (const auto &[dep_m, dep_p] : imported_bmis_) {
        if (!fs::exists(dep_p, ec))
            return false;
        auto dep_time = fs::last_write_time(dep_p, ec);
        if (ec || bmi_time < dep_time)
            return false;
    }
    return true;
}

inline StepResult CompileBMIStep::execute(ExecutionContext &ctx) {
    namespace fs = std::filesystem;
    std::error_code ec;
    auto start_time = std::chrono::steady_clock::now();

    if (is_up_to_date()) {
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
        return StepResult::up_to_date(dur);
    }

    if (bmi_output_.has_parent_path())
        fs::create_directories(bmi_output_.parent_path(), ec);
    if (obj_output_.has_parent_path())
        fs::create_directories(obj_output_.parent_path(), ec);

    std::string key = modules::BmiCache::compute_key(source_path_, ctx.toolchain.compiler_path(), compile_flags_);
    if (target_.is_global_cache_enabled() && modules::BmiCache::instance().try_restore(key, bmi_output_, obj_output_)) {
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
        return StepResult::ok(dur);
    }

    bool success = false;
    std::string output;
    if (ctx.toolchain.family() == CompilerFamily::Msvc) {
        CompileParams cp;
        cp.source_file = source_path_.string();
        cp.object_file = obj_output_.string();
        cp.module_interface_output = bmi_output_.string();
        cp.is_module_interface = true;
        cp.cpp_version =
            (target_.config().cpp_version < CppVersion::Cpp20) ? CppVersion::Cpp20 : target_.config().cpp_version;
        if (imported_bmis_.find("std") != imported_bmis_.end()) {
            cp.cpp_version = CppVersion::Cpp23;
        }
        cp.optimize = target_.config().optimize;
        cp.exceptions = target_.config().exceptions;
        cp.rtti = target_.config().rtti;
        cp.include_dirs = target_.config().include_dirs;
        cp.defines = target_.config().defines;
        cp.extra_flags = compile_flags_;
        cp.module_references = imported_bmis_;
        auto cmd = ctx.toolchain.build_compile_command(cp);
        auto res = ProcessRunner::run(cmd, "", ctx.toolchain.env_vars());
        success = res.success();
        output = res.combined_output();
    } else if (ctx.toolchain.family() == CompilerFamily::Clang) {
        fs::path actual_bmi = bmi_output_;
        if (actual_bmi.extension() != ".pcm") {
            actual_bmi.replace_extension(".pcm");
        }
        std::string cpp_std = "-std=c++20";
        if (target_.config().cpp_version == CppVersion::Cpp23)
            cpp_std = "-std=c++23";
        else if (target_.config().cpp_version == CppVersion::Cpp26)
            cpp_std = "-std=c++26";
        std::vector<std::string> pre_cmd = {ctx.toolchain.compiler_path(), cpp_std};
        for (const auto &inc : target_.config().include_dirs)
            pre_cmd.push_back("-I" + util::normalize_path(inc, false));
        for (const auto &d : target_.config().defines)
            pre_cmd.push_back("-D" + d);
        for (const auto &[m, p] : imported_bmis_) {
            fs::path dep_bmi = p;
            if (dep_bmi.extension() != ".pcm")
                dep_bmi.replace_extension(".pcm");
            pre_cmd.push_back("-fmodule-file=" + m + "=" + util::normalize_path(dep_bmi.string(), false));
        }
        for (const auto &opt : compile_flags_)
            pre_cmd.push_back(opt);
        pre_cmd.push_back("-x");
        pre_cmd.push_back("c++-module");
        pre_cmd.push_back("--precompile");
        pre_cmd.push_back(util::normalize_path(source_path_.string(), false));
        pre_cmd.push_back("-o");
        pre_cmd.push_back(util::normalize_path(actual_bmi.string(), false));

        if (ctx.verbose) {
            std::cout << "[cxxgen bmi] " << util::flatten_command(pre_cmd) << "\n";
        }
        auto res1 = ProcessRunner::run(pre_cmd, "", ctx.toolchain.env_vars());
        if (res1.success()) {
            std::vector<std::string> obj_cmd = {ctx.toolchain.compiler_path(), "-c", "-x", "pcm"};
            for (const auto &[m, p] : imported_bmis_) {
                fs::path dep_bmi = p;
                if (dep_bmi.extension() != ".pcm")
                    dep_bmi.replace_extension(".pcm");
                obj_cmd.push_back("-fmodule-file=" + m + "=" + util::normalize_path(dep_bmi.string(), false));
            }
            obj_cmd.push_back(util::normalize_path(actual_bmi.string(), false));
            obj_cmd.push_back("-o");
            obj_cmd.push_back(util::normalize_path(obj_output_.string(), false));
            if (ctx.verbose) {
                std::cout << "[cxxgen bmi obj] " << util::flatten_command(obj_cmd) << "\n";
            }
            auto res2 = ProcessRunner::run(obj_cmd, "", ctx.toolchain.env_vars());
            success = res2.success();
            output = res2.combined_output();
            if (!success) {
                output = "Command: " + util::flatten_command(obj_cmd) + "\n" + output;
            }
        } else {
            success = false;
            output = "Command: " + util::flatten_command(pre_cmd) + "\n" + res1.combined_output();
        }
    } else {
        CompileParams cp;
        cp.source_file = source_path_.string();
        cp.object_file = obj_output_.string();
        cp.is_module_interface = true;
        cp.cpp_version = CppVersion::Cpp20;
        cp.include_dirs = target_.config().include_dirs;
        cp.defines = target_.config().defines;
        cp.extra_flags = compile_flags_;
        auto cmd = ctx.toolchain.build_compile_command(cp);
        auto res = ProcessRunner::run(cmd, "", ctx.toolchain.env_vars());
        success = res.success();
        output = res.combined_output();
    }

    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
    if (!success) {
        return StepResult::fail("BMI compilation failed for module " + module_name_ + ":\n" + output, 1, dur);
    }

    if (target_.is_global_cache_enabled()) {
        modules::BmiCache::instance().store(key, bmi_output_, obj_output_);
    }

    return StepResult::ok(dur);
}

inline StepResult RunArtifactStep::execute(ExecutionContext &ctx) {
    std::filesystem::path artifact = target_.artifact_path(ctx.toolchain);
    if (!std::filesystem::exists(artifact)) {
        return StepResult::fail("Artifact not found: " + artifact.string());
    }
    std::vector<std::string> cmd;
    cmd.push_back(artifact.string());
    for (const auto &arg : args_) {
        cmd.push_back(arg);
    }
    if (ctx.verbose) {
        std::cout << "[cxxgen run] " << util::flatten_command(cmd) << "\n";
    }
    auto start = std::chrono::steady_clock::now();
    ProcessOptions popts;
    popts.command = cmd;
    popts.stream_to_terminal = true;
    if (ctx.coverage || target_.has_coverage()) {
        std::error_code ec;
        auto cov_dir = std::filesystem::absolute(".cxxgen/coverage", ec);
        std::filesystem::create_directories(cov_dir, ec);
        popts.extra_env["LLVM_PROFILE_FILE"] = (cov_dir / (target_.name() + ".profraw")).string();
    }
    auto res = ProcessRunner::run(popts);
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    if (!res.success()) {
        return StepResult::fail("Process failed with exit code " + std::to_string(res.exit_code), res.exit_code, dur);
    }
    return StepResult::ok(dur);
}

inline bool CustomCommandStep::is_up_to_date() const {
    namespace fs = std::filesystem;
    std::error_code ec;

    if (outputs_.empty()) {
        return false;
    }

    fs::file_time_type oldest_output = fs::file_time_type::max();
    for (const auto &out : outputs_) {
        fs::path p(out);
        if (!fs::exists(p, ec)) {
            return false;
        }
        auto t = fs::last_write_time(p, ec);
        if (ec) {
            return false;
        }
        if (t < oldest_output) {
            oldest_output = t;
        }
    }

    if (inputs_.empty()) {
        return true;
    }

    for (const auto &in : inputs_) {
        fs::path p(in);
        if (!fs::exists(p, ec)) {
            return false;
        }
        auto t = fs::last_write_time(p, ec);
        if (ec) {
            return false;
        }
        if (t > oldest_output) {
            return false;
        }
    }

    return true;
}

inline StepResult CustomCommandStep::execute(ExecutionContext &ctx) {
    if (is_up_to_date()) {
        if (ctx.verbose) {
            std::cout << "[cxxgen custom_command] " << name() << " is up to date.\n";
        }
        return StepResult::up_to_date();
    }

    if (command_.empty()) {
        return StepResult::fail("Custom command step '" + name() + "' has empty command");
    }

    namespace fs = std::filesystem;
    std::error_code ec;
    for (const auto &out : outputs_) {
        fs::path p(out);
        if (p.has_parent_path()) {
            fs::create_directories(p.parent_path(), ec);
        }
    }

    if (ctx.verbose) {
        std::cout << "[cxxgen custom_command] " << util::flatten_command(command_) << "\n";
    }

    auto start = std::chrono::steady_clock::now();
    ProcessOptions popts;
    popts.command = command_;
    if (!working_directory_.empty()) {
        popts.working_directory = working_directory_.string();
    }
    popts.extra_env = env_vars_;
    popts.stream_to_terminal = ctx.verbose;

    ProcessResult res = ProcessRunner::run(popts);
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);

    if (!res.success()) {
        return StepResult::fail("Custom command failed for '" + name() + "' with exit code " +
                                    std::to_string(res.exit_code) + ":\n" + res.combined_output(),
                                res.exit_code != 0 ? res.exit_code : 1, dur);
    }

    for (const auto &out : outputs_) {
        if (!fs::exists(out, ec)) {
            return StepResult::fail(
                "Custom command for '" + name() + "' succeeded but expected output was not produced: " + out, 1, dur);
        }
    }

    return StepResult::ok(dur);
}

// ----------------------------------------------------------------------------
// EmbedFileStep Implementation
// ----------------------------------------------------------------------------

inline bool EmbedFileStep::is_up_to_date() const {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::exists(header_dest_, ec))
        return false;
    if (!fs::exists(source_path_, ec))
        return false;
    auto in_time = fs::last_write_time(source_path_, ec);
    if (ec)
        return false;
    auto out_time = fs::last_write_time(header_dest_, ec);
    if (ec)
        return false;
    return out_time >= in_time;
}

inline bool EmbedFileStep::embed(const std::filesystem::path &source_path, const std::filesystem::path &header_dest,
                                 const EmbedFileOptions &options, std::string &error_msg) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (util::is_system_protected_dir(header_dest)) {
        error_msg = "Cannot write embedded header to protected system directory: " + header_dest.string();
        return false;
    }
    if (!fs::exists(source_path, ec)) {
        error_msg = "Embedded file source does not exist: " + source_path.string();
        return false;
    }

    std::ifstream in(source_path, std::ios::binary);
    if (!in) {
        error_msg = "Failed to open source file for embedding: " + source_path.string();
        return false;
    }

    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    std::string var = util::sanitize_identifier(options.variable_name.empty() ? source_path.filename().string()
                                                                              : options.variable_name);

    std::string sanitized_ns;
    if (!options.namespace_name.empty()) {
        std::vector<std::string> parts = util::split(options.namespace_name, "::");
        std::vector<std::string> clean_parts;
        for (const auto &p : parts) {
            std::string clean = util::sanitize_identifier(p);
            if (!clean.empty())
                clean_parts.push_back(clean);
        }
        sanitized_ns = util::join(clean_parts, "::");
    }

    std::string safe_src = util::normalize_path(source_path.string(), false);
    for (char &c : safe_src) {
        if (c == '\r' || c == '\n')
            c = ' ';
    }

    std::ostringstream out;
    out << "// =============================================================================\n"
        << "// Automatically generated by cxxgen embed_file. Do not modify manually.\n"
        << "// Source: " << safe_src << "\n"
        << "// =============================================================================\n\n"
        << "#pragma once\n\n"
        << "#include <cstdint>\n"
        << "#include <cstddef>\n"
        << "#include <array>\n"
        << "#include <string_view>\n\n";

    if (!sanitized_ns.empty()) {
        out << "namespace " << sanitized_ns << " {\n\n";
    }

    size_t orig_size = bytes.size();
    size_t array_len = orig_size + (options.null_terminate ? 1 : 0);
    size_t alloc_len = (array_len == 0) ? 1 : array_len;

    if (options.as_constexpr) {
        out << "inline constexpr size_t " << var << "_size = " << orig_size << ";\n"
            << "inline constexpr std::array<uint8_t, " << array_len << "> " << var << " = {";
    } else {
        out << "inline const size_t " << var << "_size = " << orig_size << ";\n"
            << "inline const uint8_t " << var << "[" << alloc_len << "] = {";
    }

    if (array_len > 0) {
        out << "\n";
        for (size_t i = 0; i < bytes.size(); ++i) {
            if (i % 16 == 0)
                out << "    ";
            out << "0x";
            uint8_t b = bytes[i];
            const char *hex = "0123456789abcdef";
            out << hex[(b >> 4) & 0x0F] << hex[b & 0x0F];
            if (i + 1 < array_len)
                out << ", ";
            if (i % 16 == 15 || i + 1 == bytes.size()) {
                if (i + 1 == bytes.size() && options.null_terminate) {
                    // will write null byte on next line or same line
                } else {
                    out << "\n";
                }
            }
        }
        if (options.null_terminate) {
            if (bytes.size() % 16 == 0)
                out << "    ";
            out << "0x00\n";
        }
    }
    out << "};\n\n";

    std::string data_ptr = options.as_constexpr ? (var + ".data()") : var;
    out << "inline std::string_view " << var << "_string_view() noexcept {\n"
        << "    return std::string_view(reinterpret_cast<const char*>(" << data_ptr << "), " << var << "_size);\n"
        << "}\n\n";

    if (options.null_terminate) {
        out << "inline const char* " << var << "_c_str() noexcept {\n"
            << "    return reinterpret_cast<const char*>(" << data_ptr << ");\n"
            << "}\n\n";
    }

    if (!sanitized_ns.empty()) {
        out << "} // namespace " << sanitized_ns << "\n";
    }

    if (header_dest.has_parent_path()) {
        fs::create_directories(header_dest.parent_path(), ec);
    }

    std::ofstream out_file(header_dest);
    if (!out_file) {
        error_msg = "Failed to write embedded header destination: " + header_dest.string();
        return false;
    }

    out_file << out.str();
    out_file.close();
    return true;
}

inline StepResult EmbedFileStep::execute(ExecutionContext &ctx) {
    namespace fs = std::filesystem;
    auto start = std::chrono::steady_clock::now();

    if (is_up_to_date()) {
        if (ctx.verbose) {
            std::cout << "[cxxgen embed] " << name() << " is up to date.\n";
        }
        return StepResult::up_to_date();
    }

    std::string err;
    if (!embed(source_path_, header_dest_, options_, err)) {
        return StepResult::fail(err);
    }

    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    if (ctx.verbose) {
        std::cout << "[cxxgen embed] Generated: " << header_dest_.string() << "\n";
    }
    return StepResult::ok(dur);
}

inline void Builder::resolve_target_dependencies() {
    namespace fs = std::filesystem;
    for (const auto &t : targets_) {
        if (!t || !t->build_step())
            continue;

        // 1. Linked targets
        for (const auto &dep_name : t->linked_targets()) {
            if (const Target *dep_t = find_target(dep_name)) {
                if (dep_t->build_step()) {
                    t->build_step()->depend_on(dep_t->build_step());
                }
            }
        }

        // 2. Custom command & embed file dependencies
        for (const auto &s : steps_) {
            std::vector<std::string> step_outputs;
            if (auto custom_step = std::dynamic_pointer_cast<CustomCommandStep>(s)) {
                step_outputs = custom_step->outputs();
            } else if (auto embed_step = std::dynamic_pointer_cast<EmbedFileStep>(s)) {
                step_outputs = embed_step->outputs();
            } else {
                continue;
            }

            bool matches = false;
            for (const auto &out_file : step_outputs) {
                fs::path norm_out = fs::path(out_file).lexically_normal();
                for (const auto &src : t->config().sources) {
                    if (fs::path(src).lexically_normal() == norm_out) {
                        matches = true;
                        break;
                    }
                }
                if (matches)
                    break;
                for (const auto &hdr : t->config().headers) {
                    if (fs::path(hdr).lexically_normal() == norm_out) {
                        matches = true;
                        break;
                    }
                }
                if (matches)
                    break;
            }

            if (matches) {
                t->build_step()->depend_on(s);
            }
        }

        // 3. Modular compilation & BMI steps (Epic 12)
        auto mod_graph = modules::ModuleScanner::scan_target(*t);
        if (mod_graph.has_cycle) {
            throw CyclicDependencyException(mod_graph.cycle_path);
        }
        if (mod_graph.has_modules && !mod_graph.sorted_interfaces.empty()) {
            std::filesystem::path obj_dir =
                t->config().output_dir.empty() ? ("obj/" + t->name()) : t->config().output_dir;
            std::string bmi_ext = ".ifc";
            std::string obj_ext = ".obj";
            auto default_tc = find_toolchain(CompilerFamily::Auto, standard_target_options());
            if (default_tc) {
                if (default_tc->family() == CompilerFamily::Clang) {
                    bmi_ext = ".pcm";
                    obj_ext = ".o";
                } else if (default_tc->family() == CompilerFamily::Gcc) {
                    bmi_ext = ".gcm";
                    obj_ext = ".o";
                }
            }
            std::unordered_map<std::string, std::shared_ptr<CompileBMIStep>> bmi_steps;
            for (const auto &iface_src : mod_graph.sorted_interfaces) {
                std::string mod_name = mod_graph.source_to_module[iface_src];
                std::string safe_name = modules::ModuleScanner::sanitize_module_name(mod_name);
                fs::path bmi_p = obj_dir / "bmi" / (safe_name + bmi_ext);
                fs::path obj_p = obj_dir / (safe_name + obj_ext);

                std::string step_name = "bmi:" + mod_name;
                std::shared_ptr<CompileBMIStep> bmi_step;
                if (auto existing_s = find_step_ptr(step_name)) {
                    bmi_step = std::dynamic_pointer_cast<CompileBMIStep>(existing_s);
                }
                if (!bmi_step) {
                    bmi_step = std::make_shared<CompileBMIStep>(mod_name, iface_src, bmi_p, obj_p, *t);
                    bmi_step->set_builder(this);
                    add_step(bmi_step);
                }
                bmi_steps[mod_name] = bmi_step;
                t->set_module_bmi(mod_name, bmi_p);
                t->build_step()->depend_on(bmi_step);
            }
            // Wire dependencies between BMI steps
            for (const auto &[m_name, deps] : mod_graph.module_dependencies) {
                auto it_step = bmi_steps.find(m_name);
                if (it_step != bmi_steps.end()) {
                    for (const auto &dep_m : deps) {
                        auto it_dep = bmi_steps.find(dep_m);
                        if (it_dep != bmi_steps.end()) {
                            it_step->second->depend_on(it_dep->second);
                            it_step->second->add_imported_bmi(dep_m, it_dep->second->bmi_output().string());
                        }
                    }
                }
            }
        }
    }
}

inline std::vector<std::shared_ptr<Step>> Builder::topological_sort(std::string_view root_step_name) {
    resolve_target_dependencies();
    if (!root_step_name.empty()) {
        auto s = find_step_ptr(root_step_name);
        if (!s) {
            if (const Target *t = find_target(root_step_name)) {
                s = t->build_step();
            }
        }
        if (!s) {
            throw std::runtime_error("cxxgen error: step or target '" + std::string(root_step_name) + "' not found");
        }
        return BuildGraph::topological_sort(s);
    }

    if (auto install_s = find_step_ptr("install")) {
        return BuildGraph::topological_sort(install_s);
    }

    return BuildGraph::topological_sort(steps_);
}

inline StepResult Builder::execute_step(std::string_view step_name, ExecutionContext &ctx) {
    auto sorted = topological_sort(step_name);
    return ParallelScheduler::execute(sorted, ctx);
}

inline StepResult Builder::execute_all(ExecutionContext &ctx) {
    auto sorted = topological_sort("");
    return ParallelScheduler::execute(sorted, ctx);
}

// ============================================================================
// 8. Multi-Format Project Generators (CMake, MSVC, Ninja, CompileCommands)
// ============================================================================

class CMakeGenerator {
public:
    explicit CMakeGenerator(const Builder &builder, std::string project_name = "")
        : builder_(builder), project_name_(std::move(project_name)) {
        if (project_name_.empty()) {
            project_name_ = builder_.project_name();
        }
        if (project_name_.empty()) {
            for (const auto &t : builder_.targets()) {
                if (t->type() == TargetType::Executable) {
                    project_name_ = t->name();
                    break;
                }
            }
        }
        if (project_name_.empty() && !builder_.targets().empty()) {
            project_name_ = builder_.targets().front()->name();
        }
        if (project_name_.empty()) {
            project_name_ = "CxxgenProject";
        }
    }

    [[nodiscard]] std::string generate() const {
        std::string safe_project_name;
        for (char c : project_name_) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-') {
                safe_project_name.push_back(c);
            } else {
                safe_project_name.push_back('_');
            }
        }
        if (safe_project_name.empty()) {
            safe_project_name = "CxxgenProject";
        }

        std::ostringstream out;
        out << "# ==============================================================================\n"
            << "# Generated by cxxgen (C++ Generator & Build System)\n"
            << "# Standard: CMake 3.20+ | Target Language: C / C++\n"
            << "# ==============================================================================\n\n"
            << "cmake_minimum_required(VERSION 3.20)\n"
            << "project(" << safe_project_name << " LANGUAGES CXX C)\n\n";

        // Global settings / CTest
        bool has_tests = false;
        for (const auto &t : builder_.targets()) {
            if (t->type() == TargetType::Test) {
                has_tests = true;
                break;
            }
        }
        if (has_tests) {
            out << "enable_testing()\n\n";
        }

        // Custom commands from builder steps
        for (const auto &step : builder_.steps()) {
            auto custom_cmd = std::dynamic_pointer_cast<CustomCommandStep>(step);
            if (custom_cmd && !custom_cmd->outputs().empty()) {
                out << "# Custom Command: " << custom_cmd->name() << "\n"
                    << "add_custom_command(\n"
                    << "    OUTPUT";
                for (const auto &out_file : custom_cmd->outputs()) {
                    out << " \"" << util::normalize_path(out_file, false) << "\"";
                }
                out << "\n    COMMAND";
                for (const auto &cmd_arg : custom_cmd->command()) {
                    out << " \"" << util::normalize_path(cmd_arg, false) << "\"";
                }
                if (!custom_cmd->inputs().empty()) {
                    out << "\n    DEPENDS";
                    for (const auto &inp : custom_cmd->inputs()) {
                        out << " \"" << util::normalize_path(inp, false) << "\"";
                    }
                }
                if (!custom_cmd->working_directory().empty()) {
                    out << "\n    WORKING_DIRECTORY \""
                        << util::normalize_path(custom_cmd->working_directory().string(), false) << "\"";
                }
                out << "\n    COMMENT \"" << custom_cmd->description() << "\"\n"
                    << "    VERBATIM\n"
                    << ")\n\n";
            }
        }

        // Targets
        for (const auto &target_ptr : builder_.targets()) {
            const auto &target = *target_ptr;
            const auto &cfg = target.config();
            std::string scope =
                (target.type() == TargetType::Interface) ? "INTERFACE"
                : (target.type() == TargetType::StaticLibrary || target.type() == TargetType::SharedLibrary)
                    ? "PUBLIC"
                    : "PRIVATE";

            out << "# ==============================================================================\n"
                << "# Target: " << target.name() << "\n"
                << "# ==============================================================================\n";

            if (target.type() == TargetType::Interface) {
                out << "add_library(" << target.name() << " INTERFACE)\n";
            } else {
                switch (target.type()) {
                case TargetType::Executable:
                case TargetType::Test:
                    out << "add_executable(" << target.name();
                    break;
                case TargetType::StaticLibrary:
                    out << "add_library(" << target.name() << " STATIC";
                    break;
                case TargetType::SharedLibrary:
                    out << "add_library(" << target.name() << " SHARED";
                    break;
                case TargetType::Plugin:
                    out << "add_library(" << target.name() << " MODULE";
                    break;
                case TargetType::Interface:
                    break;
                }

                for (const auto &src : cfg.sources) {
                    out << "\n    \"" << util::normalize_path(src, false) << "\"";
                }
                out << "\n)\n";
            }

            if (target.type() == TargetType::Test) {
                out << "add_test(NAME " << target.name() << " COMMAND " << target.name() << ")\n";
            }

            // Include directories
            if (!cfg.include_dirs.empty()) {
                out << "target_include_directories(" << target.name() << " " << scope;
                for (const auto &inc : cfg.include_dirs) {
                    out << "\n    \"" << util::normalize_path(inc, false) << "\"";
                }
                out << "\n)\n";
            }

            // Defines
            if (!cfg.defines.empty()) {
                out << "target_compile_definitions(" << target.name() << " " << scope;
                for (const auto &def : cfg.defines) {
                    out << "\n    \"" << util::escape_cmake_string(def) << "\"";
                }
                out << "\n)\n";
            }

            // Compile features (C++ standard)
            std::string cpp_feat = "cxx_std_20";
            switch (cfg.cpp_version) {
            case CppVersion::Cpp11:
                cpp_feat = "cxx_std_11";
                break;
            case CppVersion::Cpp14:
                cpp_feat = "cxx_std_14";
                break;
            case CppVersion::Cpp17:
                cpp_feat = "cxx_std_17";
                break;
            case CppVersion::Cpp20:
                cpp_feat = "cxx_std_20";
                break;
            case CppVersion::Cpp23:
                cpp_feat = "cxx_std_23";
                break;
            case CppVersion::Cpp26:
                cpp_feat = "cxx_std_26";
                break;
            }
            out << "target_compile_features(" << target.name() << " " << scope << " " << cpp_feat << ")\n";

            // Compile options
            if (!cfg.compile_options.empty() || cfg.warnings_as_errors || !cfg.exceptions || !cfg.rtti) {
                out << "target_compile_options(" << target.name() << " " << scope;
                if (cfg.warnings_as_errors) {
                    out << "\n    $<$<CXX_COMPILER_ID:MSVC>:/WX>"
                        << "\n    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>";
                }
                if (!cfg.exceptions) {
                    out << "\n    $<$<CXX_COMPILER_ID:MSVC>:/EHs-c->"
                        << "\n    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fno-exceptions>";
                }
                if (!cfg.rtti) {
                    out << "\n    $<$<CXX_COMPILER_ID:MSVC>:/GR->"
                        << "\n    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fno-rtti>";
                }
                for (const auto &opt : cfg.compile_options) {
                    out << "\n    \"" << util::escape_cmake_string(opt) << "\"";
                }
                out << "\n)\n";
            }

            // Link directories
            if (!cfg.link_directories.empty()) {
                out << "target_link_directories(" << target.name() << " " << scope;
                for (const auto &ld : cfg.link_directories) {
                    out << "\n    \"" << util::normalize_path(ld, false) << "\"";
                }
                out << "\n)\n";
            }

            // Link libraries (internal targets and external system libraries)
            bool has_links =
                !target.linked_targets().empty() || !target.linked_system_libs().empty() || !cfg.link_libraries.empty();
            if (has_links) {
                out << "target_link_libraries(" << target.name() << " " << scope;
                for (const auto &dep : target.linked_targets()) {
                    out << "\n    " << dep;
                }
                for (const auto &sys : target.linked_system_libs()) {
                    out << "\n    \"" << util::escape_cmake_string(sys) << "\"";
                }
                for (const auto &lib : cfg.link_libraries) {
                    out << "\n    \"" << util::escape_cmake_string(lib) << "\"";
                }
                out << "\n)\n";
            }

            // Link options
            if (!cfg.link_options.empty()) {
                out << "target_link_options(" << target.name() << " " << scope;
                for (const auto &lopt : cfg.link_options) {
                    out << "\n    \"" << util::escape_cmake_string(lopt) << "\"";
                }
                out << "\n)\n";
            }

            // Precompiled headers
            if (!target.pch_header().empty()) {
                out << "target_precompile_headers(" << target.name() << " PRIVATE \""
                    << util::normalize_path(target.pch_header(), false) << "\")\n";
            }

            // Target properties (output name, directories, subsystem)
            bool has_props = false;
            std::ostringstream props;
            if (!cfg.output_name.empty()) {
                props << " OUTPUT_NAME \"" << cfg.output_name << "\"";
                has_props = true;
            }
            if (!cfg.output_dir.empty()) {
                props << " RUNTIME_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/"
                      << util::normalize_path(cfg.output_dir, false) << "\""
                      << " LIBRARY_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/"
                      << util::normalize_path(cfg.output_dir, false) << "\""
                      << " ARCHIVE_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/"
                      << util::normalize_path(cfg.output_dir, false) << "\"";
                has_props = true;
            }
            if (target.type() == TargetType::Executable && cfg.subsystem == WindowsSubsystem::Windows) {
                props << " WIN32_EXECUTABLE TRUE";
                has_props = true;
            }
            if (has_props) {
                out << "set_target_properties(" << target.name() << " PROPERTIES" << props.str() << ")\n";
            }

            out << "\n";
        }

        return out.str();
    }

    bool generate_file(const std::filesystem::path &output_path = "CMakeLists.txt") const {
        if (util::is_system_protected_dir(output_path)) {
            return false;
        }
        namespace fs = std::filesystem;
        std::error_code ec;
        if (output_path.has_parent_path()) {
            fs::create_directories(output_path.parent_path(), ec);
        }
        std::ofstream out(output_path, std::ios::trunc);
        if (!out.is_open()) {
            return false;
        }
        out << generate();
        return true;
    }

private:
    const Builder &builder_;
    std::string project_name_;
};

// ============================================================================
// Visual Studio MSVC Generator (Story 6.2)
// ============================================================================

class MsvcGenerator {
public:
    explicit MsvcGenerator(const Builder &builder, std::string project_name = "")
        : builder_(builder), project_name_(std::move(project_name)) {
        if (project_name_.empty()) {
            project_name_ = builder_.project_name();
        }
        if (project_name_.empty()) {
            for (const auto &t : builder_.targets()) {
                if (t->type() == TargetType::Executable) {
                    project_name_ = t->name();
                    break;
                }
            }
        }
        if (project_name_.empty() && !builder_.targets().empty()) {
            project_name_ = builder_.targets().front()->name();
        }
        if (project_name_.empty()) {
            project_name_ = "CxxgenProject";
        }
    }

    [[nodiscard]] const std::string &project_name() const noexcept { return project_name_; }

    [[nodiscard]] static std::string target_guid(std::string_view target_name) {
        return util::generate_deterministic_guid("msvc_project:" + std::string(target_name));
    }

    [[nodiscard]] std::string solution_guid() const {
        return util::generate_deterministic_guid("msvc_solution:" + project_name_);
    }

    [[nodiscard]] std::string generate_sln() const {
        std::ostringstream out;
        out << "\r\nMicrosoft Visual Studio Solution File, Format Version 12.00\r\n"
            << "# Visual Studio Version 17\r\n"
            << "VisualStudioVersion = 17.0.31903.59\r\n"
            << "MinimumVisualStudioVersion = 10.0.40219.1\r\n";

        for (const auto &target_ptr : builder_.targets()) {
            const std::string &name = target_ptr->name();
            std::string guid = target_guid(name);
            out << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" << name << "\", \"" << name
                << ".vcxproj\", \"" << guid << "\"\r\n"
                << "EndProject\r\n";
        }

        out << "Global\r\n"
            << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\r\n"
            << "\t\tDebug|x64 = Debug|x64\r\n"
            << "\t\tRelease|x64 = Release|x64\r\n"
            << "\tEndGlobalSection\r\n"
            << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\r\n";

        for (const auto &target_ptr : builder_.targets()) {
            const std::string &name = target_ptr->name();
            std::string guid = target_guid(name);
            out << "\t\t" << guid << ".Debug|x64.ActiveCfg = Debug|x64\r\n"
                << "\t\t" << guid << ".Debug|x64.Build.0 = Debug|x64\r\n"
                << "\t\t" << guid << ".Release|x64.ActiveCfg = Release|x64\r\n"
                << "\t\t" << guid << ".Release|x64.Build.0 = Release|x64\r\n";
        }

        out << "\tEndGlobalSection\r\n"
            << "\tGlobalSection(SolutionProperties) = preSolution\r\n"
            << "\t\tHideSolutionNode = FALSE\r\n"
            << "\tEndGlobalSection\r\n"
            << "\tGlobalSection(ExtensibilityGlobals) = postSolution\r\n"
            << "\t\tSolutionGuid = " << solution_guid() << "\r\n"
            << "\tEndGlobalSection\r\n"
            << "EndGlobal\r\n";

        return out.str();
    }

    [[nodiscard]] std::string generate_vcxproj(const Target &target) const {
        const auto &cfg = target.config();
        std::string guid = target_guid(target.name());

        std::string config_type = "Application";
        switch (target.type()) {
        case TargetType::Executable:
        case TargetType::Test:
            config_type = "Application";
            break;
        case TargetType::StaticLibrary:
            config_type = "StaticLibrary";
            break;
        case TargetType::SharedLibrary:
        case TargetType::Plugin:
            config_type = "DynamicLibrary";
            break;
        case TargetType::Interface:
            config_type = "Utility";
            break;
        }

        std::string lang_std = "stdcpp20";
        switch (cfg.cpp_version) {
        case CppVersion::Cpp11:
        case CppVersion::Cpp14:
            lang_std = "stdcpp14";
            break;
        case CppVersion::Cpp17:
            lang_std = "stdcpp17";
            break;
        case CppVersion::Cpp20:
            lang_std = "stdcpp20";
            break;
        case CppVersion::Cpp23:
        case CppVersion::Cpp26:
            lang_std = "stdcpplatest";
            break;
        }

        auto eff_includes = CompilationPipeline::get_effective_include_dirs(builder_, target);
        std::string inc_str;
        for (const auto &inc : eff_includes) {
            inc_str += util::escape_xml(util::normalize_path(inc, true)) + ";";
        }
        inc_str += "%(AdditionalIncludeDirectories)";

        auto eff_defines = CompilationPipeline::get_effective_defines(builder_, target);
        std::string user_defs_debug;
        std::string user_defs_release;
        if (target.type() == TargetType::Executable || target.type() == TargetType::Test) {
            user_defs_debug += "_CONSOLE;";
            user_defs_release += "_CONSOLE;";
        } else if (target.type() == TargetType::SharedLibrary || target.type() == TargetType::Plugin) {
            std::string export_macro = target.name();
            for (char &c : export_macro) {
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }
            export_macro += "_EXPORTS;";
            user_defs_debug += export_macro;
            user_defs_release += export_macro;
        }
        for (const auto &d : eff_defines) {
            user_defs_debug += util::escape_xml(d) + ";";
            user_defs_release += util::escape_xml(d) + ";";
        }

        std::string link_dirs_str;
        for (const auto &ld : cfg.link_directories) {
            link_dirs_str += util::escape_xml(util::normalize_path(ld, true)) + ";";
        }
        link_dirs_str += "%(AdditionalLibraryDirectories)";

        std::string link_deps_str;
        for (const auto &sys : target.linked_system_libs()) {
            std::string lib_file = sys;
            if (!lib_file.ends_with(".lib")) {
                lib_file += ".lib";
            }
            link_deps_str += util::escape_xml(lib_file) + ";";
        }
        for (const auto &l : cfg.link_libraries) {
            std::string lib_file = l;
            if (!lib_file.ends_with(".lib")) {
                lib_file += ".lib";
            }
            link_deps_str += util::escape_xml(lib_file) + ";";
        }
        link_deps_str += "%(AdditionalDependencies)";

        std::string subsystem_str = (cfg.subsystem == WindowsSubsystem::Windows) ? "Windows" : "Console";
        std::string warnings_as_errors_str = cfg.warnings_as_errors ? "true" : "false";

        std::vector<std::string> compile_sources;
        std::vector<std::string> header_files;
        for (const auto &src : cfg.sources) {
            std::string norm = util::normalize_path(src, true);
            if (is_header(norm)) {
                header_files.push_back(norm);
            } else {
                compile_sources.push_back(norm);
            }
        }
        if (!target.pch_header().empty()) {
            std::string pch_h = util::normalize_path(target.pch_header(), true);
            if (std::find(header_files.begin(), header_files.end(), pch_h) == header_files.end()) {
                header_files.push_back(pch_h);
            }
        }

        std::ostringstream out;
        out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
            << "<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n"
            << "  <ItemGroup Label=\"ProjectConfigurations\">\r\n"
            << "    <ProjectConfiguration Include=\"Debug|x64\">\r\n"
            << "      <Configuration>Debug</Configuration>\r\n"
            << "      <Platform>x64</Platform>\r\n"
            << "    </ProjectConfiguration>\r\n"
            << "    <ProjectConfiguration Include=\"Release|x64\">\r\n"
            << "      <Configuration>Release</Configuration>\r\n"
            << "      <Platform>x64</Platform>\r\n"
            << "    </ProjectConfiguration>\r\n"
            << "  </ItemGroup>\r\n"
            << "  <PropertyGroup Label=\"Globals\">\r\n"
            << "    <VCProjectVersion>17.0</VCProjectVersion>\r\n"
            << "    <Keyword>Win32Proj</Keyword>\r\n"
            << "    <ProjectGuid>" << guid << "</ProjectGuid>\r\n"
            << "    <RootNamespace>" << util::escape_xml(target.name()) << "</RootNamespace>\r\n"
            << "    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>\r\n"
            << "  </PropertyGroup>\r\n"
            << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\r\n"
            << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\" Label=\"Configuration\">\r\n"
            << "    <ConfigurationType>" << config_type << "</ConfigurationType>\r\n"
            << "    <UseDebugLibraries>true</UseDebugLibraries>\r\n"
            << "    <PlatformToolset>v143</PlatformToolset>\r\n"
            << "    <CharacterSet>Unicode</CharacterSet>\r\n"
            << "  </PropertyGroup>\r\n"
            << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\" "
               "Label=\"Configuration\">\r\n"
            << "    <ConfigurationType>" << config_type << "</ConfigurationType>\r\n"
            << "    <UseDebugLibraries>false</UseDebugLibraries>\r\n"
            << "    <PlatformToolset>v143</PlatformToolset>\r\n"
            << "    <WholeProgramOptimization>true</WholeProgramOptimization>\r\n"
            << "    <CharacterSet>Unicode</CharacterSet>\r\n"
            << "  </PropertyGroup>\r\n"
            << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\r\n"
            << "  <ImportGroup Label=\"ExtensionSettings\" />\r\n"
            << "  <ImportGroup Label=\"Shared\" />\r\n"
            << "  <ImportGroup Label=\"PropertySheets\">\r\n"
            << "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" "
               "Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" "
               "Label=\"LocalAppDataPlatform\" />\r\n"
            << "  </ImportGroup>\r\n"
            << "  <PropertyGroup Label=\"UserMacros\" />\r\n"
            << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\r\n"
            << "    <LinkIncremental>true</LinkIncremental>\r\n"
            << "    <OutDir>$(SolutionDir)bin\\Debug\\</OutDir>\r\n"
            << "    <IntDir>$(SolutionDir)obj\\$(ProjectName)\\Debug\\</IntDir>\r\n";
        if (!cfg.output_name.empty()) {
            out << "    <TargetName>" << util::escape_xml(cfg.output_name) << "</TargetName>\r\n";
        }
        out << "  </PropertyGroup>\r\n"
            << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">\r\n"
            << "    <LinkIncremental>false</LinkIncremental>\r\n"
            << "    <OutDir>$(SolutionDir)bin\\Release\\</OutDir>\r\n"
            << "    <IntDir>$(SolutionDir)obj\\$(ProjectName)\\Release\\</IntDir>\r\n";
        if (!cfg.output_name.empty()) {
            out << "    <TargetName>" << util::escape_xml(cfg.output_name) << "</TargetName>\r\n";
        }
        out << "  </PropertyGroup>\r\n";

        // Debug ItemDefinitionGroup
        out << "  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\r\n"
            << "    <ClCompile>\r\n"
            << "      <WarningLevel>Level4</WarningLevel>\r\n"
            << "      <SDLCheck>true</SDLCheck>\r\n"
            << "      <PreprocessorDefinitions>_DEBUG;" << user_defs_debug
            << "%(PreprocessorDefinitions)</PreprocessorDefinitions>\r\n"
            << "      <ConformanceMode>true</ConformanceMode>\r\n"
            << "      <LanguageStandard>" << lang_std << "</LanguageStandard>\r\n"
            << "      <AdditionalIncludeDirectories>" << inc_str << "</AdditionalIncludeDirectories>\r\n"
            << "      <TreatWarningAsError>" << warnings_as_errors_str << "</TreatWarningAsError>\r\n";
        if (!cfg.exceptions) {
            out << "      <ExceptionHandling>false</ExceptionHandling>\r\n";
        }
        if (!cfg.rtti) {
            out << "      <RuntimeTypeInfo>false</RuntimeTypeInfo>\r\n";
        }
        if (!target.pch_header().empty()) {
            out << "      <PrecompiledHeader>Use</PrecompiledHeader>\r\n"
                << "      <PrecompiledHeaderFile>" << util::escape_xml(util::normalize_path(target.pch_header(), true))
                << "</PrecompiledHeaderFile>\r\n";
        }
        out << "    </ClCompile>\r\n";
        if (target.type() == TargetType::StaticLibrary) {
            out << "    <Lib>\r\n"
                << "      <AdditionalLibraryDirectories>" << link_dirs_str << "</AdditionalLibraryDirectories>\r\n"
                << "    </Lib>\r\n";
        } else if (target.type() != TargetType::Interface) {
            out << "    <Link>\r\n"
                << "      <SubSystem>" << subsystem_str << "</SubSystem>\r\n"
                << "      <GenerateDebugInformation>true</GenerateDebugInformation>\r\n"
                << "      <AdditionalLibraryDirectories>" << link_dirs_str << "</AdditionalLibraryDirectories>\r\n"
                << "      <AdditionalDependencies>" << link_deps_str << "</AdditionalDependencies>\r\n"
                << "    </Link>\r\n";
        }
        out << "  </ItemDefinitionGroup>\r\n";

        // Release ItemDefinitionGroup
        out << "  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">\r\n"
            << "    <ClCompile>\r\n"
            << "      <WarningLevel>Level4</WarningLevel>\r\n"
            << "      <FunctionLevelLinking>true</FunctionLevelLinking>\r\n"
            << "      <IntrinsicFunctions>true</IntrinsicFunctions>\r\n"
            << "      <SDLCheck>true</SDLCheck>\r\n"
            << "      <PreprocessorDefinitions>NDEBUG;" << user_defs_release
            << "%(PreprocessorDefinitions)</PreprocessorDefinitions>\r\n"
            << "      <ConformanceMode>true</ConformanceMode>\r\n"
            << "      <LanguageStandard>" << lang_std << "</LanguageStandard>\r\n"
            << "      <AdditionalIncludeDirectories>" << inc_str << "</AdditionalIncludeDirectories>\r\n"
            << "      <TreatWarningAsError>" << warnings_as_errors_str << "</TreatWarningAsError>\r\n";
        if (!cfg.exceptions) {
            out << "      <ExceptionHandling>false</ExceptionHandling>\r\n";
        }
        if (!cfg.rtti) {
            out << "      <RuntimeTypeInfo>false</RuntimeTypeInfo>\r\n";
        }
        if (!target.pch_header().empty()) {
            out << "      <PrecompiledHeader>Use</PrecompiledHeader>\r\n"
                << "      <PrecompiledHeaderFile>" << util::escape_xml(util::normalize_path(target.pch_header(), true))
                << "</PrecompiledHeaderFile>\r\n";
        }
        out << "    </ClCompile>\r\n";
        if (target.type() == TargetType::StaticLibrary) {
            out << "    <Lib>\r\n"
                << "      <AdditionalLibraryDirectories>" << link_dirs_str << "</AdditionalLibraryDirectories>\r\n"
                << "    </Lib>\r\n";
        } else if (target.type() != TargetType::Interface) {
            out << "    <Link>\r\n"
                << "      <SubSystem>" << subsystem_str << "</SubSystem>\r\n"
                << "      <EnableCOMDATFolding>true</EnableCOMDATFolding>\r\n"
                << "      <OptimizeReferences>true</OptimizeReferences>\r\n"
                << "      <GenerateDebugInformation>true</GenerateDebugInformation>\r\n"
                << "      <AdditionalLibraryDirectories>" << link_dirs_str << "</AdditionalLibraryDirectories>\r\n"
                << "      <AdditionalDependencies>" << link_deps_str << "</AdditionalDependencies>\r\n"
                << "    </Link>\r\n";
        }
        out << "  </ItemDefinitionGroup>\r\n";

        // Source Items
        if (!compile_sources.empty()) {
            out << "  <ItemGroup>\r\n";
            for (const auto &src : compile_sources) {
                if (!target.pch_source().empty() && util::normalize_path(target.pch_source(), true) == src) {
                    out << "    <ClCompile Include=\"" << util::escape_xml(src) << "\">\r\n"
                        << "      <PrecompiledHeader>Create</PrecompiledHeader>\r\n"
                        << "    </ClCompile>\r\n";
                } else {
                    out << "    <ClCompile Include=\"" << util::escape_xml(src) << "\" />\r\n";
                }
            }
            out << "  </ItemGroup>\r\n";
        }

        // Header Items
        if (!header_files.empty()) {
            out << "  <ItemGroup>\r\n";
            for (const auto &h : header_files) {
                out << "    <ClInclude Include=\"" << util::escape_xml(h) << "\" />\r\n";
            }
            out << "  </ItemGroup>\r\n";
        }

        // Project References
        if (!target.linked_targets().empty()) {
            out << "  <ItemGroup>\r\n";
            for (const auto &dep_name : target.linked_targets()) {
                out << "    <ProjectReference Include=\"" << util::escape_xml(dep_name) << ".vcxproj\">\r\n"
                    << "      <Project>" << target_guid(dep_name) << "</Project>\r\n"
                    << "    </ProjectReference>\r\n";
            }
            out << "  </ItemGroup>\r\n";
        }

        out << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\r\n"
            << "  <ImportGroup Label=\"ExtensionTargets\" />\r\n"
            << "</Project>\r\n";

        return out.str();
    }

    [[nodiscard]] std::string generate_vcxproj_filters(const Target &target) const {
        const auto &cfg = target.config();

        std::vector<std::string> compile_sources;
        std::vector<std::string> header_files;
        for (const auto &src : cfg.sources) {
            std::string norm = util::normalize_path(src, true);
            if (is_header(norm)) {
                header_files.push_back(norm);
            } else {
                compile_sources.push_back(norm);
            }
        }
        if (!target.pch_header().empty()) {
            std::string pch_h = util::normalize_path(target.pch_header(), true);
            if (std::find(header_files.begin(), header_files.end(), pch_h) == header_files.end()) {
                header_files.push_back(pch_h);
            }
        }

        std::set<std::string> leaf_filters;
        std::vector<std::pair<std::string, std::string>> src_filters;
        std::vector<std::pair<std::string, std::string>> hdr_filters;

        for (const auto &src : compile_sources) {
            std::string filt = get_filter_for_file(src, false);
            leaf_filters.insert(filt);
            src_filters.emplace_back(src, filt);
        }
        for (const auto &h : header_files) {
            std::string filt = get_filter_for_file(h, true);
            leaf_filters.insert(filt);
            hdr_filters.emplace_back(h, filt);
        }

        std::set<std::string> all_filters = get_all_filter_ancestors(leaf_filters);

        std::ostringstream out;
        out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
            << "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n";

        if (!all_filters.empty()) {
            out << "  <ItemGroup>\r\n";
            for (const auto &filt : all_filters) {
                std::string filt_guid = util::generate_deterministic_guid("filter:" + target.name() + ":" + filt);
                out << "    <Filter Include=\"" << util::escape_xml(filt) << "\">\r\n"
                    << "      <UniqueIdentifier>" << filt_guid << "</UniqueIdentifier>\r\n";
                if (filt == "Source Files") {
                    out << "      <Extensions>cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx</Extensions>\r\n";
                } else if (filt == "Header Files") {
                    out << "      <Extensions>h;hh;hpp;hxx;hm;inl;inc;ipp;xsd</Extensions>\r\n";
                }
                out << "    </Filter>\r\n";
            }
            out << "  </ItemGroup>\r\n";
        }

        if (!src_filters.empty()) {
            out << "  <ItemGroup>\r\n";
            for (const auto &[src, filt] : src_filters) {
                out << "    <ClCompile Include=\"" << util::escape_xml(src) << "\">\r\n"
                    << "      <Filter>" << util::escape_xml(filt) << "</Filter>\r\n"
                    << "    </ClCompile>\r\n";
            }
            out << "  </ItemGroup>\r\n";
        }

        if (!hdr_filters.empty()) {
            out << "  <ItemGroup>\r\n";
            for (const auto &[h, filt] : hdr_filters) {
                out << "    <ClInclude Include=\"" << util::escape_xml(h) << "\">\r\n"
                    << "      <Filter>" << util::escape_xml(filt) << "</Filter>\r\n"
                    << "    </ClInclude>\r\n";
            }
            out << "  </ItemGroup>\r\n";
        }

        out << "</Project>\r\n";
        return out.str();
    }

    bool generate_sln_file(const std::filesystem::path &path) const {
        if (util::is_system_protected_dir(path))
            return false;
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        if (!f.is_open())
            return false;
        std::string s = generate_sln();
        f.write(s.data(), static_cast<std::streamsize>(s.size()));
        return true;
    }

    bool generate_vcxproj_file(const Target &target, const std::filesystem::path &path) const {
        if (util::is_system_protected_dir(path))
            return false;
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        if (!f.is_open())
            return false;
        std::string s = generate_vcxproj(target);
        f.write(s.data(), static_cast<std::streamsize>(s.size()));
        return true;
    }

    bool generate_vcxproj_filters_file(const Target &target, const std::filesystem::path &path) const {
        if (util::is_system_protected_dir(path))
            return false;
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        if (!f.is_open())
            return false;
        std::string s = generate_vcxproj_filters(target);
        f.write(s.data(), static_cast<std::streamsize>(s.size()));
        return true;
    }

    bool generate_all(const std::filesystem::path &output_dir = ".") const {
        if (util::is_system_protected_dir(output_dir))
            return false;
        std::error_code ec;
        std::filesystem::create_directories(output_dir, ec);

        std::filesystem::path sln_path = output_dir / (project_name_ + ".sln");
        if (!generate_sln_file(sln_path)) {
            return false;
        }

        for (const auto &target_ptr : builder_.targets()) {
            const Target &target = *target_ptr;
            std::filesystem::path vcx_path = output_dir / (target.name() + ".vcxproj");
            if (!generate_vcxproj_file(target, vcx_path)) {
                return false;
            }

            std::filesystem::path flt_path = output_dir / (target.name() + ".vcxproj.filters");
            if (!generate_vcxproj_filters_file(target, flt_path)) {
                return false;
            }
        }

        return true;
    }

private:
    static bool is_header(std::string_view p) noexcept {
        size_t dot = p.rfind('.');
        if (dot == std::string_view::npos)
            return false;
        std::string_view ext = p.substr(dot);
        return ext == ".h" || ext == ".hpp" || ext == ".hxx" || ext == ".hh" || ext == ".inl";
    }

    static std::string get_filter_for_file(const std::string &norm_path, bool is_hdr) {
        std::filesystem::path p(norm_path);
        std::filesystem::path parent = p.parent_path();
        if (parent.empty() || parent == ".") {
            return is_hdr ? "Header Files" : "Source Files";
        }
        std::string parent_str = util::normalize_path(parent.string(), true);
        return (is_hdr ? "Header Files\\" : "Source Files\\") + parent_str;
    }

    static std::set<std::string> get_all_filter_ancestors(const std::set<std::string> &leaf_filters) {
        std::set<std::string> all_filters;
        for (const auto &f : leaf_filters) {
            std::string cur = f;
            while (!cur.empty()) {
                all_filters.insert(cur);
                size_t slash = cur.rfind('\\');
                if (slash == std::string::npos) {
                    break;
                }
                cur = cur.substr(0, slash);
            }
        }
        return all_filters;
    }

    const Builder &builder_;
    std::string project_name_;
};

// ============================================================================
// High-Speed Ninja Generator (Story 6.3)
// ============================================================================

class NinjaGenerator {
public:
    explicit NinjaGenerator(const Builder &builder, const IToolchain *toolchain = nullptr,
                            std::string project_name = "")
        : builder_(builder), toolchain_(toolchain), project_name_(std::move(project_name)) {
        if (!toolchain_) {
            owned_toolchain_ = find_toolchain();
            toolchain_ = owned_toolchain_.get();
        }
        if (project_name_.empty()) {
            project_name_ = builder_.project_name();
        }
        if (project_name_.empty()) {
            for (const auto &t : builder_.targets()) {
                if (t->type() == TargetType::Executable) {
                    project_name_ = t->name();
                    break;
                }
            }
        }
        if (project_name_.empty() && !builder_.targets().empty()) {
            project_name_ = builder_.targets().front()->name();
        }
        if (project_name_.empty()) {
            project_name_ = "CxxgenProject";
        }
    }

    [[nodiscard]] const std::string &project_name() const noexcept { return project_name_; }

    [[nodiscard]] std::string generate() const {
        std::ostringstream out;
        out << "# ==============================================================================\n"
            << "# Generated by cxxgen (C++ Generator & Build System)\n"
            << "# High-Speed Ninja Build File | Standard: Ninja 1.8+\n"
            << "# ==============================================================================\n\n"
            << "ninja_required_version = 1.8\n\n";

        bool is_msvc = (toolchain_ && toolchain_->family() == CompilerFamily::Msvc);

        std::string cxx_bin = toolchain_ ? toolchain_->compiler_path() : (is_msvc ? "cl.exe" : "clang++");
        std::string ar_bin = toolchain_ ? toolchain_->archiver_path() : (is_msvc ? "lib.exe" : "llvm-ar");
        std::string link_bin = toolchain_ ? toolchain_->linker_path() : (is_msvc ? "link.exe" : "clang++");

        out << "# Toolchain Executables\n"
            << "cxx = " << util::normalize_path(cxx_bin, false) << "\n"
            << "ar = " << util::normalize_path(ar_bin, false) << "\n"
            << "link = " << util::normalize_path(link_bin, false) << "\n\n";

        // Emit Rules
        out << "# Build Rules\n";
        if (is_msvc) {
            out << "rule cxx_compile\n"
                << "  command = $cxx /showIncludes $cflags /c $in /Fo:$out\n"
                << "  deps = msvc\n"
                << "  msvc_deps_prefix = Note: including file:\n"
                << "  description = [CXX] $out\n\n"
                << "rule c_compile\n"
                << "  command = $cxx /showIncludes $cflags /c $in /Fo:$out\n"
                << "  deps = msvc\n"
                << "  msvc_deps_prefix = Note: including file:\n"
                << "  description = [CC] $out\n\n"
                << "rule static_lib\n"
                << "  command = $ar /OUT:$out $in\n"
                << "  description = [LIB] $out\n\n"
                << "rule link_shared\n"
                << "  command = $link /DLL /OUT:$out $in $linkflags\n"
                << "  description = [LINK DLL] $out\n\n"
                << "rule link_exe\n"
                << "  command = $link /OUT:$out $in $linkflags\n"
                << "  description = [LINK] $out\n\n";
        } else {
            out << "rule cxx_compile\n"
                << "  command = $cxx -MMD -MF $out.d $cflags -c $in -o $out\n"
                << "  depfile = $out.d\n"
                << "  deps = gcc\n"
                << "  description = [CXX] $out\n\n"
                << "rule c_compile\n"
                << "  command = $cxx -MMD -MF $out.d $cflags -c $in -o $out\n"
                << "  depfile = $out.d\n"
                << "  deps = gcc\n"
                << "  description = [CC] $out\n\n"
                << "rule static_lib\n"
                << "  command = $ar crs $out $in\n"
                << "  description = [AR] $out\n\n"
                << "rule link_shared\n"
                << "  command = $cxx -shared $in $linkflags -o $out\n"
                << "  description = [LINK SHARED] $out\n\n"
                << "rule link_exe\n"
                << "  command = $cxx $in $linkflags -o $out\n"
                << "  description = [LINK] $out\n\n";
        }

        out << "rule custom_cmd\n"
            << "  command = $cmd\n"
            << "  description = $desc\n\n";

        // Custom Commands from builder steps
        for (const auto &step : builder_.steps()) {
            auto custom_cmd = std::dynamic_pointer_cast<CustomCommandStep>(step);
            if (custom_cmd && !custom_cmd->outputs().empty()) {
                out << "# Custom Step: " << custom_cmd->name() << "\n"
                    << "build";
                for (const auto &out_file : custom_cmd->outputs()) {
                    out << " " << escape_ninja_path(util::normalize_path(out_file, false));
                }
                out << ": custom_cmd";
                for (const auto &inp : custom_cmd->inputs()) {
                    out << " " << escape_ninja_path(util::normalize_path(inp, false));
                }
                out << "\n  cmd = " << util::flatten_command(custom_cmd->command(), false) << "\n"
                    << "  desc = " << custom_cmd->description() << "\n\n";
            }
        }

        std::vector<std::string> default_targets;

        // Targets compilation & linking
        for (const auto &target_ptr : builder_.targets()) {
            const Target &target = *target_ptr;
            if (target.type() == TargetType::Interface) {
                continue;
            }

            const auto &cfg = target.config();

            // Construct cflags
            std::string cflags;

            // C++ standard
            if (is_msvc) {
                switch (cfg.cpp_version) {
                case CppVersion::Cpp11:
                case CppVersion::Cpp14:
                    cflags += " /std:c++14";
                    break;
                case CppVersion::Cpp17:
                    cflags += " /std:c++17";
                    break;
                case CppVersion::Cpp20:
                    cflags += " /std:c++20";
                    break;
                case CppVersion::Cpp23:
                case CppVersion::Cpp26:
                    cflags += " /std:c++latest";
                    break;
                }
            } else {
                switch (cfg.cpp_version) {
                case CppVersion::Cpp11:
                    cflags += " -std=c++11";
                    break;
                case CppVersion::Cpp14:
                    cflags += " -std=c++14";
                    break;
                case CppVersion::Cpp17:
                    cflags += " -std=c++17";
                    break;
                case CppVersion::Cpp20:
                    cflags += " -std=c++20";
                    break;
                case CppVersion::Cpp23:
                    cflags += " -std=c++23";
                    break;
                case CppVersion::Cpp26:
                    cflags += " -std=c++26";
                    break;
                }
            }

            // Warnings
            if (is_msvc) {
                cflags += " /W4";
                if (cfg.warnings_as_errors) {
                    cflags += " /WX";
                }
            } else {
                cflags += " -Wall -Wextra";
                if (cfg.warnings_as_errors) {
                    cflags += " -Werror";
                }
            }

            // Optimization
            if (is_msvc) {
                switch (cfg.optimize) {
                case Optimization::Debug:
                    cflags += " /Od /MDd";
                    break;
                case Optimization::ReleaseFast:
                    cflags += " /O2 /MD /DNDEBUG";
                    break;
                case Optimization::ReleaseSafe:
                    cflags += " /O2 /MD /DNDEBUG";
                    break;
                case Optimization::ReleaseSmall:
                    cflags += " /O1 /MD /DNDEBUG";
                    break;
                }
            } else {
                switch (cfg.optimize) {
                case Optimization::Debug:
                    cflags += " -O0 -g";
                    break;
                case Optimization::ReleaseFast:
                    cflags += " -O3 -DNDEBUG";
                    break;
                case Optimization::ReleaseSafe:
                    cflags += " -O2 -g -DNDEBUG";
                    break;
                case Optimization::ReleaseSmall:
                    cflags += " -Os -DNDEBUG";
                    break;
                }
            }

            // Exceptions and RTTI
            if (!cfg.exceptions) {
                cflags += is_msvc ? " /EHs-c-" : " -fno-exceptions";
            }
            if (!cfg.rtti) {
                cflags += is_msvc ? " /GR-" : " -fno-rtti";
            }

            // PIC for shared libs on POSIX
            if (!is_msvc && (target.type() == TargetType::SharedLibrary || target.type() == TargetType::Plugin)) {
                cflags += " -fPIC";
            }

            // Effective Includes
            auto eff_includes = CompilationPipeline::get_effective_include_dirs(builder_, target);
            for (const auto &inc : eff_includes) {
                cflags += (is_msvc ? " /I\"" : " -I\"") + util::normalize_path(inc, false) + "\"";
            }

            // Effective Defines
            auto eff_defines = CompilationPipeline::get_effective_defines(builder_, target);
            for (const auto &def : eff_defines) {
                cflags += (is_msvc ? " /D\"" : " -D\"") + def + "\"";
            }

            // Additional compile options
            for (const auto &opt : cfg.compile_options) {
                cflags += " " + opt;
            }

            // Build object files
            std::vector<std::string> obj_files;
            std::string obj_ext = is_msvc ? ".obj" : ".o";

            out << "# Target: " << target.name() << " (Objects)\n";
            for (const auto &src : cfg.sources) {
                std::string norm_src = util::normalize_path(src, false);
                if (is_header(norm_src)) {
                    continue;
                }
                std::string obj_file =
                    "obj/" + target.name() + "/" + CompilationPipeline::sanitize_source_for_object(norm_src) + obj_ext;
                obj_files.push_back(obj_file);

                bool is_c = norm_src.ends_with(".c");
                std::string rule_name = is_c ? "c_compile" : "cxx_compile";

                out << "build " << escape_ninja_path(obj_file) << ": " << rule_name << " "
                    << escape_ninja_path(norm_src) << "\n"
                    << "  cflags =" << cflags << "\n\n";
            }

            // Output artifact path
            std::string artifact_path;
            if (toolchain_) {
                artifact_path =
                    util::normalize_path(CompilationPipeline::get_artifact_path(target, *toolchain_).string(), false);
            } else {
                std::string out_dir = cfg.output_dir.empty()
                                          ? (target.type() == TargetType::StaticLibrary ? "lib" : "bin")
                                          : cfg.output_dir;
                std::string base = cfg.output_name.empty() ? target.name() : cfg.output_name;
                if (target.type() == TargetType::Executable || target.type() == TargetType::Test) {
#if defined(_WIN32)
                    artifact_path = out_dir + "/" + base + ".exe";
#else
                    artifact_path = out_dir + "/" + base;
#endif
                } else if (target.type() == TargetType::StaticLibrary) {
                    artifact_path = is_msvc ? (out_dir + "/" + base + ".lib") : (out_dir + "/lib" + base + ".a");
                } else if (target.type() == TargetType::SharedLibrary || target.type() == TargetType::Plugin) {
#if defined(_WIN32)
                    artifact_path = out_dir + "/" + base + ".dll";
#elif defined(__APPLE__)
                    artifact_path = out_dir + "/lib" + base + ".dylib";
#else
                    artifact_path = out_dir + "/lib" + base + ".so";
#endif
                }
            }
            default_targets.push_back(artifact_path);

            // Linking
            out << "# Target: " << target.name() << " (Artifact)\n";
            if (target.type() == TargetType::StaticLibrary) {
                out << "build " << escape_ninja_path(artifact_path) << ": static_lib";
                for (const auto &obj : obj_files) {
                    out << " " << escape_ninja_path(obj);
                }
                out << "\n\n";
            } else {
                std::string link_rule = (target.type() == TargetType::Executable || target.type() == TargetType::Test)
                                            ? "link_exe"
                                            : "link_shared";

                std::vector<std::string> link_inputs = obj_files;
                std::string linkflags;

                // Link directories
                for (const auto &ld : cfg.link_directories) {
                    linkflags += (is_msvc ? " /LIBPATH:\"" : " -L\"") + util::normalize_path(ld, false) + "\"";
                }

                // Linked internal targets
                for (const auto &dep_name : target.linked_targets()) {
                    const Target *dep = builder_.find_target(dep_name);
                    if (dep && dep->type() != TargetType::Interface) {
                        std::string dep_artifact;
                        if (toolchain_) {
                            dep_artifact = util::normalize_path(
                                CompilationPipeline::get_artifact_path(*dep, *toolchain_).string(), false);
                        } else {
                            std::string dep_base =
                                dep->config().output_name.empty() ? dep->name() : dep->config().output_name;
                            dep_artifact = is_msvc ? ("lib/" + dep_base + ".lib") : ("lib/lib" + dep_base + ".a");
                        }
                        link_inputs.push_back(dep_artifact);
                    }
                }

                // Linked system libraries
                for (const auto &sys : target.linked_system_libs()) {
                    if (is_msvc) {
                        linkflags += " " + (sys.ends_with(".lib") ? sys : sys + ".lib");
                    } else {
                        linkflags += " -l" + sys;
                    }
                }

                // External link libraries
                for (const auto &lib : cfg.link_libraries) {
                    if (is_msvc) {
                        linkflags += " " + (lib.ends_with(".lib") ? lib : lib + ".lib");
                    } else {
                        linkflags += " -l" + lib;
                    }
                }

                // Extra link options
                for (const auto &lopt : cfg.link_options) {
                    linkflags += " " + lopt;
                }

                out << "build " << escape_ninja_path(artifact_path) << ": " << link_rule;
                for (const auto &in_file : link_inputs) {
                    out << " " << escape_ninja_path(in_file);
                }
                out << "\n";
                if (!linkflags.empty()) {
                    out << "  linkflags =" << linkflags << "\n";
                }
                out << "\n";
            }
        }

        // Default targets
        if (!default_targets.empty()) {
            out << "# Default Build Artifacts\n"
                << "default";
            for (const auto &dt : default_targets) {
                out << " " << escape_ninja_path(dt);
            }
            out << "\n";
        }

        return out.str();
    }

    bool generate_file(const std::filesystem::path &output_path = "build.ninja") const {
        if (util::is_system_protected_dir(output_path))
            return false;
        std::error_code ec;
        if (output_path.has_parent_path()) {
            std::filesystem::create_directories(output_path.parent_path(), ec);
        }
        std::ofstream f(output_path, std::ios::binary | std::ios::trunc);
        if (!f.is_open())
            return false;
        std::string s = generate();
        f.write(s.data(), static_cast<std::streamsize>(s.size()));
        return true;
    }

private:
    static bool is_header(std::string_view p) noexcept {
        size_t dot = p.rfind('.');
        if (dot == std::string_view::npos)
            return false;
        std::string_view ext = p.substr(dot);
        return ext == ".h" || ext == ".hpp" || ext == ".hxx" || ext == ".hh" || ext == ".inl";
    }

    static std::string escape_ninja_path(std::string_view path) {
        std::string res;
        res.reserve(path.size() + 8);
        for (char c : path) {
            if (c == '$') {
                res += "$$";
            } else if (c == ':') {
                res += "$:";
            } else if (c == ' ') {
                res += "$ ";
            } else {
                res += c;
            }
        }
        return res;
    }

    const Builder &builder_;
    const IToolchain *toolchain_ = nullptr;
    std::shared_ptr<IToolchain> owned_toolchain_ = nullptr;
    std::string project_name_;
};

// ============================================================================
// Clang Compilation Database Generator (Story 6.4)
// ============================================================================

class CompileCommandsGenerator {
public:
    explicit CompileCommandsGenerator(const Builder &builder, const IToolchain *toolchain = nullptr,
                                      std::filesystem::path working_dir = {})
        : builder_(builder), toolchain_(toolchain), working_dir_(std::move(working_dir)) {
        if (!toolchain_) {
            owned_toolchain_ = find_toolchain();
            toolchain_ = owned_toolchain_.get();
        }
        if (working_dir_.empty()) {
            std::error_code ec;
            working_dir_ = std::filesystem::current_path(ec);
        }
    }

    [[nodiscard]] std::string generate() const {
        std::vector<util::JsonValue> entries;
        std::string work_dir_str = util::normalize_path(working_dir_.string(), false);

        bool is_msvc = (toolchain_ && toolchain_->family() == CompilerFamily::Msvc);
        std::string obj_ext = is_msvc ? ".obj" : ".o";

        for (const auto &target_ptr : builder_.targets()) {
            const Target &target = *target_ptr;
            if (target.type() == TargetType::Interface) {
                continue;
            }

            const auto &cfg = target.config();

            for (const auto &src : cfg.sources) {
                std::string norm_src = util::normalize_path(src, false);
                if (is_header(norm_src)) {
                    continue;
                }

                CompileParams params;
                params.source_file = norm_src;
                params.object_file =
                    "obj/" + target.name() + "/" + CompilationPipeline::sanitize_source_for_object(norm_src) + obj_ext;
                params.cpp_version = cfg.cpp_version;
                params.c_version = cfg.c_version;
                params.optimize = cfg.optimize;
                params.warnings_as_errors = cfg.warnings_as_errors;
                params.exceptions = cfg.exceptions;
                params.rtti = cfg.rtti;
                params.include_dirs = CompilationPipeline::get_effective_include_dirs(builder_, target);
                params.defines = CompilationPipeline::get_effective_defines(builder_, target);
                params.extra_flags = cfg.compile_options;
                params.sanitizers = cfg.sanitizers;
                params.coverage = cfg.coverage;
                params.target_triple =
                    !target.target_triple().empty() ? target.target_triple() : builder_.target_triple();
                params.sysroot = !target.sysroot().empty() ? target.sysroot() : builder_.sysroot();

                std::string cmd;
                if (toolchain_) {
                    auto cmd_vec = toolchain_->build_compile_command(params);
                    cmd = util::flatten_command(cmd_vec, false);
                } else {
                    std::vector<std::string> cmd_vec;
                    cmd_vec.push_back("clang++");
                    cmd_vec.push_back("-std=" + std::string(to_string_view(cfg.cpp_version)));
                    for (const auto &inc : params.include_dirs) {
                        cmd_vec.push_back("-I" + util::normalize_path(inc, false));
                    }
                    for (const auto &def : params.defines) {
                        cmd_vec.push_back("-D" + def);
                    }
                    if (cfg.warnings_as_errors) {
                        cmd_vec.push_back("-Werror");
                    }
                    if (!cfg.exceptions) {
                        cmd_vec.push_back("-fno-exceptions");
                    }
                    if (!cfg.rtti) {
                        cmd_vec.push_back("-fno-rtti");
                    }
                    for (const auto &opt : cfg.compile_options) {
                        cmd_vec.push_back(opt);
                    }
                    cmd_vec.push_back("-c");
                    cmd_vec.push_back(params.source_file);
                    cmd_vec.push_back("-o");
                    cmd_vec.push_back(params.object_file);
                    cmd = util::flatten_command(cmd_vec, false);
                }

                std::map<std::string, util::JsonValue> entry;
                entry["directory"] = util::JsonValue(work_dir_str);
                entry["command"] = util::JsonValue(cmd);
                entry["file"] = util::JsonValue(params.source_file);
                entry["output"] = util::JsonValue(params.object_file);
                entries.emplace_back(std::move(entry));
            }
        }

        util::JsonValue root(std::move(entries));
        return root.serialize(0);
    }

    bool generate_file(const std::filesystem::path &output_path = "compile_commands.json") const {
        if (util::is_system_protected_dir(output_path))
            return false;
        std::error_code ec;
        if (output_path.has_parent_path()) {
            std::filesystem::create_directories(output_path.parent_path(), ec);
        }
        std::ofstream f(output_path, std::ios::binary | std::ios::trunc);
        if (!f.is_open())
            return false;
        std::string s = generate();
        f.write(s.data(), static_cast<std::streamsize>(s.size()));
        return true;
    }

private:
    static bool is_header(std::string_view p) noexcept {
        size_t dot = p.rfind('.');
        if (dot == std::string_view::npos)
            return false;
        std::string_view ext = p.substr(dot);
        return ext == ".h" || ext == ".hpp" || ext == ".hxx" || ext == ".hh" || ext == ".inl";
    }

    const Builder &builder_;
    const IToolchain *toolchain_ = nullptr;
    std::shared_ptr<IToolchain> owned_toolchain_ = nullptr;
    std::filesystem::path working_dir_;
};

// ============================================================================
// 8.9. JIT Compilation & Fast Hash Caching for build.cpp
// ============================================================================

struct JitRunnerConfig {
    std::filesystem::path working_dir = {};
    std::filesystem::path build_script = {};
    std::filesystem::path cxxgen_header = {};
    std::shared_ptr<IToolchain> toolchain = nullptr;
    bool verbose = false;
    bool force_rebuild = false;
};

struct JitResult {
    bool success = false;
    bool cache_hit = false;
    std::filesystem::path runner_binary = {};
    std::string hash = "";
    int exit_code = 0;
    std::string error_message = "";
    std::chrono::milliseconds duration{0};
};

class JitRunner {
public:
    static std::optional<std::filesystem::path>
    find_build_script(const std::filesystem::path &start_dir = std::filesystem::current_path(),
                      const std::optional<std::filesystem::path> &stop_at = std::nullopt) {
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path cur = fs::absolute(start_dir, ec);
        if (ec)
            cur = start_dir;
        std::optional<fs::path> abs_stop;
        if (stop_at) {
            fs::path p = fs::absolute(*stop_at, ec);
            abs_stop = ec ? *stop_at : p;
        }
        while (true) {
            fs::path candidate = cur / "build.cpp";
            if (fs::exists(candidate, ec)) {
                return candidate;
            }
            if (abs_stop) {
                bool reached_stop = false;
                if (cur == *abs_stop) {
                    reached_stop = true;
                } else {
                    reached_stop = fs::equivalent(cur, *abs_stop, ec);
                    if (ec)
                        reached_stop = false;
                }
                if (reached_stop) {
                    break;
                }
            }
            if (!cur.has_relative_path() || cur == cur.parent_path()) {
                break;
            }
            cur = cur.parent_path();
        }
        return std::nullopt;
    }

    static std::optional<std::filesystem::path> find_cxxgen_header(const std::filesystem::path &build_script) {
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path root = build_script.parent_path();
        if (fs::exists(root / "include" / "cxxgen" / "cxxgen.hpp", ec)) {
            return root / "include" / "cxxgen" / "cxxgen.hpp";
        }
        if (fs::exists(root / "include" / "cxxgen.hpp", ec)) {
            return root / "include" / "cxxgen.hpp";
        }
        if (fs::exists(root / "cxxgen.hpp", ec)) {
            return root / "cxxgen.hpp";
        }
        const char *env_inc = std::getenv("CXXGEN_INCLUDE_DIR");
        if (env_inc) {
            fs::path env_candidate1 = fs::path(env_inc) / "cxxgen" / "cxxgen.hpp";
            if (fs::exists(env_candidate1, ec)) {
                return env_candidate1;
            }
            fs::path env_candidate2 = fs::path(env_inc) / "cxxgen.hpp";
            if (fs::exists(env_candidate2, ec)) {
                return env_candidate2;
            }
        }
        fs::path cwd_inc_nested = fs::current_path() / "include" / "cxxgen" / "cxxgen.hpp";
        if (fs::exists(cwd_inc_nested, ec)) {
            return cwd_inc_nested;
        }
        fs::path cwd_inc = fs::current_path() / "include" / "cxxgen.hpp";
        if (fs::exists(cwd_inc, ec)) {
            return cwd_inc;
        }
        fs::path cwd_direct = fs::current_path() / "cxxgen.hpp";
        if (fs::exists(cwd_direct, ec)) {
            return cwd_direct;
        }
        fs::path cur = root;
        while (cur.has_relative_path() && cur != cur.parent_path()) {
            if (fs::exists(cur / "include" / "cxxgen" / "cxxgen.hpp", ec)) {
                return cur / "include" / "cxxgen" / "cxxgen.hpp";
            }
            if (fs::exists(cur / "include" / "cxxgen.hpp", ec)) {
                return cur / "include" / "cxxgen.hpp";
            }
            if (fs::exists(cur / "cxxgen.hpp", ec)) {
                return cur / "cxxgen.hpp";
            }
            cur = cur.parent_path();
        }
        return std::nullopt;
    }

    static std::string compute_script_hash(const std::filesystem::path &build_script,
                                           const std::filesystem::path &header_path = {}) {
        hash::Sha256 hasher;
        std::ifstream script_file(build_script, std::ios::binary);
        if (script_file.is_open()) {
            char buf[8192];
            while (script_file.read(buf, sizeof(buf)) || script_file.gcount() > 0) {
                hasher.update(buf, static_cast<size_t>(script_file.gcount()));
            }
        }
        if (!header_path.empty()) {
            std::ifstream hdr_file(header_path, std::ios::binary);
            if (hdr_file.is_open()) {
                char buf[8192];
                while (hdr_file.read(buf, sizeof(buf)) || hdr_file.gcount() > 0) {
                    hasher.update(buf, static_cast<size_t>(hdr_file.gcount()));
                }
            }
        }
        return hasher.finalize();
    }

    static JitResult prepare_runner(const JitRunnerConfig &config) {
        namespace fs = std::filesystem;
        auto start_time = std::chrono::steady_clock::now();
        JitResult result;
        std::error_code ec;

        fs::path script = config.build_script;
        if (script.empty()) {
            auto found = find_build_script(config.working_dir.empty() ? fs::current_path() : config.working_dir);
            if (!found) {
                result.success = false;
                result.exit_code = 1;
                result.error_message =
                    "cxxgen error: could not find build.cpp in current directory or parent directories";
                return result;
            }
            script = *found;
        }

        fs::path root = script.parent_path();
        fs::path cache_dir = root / ".cxxgen" / "cache";
        fs::path bin_dir = root / ".cxxgen" / "bin";
        fs::path hash_file = cache_dir / "build_runner.hash";

#if defined(_WIN32)
        fs::path runner_bin = bin_dir / "build_runner.exe";
#else
        fs::path runner_bin = bin_dir / "build_runner";
#endif

        fs::path hdr = config.cxxgen_header;
        if (hdr.empty()) {
            auto found_hdr = find_cxxgen_header(script);
            if (found_hdr) {
                hdr = *found_hdr;
            }
        }

        std::string current_hash = compute_script_hash(script, hdr);
        result.hash = current_hash;
        result.runner_binary = runner_bin;

        // Check cache hit
        if (!config.force_rebuild && fs::exists(runner_bin, ec) && fs::exists(hash_file, ec)) {
            std::ifstream hf(hash_file);
            std::string stored_hash;
            if (hf.is_open() && (hf >> stored_hash) && stored_hash == current_hash) {
                result.success = true;
                result.cache_hit = true;
                result.exit_code = 0;
                result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start_time);
                return result;
            }
        }

        // Cache miss: compile build.cpp
        std::shared_ptr<IToolchain> tc = config.toolchain;
        if (!tc) {
            tc = find_toolchain(CompilerFamily::Auto);
        }
        if (!tc) {
            result.success = false;
            result.exit_code = 1;
            result.error_message = "cxxgen error: no compatible toolchain found to compile build.cpp";
            return result;
        }

        fs::create_directories(bin_dir, ec);
        fs::create_directories(cache_dir, ec);

        std::vector<std::string> inc_dirs;
        if (!hdr.empty()) {
            inc_dirs.push_back(hdr.parent_path().string());
            if (hdr.parent_path().filename() == "cxxgen") {
                inc_dirs.push_back(hdr.parent_path().parent_path().string());
            }
        } else {
            inc_dirs.push_back((root / "include").string());
            inc_dirs.push_back((root / "include" / "cxxgen").string());
        }

        std::vector<std::string> compile_cmd;
        if (tc->family() == CompilerFamily::Msvc) {
            compile_cmd = {tc->compiler_path(), "/nologo", "/std:c++20", "/EHsc", "/O2", "/MD"};
            for (const auto &d : inc_dirs) {
                compile_cmd.push_back("/I" + util::normalize_path(d));
            }
            compile_cmd.push_back(util::normalize_path(script.string()));
            compile_cmd.push_back("/Fe:" + util::normalize_path(runner_bin.string()));
            compile_cmd.push_back("/Fo:" + util::normalize_path((cache_dir / "build_runner.obj").string()));
        } else {
            compile_cmd = {tc->compiler_path(), "-std=c++20", "-O2"};
            for (const auto &d : inc_dirs) {
                compile_cmd.push_back("-I" + util::normalize_path(d, false));
            }
            compile_cmd.push_back(util::normalize_path(script.string(), false));
            compile_cmd.push_back("-o");
            compile_cmd.push_back(util::normalize_path(runner_bin.string(), false));
        }

        if (config.verbose) {
            std::cout << "[cxxgen jit] Compiling " << script.string() << " with " << tc->name() << "...\n";
            std::cout << "[cxxgen jit] " << util::flatten_command(compile_cmd) << "\n";
        }

        ProcessOptions popts;
        popts.command = compile_cmd;
        popts.extra_env = tc->env_vars();
        popts.working_directory = root.string();
        popts.stream_to_terminal = config.verbose;

        ProcessResult pres = ProcessRunner::run(popts);
        if (!pres.success() || !fs::exists(runner_bin, ec)) {
            result.success = false;
            result.cache_hit = false;
            result.exit_code = pres.exit_code != 0 ? pres.exit_code : 1;
            result.error_message =
                "cxxgen error: failed to compile " + script.string() + ":\n" + pres.combined_output();
            return result;
        }

        // Store hash
        std::ofstream hf_out(hash_file, std::ios::trunc);
        if (hf_out.is_open()) {
            hf_out << current_hash << "\n";
        }

        result.success = true;
        result.cache_hit = false;
        result.exit_code = 0;
        result.duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
        return result;
    }

    static int execute_runner(const std::filesystem::path &runner_binary, const std::vector<std::string> &args = {},
                              const std::filesystem::path &working_dir = {}, bool verbose = false) {
        std::vector<std::string> cmd;
        cmd.push_back(runner_binary.string());
        for (const auto &a : args) {
            cmd.push_back(a);
        }
        ProcessOptions popts;
        popts.command = cmd;
        if (!working_dir.empty()) {
            popts.working_directory = working_dir.string();
        }
        popts.stream_to_terminal = true;
        if (verbose) {
            std::cout << "[cxxgen runner] " << util::flatten_command(cmd) << "\n";
        }
        ProcessResult res = ProcessRunner::run(popts);
        return res.exit_code;
    }
};

// ============================================================================
// 8.10. Project Scaffolder (cxxgen init)
// ============================================================================

struct ProjectScaffolderConfig {
    std::filesystem::path target_dir = {};
    std::string project_name = "";
    std::string type = "exe"; // exe, lib, plugin
    bool force = false;
    bool verbose = false;
    std::filesystem::path header_source = {};
    std::string_view header_content = "";
};

struct ScaffolderResult {
    bool success = false;
    std::string error_message = "";
    std::vector<std::filesystem::path> generated_files = {};
};

class ProjectScaffolder {
public:
    static ScaffolderResult scaffold(const ProjectScaffolderConfig &config) {
        namespace fs = std::filesystem;
        ScaffolderResult result;
        std::error_code ec;

        fs::path target_dir = config.target_dir.empty() ? fs::current_path() : config.target_dir;
        target_dir = fs::absolute(target_dir, ec);
        if (ec)
            target_dir = config.target_dir;

        // Security check: Refuse to scaffold in system root or protected OS directories
        if (util::is_system_protected_dir(target_dir)) {
            result.success = false;
            result.error_message =
                "cxxgen security error: refusing to scaffold project into system protected directory '" +
                target_dir.string() + "'";
            return result;
        }

        std::string proj_name = config.project_name;
        if (proj_name.empty()) {
            proj_name = target_dir.filename().string();
        }
        if (proj_name.empty() || proj_name == ".") {
            proj_name = "my_project";
        }

        // Normalize proj_name to valid identifier for macros / namespaces
        std::string ident_name;
        for (char c : proj_name) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                ident_name.push_back(c);
            } else {
                ident_name.push_back('_');
            }
        }
        if (ident_name.empty() || std::isdigit(static_cast<unsigned char>(ident_name[0]))) {
            ident_name = "app_" + ident_name;
        }

        std::string upper_name;
        for (char c : ident_name) {
            upper_name.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
        }

        // Determine list of files to generate
        struct FileSpec {
            fs::path relative_path;
            std::string content;
        };
        std::vector<FileSpec> files;

        // .gitignore
        std::string gitignore_content = "# cxxgen build outputs & cache\n"
                                        ".cxxgen/\n"
                                        "bin/\n"
                                        "obj/\n"
                                        "lib/\n"
                                        "*.obj\n"
                                        "*.o\n"
                                        "*.exe\n"
                                        "*.dll\n"
                                        "*.dylib\n"
                                        "*.so\n"
                                        "*.a\n"
                                        "*.lib\n"
                                        "*.pdb\n"
                                        "*.ilk\n"
                                        "*.exp\n";
        files.push_back({".gitignore", gitignore_content});

        // .clang-format
        std::string clang_format_content = "---\n"
                                           "BasedOnStyle: LLVM\n"
                                           "IndentWidth: 4\n"
                                           "ColumnLimit: 120\n"
                                           "UseTab: Never\n"
                                           "Standard: c++20\n"
                                           "AccessModifierOffset: -4\n"
                                           "AlignAfterOpenBracket: Align\n"
                                           "AlignConsecutiveAssignments: None\n"
                                           "AlignConsecutiveDeclarations: None\n"
                                           "AlignOperands: Align\n"
                                           "AllowShortBlocksOnASingleLine: false\n"
                                           "AllowShortCaseLabelsOnASingleLine: false\n"
                                           "AllowShortFunctionsOnASingleLine: Inline\n"
                                           "AllowShortIfStatementsOnASingleLine: false\n"
                                           "AllowShortLoopsOnASingleLine: false\n"
                                           "BreakBeforeBraces: Attach\n"
                                           "IncludeBlocks: Regroup\n"
                                           "SortIncludes: CaseSensitive\n"
                                           "PointerAlignment: Right\n"
                                           "ReferenceAlignment: Right\n"
                                           "SpacesBeforeTrailingComments: 1\n"
                                           "---\n";
        files.push_back({".clang-format", clang_format_content});

        // .clang-tidy
        std::string clang_tidy_content = "---\n"
                                         "Checks: >\n"
                                         "  -*,\n"
                                         "  clang-analyzer-*,\n"
                                         "  bugprone-*,\n"
                                         "  modernize-*,\n"
                                         "  performance-*,\n"
                                         "  readability-*,\n"
                                         "  -modernize-use-trailing-return-type,\n"
                                         "  -readability-identifier-length\n"
                                         "\n"
                                         "WarningsAsErrors: ''\n"
                                         "HeaderFilterRegex: '.*'\n"
                                         "FormatStyle: file\n"
                                         "---\n";
        files.push_back({".clang-tidy", clang_tidy_content});

        // .clangd
        std::string clangd_content = "CompileFlags:\n"
                                     "  Add:\n"
                                     "    - -std=c++20\n"
                                     "    - -Wall\n"
                                     "    - -Wextra\n"
                                     "  CompilationDatabase: .\n"
                                     "Diagnostics:\n"
                                     "  UnusedIncludes: Strict\n"
                                     "  ClangTidy:\n"
                                     "    Add:\n"
                                     "      - modernize*\n"
                                     "      - bugprone*\n"
                                     "      - performance*\n"
                                     "Hover:\n"
                                     "  ShowAKA: Yes\n"
                                     "InlayHints:\n"
                                     "  Designators: Yes\n"
                                     "  Enabled: Yes\n"
                                     "  ParameterNames: Yes\n"
                                     "  DeducedTypes: Yes\n";
        files.push_back({".clangd", clangd_content});

        if (config.type == "lib") {
            // C++ Library (shared & static)
            std::string header_content = "#pragma once\n\n"
                                         "#if defined(_WIN32)\n"
                                         "  #if defined(" +
                                         upper_name +
                                         "_EXPORTS)\n"
                                         "    #define " +
                                         upper_name +
                                         "_API __declspec(dllexport)\n"
                                         "  #else\n"
                                         "    #define " +
                                         upper_name +
                                         "_API __declspec(dllimport)\n"
                                         "  #endif\n"
                                         "#else\n"
                                         "  #define " +
                                         upper_name +
                                         "_API __attribute__((visibility(\"default\")))\n"
                                         "#endif\n\n"
                                         "namespace " +
                                         ident_name + " {\n\n" + upper_name +
                                         "_API int add(int a, int b);\n\n"
                                         "} // namespace " +
                                         ident_name + "\n";
            files.push_back({"include/" + ident_name + "/" + ident_name + ".hpp", header_content});

            std::string src_content = "#include \"" + ident_name + "/" + ident_name +
                                      ".hpp\"\n\n"
                                      "namespace " +
                                      ident_name +
                                      " {\n\n"
                                      "int add(int a, int b) {\n"
                                      "    return a + b;\n"
                                      "}\n\n"
                                      "} // namespace " +
                                      ident_name + "\n";
            files.push_back({"src/" + ident_name + ".cpp", src_content});

            std::string build_content = "#include \"cxxgen/cxxgen.hpp\"\n\n"
                                        "void build(cxxgen::Builder &b) {\n"
                                        "    b.set_project_name(\"" +
                                        ident_name +
                                        "\");\n"
                                        "    auto opt = b.standard_optimize_options();\n\n"
                                        "    auto &static_lib = b.add_static_library({\n"
                                        "        .name = \"" +
                                        ident_name +
                                        "_static\",\n"
                                        "        .sources = {\"src/" +
                                        ident_name +
                                        ".cpp\"},\n"
                                        "        .include_dirs = {\"include\"},\n"
                                        "        .optimize = opt.mode,\n"
                                        "    });\n\n"
                                        "    auto &shared_lib = b.add_shared_library({\n"
                                        "        .name = \"" +
                                        ident_name +
                                        "\",\n"
                                        "        .sources = {\"src/" +
                                        ident_name +
                                        ".cpp\"},\n"
                                        "        .include_dirs = {\"include\"},\n"
                                        "        .optimize = opt.mode,\n"
                                        "    });\n\n"
                                        "    b.install_artifact(static_lib);\n"
                                        "    b.install_artifact(shared_lib);\n"
                                        "}\n\n"
                                        "CXXGEN_MAIN(build)\n";
            files.push_back({"build.cpp", build_content});

        } else if (config.type == "plugin") {
            // Dynamic Plugin
            std::string header_content =
                "#pragma once\n\n"
                "#if defined(_WIN32)\n"
                "  #define PLUGIN_EXPORT extern \"C\" __declspec(dllexport)\n"
                "#else\n"
                "  #define PLUGIN_EXPORT extern \"C\" __attribute__((visibility(\"default\")))\n"
                "#endif\n\n"
                "struct PluginInfo {\n"
                "    const char *name;\n"
                "    const char *version;\n"
                "};\n\n"
                "PLUGIN_EXPORT PluginInfo get_plugin_info();\n"
                "PLUGIN_EXPORT int run_plugin();\n";
            files.push_back({"include/plugin_api.hpp", header_content});

            std::string src_content = "#include \"plugin_api.hpp\"\n"
                                      "#include <iostream>\n\n"
                                      "PLUGIN_EXPORT PluginInfo get_plugin_info() {\n"
                                      "    return {\"" +
                                      ident_name +
                                      "\", \"1.0.0\"};\n"
                                      "}\n\n"
                                      "PLUGIN_EXPORT int run_plugin() {\n"
                                      "    std::cout << \"Plugin " +
                                      ident_name +
                                      " executed successfully!\\n\";\n"
                                      "    return 0;\n"
                                      "}\n";
            files.push_back({"src/plugin.cpp", src_content});

            std::string build_content = "#include \"cxxgen/cxxgen.hpp\"\n\n"
                                        "void build(cxxgen::Builder &b) {\n"
                                        "    b.set_project_name(\"" +
                                        ident_name +
                                        "\");\n"
                                        "    auto opt = b.standard_optimize_options();\n\n"
                                        "    auto &plug = b.add_plugin({\n"
                                        "        .name = \"" +
                                        ident_name +
                                        "\",\n"
                                        "        .sources = {\"src/plugin.cpp\"},\n"
                                        "        .include_dirs = {\"include\"},\n"
                                        "        .optimize = opt.mode,\n"
                                        "    });\n\n"
                                        "    b.install_artifact(plug);\n"
                                        "}\n\n"
                                        "CXXGEN_MAIN(build)\n";
            files.push_back({"build.cpp", build_content});

        } else {
            // Executable (default)
            std::string math_header_content = "#pragma once\n\n"
                                              "#include <cstdint>\n\n"
                                              "namespace mathlib {\n\n"
                                              "/// Adds two integers\n"
                                              "int add(int a, int b);\n\n"
                                              "/// Subtracts b from a\n"
                                              "int subtract(int a, int b);\n\n"
                                              "/// Multiplies two integers\n"
                                              "int multiply(int a, int b);\n\n"
                                              "/// Divides a by b (returns 0.0 if b is zero)\n"
                                              "double divide(double a, double b);\n\n"
                                              "/// Computes factorial of n\n"
                                              "uint64_t factorial(unsigned int n);\n\n"
                                              "/// Checks if n is a prime number\n"
                                              "bool is_prime(uint64_t n);\n\n"
                                              "} // namespace mathlib\n";
            files.push_back({"src/mathlib/math.hpp", math_header_content});

            std::string math_src_content = "#include \"mathlib/math.hpp\"\n\n"
                                           "namespace mathlib {\n\n"
                                           "int add(int a, int b) {\n"
                                           "    return a + b;\n"
                                           "}\n\n"
                                           "int subtract(int a, int b) {\n"
                                           "    return a - b;\n"
                                           "}\n\n"
                                           "int multiply(int a, int b) {\n"
                                           "    return a * b;\n"
                                           "}\n\n"
                                           "double divide(double a, double b) {\n"
                                           "    if (b == 0.0) {\n"
                                           "        return 0.0;\n"
                                           "    }\n"
                                           "    return a / b;\n"
                                           "}\n\n"
                                           "uint64_t factorial(unsigned int n) {\n"
                                           "    uint64_t result = 1;\n"
                                           "    for (unsigned int i = 2; i <= n; ++i) {\n"
                                           "        result *= i;\n"
                                           "    }\n"
                                           "    return result;\n"
                                           "}\n\n"
                                           "bool is_prime(uint64_t n) {\n"
                                           "    if (n < 2) {\n"
                                           "        return false;\n"
                                           "    }\n"
                                           "    if (n == 2 || n == 3) {\n"
                                           "        return true;\n"
                                           "    }\n"
                                           "    if (n % 2 == 0 || n % 3 == 0) {\n"
                                           "        return false;\n"
                                           "    }\n"
                                           "    for (uint64_t i = 5; i * i <= n; i += 6) {\n"
                                           "        if (n % i == 0 || n % (i + 2) == 0) {\n"
                                           "            return false;\n"
                                           "        }\n"
                                           "    }\n"
                                           "    return true;\n"
                                           "}\n\n"
                                           "} // namespace mathlib\n";
            files.push_back({"src/mathlib/math.cpp", math_src_content});

            std::string test_math_content = "#include <gmock/gmock.h>\n"
                                            "#include <gtest/gtest.h>\n"
                                            "#include \"mathlib/math.hpp\"\n\n"
                                            "using ::testing::Return;\n\n"
                                            "// 1. Google Test assertions for mathlib\n"
                                            "TEST(MathLibTest, BasicArithmetic) {\n"
                                            "    EXPECT_EQ(mathlib::add(2, 3), 5);\n"
                                            "    EXPECT_EQ(mathlib::subtract(10, 4), 6);\n"
                                            "    EXPECT_EQ(mathlib::multiply(6, 7), 42);\n"
                                            "    EXPECT_DOUBLE_EQ(mathlib::divide(10.0, 2.0), 5.0);\n"
                                            "    EXPECT_DOUBLE_EQ(mathlib::divide(5.0, 0.0), 0.0);\n"
                                            "}\n\n"
                                            "TEST(MathLibTest, Factorial) {\n"
                                            "    EXPECT_EQ(mathlib::factorial(0), 1u);\n"
                                            "    EXPECT_EQ(mathlib::factorial(1), 1u);\n"
                                            "    EXPECT_EQ(mathlib::factorial(5), 120u);\n"
                                            "    EXPECT_EQ(mathlib::factorial(10), 3628800u);\n"
                                            "}\n\n"
                                            "TEST(MathLibTest, IsPrime) {\n"
                                            "    EXPECT_FALSE(mathlib::is_prime(0));\n"
                                            "    EXPECT_FALSE(mathlib::is_prime(1));\n"
                                            "    EXPECT_TRUE(mathlib::is_prime(2));\n"
                                            "    EXPECT_TRUE(mathlib::is_prime(3));\n"
                                            "    EXPECT_FALSE(mathlib::is_prime(4));\n"
                                            "    EXPECT_TRUE(mathlib::is_prime(17));\n"
                                            "    EXPECT_FALSE(mathlib::is_prime(25));\n"
                                            "    EXPECT_TRUE(mathlib::is_prime(97));\n"
                                            "}\n\n"
                                            "// 2. Google Mock interface example\n"
                                            "class ICalculator {\n"
                                            "public:\n"
                                            "    virtual ~ICalculator() = default;\n"
                                            "    virtual int compute(int x) = 0;\n"
                                            "};\n\n"
                                            "class MockCalculator : public ICalculator {\n"
                                            "public:\n"
                                            "    MOCK_METHOD(int, compute, (int x), (override));\n"
                                            "};\n\n"
                                            "TEST(MathLibTest, MockExample) {\n"
                                            "    MockCalculator mock;\n"
                                            "    EXPECT_CALL(mock, compute(5)).WillOnce(Return(120));\n"
                                            "    EXPECT_EQ(mock.compute(5), 120);\n"
                                            "}\n";
            files.push_back({"tests/test_math.cpp", test_math_content});

            std::string src_content =
                "#include \"mathlib/math.hpp\"\n\n"
                "#include <iostream>\n\n"
                "int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {\n"
                "    std::cout << \"Hello from " +
                ident_name +
                " built with cxxgen!\\n\";\n"
                "    std::cout << \"mathlib::add(3, 4) = \" << mathlib::add(3, 4) << \"\\n\";\n"
                "    std::cout << \"mathlib::factorial(5) = \" << mathlib::factorial(5) << \"\\n\";\n"
                "    return 0;\n"
                "}\n";
            files.push_back({"src/main.cpp", src_content});

            std::string build_content = "#include \"cxxgen/cxxgen.hpp\"\n\n"
                                        "void build(cxxgen::Builder &b) {\n"
                                        "    b.set_project_name(\"" +
                                        ident_name +
                                        "\");\n"
                                        "    auto opt = b.standard_optimize_options();\n\n"
                                        "    // Google Test & Google Mock dependency\n"
                                        "    auto &googletest = b.dependency(\"googletest\", {\n"
                                        "        .git = \"https://github.com/google/googletest.git\",\n"
                                        "        .tag = \"v1.18.0\",\n"
                                        "    });\n\n"
                                        "    // Build combined gmock_main static library\n"
                                        "    auto &gmock_lib = googletest.add_static_library({\n"
                                        "        .name = \"gmock_main\",\n"
                                        "        .sources = {\n"
                                        "            \"googletest/src/gtest-all.cc\",\n"
                                        "            \"googlemock/src/gmock-all.cc\",\n"
                                        "            \"googlemock/src/gmock_main.cc\",\n"
                                        "        },\n"
                                        "        .include_dirs = {\n"
                                        "            \"googletest/include\",\n"
                                        "            \"googletest\",\n"
                                        "            \"googlemock/include\",\n"
                                        "            \"googlemock\",\n"
                                        "        },\n"
                                        "        .cpp_version = cxxgen::CppVersion::Cpp20,\n"
                                        "    });\n\n"
                                        "    auto &exe = b.add_executable({\n"
                                        "        .name = \"" +
                                        ident_name +
                                        "\",\n"
                                        "        .sources = {\"src/main.cpp\", \"src/mathlib/math.cpp\"},\n"
                                        "        .include_dirs = {\"include\", \"src\"},\n"
                                        "        .optimize = opt.mode,\n"
                                        "    });\n\n"
                                        "    b.install_artifact(exe);\n\n"
                                        "    auto run_cmd = b.add_run_artifact(exe);\n"
                                        "    b.step(\"run\", \"Run the app\").depend_on(run_cmd);\n\n"
                                        "    auto &test_math = b.add_test({\n"
                                        "        .name = \"test_math\",\n"
                                        "        .sources = {\"tests/test_math.cpp\", \"src/mathlib/math.cpp\"},\n"
                                        "        .include_dirs = {\n"
                                        "            \"include\",\n"
                                        "            \"src\",\n"
                                        "            (googletest.path() / \"googletest/include\").string(),\n"
                                        "            (googletest.path() / \"googlemock/include\").string(),\n"
                                        "        },\n"
                                        "        .cpp_version = cxxgen::CppVersion::Cpp20,\n"
                                        "    });\n"
                                        "    test_math.link(gmock_lib);\n"
                                        "#if !defined(_WIN32)\n"
                                        "    test_math.add_system_library(\"pthread\");\n"
                                        "#endif\n"
                                        "}\n\n"
                                        "CXXGEN_MAIN(build)\n";
            files.push_back({"build.cpp", build_content});
        }

        // 1. Transactional existence check (refuse overwrite unless --force)
        if (!config.force) {
            for (const auto &f : files) {
                fs::path full_p = target_dir / f.relative_path;
                if (fs::exists(full_p, ec)) {
                    result.success = false;
                    result.error_message =
                        "cxxgen error: file already exists: '" + full_p.string() + "'. Use --force to overwrite.";
                    return result;
                }
            }
        }

        // 2. Write project files
        for (const auto &f : files) {
            fs::path full_p = target_dir / f.relative_path;
            if (full_p.has_parent_path()) {
                fs::create_directories(full_p.parent_path(), ec);
            }
            std::ofstream out(full_p, std::ios::trunc);
            if (!out.is_open()) {
                result.success = false;
                result.error_message = "cxxgen error: failed to write file: " + full_p.string();
                return result;
            }
            out << f.content;
            result.generated_files.push_back(full_p);
        }

        // 3. Bundle cxxgen.hpp header
        // Primary path: include/cxxgen/cxxgen.hpp
        fs::path dest_header_dir = target_dir / "include" / "cxxgen";
        fs::path dest_header = dest_header_dir / "cxxgen.hpp";

        if (!fs::exists(dest_header, ec) || config.force) {
            fs::create_directories(dest_header_dir, ec);
            if (!config.header_content.empty()) {
                std::ofstream out(dest_header, std::ios::trunc | std::ios::binary);
                if (out.is_open()) {
                    out.write(config.header_content.data(), config.header_content.size());
                    result.generated_files.push_back(dest_header);
                }
            } else {
                fs::path src_header = config.header_source;
                if (src_header.empty()) {
                    const char *env_inc = std::getenv("CXXGEN_INCLUDE_DIR");
                    if (env_inc) {
                        if (fs::exists(fs::path(env_inc) / "cxxgen" / "cxxgen.hpp", ec)) {
                            src_header = fs::path(env_inc) / "cxxgen" / "cxxgen.hpp";
                        } else if (fs::exists(fs::path(env_inc) / "cxxgen.hpp", ec)) {
                            src_header = fs::path(env_inc) / "cxxgen.hpp";
                        }
                    }
                }
                if (src_header.empty()) {
                    if (fs::exists("include/cxxgen/cxxgen.hpp", ec)) {
                        src_header = fs::absolute("include/cxxgen/cxxgen.hpp", ec);
                    } else if (fs::exists("include/cxxgen.hpp", ec)) {
                        src_header = fs::absolute("include/cxxgen.hpp", ec);
                    } else if (fs::exists("../include/cxxgen/cxxgen.hpp", ec)) {
                        src_header = fs::absolute("../include/cxxgen/cxxgen.hpp", ec);
                    } else if (fs::exists("../include/cxxgen.hpp", ec)) {
                        src_header = fs::absolute("../include/cxxgen.hpp", ec);
                    }
                }
                if (!src_header.empty() && fs::exists(src_header, ec) && src_header != dest_header) {
                    fs::copy_file(src_header, dest_header, fs::copy_options::overwrite_existing, ec);
                    result.generated_files.push_back(dest_header);
                }
            }
        }

        result.success = true;
        return result;
    }
};

// ============================================================================
// 8.11. Live File Watcher Mode (cxxgen watch)
// ============================================================================

enum class WatchEvent { Created, Modified, Deleted };

struct FileChangeEvent {
    std::filesystem::path path;
    WatchEvent event = WatchEvent::Modified;
};

struct WatcherConfig {
    std::vector<std::filesystem::path> watch_directories = {"src", "include"};
    std::unordered_set<std::string> filter_extensions = {".cpp", ".cxx", ".cc", ".c",  ".hpp",
                                                         ".hxx", ".hh",  ".h",  ".inl"};
    std::chrono::milliseconds debounce = std::chrono::milliseconds(150);
    std::chrono::milliseconds poll_interval = std::chrono::milliseconds(50);
    bool watch_build_script = true;
};

class FileWatcher {
public:
    using ChangeCallback = std::function<void(const std::vector<FileChangeEvent> &events)>;

    explicit FileWatcher(WatcherConfig config = {}) : config_(std::move(config)) {
        if (config_.watch_directories.empty()) {
            config_.watch_directories.push_back(std::filesystem::current_path());
        }
    }

    ~FileWatcher() { stop(); }

    FileWatcher(const FileWatcher &) = delete;
    FileWatcher &operator=(const FileWatcher &) = delete;

    [[nodiscard]] bool is_running() const noexcept { return running_.load(); }

    struct FileEntry {
        std::filesystem::file_time_type mtime;
        uintmax_t size = 0;
    };
    using Snapshot = std::map<std::filesystem::path, FileEntry>;

    Snapshot take_snapshot() const {
        namespace fs = std::filesystem;
        Snapshot snap;
        std::error_code ec;

        auto should_ignore = [](const fs::path &p) {
            std::string s = p.filename().string();
            return s == ".cxxgen" || s == "bin" || s == "obj" || s == ".git" || s == ".vs" || s == "node_modules";
        };

        auto matches_filter = [this](const fs::path &p) {
            if (config_.watch_build_script && p.filename() == "build.cpp") {
                return true;
            }
            std::string ext = p.extension().string();
            for (char &c : ext) {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            return config_.filter_extensions.find(ext) != config_.filter_extensions.end();
        };

        for (const auto &dir : config_.watch_directories) {
            if (!fs::exists(dir, ec)) {
                continue;
            }
            if (fs::is_regular_file(dir, ec)) {
                if (matches_filter(dir)) {
                    auto t = fs::last_write_time(dir, ec);
                    auto sz = fs::file_size(dir, ec);
                    if (!ec) {
                        snap[fs::canonical(dir, ec)] = FileEntry{.mtime = t, .size = sz};
                    }
                }
                continue;
            }
            for (fs::recursive_directory_iterator it(dir, fs::directory_options::skip_permission_denied, ec), end;
                 it != end && !ec; it.increment(ec)) {
                const auto &entry = *it;
                if (should_ignore(entry.path())) {
                    it.disable_recursion_pending();
                    continue;
                }
                if (entry.is_regular_file(ec) && matches_filter(entry.path())) {
                    auto t = entry.last_write_time(ec);
                    auto sz = entry.file_size(ec);
                    if (!ec) {
                        snap[fs::canonical(entry.path(), ec)] = FileEntry{.mtime = t, .size = sz};
                    }
                }
            }
        }
        return snap;
    }

    static std::vector<FileChangeEvent> diff_snapshots(const Snapshot &old_snap, const Snapshot &new_snap) {
        std::vector<FileChangeEvent> changes;

        // Created or modified
        for (const auto &[path, new_entry] : new_snap) {
            auto it = old_snap.find(path);
            if (it == old_snap.end()) {
                changes.push_back({.path = path, .event = WatchEvent::Created});
            } else if (new_entry.mtime != it->second.mtime || new_entry.size != it->second.size) {
                changes.push_back({.path = path, .event = WatchEvent::Modified});
            }
        }

        // Deleted
        for (const auto &[path, _] : old_snap) {
            if (new_snap.find(path) == new_snap.end()) {
                changes.push_back({.path = path, .event = WatchEvent::Deleted});
            }
        }

        return changes;
    }

    std::vector<FileChangeEvent> scan_changes() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            last_snapshot_ = take_snapshot();
            initialized_ = true;
            return {};
        }
        Snapshot current = take_snapshot();
        auto diff = diff_snapshots(last_snapshot_, current);
        last_snapshot_ = std::move(current);
        return diff;
    }

    void start(ChangeCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (running_.load()) {
            return;
        }
        callback_ = std::move(callback);
        running_.store(true);
        last_snapshot_ = take_snapshot();
        initialized_ = true;

        worker_ = std::thread([this]() {
            Snapshot baseline = last_snapshot_;
            std::map<std::filesystem::path, FileChangeEvent> pending_changes;
            std::chrono::steady_clock::time_point last_change_time;
            bool has_pending = false;

            while (running_.load()) {
                std::this_thread::sleep_for(config_.poll_interval);
                if (!running_.load()) {
                    break;
                }

                Snapshot current = take_snapshot();
                auto changes = diff_snapshots(baseline, current);

                if (!changes.empty()) {
                    for (const auto &c : changes) {
                        pending_changes[c.path] = c;
                    }
                    baseline = current;
                    last_change_time = std::chrono::steady_clock::now();
                    has_pending = true;
                }

                if (has_pending) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - last_change_time);
                    if (elapsed >= config_.debounce) {
                        std::vector<FileChangeEvent> dispatched;
                        dispatched.reserve(pending_changes.size());
                        for (const auto &[_, ev] : pending_changes) {
                            dispatched.push_back(ev);
                        }
                        pending_changes.clear();
                        has_pending = false;

                        if (callback_) {
                            try {
                                callback_(dispatched);
                            } catch (...) {}
                        }
                    }
                }
            }
        });
    }

    void stop() {
        if (!running_.exchange(false)) {
            return;
        }
        if (worker_.joinable()) {
            if (std::this_thread::get_id() == worker_.get_id()) {
                worker_.detach();
            } else {
                worker_.join();
            }
        }
    }

private:
    WatcherConfig config_;
    Snapshot last_snapshot_;
    bool initialized_{false};
    std::atomic<bool> running_{false};
    std::thread worker_;
    std::mutex mutex_;
    ChangeCallback callback_;
};

// ============================================================================
// 8.12. Package & External Dependency Management (b.dependency)
// ============================================================================

struct DependencyLockEntry {
    std::string name = "";
    std::string type = "git"; // "git" or "url" or "path"
    std::string url = "";
    std::string tag = "";
    std::string branch = "";
    std::string commit = "";
    std::string sha256 = "";
};

class DependencyLockfile {
public:
    static constexpr const char *FILENAME = "cxxgen.lock";

    static DependencyLockfile load(const std::filesystem::path &project_root) {
        namespace fs = std::filesystem;
        DependencyLockfile lf;
        lf.lock_path_ = project_root.empty() ? fs::path(FILENAME) : (project_root / FILENAME);
        std::error_code ec;
        if (!fs::exists(lf.lock_path_, ec)) {
            return lf;
        }

        std::ifstream file(lf.lock_path_);
        if (!file.is_open())
            return lf;
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        auto json_opt = util::JsonValue::parse(content);
        if (!json_opt || !json_opt->is_object())
            return lf;

        const auto &root = *json_opt;
        if (root.contains("version")) {
            lf.version_ = static_cast<int>(root["version"].as_int64(1));
        }
        if (root.contains("packages") && root["packages"].is_object()) {
            for (const auto &[pkg_name, pkg_val] : root["packages"].as_object()) {
                if (pkg_val.is_object()) {
                    DependencyLockEntry entry;
                    entry.name = pkg_name;
                    if (pkg_val.contains("type"))
                        entry.type = pkg_val["type"].as_string();
                    if (pkg_val.contains("url"))
                        entry.url = pkg_val["url"].as_string();
                    if (pkg_val.contains("tag"))
                        entry.tag = pkg_val["tag"].as_string();
                    if (pkg_val.contains("branch"))
                        entry.branch = pkg_val["branch"].as_string();
                    if (pkg_val.contains("commit"))
                        entry.commit = pkg_val["commit"].as_string();
                    if (pkg_val.contains("sha256"))
                        entry.sha256 = pkg_val["sha256"].as_string();
                    lf.entries_[pkg_name] = entry;
                }
            }
        }
        lf.exists_on_disk_ = true;
        return lf;
    }

    [[nodiscard]] bool exists_on_disk() const noexcept { return exists_on_disk_; }
    [[nodiscard]] const DependencyLockEntry *find(const std::string &name) const noexcept {
        auto it = entries_.find(name);
        return (it != entries_.end()) ? &it->second : nullptr;
    }

    void set(const DependencyLockEntry &entry) {
        entries_[entry.name] = entry;
        dirty_ = true;
    }

    void remove(const std::string &name) {
        entries_.erase(name);
        dirty_ = true;
    }

    void save(const std::filesystem::path &project_root = {}) const {
        namespace fs = std::filesystem;
        fs::path p = project_root.empty() ? lock_path_ : (project_root / FILENAME);
        if (p.empty())
            p = FILENAME;

        std::map<std::string, util::JsonValue> pkgs_obj;
        for (const auto &[name, entry] : entries_) {
            std::map<std::string, util::JsonValue> e_obj;
            e_obj["type"] = util::JsonValue(entry.type);
            if (!entry.url.empty())
                e_obj["url"] = util::JsonValue(entry.url);
            if (!entry.tag.empty())
                e_obj["tag"] = util::JsonValue(entry.tag);
            if (!entry.branch.empty())
                e_obj["branch"] = util::JsonValue(entry.branch);
            if (!entry.commit.empty())
                e_obj["commit"] = util::JsonValue(entry.commit);
            if (!entry.sha256.empty())
                e_obj["sha256"] = util::JsonValue(entry.sha256);
            pkgs_obj[name] = util::JsonValue(std::move(e_obj));
        }

        std::map<std::string, util::JsonValue> root_obj;
        root_obj["version"] = util::JsonValue(static_cast<double>(version_));
        root_obj["packages"] = util::JsonValue(std::move(pkgs_obj));

        std::error_code ec;
        if (p.has_parent_path()) {
            fs::create_directories(p.parent_path(), ec);
        }
        std::ofstream out(p, std::ios::trunc);
        if (out.is_open()) {
            out << util::JsonValue(std::move(root_obj)).serialize(2) << "\n";
            out.close();
            dirty_ = false;
        }
    }

    [[nodiscard]] const std::map<std::string, DependencyLockEntry> &entries() const noexcept { return entries_; }
    [[nodiscard]] bool is_dirty() const noexcept { return dirty_; }

private:
    int version_ = 1;
    std::filesystem::path lock_path_;
    bool exists_on_disk_ = false;
    mutable bool dirty_ = false;
    std::map<std::string, DependencyLockEntry> entries_;
};

class DependencyFetcher {
public:
    struct FetchResult {
        bool success{false};
        std::filesystem::path package_path{};
        std::string resolved_commit{};
        std::string resolved_sha256{};
        bool cached{false};
        std::string error_message{};
    };

    static std::filesystem::path default_cache_dir() {
        namespace fs = std::filesystem;
        const char *env_cache = std::getenv("CXXGEN_PACKAGE_CACHE");
        if (env_cache && *env_cache) {
            return fs::path(env_cache);
        }
#if defined(_WIN32)
        const char *userprofile = std::getenv("USERPROFILE");
        if (userprofile && *userprofile) {
            return fs::path(userprofile) / ".cxxgen" / "pkg";
        }
#else
        const char *home = std::getenv("HOME");
        if (home && *home) {
            return fs::path(home) / ".cxxgen" / "pkg";
        }
#endif
        return fs::path(".cxxgen") / "pkg";
    }

    static FetchResult fetch(const std::string &name, const DependencyConfig &config) {
        namespace fs = std::filesystem;
        FetchResult result;
        std::error_code ec;

        // Validate name to prevent directory traversal
        if (name.empty() || name.find('/') != std::string::npos || name.find('\\') != std::string::npos ||
            name == "." || name == "..") {
            result.success = false;
            result.error_message =
                "cxxgen error: invalid dependency name '" + name + "': path traversal characters are forbidden";
            return result;
        }

        // 1. If local path is provided directly
        if (!config.path.empty()) {
            fs::path p = fs::absolute(config.path, ec);
            if (!fs::exists(p, ec)) {
                result.success = false;
                result.error_message =
                    "cxxgen error: dependency '" + name + "' local path '" + config.path.string() + "' does not exist";
                return result;
            }
            result.package_path = p;
            result.success = true;
            result.cached = true;

            // Try reading git commit if it is a git repo
            ProcessResult rev = ProcessRunner::run({"git", "-C", p.string(), "rev-parse", "HEAD"});
            if (rev.success()) {
                std::string hash = rev.stdout_output;
                while (!hash.empty() && (hash.back() == '\r' || hash.back() == '\n' || hash.back() == ' ')) {
                    hash.pop_back();
                }
                result.resolved_commit = hash;
            }
            return result;
        }

        // 2. HTTP / HTTPS / Archive file URL
        if (!config.url.empty()) {
            if (config.url[0] == '-') {
                result.success = false;
                result.error_message = "cxxgen error: invalid dependency URL '" + config.url + "'";
                return result;
            }
            fs::path cache_base = config.cache_dir.empty() ? default_cache_dir() : config.cache_dir;
            fs::create_directories(cache_base, ec);

            std::string url_key = config.url + (config.sha256.empty() ? "" : ("@" + config.sha256));
            std::string short_hash = hash::Sha256::hash(url_key).substr(0, 16);
            fs::path pkg_dir = cache_base / (name + "_" + short_hash);
            fs::path ready_marker = pkg_dir / ".cxxgen_pkg_ready";

            if (!config.force_fetch && fs::exists(pkg_dir, ec) && fs::exists(ready_marker, ec)) {
                std::ifstream f(ready_marker);
                if (f.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                    auto json = util::JsonValue::parse(content);
                    if (json && json->is_object()) {
                        result.resolved_sha256 = (*json)["sha256"].as_string();
                    }
                }
                if (result.resolved_sha256.empty()) {
                    result.resolved_sha256 = config.sha256.empty() ? short_hash : config.sha256;
                }
                result.package_path = pkg_dir;
                result.cached = true;
                result.success = true;
                return result;
            }

            // Download or copy archive
            fs::path temp_download =
                cache_base / (".dl_" + name + "_" + short_hash + "_" +
                              std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));

            bool download_ok = false;
            // Check if URL is actually a local file path
            if (fs::exists(config.url, ec)) {
                fs::copy_file(config.url, temp_download, fs::copy_options::overwrite_existing, ec);
                download_ok = !ec;
            } else {
                // Remote download via curl
                ProcessResult curl_res =
                    ProcessRunner::run({"curl", "-fSL", "--output", temp_download.string(), "--", config.url});
                if (curl_res.success() && fs::exists(temp_download, ec) && fs::file_size(temp_download, ec) > 0) {
                    download_ok = true;
                } else {
#if defined(_WIN32)
                    ProcessOptions ps_opts;
                    ps_opts.command = {"powershell", "-NoProfile", "-NonInteractive", "-Command",
                                       "Invoke-WebRequest -Uri $env:CXXGEN_DL_URL -OutFile $env:CXXGEN_DL_OUT"};
                    ps_opts.extra_env["CXXGEN_DL_URL"] = config.url;
                    ps_opts.extra_env["CXXGEN_DL_OUT"] = temp_download.string();
                    ProcessResult ps_res = ProcessRunner::run(ps_opts);
                    if (ps_res.success() && fs::exists(temp_download, ec) && fs::file_size(temp_download, ec) > 0) {
                        download_ok = true;
                    } else {
                        result.error_message =
                            "cxxgen error: failed to download dependency '" + name + "' from '" + config.url +
                            "': " + (curl_res.stderr_output.empty() ? ps_res.stderr_output : curl_res.stderr_output);
                    }
#else
                    result.error_message = "cxxgen error: failed to download dependency '" + name + "' from '" +
                                           config.url + "': " + curl_res.stderr_output;
#endif
                }
            }

            if (!download_ok || !fs::exists(temp_download, ec)) {
                result.success = false;
                if (result.error_message.empty()) {
                    result.error_message =
                        "cxxgen error: failed to download dependency '" + name + "' from '" + config.url + "'";
                }
                return result;
            }

            // Verify SHA-256
            std::string actual_sha = hash::Sha256::hash_file(temp_download);
            if (!config.sha256.empty()) {
                std::string exp_sha = config.sha256;
                std::string act_lower = util::to_lower(actual_sha);
                std::string exp_lower = util::to_lower(exp_sha);
                if (act_lower != exp_lower) {
                    fs::remove(temp_download, ec);
                    result.success = false;
                    result.error_message = "cxxgen error: SHA-256 integrity check failed for dependency '" + name +
                                           "': expected '" + exp_sha + "', but got '" + actual_sha + "'";
                    return result;
                }
            }

            if (fs::exists(pkg_dir, ec)) {
                fs::remove_all(pkg_dir, ec);
            }
            fs::create_directories(pkg_dir, ec);

            // Extract archive
            std::string url_lower = util::to_lower(config.url);
            bool extracted = false;
            if (url_lower.ends_with(".zip") || url_lower.find(".zip?") != std::string::npos) {
                // Try bsdtar first
                ProcessResult pres = ProcessRunner::run({"tar", "-xf", temp_download.string(), "-C", pkg_dir.string()});
                if (pres.success()) {
                    extracted = true;
                } else {
#if defined(_WIN32)
                    ProcessOptions ps_opts;
                    ps_opts.command = {
                        "powershell", "-NoProfile", "-NonInteractive", "-Command",
                        "Expand-Archive -Path $env:CXXGEN_EXTRACT_IN -DestinationPath $env:CXXGEN_EXTRACT_OUT -Force"};
                    ps_opts.extra_env["CXXGEN_EXTRACT_IN"] = temp_download.string();
                    ps_opts.extra_env["CXXGEN_EXTRACT_OUT"] = pkg_dir.string();
                    ProcessResult ps_res = ProcessRunner::run(ps_opts);
                    extracted = ps_res.success();
#else
                    ProcessResult ures =
                        ProcessRunner::run({"unzip", "-q", temp_download.string(), "-d", pkg_dir.string()});
                    extracted = ures.success();
#endif
                }
            } else if (url_lower.ends_with(".tar.gz") || url_lower.ends_with(".tgz") || url_lower.ends_with(".tar") ||
                       url_lower.find(".tar.gz?") != std::string::npos ||
                       url_lower.find(".tgz?") != std::string::npos) {
                ProcessResult pres = ProcessRunner::run({"tar", "-xf", temp_download.string(), "-C", pkg_dir.string()});
                extracted = pres.success();
            } else {
                // Single source or header file
                std::string raw_url = config.url;
                auto q_idx = raw_url.find('?');
                if (q_idx != std::string::npos)
                    raw_url = raw_url.substr(0, q_idx);
                auto hash_idx = raw_url.find('#');
                if (hash_idx != std::string::npos)
                    raw_url = raw_url.substr(0, hash_idx);
                fs::path single_fname = fs::path(raw_url).filename();
                if (single_fname.empty() || single_fname == "." || single_fname == ".." ||
                    single_fname.string().find('/') != std::string::npos ||
                    single_fname.string().find('\\') != std::string::npos) {
                    single_fname = name + ".h";
                }
                fs::path dest_file = pkg_dir / single_fname;
                fs::path norm_dest = dest_file.lexically_normal();
                fs::path norm_pkg = pkg_dir.lexically_normal();
                auto [b_norm, m_norm] =
                    std::mismatch(norm_pkg.begin(), norm_pkg.end(), norm_dest.begin(), norm_dest.end());
                if (b_norm != norm_pkg.end()) {
                    dest_file = pkg_dir / (name + ".h");
                }
                fs::copy_file(temp_download, dest_file, fs::copy_options::overwrite_existing, ec);
                extracted = !ec;
            }

            fs::remove(temp_download, ec);

            if (!extracted) {
                result.success = false;
                result.error_message = "cxxgen error: failed to extract dependency archive for '" + name + "'";
                return result;
            }

            // Post-extraction security verification: Zip Slip and escaping symlink defense
            fs::path canonical_base = fs::weakly_canonical(pkg_dir, ec);
            for (auto it = fs::recursive_directory_iterator(pkg_dir, fs::directory_options::none, ec);
                 it != fs::recursive_directory_iterator(); it.increment(ec)) {
                if (ec)
                    break;
                fs::path entry_canonical = fs::weakly_canonical(it->path(), ec);
                auto [b_base, m_ent] = std::mismatch(canonical_base.begin(), canonical_base.end(),
                                                     entry_canonical.begin(), entry_canonical.end());
                if (b_base != canonical_base.end()) {
                    fs::remove_all(pkg_dir, ec);
                    result.success = false;
                    result.error_message = "cxxgen security error: archive contains malicious path traversal or "
                                           "escaping symlink (Zip Slip)";
                    return result;
                }
            }

            // Flatten single subfolder if present
            std::vector<fs::path> subdirs;
            std::vector<fs::path> subfiles;
            for (const auto &it : fs::directory_iterator(pkg_dir, ec)) {
                if (it.is_directory(ec))
                    subdirs.push_back(it.path());
                else
                    subfiles.push_back(it.path());
            }
            if (subdirs.size() == 1 && subfiles.empty()) {
                fs::path single_dir = subdirs[0];
                for (const auto &it : fs::directory_iterator(single_dir, ec)) {
                    fs::rename(it.path(), pkg_dir / it.path().filename(), ec);
                }
                fs::remove(single_dir, ec);
            }

            // Write ready marker
            std::map<std::string, util::JsonValue> marker;
            marker["name"] = util::JsonValue(name);
            marker["url"] = util::JsonValue(config.url);
            marker["sha256"] = util::JsonValue(actual_sha);
            marker["ready"] = util::JsonValue(true);

            std::ofstream mf(ready_marker, std::ios::trunc);
            if (mf.is_open()) {
                mf << util::JsonValue(std::move(marker)).serialize(2);
                mf.close();
            }

            result.success = true;
            result.package_path = pkg_dir;
            result.resolved_sha256 = actual_sha;
            result.cached = false;
            return result;
        }

        // 3. Git repository
        if (config.git.empty()) {
            result.success = false;
            result.error_message =
                "cxxgen error: dependency '" + name + "' has neither 'git' URL, 'url', nor 'path' specified";
            return result;
        }

        std::string git_trimmed = config.git;
        while (!git_trimmed.empty() && std::isspace(static_cast<unsigned char>(git_trimmed.front()))) {
            git_trimmed.erase(0, 1);
        }
        if (git_trimmed.empty() || git_trimmed[0] == '-') {
            result.success = false;
            result.error_message = "cxxgen error: invalid git URL '" + config.git + "'";
            return result;
        }
        std::string git_lower = util::to_lower(git_trimmed);
        if (git_lower.rfind("ext::", 0) == 0 || git_lower.rfind("fd::", 0) == 0) {
            result.success = false;
            result.error_message = "cxxgen security error: dangerous git protocol rejected in URL '" + config.git + "'";
            return result;
        }
        for (char c : config.git) {
            if (c == '\r' || c == '\n' || c == '\0') {
                result.success = false;
                result.error_message = "cxxgen error: invalid characters in git URL";
                return result;
            }
        }

        auto validate_ref = [](const std::string &ref, const std::string &type_name) -> std::string {
            if (ref.empty())
                return "";
            if (ref[0] == '-')
                return "cxxgen error: git " + type_name + " cannot start with '-'";
            for (char c : ref) {
                if (std::isspace(static_cast<unsigned char>(c)) || c == '\r' || c == '\n' || c == '\0' || c == ';' ||
                    c == '&' || c == '|' || c == '$' || c == '`') {
                    return "cxxgen error: git " + type_name + " contains illegal characters";
                }
            }
            return "";
        };

        std::string ref_err = validate_ref(config.tag, "tag");
        if (ref_err.empty())
            ref_err = validate_ref(config.branch, "branch");
        if (ref_err.empty())
            ref_err = validate_ref(config.commit, "commit");
        if (!ref_err.empty()) {
            result.success = false;
            result.error_message = ref_err;
            return result;
        }

        // Compute permanent cache directory
        fs::path cache_base = config.cache_dir.empty() ? default_cache_dir() : config.cache_dir;
        fs::create_directories(cache_base, ec);

        std::string ref_key = !config.tag.empty()      ? config.tag
                              : !config.branch.empty() ? config.branch
                              : !config.commit.empty() ? config.commit
                                                       : "HEAD";

        std::string url_hash = hash::Sha256::hash(config.git + "@" + ref_key);
        std::string short_hash = url_hash.substr(0, 16);
        fs::path pkg_dir = cache_base / (name + "_" + short_hash);
        fs::path ready_marker = pkg_dir / ".cxxgen_pkg_ready";

        auto try_cached_dir = [&](const fs::path &dir, std::string &out_commit) -> bool {
            fs::path marker = dir / ".cxxgen_pkg_ready";
            if (!fs::exists(dir, ec) || !fs::exists(marker, ec)) {
                return false;
            }
            std::ifstream f(marker);
            if (!f.is_open()) {
                return false;
            }
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            auto json = util::JsonValue::parse(content);
            if (!json || !json->is_object()) {
                return false;
            }
            std::string c = (*json)["commit"].as_string();
            if (c.empty()) {
                c = dir.filename().string();
            }
            if (!config.commit.empty() && c != config.commit) {
                return false;
            }
            out_commit = c;
            return true;
        };

        // Check if already cached
        if (!config.force_fetch) {
            std::string cached_commit;
            if (try_cached_dir(pkg_dir, cached_commit)) {
                result.resolved_commit = cached_commit;
                result.package_path = pkg_dir;
                result.cached = true;
                result.success = true;
                return result;
            }
            // If commit was populated from lockfile/config and differs from ref_key, also check commit-based cache dir
            if (!config.commit.empty() && config.commit != ref_key) {
                std::string c_hash = hash::Sha256::hash(config.git + "@" + config.commit).substr(0, 16);
                fs::path c_dir = cache_base / (name + "_" + c_hash);
                if (try_cached_dir(c_dir, cached_commit)) {
                    result.resolved_commit = cached_commit;
                    result.package_path = c_dir;
                    result.cached = true;
                    result.success = true;
                    return result;
                }
            }
        }

        // Clone repository
        if (fs::exists(pkg_dir, ec)) {
            fs::remove_all(pkg_dir, ec);
        }
        fs::create_directories(pkg_dir.parent_path(), ec);

        std::vector<std::string> clone_cmd = {"git", "-c", "protocol.ext.allow=never", "clone"};
        if (config.shallow) {
            if (!config.tag.empty()) {
                clone_cmd.push_back("--depth");
                clone_cmd.push_back("1");
                clone_cmd.push_back("--branch");
                clone_cmd.push_back(config.tag);
            } else if (!config.branch.empty()) {
                clone_cmd.push_back("--depth");
                clone_cmd.push_back("1");
                clone_cmd.push_back("--branch");
                clone_cmd.push_back(config.branch);
            }
        }
        clone_cmd.push_back("--");
        clone_cmd.push_back(config.git);
        clone_cmd.push_back(pkg_dir.string());

        ProcessResult clone_res = ProcessRunner::run(clone_cmd);
        if (!clone_res.success()) {
            // Shallow clone fallback
            if (config.shallow && (!config.tag.empty() || !config.branch.empty())) {
                fs::remove_all(pkg_dir, ec);
                std::vector<std::string> fallback_cmd = {
                    "git", "-c", "protocol.ext.allow=never", "clone", "--", config.git, pkg_dir.string()};
                clone_res = ProcessRunner::run(fallback_cmd);
                if (clone_res.success()) {
                    std::string ref = !config.tag.empty() ? config.tag : config.branch;
                    ProcessResult chk = ProcessRunner::run({"git", "-C", pkg_dir.string(), "checkout", ref});
                    if (!chk.success()) {
                        result.success = false;
                        result.error_message = "cxxgen error: failed to checkout '" + ref + "' in dependency '" + name +
                                               "': " + chk.stderr_output;
                        return result;
                    }
                }
            }
            if (!clone_res.success()) {
                result.success = false;
                result.error_message = "cxxgen error: failed to clone dependency '" + name + "' from '" + config.git +
                                       "':\n" + clone_res.stderr_output +
                                       "\nHint: Ensure git is installed and in PATH, and the repository URL and "
                                       "network/credentials are accessible.";
                return result;
            }
        }

        // If specific commit was requested, checkout that commit if HEAD is not already there
        if (!config.commit.empty()) {
            ProcessResult rev = ProcessRunner::run({"git", "-C", pkg_dir.string(), "rev-parse", "HEAD"});
            std::string current_head = rev.success() ? util::trim(rev.stdout_output) : "";
            if (current_head != config.commit) {
                ProcessResult chk = ProcessRunner::run({"git", "-C", pkg_dir.string(), "checkout", config.commit});
                if (!chk.success()) {
                    result.success = false;
                    result.error_message = "cxxgen error: failed to checkout commit '" + config.commit +
                                           "' in dependency '" + name + "': " + chk.stderr_output;
                    return result;
                }
            }
        }

        // Query resolved commit hash
        ProcessResult rev_res = ProcessRunner::run({"git", "-C", pkg_dir.string(), "rev-parse", "HEAD"});
        std::string commit_hash;
        if (rev_res.success()) {
            commit_hash = rev_res.stdout_output;
            while (!commit_hash.empty() &&
                   (commit_hash.back() == '\r' || commit_hash.back() == '\n' || commit_hash.back() == ' ')) {
                commit_hash.pop_back();
            }
        } else {
            commit_hash = short_hash;
        }

        // Write ready marker
        std::map<std::string, util::JsonValue> marker;
        marker["name"] = util::JsonValue(name);
        marker["url"] = util::JsonValue(config.git);
        marker["commit"] = util::JsonValue(commit_hash);
        marker["tag"] = util::JsonValue(config.tag);
        marker["branch"] = util::JsonValue(config.branch);
        marker["ready"] = util::JsonValue(true);

        std::ofstream mf(ready_marker, std::ios::trunc);
        if (mf.is_open()) {
            mf << util::JsonValue(std::move(marker)).serialize(2);
            mf.close();
        }

        result.success = true;
        result.package_path = pkg_dir;
        result.resolved_commit = commit_hash;
        result.cached = false;
        return result;
    }
};

using GitDependencyFetcher = DependencyFetcher;

class Dependency {
public:
    Dependency(std::string name, std::filesystem::path package_path, Builder &parent_builder,
               std::string commit_hash = "", std::string sha256_hash = "")
        : name_(std::move(name)), package_path_(std::move(package_path)), parent_builder_(parent_builder),
          commit_hash_(std::move(commit_hash)), sha256_hash_(std::move(sha256_hash)) {}

    [[nodiscard]] const std::string &name() const noexcept { return name_; }
    [[nodiscard]] const std::filesystem::path &path() const noexcept { return package_path_; }
    [[nodiscard]] const std::string &commit_hash() const noexcept { return commit_hash_; }
    [[nodiscard]] const std::string &sha256_hash() const noexcept { return sha256_hash_; }

    [[nodiscard]] bool is_cached() const noexcept { return is_cached_; }
    void set_cached(bool cached) noexcept { is_cached_ = cached; }

    [[nodiscard]] Target &target(std::string_view target_name) {
        Target *t = parent_builder_.find_target(target_name);
        if (!t) {
            std::string prefixed = name_ + "::" + std::string(target_name);
            t = parent_builder_.find_target(prefixed);
        }
        if (!t) {
            throw std::runtime_error("cxxgen error: target '" + std::string(target_name) +
                                     "' not found in dependency '" + name_ + "'");
        }
        return *t;
    }

    [[nodiscard]] Target &artifact(std::string_view target_name) { return target(target_name); }

    [[nodiscard]] bool has_target(std::string_view target_name) const {
        if (parent_builder_.find_target(target_name))
            return true;
        std::string prefixed = name_ + "::" + std::string(target_name);
        return parent_builder_.find_target(prefixed) != nullptr;
    }

    [[nodiscard]] const std::vector<std::string> &target_names() const noexcept { return target_names_; }

    void register_imported_target(std::string target_name) {
        if (std::find(target_names_.begin(), target_names_.end(), target_name) == target_names_.end()) {
            target_names_.push_back(std::move(target_name));
        }
    }

    void adjust_config_paths(TargetConfig &config) const {
        namespace fs = std::filesystem;
        for (auto &src : config.sources) {
            fs::path p(src);
            if (!p.is_absolute()) {
                src = (package_path_ / p).lexically_normal().string();
            }
        }
        for (auto &hdr : config.headers) {
            fs::path p(hdr);
            if (!p.is_absolute()) {
                hdr = (package_path_ / p).lexically_normal().string();
            }
        }
        for (auto &inc : config.include_dirs) {
            fs::path p(inc);
            if (!p.is_absolute()) {
                inc = (package_path_ / p).lexically_normal().string();
            }
        }
        std::error_code ec;
        if (config.include_dirs.empty() && fs::exists(package_path_ / "include", ec)) {
            config.include_dirs.push_back((package_path_ / "include").lexically_normal().string());
        }
        for (auto &ld : config.link_directories) {
            fs::path p(ld);
            if (!p.is_absolute()) {
                ld = (package_path_ / p).lexically_normal().string();
            }
        }
        if (!config.pch_header.empty()) {
            fs::path p(config.pch_header);
            if (!p.is_absolute()) {
                config.pch_header = (package_path_ / p).lexically_normal().string();
            }
        }
        if (!config.pch_source.empty()) {
            fs::path p(config.pch_source);
            if (!p.is_absolute()) {
                config.pch_source = (package_path_ / p).lexically_normal().string();
            }
        }
    }

    Target &add_static_library(TargetConfig config) {
        adjust_config_paths(config);
        config.type = TargetType::StaticLibrary;
        Target &t = parent_builder_.register_target(std::move(config));
        register_imported_target(t.name());
        return t;
    }

    Target &add_shared_library(TargetConfig config) {
        adjust_config_paths(config);
        config.type = TargetType::SharedLibrary;
        Target &t = parent_builder_.register_target(std::move(config));
        register_imported_target(t.name());
        return t;
    }

    Target &add_interface_library(TargetConfig config) {
        adjust_config_paths(config);
        config.type = TargetType::Interface;
        Target &t = parent_builder_.register_target(std::move(config));
        register_imported_target(t.name());
        return t;
    }

    Target &add_executable(TargetConfig config) {
        adjust_config_paths(config);
        config.type = TargetType::Executable;
        Target &t = parent_builder_.register_target(std::move(config));
        register_imported_target(t.name());
        return t;
    }

    void import_build(std::function<void(Builder &)> build_fn) {
        Builder child_builder;
        build_fn(child_builder);
        std::string manifest_str = child_builder.dump_manifest_json();
        std::vector<std::string> imported;
        parent_builder_.import_manifest_from_string(manifest_str, package_path_, &imported);
        for (const auto &tname : imported) {
            register_imported_target(tname);
        }
    }

    [[nodiscard]] std::filesystem::path include_dir(const std::filesystem::path &sub = "include") const {
        return (package_path_ / sub).lexically_normal();
    }

    [[nodiscard]] std::filesystem::path source_dir(const std::filesystem::path &sub = "src") const {
        return (package_path_ / sub).lexically_normal();
    }

private:
    std::string name_;
    std::filesystem::path package_path_;
    Builder &parent_builder_;
    std::string commit_hash_;
    std::string sha256_hash_;
    bool is_cached_{false};
    std::vector<std::string> target_names_;
};

// Builder manifest & dependency method implementations
inline std::string Builder::dump_manifest_json() const {
    std::vector<util::JsonValue> target_entries;
    for (const auto &t : targets_) {
        std::map<std::string, util::JsonValue> m;
        m["name"] = util::JsonValue(t->name());
        m["type"] = util::JsonValue(to_string(t->type()));

        auto serialize_str_vec = [](const std::vector<std::string> &vec) {
            std::vector<util::JsonValue> arr;
            arr.reserve(vec.size());
            for (const auto &s : vec) {
                arr.emplace_back(s);
            }
            return util::JsonValue(std::move(arr));
        };

        m["sources"] = serialize_str_vec(t->config().sources);
        m["headers"] = serialize_str_vec(t->config().headers);
        m["include_dirs"] = serialize_str_vec(t->config().include_dirs);
        m["defines"] = serialize_str_vec(t->config().defines);
        m["compile_options"] = serialize_str_vec(t->config().compile_options);
        m["link_options"] = serialize_str_vec(t->config().link_options);
        m["link_libraries"] = serialize_str_vec(t->config().link_libraries);
        m["link_directories"] = serialize_str_vec(t->config().link_directories);
        m["linked_targets"] = serialize_str_vec(t->linked_targets());
        m["linked_system_libs"] = serialize_str_vec(t->linked_system_libs());
        m["output_dir"] = util::JsonValue(t->config().output_dir);
        m["output_name"] = util::JsonValue(t->config().output_name);
        m["pch_header"] = util::JsonValue(t->config().pch_header);
        m["pch_source"] = util::JsonValue(t->config().pch_source);

        target_entries.emplace_back(std::move(m));
    }

    std::map<std::string, util::JsonValue> root_map;
    root_map["version"] = util::JsonValue(static_cast<int64_t>(1));
    root_map["targets"] = util::JsonValue(std::move(target_entries));
    return util::JsonValue(std::move(root_map)).serialize(2);
}

inline bool Builder::dump_manifest_to_file(const std::filesystem::path &filepath) const {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (filepath.has_parent_path()) {
        fs::create_directories(filepath.parent_path(), ec);
    }
    std::ofstream f(filepath, std::ios::trunc);
    if (!f.is_open()) {
        return false;
    }
    f << dump_manifest_json();
    return true;
}

inline bool Builder::import_manifest_from_string(std::string_view json_str, const std::filesystem::path &pkg_root,
                                                 std::vector<std::string> *imported_names) {
    namespace fs = std::filesystem;
    auto root = util::JsonValue::parse(json_str);
    if (!root || !root->is_object()) {
        return false;
    }

    auto targets_val = (*root)["targets"];
    if (!targets_val.is_array()) {
        return false;
    }

    auto deserialize_str_vec = [](const util::JsonValue &val) {
        std::vector<std::string> res;
        if (val.is_array()) {
            for (const auto &item : val.as_array()) {
                if (item.is_string()) {
                    res.push_back(item.as_string());
                }
            }
        }
        return res;
    };

    auto adjust_path = [&](const std::string &p) -> std::string {
        if (p.empty() || pkg_root.empty()) {
            return p;
        }
        fs::path path_obj(p);
        if (path_obj.is_absolute()) {
            return p;
        }
        return (pkg_root / path_obj).lexically_normal().string();
    };

    for (const auto &tval : targets_val.as_array()) {
        if (!tval.is_object()) {
            continue;
        }
        TargetConfig cfg;
        cfg.name = tval["name"].as_string();
        auto parsed_type = parse_target_type(tval["type"].as_string());
        cfg.type = parsed_type.value_or(TargetType::Executable);

        auto raw_sources = deserialize_str_vec(tval["sources"]);
        for (const auto &s : raw_sources) {
            cfg.sources.push_back(adjust_path(s));
        }

        auto raw_headers = deserialize_str_vec(tval["headers"]);
        for (const auto &h : raw_headers) {
            cfg.headers.push_back(adjust_path(h));
        }

        auto raw_includes = deserialize_str_vec(tval["include_dirs"]);
        for (const auto &inc : raw_includes) {
            cfg.include_dirs.push_back(adjust_path(inc));
        }

        cfg.defines = deserialize_str_vec(tval["defines"]);
        cfg.compile_options = deserialize_str_vec(tval["compile_options"]);
        cfg.link_options = deserialize_str_vec(tval["link_options"]);
        cfg.link_libraries = deserialize_str_vec(tval["link_libraries"]);
        cfg.link_directories = deserialize_str_vec(tval["link_directories"]);
        for (auto &ld : cfg.link_directories) {
            ld = adjust_path(ld);
        }

        cfg.output_dir = tval["output_dir"].as_string();
        cfg.output_name = tval["output_name"].as_string();
        std::string raw_pch_h = tval["pch_header"].as_string();
        cfg.pch_header = adjust_path(raw_pch_h);
        std::string raw_pch_s = tval["pch_source"].as_string();
        cfg.pch_source = adjust_path(raw_pch_s);

        if (find_target(cfg.name)) {
            if (imported_names) {
                imported_names->push_back(cfg.name);
            }
            continue;
        }

        Target &registered = register_target(std::move(cfg));
        auto linked_t = deserialize_str_vec(tval["linked_targets"]);
        for (const auto &lt : linked_t) {
            registered.link(lt);
        }
        auto linked_sys = deserialize_str_vec(tval["linked_system_libs"]);
        for (const auto &ls : linked_sys) {
            registered.link_system_library(ls);
        }

        if (imported_names) {
            imported_names->push_back(registered.name());
        }
    }
    return true;
}

inline bool Builder::import_manifest(const std::filesystem::path &filepath, const std::filesystem::path &pkg_root,
                                     std::vector<std::string> *imported_names) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::exists(filepath, ec)) {
        return false;
    }
    std::ifstream f(filepath);
    if (!f.is_open()) {
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    return import_manifest_from_string(content, pkg_root, imported_names);
}

inline Dependency &Builder::dependency(std::string name, DependencyConfig config) {
    auto it = dependency_lookup_.find(name);
    if (it != dependency_lookup_.end()) {
        return *dependencies_[it->second];
    }

    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path project_root = working_directory_.empty() ? fs::current_path(ec) : working_directory_;
    DependencyLockfile lockfile = DependencyLockfile::load(project_root);

    // 1. If in locked mode, verify against lockfile
    if (is_locked()) {
        const auto *locked_entry = lockfile.find(name);
        if (!locked_entry) {
            throw std::runtime_error("cxxgen error: dependency '" + name +
                                     "' is not recorded in cxxgen.lock (--locked mode is active)");
        }
        if (!config.git.empty()) {
            if (config.commit.empty()) {
                config.commit = locked_entry->commit;
            } else if (config.commit != locked_entry->commit) {
                throw std::runtime_error("cxxgen error: commit mismatch for dependency '" + name +
                                         "': requested commit '" + config.commit + "' but cxxgen.lock specifies '" +
                                         locked_entry->commit + "' (--locked mode is active)");
            }
        } else if (!config.url.empty()) {
            if (config.sha256.empty()) {
                config.sha256 = locked_entry->sha256;
            } else if (!util::iequals(config.sha256, locked_entry->sha256)) {
                throw std::runtime_error("cxxgen error: SHA-256 mismatch for dependency '" + name +
                                         "': requested hash '" + config.sha256 + "' but cxxgen.lock specifies '" +
                                         locked_entry->sha256 + "' (--locked mode is active)");
            }
        }
    } else {
        // Not in locked mode: check if update is requested
        if (should_update_package(name)) {
            config.force_fetch = true;
        } else {
            const auto *locked_entry = lockfile.find(name);
            if (locked_entry) {
                if (config.commit.empty() && !config.git.empty()) {
                    config.commit = locked_entry->commit;
                }
                if (config.sha256.empty() && !config.url.empty()) {
                    config.sha256 = locked_entry->sha256;
                }
            }
        }
    }

    auto fetch_res = DependencyFetcher::fetch(name, config);
    if (!fetch_res.success) {
        throw std::runtime_error(fetch_res.error_message);
    }

    // Update lockfile on successful resolution (if not in locked mode)
    if (!is_locked()) {
        DependencyLockEntry entry;
        entry.name = name;
        if (!config.git.empty()) {
            entry.type = "git";
            entry.url = config.git;
            entry.tag = config.tag;
            entry.branch = config.branch;
            entry.commit = fetch_res.resolved_commit;
        } else if (!config.url.empty()) {
            entry.type = "url";
            entry.url = config.url;
            entry.sha256 = fetch_res.resolved_sha256;
        } else if (!config.path.empty()) {
            entry.type = "path";
            entry.url = config.path.string();
            entry.commit = fetch_res.resolved_commit;
        }
        lockfile.set(entry);
        lockfile.save(project_root);
    }

    auto dep_ptr = std::make_unique<Dependency>(name, fetch_res.package_path, *this, fetch_res.resolved_commit,
                                                fetch_res.resolved_sha256);
    dep_ptr->set_cached(fetch_res.cached);

    // If child repository has a build.cpp, import its targets!
    namespace fs = std::filesystem;
    fs::path child_build = fetch_res.package_path / "build.cpp";
    if (fs::exists(child_build, ec)) {
        JitRunnerConfig jcfg;
        jcfg.build_script = child_build;
        jcfg.working_dir = fetch_res.package_path;
        JitResult jres = JitRunner::prepare_runner(jcfg);
        if (!jres.success) {
            throw std::runtime_error("cxxgen error: failed to prepare runner for dependency '" + name +
                                     "': " + jres.error_message);
        }

        fs::path manifest_path = fetch_res.package_path / ".cxxgen" / "manifest.json";
        fs::create_directories(manifest_path.parent_path(), ec);

        int code = JitRunner::execute_runner(jres.runner_binary, {"dump-manifest", manifest_path.string()},
                                             fetch_res.package_path);
        if (code != 0 || !fs::exists(manifest_path, ec)) {
            throw std::runtime_error("cxxgen error: child build.cpp in dependency '" + name +
                                     "' failed to dump target manifest");
        }

        std::vector<std::string> imported_targets;
        if (!import_manifest(manifest_path, fetch_res.package_path, &imported_targets)) {
            throw std::runtime_error("cxxgen error: failed to import target manifest for dependency '" + name + "'");
        }

        for (const auto &tname : imported_targets) {
            dep_ptr->register_imported_target(tname);
        }
    }

    Dependency &ref = *dep_ptr;
    size_t idx = dependencies_.size();
    dependencies_.push_back(std::move(dep_ptr));
    dependency_lookup_[name] = idx;
    return ref;
}

inline Dependency *Builder::find_dependency(std::string_view name) noexcept {
    auto it = dependency_lookup_.find(std::string(name));
    if (it != dependency_lookup_.end()) {
        return dependencies_[it->second].get();
    }
    return nullptr;
}

inline const Dependency *Builder::find_dependency(std::string_view name) const noexcept {
    auto it = dependency_lookup_.find(std::string(name));
    if (it != dependency_lookup_.end()) {
        return dependencies_[it->second].get();
    }
    return nullptr;
}

// ============================================================================
// 8.13. System Package Discovery Bridge (b.find_system_package)
// ============================================================================

inline std::vector<std::string> split_arguments(std::string_view sv) {
    std::vector<std::string> args;
    std::string cur;
    bool in_quote = false;
    char quote_char = '\0';
    for (size_t i = 0; i < sv.size(); ++i) {
        char c = sv[i];
        if (in_quote) {
            if (c == quote_char) {
                in_quote = false;
            } else {
                cur.push_back(c);
            }
        } else {
            if (c == '"' || c == '\'') {
                in_quote = true;
                quote_char = c;
            } else if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                if (!cur.empty()) {
                    args.push_back(std::move(cur));
                    cur.clear();
                }
            } else {
                cur.push_back(c);
            }
        }
    }
    if (!cur.empty()) {
        args.push_back(std::move(cur));
    }
    return args;
}

inline void parse_pkg_config_cflags(std::string_view output, std::vector<std::string> &include_dirs,
                                    std::vector<std::string> &defines, std::vector<std::string> &compile_flags) {
    std::vector<std::string> tokens = split_arguments(output);
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string &tok = tokens[i];
        if (tok.rfind("-I", 0) == 0) {
            std::string path = tok.substr(2);
            if (path.empty() && i + 1 < tokens.size()) {
                path = tokens[++i];
            }
            if (!path.empty() && std::find(include_dirs.begin(), include_dirs.end(), path) == include_dirs.end()) {
                include_dirs.push_back(path);
            }
        } else if (tok.rfind("-D", 0) == 0) {
            std::string def = tok.substr(2);
            if (def.empty() && i + 1 < tokens.size()) {
                def = tokens[++i];
            }
            if (!def.empty() && std::find(defines.begin(), defines.end(), def) == defines.end()) {
                defines.push_back(def);
            }
        } else if (!tok.empty()) {
            compile_flags.push_back(tok);
        }
    }
}

inline void parse_pkg_config_libs(std::string_view output, std::vector<std::string> &lib_dirs,
                                  std::vector<std::string> &link_libraries, std::vector<std::string> &link_flags) {
    std::vector<std::string> tokens = split_arguments(output);
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string &tok = tokens[i];
        if (tok.rfind("-L", 0) == 0) {
            std::string path = tok.substr(2);
            if (path.empty() && i + 1 < tokens.size()) {
                path = tokens[++i];
            }
            if (!path.empty() && std::find(lib_dirs.begin(), lib_dirs.end(), path) == lib_dirs.end()) {
                lib_dirs.push_back(path);
            }
        } else if (tok.rfind("-l", 0) == 0) {
            std::string lib = tok.substr(2);
            if (lib.empty() && i + 1 < tokens.size()) {
                lib = tokens[++i];
            }
            if (!lib.empty() && std::find(link_libraries.begin(), link_libraries.end(), lib) == link_libraries.end()) {
                link_libraries.push_back(lib);
            }
        } else if (!tok.empty()) {
            link_flags.push_back(tok);
        }
    }
}

class SystemPackageBridge {
public:
    static SystemPackage find_package(std::string_view name, const SystemPackageConfig &config = {}) {
        SystemPackage pkg;
        pkg.name = std::string(name);

        if (name.empty() || name[0] == '-' || !util::is_safe_package_name(name)) {
            pkg.found = false;
            pkg.error_message = "cxxgen error: invalid system package name '" + std::string(name) + "'";
            if (config.required) {
                throw std::runtime_error(pkg.error_message);
            }
            return pkg;
        }

        std::string lower_name = detail::to_lower_copy(name);

        // 1. Try pkg-config
        if (try_pkg_config(name, config, pkg)) {
            pkg.found = true;
            return pkg;
        }

        // 2. Try standard / environment search paths
        if (try_environment_and_standard_paths(lower_name, config, pkg)) {
            pkg.found = true;
            return pkg;
        }

        // 3. Try user hints
        if (try_hints(lower_name, config, pkg)) {
            pkg.found = true;
            return pkg;
        }

        // 4. Missing package handling
        pkg.found = false;
        pkg.install_instructions = generate_install_instructions(lower_name, config);
        pkg.error_message = "cxxgen error: system package '" + std::string(name) + "' not found on this system.\n" +
                            pkg.install_instructions;

        if (config.required) {
            throw std::runtime_error(pkg.error_message);
        }

        return pkg;
    }

private:
    static bool try_pkg_config(std::string_view name, const SystemPackageConfig &config, SystemPackage &pkg) {
        ProcessResult check = ProcessRunner::run({"pkg-config", "--version"});
        if (!check.success()) {
            return false;
        }

        std::vector<std::string> exists_cmd = {"pkg-config", "--exists"};
        if (!config.version.empty()) {
            exists_cmd.push_back("--atleast-version=" + config.version);
        }
        exists_cmd.push_back(std::string(name));

        ProcessResult res = ProcessRunner::run(exists_cmd);
        if (!res.success()) {
            return false;
        }

        ProcessResult ver_res = ProcessRunner::run({"pkg-config", "--modversion", std::string(name)});
        if (ver_res.success()) {
            std::string v = ver_res.stdout_output;
            while (!v.empty() && (v.back() == '\r' || v.back() == '\n' || v.back() == ' '))
                v.pop_back();
            pkg.version = v;
        }

        ProcessResult cflags_res = ProcessRunner::run({"pkg-config", "--cflags", std::string(name)});
        if (cflags_res.success()) {
            parse_pkg_config_cflags(cflags_res.stdout_output, pkg.include_dirs, pkg.defines, pkg.compile_flags);
        }

        ProcessResult libs_res = ProcessRunner::run({"pkg-config", "--libs", std::string(name)});
        if (libs_res.success()) {
            parse_pkg_config_libs(libs_res.stdout_output, pkg.lib_dirs, pkg.link_libraries, pkg.link_flags);
        }

        return true;
    }

    static bool try_environment_and_standard_paths(std::string_view lower_name, const SystemPackageConfig &config,
                                                   SystemPackage &pkg) {
        (void)config;
        namespace fs = std::filesystem;
        std::error_code ec;

        // Vulkan
        if (lower_name == "vulkan" || lower_name == "vulkan-sdk") {
            const char *vulkan_env = std::getenv("VULKAN_SDK");
            if (vulkan_env && *vulkan_env) {
                fs::path base(vulkan_env);
                fs::path inc = base / "Include";
                if (!fs::exists(inc, ec))
                    inc = base / "include";
                if (fs::exists(inc, ec)) {
                    pkg.include_dirs.push_back(inc.string());
#if defined(_WIN32)
                    fs::path lib_dir = base / "Lib";
                    if (!fs::exists(lib_dir, ec))
                        lib_dir = base / "lib";
                    if (fs::exists(lib_dir, ec))
                        pkg.lib_dirs.push_back(lib_dir.string());
                    pkg.link_libraries.push_back("vulkan-1");
#else
                    fs::path lib_dir = base / "lib";
                    if (fs::exists(lib_dir, ec))
                        pkg.lib_dirs.push_back(lib_dir.string());
                    pkg.link_libraries.push_back("vulkan");
#endif
                    return true;
                }
            }
#if !defined(_WIN32)
            if (fs::exists("/usr/include/vulkan/vulkan.h", ec)) {
                pkg.link_libraries.push_back("vulkan");
                return true;
            }
#endif
        }

        // OpenGL
        if (lower_name == "opengl" || lower_name == "gl") {
#if defined(_WIN32)
            pkg.link_libraries.push_back("opengl32");
            pkg.link_libraries.push_back("glu32");
            return true;
#elif defined(__APPLE__)
            pkg.link_flags.push_back("-framework");
            pkg.link_flags.push_back("OpenGL");
            return true;
#else
            if (fs::exists("/usr/include/GL/gl.h", ec) || fs::exists("/usr/local/include/GL/gl.h", ec)) {
                pkg.link_libraries.push_back("GL");
                return true;
            }
#endif
        }

        // ZLIB
        if (lower_name == "zlib" || lower_name == "z") {
            const char *zlib_env = std::getenv("ZLIB_ROOT");
            if (!zlib_env)
                zlib_env = std::getenv("ZLIB_DIR");
            if (zlib_env && *zlib_env) {
                fs::path base(zlib_env);
                fs::path inc = base / "include";
                if (fs::exists(inc, ec)) {
                    pkg.include_dirs.push_back(inc.string());
                    fs::path lib = base / "lib";
                    if (fs::exists(lib, ec))
                        pkg.lib_dirs.push_back(lib.string());
#if defined(_WIN32)
                    pkg.link_libraries.push_back("zlib");
#else
                    pkg.link_libraries.push_back("z");
#endif
                    return true;
                }
            }
#if !defined(_WIN32)
            if (fs::exists("/usr/include/zlib.h", ec) || fs::exists("/usr/local/include/zlib.h", ec)) {
                pkg.link_libraries.push_back("z");
                return true;
            }
#endif
        }

        // SDL2
        if (lower_name == "sdl2") {
            const char *sdl_env = std::getenv("SDL2_DIR");
            if (!sdl_env)
                sdl_env = std::getenv("SDL2_PATH");
            if (sdl_env && *sdl_env) {
                fs::path base(sdl_env);
                fs::path inc = base / "include";
                if (fs::exists(inc / "SDL2", ec))
                    inc = inc / "SDL2";
                if (fs::exists(inc, ec)) {
                    pkg.include_dirs.push_back(inc.string());
                    fs::path lib = base / "lib";
                    if (fs::exists(lib, ec))
                        pkg.lib_dirs.push_back(lib.string());
                    pkg.link_libraries.push_back("SDL2");
#if defined(_WIN32)
                    pkg.link_libraries.push_back("SDL2main");
#endif
                    return true;
                }
            }
#if !defined(_WIN32)
            if (fs::exists("/usr/include/SDL2/SDL.h", ec)) {
                pkg.include_dirs.push_back("/usr/include/SDL2");
                pkg.link_libraries.push_back("SDL2");
                return true;
            }
#endif
        }

        // Standard Windows API libraries
#if defined(_WIN32)
        if (lower_name == "ws2_32" || lower_name == "winsock2" || lower_name == "winsock") {
            pkg.link_libraries.push_back("ws2_32");
            return true;
        }
        if (lower_name == "user32" || lower_name == "gdi32" || lower_name == "kernel32" || lower_name == "shell32" ||
            lower_name == "advapi32" || lower_name == "ole32" || lower_name == "d3d11" || lower_name == "d3d12" ||
            lower_name == "dxgi") {
            pkg.link_libraries.push_back(std::string(lower_name));
            return true;
        }
#endif

        // Standard POSIX libraries
#if !defined(_WIN32)
        if (lower_name == "pthread" || lower_name == "dl" || lower_name == "m" || lower_name == "rt") {
            pkg.link_libraries.push_back(std::string(lower_name));
            return true;
        }
#endif

        return false;
    }

    static bool try_hints(std::string_view lower_name, const SystemPackageConfig &config, SystemPackage &pkg) {
        namespace fs = std::filesystem;
        std::error_code ec;

        bool found_any = false;
        for (const auto &inc : config.include_hints) {
            fs::path p(inc);
            if (fs::exists(p, ec) && fs::is_directory(p, ec)) {
                pkg.include_dirs.push_back(inc);
                found_any = true;
            }
        }

        for (const auto &lib_dir : config.lib_hints) {
            fs::path p(lib_dir);
            if (fs::exists(p, ec) && fs::is_directory(p, ec)) {
                pkg.lib_dirs.push_back(lib_dir);
                for (const auto &entry : fs::directory_iterator(p, ec)) {
                    std::string stem = entry.path().stem().string();
                    std::string stem_lower = detail::to_lower_copy(stem);
                    if (stem_lower.find(lower_name) != std::string::npos) {
                        std::string ext = entry.path().extension().string();
                        if (ext == ".lib" || ext == ".a" || ext == ".so" || ext == ".dylib") {
                            std::string lib_name = stem;
                            if (lib_name.rfind("lib", 0) == 0 && lib_name.size() > 3 && ext != ".lib") {
                                lib_name = lib_name.substr(3);
                            }
                            if (std::find(pkg.link_libraries.begin(), pkg.link_libraries.end(), lib_name) ==
                                pkg.link_libraries.end()) {
                                pkg.link_libraries.push_back(lib_name);
                            }
                            found_any = true;
                        }
                    }
                }
            }
        }

        return found_any;
    }

    static std::string generate_install_instructions(std::string_view lower_name, const SystemPackageConfig &config) {
        if (!config.install_hint.empty()) {
            return "Actionable Installation Hint: " + config.install_hint + "\n";
        }

        std::string s = "Actionable Installation Instructions:\n";
#if defined(_WIN32)
        s += "  - vcpkg:  vcpkg install " + std::string(lower_name) + "\n";
        s += "  - scoop:  scoop install " + std::string(lower_name) + "\n";
        s += "  - Or provide explicit include and library hints via SystemPackageConfig:\n";
        s += "      b.find_system_package(\"" + std::string(lower_name) +
             "\", {.include_hints = {\"path/to/include\"}, .lib_hints = {\"path/to/lib\"}});\n";
#elif defined(__APPLE__)
        s += "  - Homebrew: brew install " + std::string(lower_name) + "\n";
#else
        s += "  - Ubuntu/Debian: sudo apt-get install lib" + std::string(lower_name) + "-dev\n";
        s += "  - Fedora/RHEL:   sudo dnf install " + std::string(lower_name) + "-devel\n";
        s += "  - Arch Linux:    sudo pacman -S " + std::string(lower_name) + "\n";
#endif
        return s;
    }
};

inline Target &Target::link(const SystemPackage &pkg) {
    for (const auto &inc : pkg.include_dirs) {
        if (std::find(config_.include_dirs.begin(), config_.include_dirs.end(), inc) == config_.include_dirs.end()) {
            config_.include_dirs.push_back(inc);
        }
    }
    for (const auto &def : pkg.defines) {
        if (std::find(config_.defines.begin(), config_.defines.end(), def) == config_.defines.end()) {
            config_.defines.push_back(def);
        }
    }
    for (const auto &opt : pkg.compile_flags) {
        if (std::find(config_.compile_options.begin(), config_.compile_options.end(), opt) ==
            config_.compile_options.end()) {
            config_.compile_options.push_back(opt);
        }
    }
    for (const auto &ld : pkg.lib_dirs) {
        if (std::find(config_.link_directories.begin(), config_.link_directories.end(), ld) ==
            config_.link_directories.end()) {
            config_.link_directories.push_back(ld);
        }
    }
    for (const auto &lib : pkg.link_libraries) {
        link_system_lib(lib);
    }
    for (const auto &lopt : pkg.link_flags) {
        if (std::find(config_.link_options.begin(), config_.link_options.end(), lopt) == config_.link_options.end()) {
            config_.link_options.push_back(lopt);
        }
    }
    return *this;
}

inline SystemPackage Builder::find_system_package(std::string name, SystemPackageConfig config) {
    return SystemPackageBridge::find_package(name, config);
}

inline Target &Builder::add_system_package(std::string name, SystemPackageConfig config) {
    SystemPackage pkg = find_system_package(name, config);
    TargetConfig tcfg;
    tcfg.name = "sys::" + name;
    tcfg.type = TargetType::Interface;
    tcfg.include_dirs = pkg.include_dirs;
    tcfg.defines = pkg.defines;
    tcfg.compile_options = pkg.compile_flags;
    tcfg.link_directories = pkg.lib_dirs;
    tcfg.link_libraries = pkg.link_libraries;
    tcfg.link_options = pkg.link_flags;
    Target &ref = register_target(std::move(tcfg));
    return ref;
}

// ============================================================================
// 8.14. Code Formatter (cxxgen fmt)
// ============================================================================

inline std::optional<std::string> find_clang_tidy();
inline std::optional<std::string> find_llvm_profdata();
inline std::optional<std::string> find_llvm_cov();
inline std::optional<std::string> find_gcov();

inline std::optional<std::string> find_clang_format() {
    namespace fs = std::filesystem;
    std::vector<std::string> candidates = {"clang-format"};
#if defined(_WIN32)
    candidates.push_back("clang-format.exe");
    const char *userprofile = std::getenv("USERPROFILE");
    if (userprofile) {
        fs::path scoop_fmt = fs::path(userprofile) / "scoop/apps/llvm/current/bin/clang-format.exe";
        if (fs::exists(scoop_fmt)) {
            candidates.insert(candidates.begin(), scoop_fmt.string());
        }
    }
    const char *pf = std::getenv("ProgramFiles");
    if (pf) {
        fs::path pf_fmt = fs::path(pf) / "LLVM/bin/clang-format.exe";
        if (fs::exists(pf_fmt)) {
            candidates.push_back(pf_fmt.string());
        }
    }
#endif
    auto clang = find_clang_toolchain();
    if (clang) {
        fs::path cdir = fs::path(clang->compiler_path()).parent_path();
#if defined(_WIN32)
        fs::path sibling = cdir / "clang-format.exe";
#else
        fs::path sibling = cdir / "clang-format";
#endif
        std::error_code ec;
        if (fs::exists(sibling, ec)) {
            candidates.insert(candidates.begin(), sibling.string());
        }
    }

    for (const auto &c : candidates) {
        std::error_code ec;
        if (fs::exists(c, ec) && !fs::is_directory(c, ec)) {
            return c;
        }
        auto res = ProcessRunner::run({c, "--version"});
        if (res.success()) {
            return c;
        }
    }
    return std::nullopt;
}

struct FormatConfig {
    std::vector<std::filesystem::path> paths = {"."};
    bool check = false;
    bool verbose = false;
};

struct FormatResult {
    bool success = true;
    size_t files_scanned = 0;
    size_t files_formatted = 0;
    std::vector<std::string> mismatched_files = {};
    std::string error_message = "";
};

class CodeFormatter {
public:
    static bool is_formattable(const std::filesystem::path &p) {
        std::string fname = p.filename().string();
        if (fname.rfind("embedded_", 0) == 0) {
            return false;
        }
        std::string ext = p.extension().string();
        for (char &c : ext) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return ext == ".cpp" || ext == ".hpp" || ext == ".cc" || ext == ".cxx" || ext == ".c" || ext == ".h" ||
               ext == ".hh" || ext == ".hxx" || ext == ".ixx" || ext == ".cppm";
    }

    static bool is_ignored_directory(const std::string &name) {
        return name == ".git" || name == ".cxxgen" || name == "bin" || name == "obj" || name == "build" ||
               name == "out" || name == ".vs" || name == ".cache" || name == "node_modules" || name == "scratch" ||
               name == "test_compile_commands_fixture" || name.rfind(".cxxgen_test_", 0) == 0 ||
               name.rfind("attempt", 0) == 0;
    }

    static FormatResult format_all(const std::string &clang_format_path, const FormatConfig &config) {
        namespace fs = std::filesystem;
        FormatResult result;

        std::vector<fs::path> target_files;
        std::error_code ec;

        for (const auto &input_path : config.paths) {
            fs::path p = input_path;
            if (!fs::exists(p, ec)) {
                result.success = false;
                result.error_message = "cxxgen error: path does not exist: " + p.string();
                return result;
            }

            if (fs::is_regular_file(p, ec)) {
                if (is_formattable(p)) {
                    target_files.push_back(p);
                }
            } else if (fs::is_directory(p, ec)) {
                for (auto it = fs::recursive_directory_iterator(p, fs::directory_options::skip_permission_denied, ec);
                     it != fs::recursive_directory_iterator(); ++it) {
                    if (it.depth() > 25) {
                        it.pop();
                        continue;
                    }
                    if (it->is_directory(ec)) {
                        std::string dirname = it->path().filename().string();
                        if (is_ignored_directory(dirname)) {
                            it.disable_recursion_pending();
                            continue;
                        }
                    } else if (it->is_regular_file(ec)) {
                        if (is_formattable(it->path())) {
                            target_files.push_back(it->path());
                        }
                    }
                }
            }
        }

        result.files_scanned = target_files.size();

        for (const auto &f : target_files) {
            if (config.check) {
                std::ifstream ifs(f, std::ios::binary);
                if (!ifs.is_open()) {
                    result.success = false;
                    result.error_message = "cxxgen error: unable to read file: " + f.string();
                    return result;
                }
                std::string orig((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                ifs.close();

                auto pres = ProcessRunner::run({clang_format_path, f.string()});
                if (!pres.success()) {
                    result.success = false;
                    result.error_message = "clang-format error on " + f.string() + ":\n" + pres.combined_output();
                    return result;
                }
                std::string orig_norm = orig;
                orig_norm.erase(std::remove(orig_norm.begin(), orig_norm.end(), '\r'), orig_norm.end());
                std::string out_norm = pres.stdout_output;
                out_norm.erase(std::remove(out_norm.begin(), out_norm.end(), '\r'), out_norm.end());

                if (orig_norm != out_norm) {
                    result.mismatched_files.push_back(f.string());
                }
            } else {
                auto pres = ProcessRunner::run({clang_format_path, "-i", f.string()});
                if (!pres.success()) {
                    result.success = false;
                    result.error_message = "clang-format error on " + f.string() + ":\n" + pres.combined_output();
                    return result;
                }
                result.files_formatted++;
                if (config.verbose) {
                    std::cout << "  formatted: " << f.string() << "\n";
                }
            }
        }

        return result;
    }
};

// ============================================================================
// 8.15. System Environment & Toolchain Diagnostics (cxxgen doctor)
// ============================================================================

enum class DoctorStatus { Ok, Warning, Missing, Info };

struct DoctorItem {
    std::string category;
    std::string name;
    DoctorStatus status = DoctorStatus::Missing;
    std::string version = "";
    std::string path = "";
    std::string details = "";
    std::string install_hint = "";
};

struct DoctorReport {
    std::vector<DoctorItem> items;
    int ok_count = 0;
    int warning_count = 0;
    int missing_count = 0;
    bool has_compiler = false;
};

class SystemDoctor {
public:
    static std::optional<std::string> find_in_path(const std::string &executable_name) {
        namespace fs = std::filesystem;
        const char *path_env = std::getenv("PATH");
        if (!path_env) {
            return std::nullopt;
        }

        std::vector<std::string> exts = {""};
#if defined(_WIN32)
        exts.push_back(".exe");
        exts.push_back(".cmd");
        exts.push_back(".bat");
        char delim = ';';
#else
        char delim = ':';
#endif

        std::string path_str(path_env);
        size_t start = 0;
        while (start < path_str.size()) {
            size_t end = path_str.find(delim, start);
            if (end == std::string::npos) {
                end = path_str.size();
            }
            std::string dir_str = path_str.substr(start, end - start);
            start = end + 1;
            if (dir_str.empty()) {
                continue;
            }

            fs::path dir(dir_str);
            for (const auto &ext : exts) {
                fs::path candidate = dir / (executable_name + ext);
                std::error_code ec;
                if (fs::exists(candidate, ec) && !fs::is_directory(candidate, ec)) {
                    return candidate.string();
                }
            }
        }
        return std::nullopt;
    }

    static std::pair<bool, std::string> probe_executable_version(const std::string &exe_path,
                                                                 const std::string &flag = "--version") {
        std::string cmd = "\"" + exe_path + "\" " + flag;
#if defined(_WIN32)
        FILE *pipe = _popen(cmd.c_str(), "r");
#else
        FILE *pipe = popen(cmd.c_str(), "r");
#endif
        if (!pipe) {
            return {false, ""};
        }

        char buf[512];
        std::string first_line;
        while (fgets(buf, sizeof(buf), pipe) != nullptr) {
            std::string line(buf);
            while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
                line.pop_back();
            }
            if (!line.empty() && first_line.empty()) {
                first_line = line;
                break;
            }
        }
#if defined(_WIN32)
        int code = _pclose(pipe);
#else
        int code = pclose(pipe);
#endif
        return {code == 0, first_line};
    }

    static DoctorReport diagnose(bool verbose = false) {
        namespace fs = std::filesystem;
        (void)verbose;
        DoctorReport report;

        // 1. Compilers
        // 1a. Clang
        {
            DoctorItem item;
            item.category = "Compilers";
            item.name = "LLVM Clang (clang++)";
            auto clang = find_clang_toolchain();
            if (clang) {
                item.status = DoctorStatus::Ok;
                item.path = clang->compiler_path();
                auto [ok, ver] = probe_executable_version(item.path, "--version");
                item.version = ok && !ver.empty() ? ver : clang->name();
                report.has_compiler = true;
            } else {
                item.status = DoctorStatus::Warning;
                item.details = "Not detected on system PATH or standard directories";
#if defined(_WIN32)
                item.install_hint = "Install via scoop: 'scoop install llvm' or winget: 'winget install LLVM.LLVM'";
#elif defined(__APPLE__)
                item.install_hint = "Install via Homebrew: 'brew install llvm'";
#else
                item.install_hint = "Install via package manager: 'sudo apt install clang'";
#endif
            }
            report.items.push_back(item);
        }

        // 1b. GCC
        {
            DoctorItem item;
            item.category = "Compilers";
            item.name = "GCC (g++)";
            auto gcc = find_gcc_toolchain();
            if (gcc) {
                item.status = DoctorStatus::Ok;
                item.path = gcc->compiler_path();
                auto [ok, ver] = probe_executable_version(item.path, "--version");
                item.version = ok && !ver.empty() ? ver : gcc->name();
                report.has_compiler = true;
            } else {
                item.status = DoctorStatus::Warning;
                item.details = "Not detected on system PATH or standard directories";
#if defined(_WIN32)
                item.install_hint = "Install via scoop: 'scoop install gcc' or MSYS2: 'pacman -S mingw-w64-x86_64-gcc'";
#elif defined(__APPLE__)
                item.install_hint = "Install via Homebrew: 'brew install gcc'";
#else
                item.install_hint = "Install via package manager: 'sudo apt install g++'";
#endif
            }
            report.items.push_back(item);
        }

        // 1c. MSVC (Windows only)
#if defined(_WIN32)
        {
            DoctorItem item;
            item.category = "Compilers";
            item.name = "Microsoft Visual C++ (MSVC / cl.exe)";
            auto msvc = find_msvc_environment();
            if (msvc.found) {
                item.status = DoctorStatus::Ok;
                item.path = msvc.cl_path;
                item.version = "MSVC v" + msvc.version;
                if (!msvc.installation_path.empty()) {
                    item.details = "Installed in: " + msvc.installation_path;
                }
                report.has_compiler = true;
            } else {
                item.status = DoctorStatus::Warning;
                item.details = "No Visual Studio C++ toolchain found via vswhere";
                item.install_hint = "Install Build Tools: 'winget install Microsoft.VisualStudio.2022.BuildTools "
                                    "--override \"--add Microsoft.VisualStudio.Workload.VCTools --passive\"'";
            }
            report.items.push_back(item);
        }
#endif
        // 1d. Emscripten (WebAssembly SDK)
        {
            DoctorItem item;
            item.category = "Compilers";
            item.name = "Emscripten SDK (em++ / WebAssembly)";
            auto em = find_emscripten_toolchain();
            if (em) {
                item.status = DoctorStatus::Ok;
                item.path = em->compiler_path();
                auto [ok, ver] = probe_executable_version(item.path, "--version");
                item.version = ok && !ver.empty() ? ver : em->name();
            } else {
                item.status = DoctorStatus::Info;
                item.details = "Optional: required for compiling C++ to WebAssembly (.wasm / .html)";
                item.install_hint = "Install EMSDK: 'git clone https://github.com/emscripten-core/emsdk.git && ./emsdk "
                                    "install latest && ./emsdk activate latest'";
            }
            report.items.push_back(item);
        }

        // 2. Platform SDKs
#if defined(_WIN32)
        {
            DoctorItem item;
            item.category = "Platform SDKs";
            item.name = "Windows 10/11 SDK & Universal C Runtime";

            auto msvc = find_msvc_environment();
            std::string sdk_ver;
            std::string sdk_dir;
            if (msvc.found) {
                auto it_ver = msvc.env_vars.find("WindowsSDKVersion");
                if (it_ver != msvc.env_vars.end()) {
                    sdk_ver = it_ver->second;
                }
                auto it_dir = msvc.env_vars.find("WindowsSdkDir");
                if (it_dir != msvc.env_vars.end()) {
                    sdk_dir = it_dir->second;
                }
            }

            if (sdk_ver.empty()) {
                std::vector<fs::path> kit_dirs = {"C:\\Program Files (x86)\\Windows Kits\\10\\Include",
                                                  "C:\\Program Files\\Windows Kits\\10\\Include"};
                for (const auto &kd : kit_dirs) {
                    std::error_code ec;
                    if (fs::exists(kd, ec)) {
                        for (const auto &entry : fs::directory_iterator(kd, ec)) {
                            if (entry.is_directory(ec)) {
                                std::string dir_name = entry.path().filename().string();
                                if (dir_name.rfind("10.0.", 0) == 0) {
                                    if (dir_name > sdk_ver) {
                                        sdk_ver = dir_name;
                                        sdk_dir = entry.path().string();
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (!sdk_ver.empty()) {
                item.status = DoctorStatus::Ok;
                while (!sdk_ver.empty() && (sdk_ver.back() == '\\' || sdk_ver.back() == '/')) {
                    sdk_ver.pop_back();
                }
                item.version = sdk_ver;
                item.path = sdk_dir;
            } else {
                item.status = DoctorStatus::Warning;
                item.details = "Windows SDK headers/libraries not located";
                item.install_hint =
                    "Install Windows SDK via Visual Studio Installer or 'winget install Microsoft.WindowsSDK.10'";
            }
            report.items.push_back(item);
        }
#elif defined(__APPLE__)
        {
            DoctorItem item;
            item.category = "Platform SDKs";
            item.name = "macOS Apple SDK / CommandLineTools";
            auto [ok, path] = probe_executable_version("xcrun", "--show-sdk-path");
            if (ok && !path.empty()) {
                item.status = DoctorStatus::Ok;
                item.path = path;
                item.version = "Apple SDK";
            } else {
                item.status = DoctorStatus::Warning;
                item.details = "Xcode Command Line Tools not detected";
                item.install_hint = "Install via terminal: 'xcode-select --install'";
            }
            report.items.push_back(item);
        }
#else
        {
            DoctorItem item;
            item.category = "Platform SDKs";
            item.name = "Linux C Runtime (libc-dev)";
            std::error_code ec;
            if (fs::exists("/usr/include/features.h", ec) || fs::exists("/usr/include/stdlib.h", ec)) {
                item.status = DoctorStatus::Ok;
                item.path = "/usr/include";
                item.version = "Standard C Library Headers";
            } else {
                item.status = DoctorStatus::Warning;
                item.details = "Standard C headers (/usr/include) not found";
                item.install_hint =
                    "Install libc development headers: 'sudo apt install libc6-dev' or 'sudo dnf install glibc-devel'";
            }
            report.items.push_back(item);
        }
#endif

        // 3. Developer & Language Tools
        // 3a. clang-format
        {
            DoctorItem item;
            item.category = "Developer Tools";
            item.name = "clang-format (Code Formatter)";
            auto fmt_path = find_clang_format();
            if (fmt_path) {
                item.status = DoctorStatus::Ok;
                item.path = *fmt_path;
                auto [ok, ver] = probe_executable_version(*fmt_path, "--version");
                item.version = ok && !ver.empty() ? ver : "Available";
            } else {
                item.status = DoctorStatus::Warning;
                item.details = "Not detected (required for 'cxxgen fmt')";
#if defined(_WIN32)
                item.install_hint = "Install via scoop: 'scoop install llvm' or winget: 'winget install LLVM.LLVM'";
#else
                item.install_hint = "Install via package manager: 'sudo apt install clang-format'";
#endif
            }
            report.items.push_back(item);
        }

        // 3b. clang-tidy
        {
            DoctorItem item;
            item.category = "Developer Tools";
            item.name = "clang-tidy (Static Analyzer & Linter)";
            auto tidy_path = find_clang_tidy();
            if (tidy_path) {
                item.status = DoctorStatus::Ok;
                item.path = *tidy_path;
                auto [ok, ver] = probe_executable_version(*tidy_path, "--version");
                item.version = ok && !ver.empty() ? ver : "Available";
            } else {
                item.status = DoctorStatus::Info;
                item.details = "Optional (enables static analysis and code checks)";
#if defined(_WIN32)
                item.install_hint = "Install via scoop: 'scoop install llvm'";
#else
                item.install_hint = "Install via package manager: 'sudo apt install clang-tidy'";
#endif
            }
            report.items.push_back(item);
        }

        // 3c. clangd
        {
            DoctorItem item;
            item.category = "Developer Tools";
            item.name = "clangd (Language Server Protocol)";
            std::vector<std::string> clangd_candidates = {"clangd"};
#if defined(_WIN32)
            clangd_candidates.push_back("clangd.exe");
            const char *up = std::getenv("USERPROFILE");
            if (up) {
                clangd_candidates.push_back((fs::path(up) / "scoop/apps/llvm/current/bin/clangd.exe").string());
            }
            const char *pf = std::getenv("ProgramFiles");
            if (pf) {
                clangd_candidates.push_back((fs::path(pf) / "LLVM/bin/clangd.exe").string());
            }
#endif
            auto clang = find_clang_toolchain();
            if (clang) {
                fs::path cdir = fs::path(clang->compiler_path()).parent_path();
#if defined(_WIN32)
                clangd_candidates.insert(clangd_candidates.begin(), (cdir / "clangd.exe").string());
#else
                clangd_candidates.insert(clangd_candidates.begin(), (cdir / "clangd").string());
#endif
            }

            std::string found_path;
            for (const auto &c : clangd_candidates) {
                std::error_code ec;
                if (fs::exists(c, ec) && !fs::is_directory(c, ec)) {
                    found_path = c;
                    break;
                }
                auto in_p = find_in_path(c);
                if (in_p) {
                    found_path = *in_p;
                    break;
                }
            }

            if (!found_path.empty()) {
                item.status = DoctorStatus::Ok;
                item.path = found_path;
                auto [ok, ver] = probe_executable_version(found_path, "--version");
                item.version = ok && !ver.empty() ? ver : "Available";
            } else {
                item.status = DoctorStatus::Info;
                item.details = "Optional (provides real-time IDE diagnostics for compile_commands.json)";
#if defined(_WIN32)
                item.install_hint = "Install via scoop: 'scoop install llvm'";
#else
                item.install_hint = "Install via package manager: 'sudo apt install clangd'";
#endif
            }
            report.items.push_back(item);
        }

        // 3d. llvm-profdata
        {
            DoctorItem item;
            item.category = "Developer Tools";
            item.name = "llvm-profdata (Coverage Profile Merger)";
            auto prof_path = find_llvm_profdata();
            if (prof_path) {
                item.status = DoctorStatus::Ok;
                item.path = *prof_path;
                auto [ok, ver] = probe_executable_version(*prof_path, "--version");
                item.version = ok && !ver.empty() ? ver : "Available";
            } else {
                item.status = DoctorStatus::Info;
                item.details = "Optional (required for 'cxxgen test --coverage')";
#if defined(_WIN32)
                item.install_hint = "Install via scoop: 'scoop install llvm'";
#else
                item.install_hint = "Install via package manager: 'sudo apt install llvm'";
#endif
            }
            report.items.push_back(item);
        }

        // 3e. llvm-cov
        {
            DoctorItem item;
            item.category = "Developer Tools";
            item.name = "llvm-cov (Code Coverage Tool)";
            auto cov_path = find_llvm_cov();
            if (cov_path) {
                item.status = DoctorStatus::Ok;
                item.path = *cov_path;
                auto [ok, ver] = probe_executable_version(*cov_path, "--version");
                item.version = ok && !ver.empty() ? ver : "Available";
            } else {
                item.status = DoctorStatus::Info;
                item.details = "Optional (required for 'cxxgen test --coverage')";
#if defined(_WIN32)
                item.install_hint = "Install via scoop: 'scoop install llvm'";
#else
                item.install_hint = "Install via package manager: 'sudo apt install llvm'";
#endif
            }
            report.items.push_back(item);
        }

        // 3f. git
        {
            DoctorItem item;
            item.category = "Developer Tools";
            item.name = "git (Version Control & Package Manager)";
            auto git_path = find_in_path("git");
            if (git_path) {
                item.status = DoctorStatus::Ok;
                item.path = *git_path;
                auto [ok, ver] = probe_executable_version(*git_path, "--version");
                item.version = ok && !ver.empty() ? ver : "Available";
            } else {
                item.status = DoctorStatus::Warning;
                item.details = "Not detected (required for b.dependency remote git packages)";
#if defined(_WIN32)
                item.install_hint = "Install via winget: 'winget install Git.Git' or scoop: 'scoop install git'";
#else
                item.install_hint = "Install via package manager: 'sudo apt install git'";
#endif
            }
            report.items.push_back(item);
        }

        // 3e. ninja
        {
            DoctorItem item;
            item.category = "Build Generators";
            item.name = "ninja (Build System Exporter)";
            auto ninja_path = find_in_path("ninja");
            if (ninja_path) {
                item.status = DoctorStatus::Ok;
                item.path = *ninja_path;
                auto [ok, ver] = probe_executable_version(*ninja_path, "--version");
                item.version = ok && !ver.empty() ? ver : "Available";
            } else {
                item.status = DoctorStatus::Info;
                item.details = "Optional (enables executing 'cxxgen generate ninja' outputs)";
#if defined(_WIN32)
                item.install_hint =
                    "Install via scoop: 'scoop install ninja' or winget: 'winget install Ninja-build.Ninja'";
#else
                item.install_hint = "Install via package manager: 'sudo apt install ninja-build'";
#endif
            }
            report.items.push_back(item);
        }

        // 3f. pkg-config
        {
            DoctorItem item;
            item.category = "Build Generators";
            item.name = "pkg-config (System Package Discovery)";
            auto pkg_path = find_in_path("pkg-config");
            if (!pkg_path) {
                pkg_path = find_in_path("pkgconf");
            }
            if (pkg_path) {
                item.status = DoctorStatus::Ok;
                item.path = *pkg_path;
                auto [ok, ver] = probe_executable_version(*pkg_path, "--version");
                item.version = ok && !ver.empty() ? ver : "Available";
            } else {
                item.status = DoctorStatus::Info;
                item.details = "Optional (enables automatic library resolution for b.find_system_package)";
#if defined(_WIN32)
                item.install_hint = "Install via scoop: 'scoop install pkg-config-lite'";
#elif defined(__APPLE__)
                item.install_hint = "Install via Homebrew: 'brew install pkg-config'";
#else
                item.install_hint = "Install via package manager: 'sudo apt install pkg-config'";
#endif
            }
            report.items.push_back(item);
        }

        // Count totals
        for (const auto &item : report.items) {
            if (item.status == DoctorStatus::Ok) {
                report.ok_count++;
            } else if (item.status == DoctorStatus::Warning) {
                report.warning_count++;
            } else if (item.status == DoctorStatus::Missing) {
                report.missing_count++;
            }
        }

        return report;
    }

    static void print_report(const DoctorReport &report, std::ostream &os = std::cout, bool verbose = false) {
        using namespace terminal;
        (void)verbose;
        os << "================================================================================\n";
        os << bold("cxxgen doctor - Development Environment & Toolchain Diagnostics\n");
        os << "================================================================================\n\n";

        std::string current_category = "";
        std::vector<DoctorItem> recommendations;

        for (const auto &item : report.items) {
            if (item.category != current_category) {
                current_category = item.category;
                os << "[" << bold(current_category) << "]\n";
            }

            std::string mark;
            if (item.status == DoctorStatus::Ok) {
                mark = colorize("[OK]", Color::Green, Style::Bold);
            } else if (item.status == DoctorStatus::Warning) {
                mark = colorize("[!]", Color::Yellow, Style::Bold);
            } else if (item.status == DoctorStatus::Missing) {
                mark = colorize("[X]", Color::Red, Style::Bold);
            } else {
                mark = colorize("[-]", Color::Cyan, Style::None);
            }

            os << "  " << mark << " " << bold(item.name) << "\n";
            if (!item.version.empty()) {
                os << "       Version: " << item.version << "\n";
            }
            if (!item.path.empty()) {
                os << "       Path:    " << item.path << "\n";
            }
            if (!item.details.empty()) {
                os << "       Details: " << item.details << "\n";
            }
            if (!item.install_hint.empty() && item.status != DoctorStatus::Ok) {
                recommendations.push_back(item);
            }
        }

        if (!recommendations.empty()) {
            os << "\n" << bold("[Recommendations & Actionable Hints]") << "\n";
            for (const auto &r : recommendations) {
                os << "  - " << bold(r.name) << ":\n";
                os << "      " << r.install_hint << "\n";
            }
        }

        os << "\n--------------------------------------------------------------------------------\n";
        os << "Summary: " << colorize(std::to_string(report.ok_count) + " OK", Color::Green, Style::Bold) << ", "
           << colorize(std::to_string(report.warning_count) + " Warning(s)", Color::Yellow, Style::Bold) << ", "
           << colorize(std::to_string(report.missing_count) + " Error(s)", Color::Red, Style::Bold) << "\n";

        if (report.has_compiler) {
            os << "Status:  "
               << colorize("[cxxgen doctor] Working C++ compiler detected. Environment is ready for builds!",
                           Color::Green, Style::Bold)
               << "\n";
        } else {
            os << "Status:  "
               << colorize(
                      "[cxxgen doctor] Error: No working C++ compiler detected. Please install Clang, GCC, or MSVC.",
                      Color::Red, Style::Bold)
               << "\n";
        }
        os << "================================================================================\n";
    }

    static int run_doctor(bool verbose = false, std::ostream &os = std::cout) {
        auto report = diagnose(verbose);
        print_report(report, os, verbose);
        return report.has_compiler ? 0 : 1;
    }
};

// ============================================================================
// 8.16. Build Graph Visualizer (cxxgen graph)
// ============================================================================

class BuildGraphVisualizer {
public:
    static std::string sanitize_id(std::string_view name) {
        std::string out;
        out.reserve(name.size());
        for (char c : name) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                out.push_back(c);
            } else {
                out.push_back('_');
            }
        }
        if (out.empty() || std::isdigit(static_cast<unsigned char>(out[0]))) {
            out = "node_" + out;
        }
        return out;
    }

    static std::string escape_mermaid_label(std::string_view label) {
        std::string res;
        for (char c : label) {
            if (c == '"')
                res += "&quot;";
            else if (c == '<')
                res += "&lt;";
            else if (c == '>')
                res += "&gt;";
            else if (c == '[' || c == ']' || c == '(' || c == ')' || c == '{' || c == '}')
                res += '_';
            else
                res.push_back(c);
        }
        return res;
    }

    static std::string escape_dot_label(std::string_view label) {
        std::string res;
        for (char c : label) {
            if (c == '"')
                res += "\\\"";
            else if (c == '\\')
                res += "\\\\";
            else if (c == '\n')
                res += "\\n";
            else if (c == '\r')
                continue;
            else
                res.push_back(c);
        }
        return res;
    }

    static std::string target_type_name(TargetType t) {
        switch (t) {
        case TargetType::Executable:
            return "Executable";
        case TargetType::StaticLibrary:
            return "Static Library";
        case TargetType::SharedLibrary:
            return "Shared Library / DLL";
        case TargetType::Plugin:
            return "Dynamic Plugin";
        case TargetType::Interface:
            return "Interface Library";
        case TargetType::Test:
            return "Unit Test";
        default:
            return "Target";
        }
    }

    static std::unordered_set<std::string> get_filtered_targets(const Builder &b, const std::string &filter_target) {
        std::unordered_set<std::string> result;
        if (filter_target.empty()) {
            for (const auto &t : b.targets()) {
                result.insert(t->name());
            }
            return result;
        }

        const Target *target = b.find_target(filter_target);
        if (!target)
            return result;

        result.insert(filter_target);

        // Upstream dependencies
        std::vector<std::string> queue = {filter_target};
        while (!queue.empty()) {
            std::string curr = queue.back();
            queue.pop_back();
            const Target *t = b.find_target(curr);
            if (!t)
                continue;
            for (const auto &dep : t->linked_targets()) {
                if (result.insert(dep).second) {
                    queue.push_back(dep);
                }
            }
        }

        // Downstream dependents (transitive closure)
        std::vector<std::string> down_queue = {filter_target};
        while (!down_queue.empty()) {
            std::string curr = down_queue.back();
            down_queue.pop_back();
            for (const auto &t : b.targets()) {
                for (const auto &dep : t->linked_targets()) {
                    if (dep == curr) {
                        if (result.insert(t->name()).second) {
                            down_queue.push_back(t->name());
                        }
                    }
                }
            }
        }

        return result;
    }

    struct TreeGlyphs {
        std::string branch;
        std::string last;
        std::string pipe;
        std::string space;
    };

    static TreeGlyphs get_glyphs(bool unicode) {
        if (unicode) {
            return {"├── ", "└── ", "│   ", "    "};
        } else {
            return {"|-- ", "\\-- ", "|   ", "    "};
        }
    }

    static std::string to_tree(const Builder &b, const std::string &filter_target = "", bool force_ascii = false,
                               bool force_unicode = false) {
        using namespace terminal;
        std::ostringstream ss;
        auto allowed = get_filtered_targets(b, filter_target);

        bool use_unicode = force_unicode || (!force_ascii && terminal::is_unicode_supported());
        TreeGlyphs glyphs = get_glyphs(use_unicode);

        ss << "================================================================================\n";
        ss << bold("cxxgen graph - Build Dependency Graph") << "\n";
        if (!filter_target.empty()) {
            ss << "Filtered to target: " << colorize(filter_target, Color::Cyan, Style::Bold) << "\n";
        }
        ss << "================================================================================\n\n";

        if (b.targets().empty() && b.steps().empty()) {
            ss << "No targets or execution steps registered in build graph.\n";
            return ss.str();
        }

        std::vector<const Target *> visible_targets;
        for (const auto &t : b.targets()) {
            if (allowed.find(t->name()) != allowed.end()) {
                visible_targets.push_back(t.get());
            }
        }

        if (!visible_targets.empty()) {
            ss << colorize("Targets:", Color::Green, Style::Bold) << "\n";
            for (const auto *t : visible_targets) {
                ss << "  " << colorize("Target: ", Color::Green, Style::Bold) << bold(t->name()) << " ("
                   << target_type_name(t->type()) << ")"
                   << " [" << t->config().sources.size() << " source(s)]\n";

                bool has_links = !t->linked_targets().empty();
                bool has_sys_libs = !t->linked_system_libs().empty();

                if (has_links) {
                    std::string branch = has_sys_libs ? glyphs.branch : glyphs.last;
                    ss << "  " << branch << bold("Links:") << "\n";
                    std::string base_indent = "  " + (has_sys_libs ? glyphs.pipe : glyphs.space);

                    std::function<void(const std::string &, const std::string &, bool,
                                       std::unordered_set<std::string> &)>
                        render_target_link = [&](const std::string &dep_name, const std::string &indent, bool is_last,
                                                 std::unordered_set<std::string> &visited) {
                            const Target *dt = b.find_target(dep_name);
                            std::string dep_type = dt ? target_type_name(dt->type()) : "Target";
                            std::string b_str = is_last ? glyphs.last : glyphs.branch;
                            ss << indent << b_str << colorize(dep_name, Color::Cyan, Style::Bold) << " (" << dep_type
                               << ")";

                            if (visited.count(dep_name)) {
                                ss << " [already linked]\n";
                                return;
                            }
                            ss << "\n";

                            if (dt && !dt->linked_targets().empty()) {
                                visited.insert(dep_name);
                                std::string next_indent = indent + (is_last ? glyphs.space : glyphs.pipe);
                                for (size_t j = 0; j < dt->linked_targets().size(); ++j) {
                                    bool last_sub = (j + 1 == dt->linked_targets().size());
                                    render_target_link(dt->linked_targets()[j], next_indent, last_sub, visited);
                                }
                                visited.erase(dep_name);
                            }
                        };

                    std::unordered_set<std::string> visited = {t->name()};
                    for (size_t i = 0; i < t->linked_targets().size(); ++i) {
                        bool is_last = (i + 1 == t->linked_targets().size());
                        render_target_link(t->linked_targets()[i], base_indent, is_last, visited);
                    }
                }

                if (has_sys_libs) {
                    ss << "  " << glyphs.last << bold("System Libraries: ");
                    for (size_t i = 0; i < t->linked_system_libs().size(); ++i) {
                        if (i > 0)
                            ss << ", ";
                        ss << colorize(t->linked_system_libs()[i], Color::Magenta);
                    }
                    ss << "\n";
                }

                ss << "\n";
            }
        }

        // Execution Steps (DAG)
        std::unordered_set<std::string> child_steps;
        for (const auto &s : b.steps()) {
            if (!s)
                continue;
            for (const auto &dep : s->dependencies()) {
                if (dep)
                    child_steps.insert(dep->name());
            }
        }
        for (const auto &t : b.targets()) {
            if (t && t->build_step()) {
                for (const auto &dep : t->build_step()->dependencies()) {
                    if (dep)
                        child_steps.insert(dep->name());
                }
            }
        }

        std::vector<std::shared_ptr<Step>> root_steps;
        std::unordered_set<std::string> root_step_names;

        // Prioritize standard lifecycle steps first in natural order: install, run, test, clean
        const std::vector<std::string> priority_names = {"install", "run", "test", "clean"};
        for (const auto &pname : priority_names) {
            if (auto s = b.find_step_ptr(pname)) {
                root_steps.push_back(s);
                root_step_names.insert(s->name());
            }
        }

        for (const auto &s : b.steps()) {
            if (!s || s->name().empty())
                continue;
            if (root_step_names.count(s->name()))
                continue;
            if (b.find_target(s->name()))
                continue;
            if (child_steps.count(s->name()))
                continue;
            root_steps.push_back(s);
            root_step_names.insert(s->name());
        }

        auto step_touches_target = [&](const std::shared_ptr<Step> &root, const std::string &target_name) -> bool {
            if (target_name.empty())
                return true;
            std::vector<std::shared_ptr<Step>> stack = {root};
            std::unordered_set<std::string> seen;
            while (!stack.empty()) {
                auto curr = stack.back();
                stack.pop_back();
                if (!curr || seen.count(curr->name()))
                    continue;
                seen.insert(curr->name());
                if (curr->name() == target_name)
                    return true;
                for (const auto &d : curr->dependencies()) {
                    stack.push_back(d);
                }
            }
            return false;
        };

        std::vector<std::shared_ptr<Step>> filtered_root_steps;
        for (const auto &rs : root_steps) {
            if (step_touches_target(rs, filter_target)) {
                filtered_root_steps.push_back(rs);
            }
        }

        if (!filtered_root_steps.empty()) {
            ss << colorize("Execution Steps (DAG):", Color::Yellow, Style::Bold) << "\n";

            std::function<void(const std::shared_ptr<Step> &, const std::string &, bool,
                               std::unordered_set<std::string> &)>
                render_step_tree = [&](const std::shared_ptr<Step> &step, const std::string &indent, bool is_last,
                                       std::unordered_set<std::string> &visited) {
                    if (!step)
                        return;
                    std::string branch = is_last ? glyphs.last : glyphs.branch;
                    std::string sname = step->name();

                    const Target *tnode = b.find_target(sname);
                    if (tnode) {
                        ss << indent << branch << colorize("Target: ", Color::Green, Style::Bold) << bold(tnode->name())
                           << " (" << target_type_name(tnode->type()) << ")";
                        if (visited.count(sname)) {
                            ss << " [already built]\n";
                            return;
                        }
                        ss << "\n";

                        if (!tnode->linked_targets().empty()) {
                            visited.insert(sname);
                            std::string next_indent = indent + (is_last ? glyphs.space : glyphs.pipe);
                            for (size_t i = 0; i < tnode->linked_targets().size(); ++i) {
                                bool last_link = (i + 1 == tnode->linked_targets().size());
                                const auto &link_name = tnode->linked_targets()[i];
                                const Target *lt = b.find_target(link_name);
                                std::string ltype = lt ? target_type_name(lt->type()) : "Target";
                                std::string lbranch = last_link ? glyphs.last : glyphs.branch;
                                ss << next_indent << lbranch
                                   << "Links: " << colorize(link_name, Color::Cyan, Style::Bold) << " (" << ltype
                                   << ")\n";
                            }
                            visited.erase(sname);
                        }
                        return;
                    }

                    ss << indent << branch << colorize("Step: ", Color::Yellow, Style::Bold) << bold(sname);
                    if (!step->description().empty()) {
                        ss << " - " << step->description();
                    }
                    if (visited.count(sname)) {
                        ss << " [cycle detected]\n";
                        return;
                    }
                    ss << "\n";

                    if (!step->dependencies().empty()) {
                        visited.insert(sname);
                        std::string next_indent = indent + (is_last ? glyphs.space : glyphs.pipe);
                        for (size_t i = 0; i < step->dependencies().size(); ++i) {
                            bool last_dep = (i + 1 == step->dependencies().size());
                            render_step_tree(step->dependencies()[i], next_indent, last_dep, visited);
                        }
                        visited.erase(sname);
                    }
                };

            for (const auto &rs : filtered_root_steps) {
                ss << "  " << colorize("Step: ", Color::Yellow, Style::Bold) << bold(rs->name());
                if (!rs->description().empty()) {
                    ss << " - " << rs->description();
                }
                ss << "\n";

                std::unordered_set<std::string> visited = {rs->name()};
                for (size_t i = 0; i < rs->dependencies().size(); ++i) {
                    bool is_last = (i + 1 == rs->dependencies().size());
                    render_step_tree(rs->dependencies()[i], "  ", is_last, visited);
                }
                ss << "\n";
            }
        }

        return ss.str();
    }

    static std::string to_ascii(const Builder &b, const std::string &filter_target = "", bool force_ascii = false) {
        return to_tree(b, filter_target, force_ascii);
    }

    static std::string to_mermaid(const Builder &b, const std::string &filter_target = "") {
        std::ostringstream ss;
        auto allowed = get_filtered_targets(b, filter_target);

        ss << "```mermaid\n";
        ss << "graph TD\n";

        for (const auto &t : b.targets()) {
            if (allowed.find(t->name()) == allowed.end())
                continue;
            std::string t_id = sanitize_id(t->name());
            ss << "    " << t_id << "[\"" << escape_mermaid_label(t->name()) << " (" << target_type_name(t->type())
               << ")\"]\n";
        }

        for (const auto &t : b.targets()) {
            if (allowed.find(t->name()) == allowed.end())
                continue;
            std::string t_id = sanitize_id(t->name());

            for (const auto &dep : t->linked_targets()) {
                if (allowed.find(dep) != allowed.end()) {
                    std::string dep_id = sanitize_id(dep);
                    ss << "    " << t_id << " --> " << dep_id << "\n";
                }
            }

            if (t->build_step()) {
                for (const auto &step_dep : t->build_step()->dependencies()) {
                    std::string s_id = sanitize_id(step_dep->name());
                    ss << "    " << s_id << "[\"Step: " << escape_mermaid_label(step_dep->name()) << "\"]\n";
                    ss << "    " << t_id << " -.-> " << s_id << "\n";
                }
            }
        }

        ss << "```";
        return ss.str();
    }

    static std::string to_dot(const Builder &b, const std::string &filter_target = "") {
        std::ostringstream ss;
        auto allowed = get_filtered_targets(b, filter_target);

        ss << "digraph G {\n";
        ss << "    rankdir=TB;\n";
        ss << "    node [shape=box, style=\"rounded,filled\", fillcolor=\"#f0f4f8\", fontname=\"Helvetica\"];\n";
        ss << "    edge [color=\"#666666\"];\n\n";

        for (const auto &t : b.targets()) {
            if (allowed.find(t->name()) == allowed.end())
                continue;
            ss << "    \"" << escape_dot_label(t->name()) << "\" [label=\"" << escape_dot_label(t->name()) << "\\n("
               << target_type_name(t->type()) << ")\"];\n";
        }

        ss << "\n";
        for (const auto &t : b.targets()) {
            if (allowed.find(t->name()) == allowed.end())
                continue;

            for (const auto &dep : t->linked_targets()) {
                if (allowed.find(dep) != allowed.end()) {
                    ss << "    \"" << escape_dot_label(t->name()) << "\" -> \"" << escape_dot_label(dep) << "\";\n";
                }
            }

            if (t->build_step()) {
                for (const auto &step_dep : t->build_step()->dependencies()) {
                    ss << "    \"" << escape_dot_label(step_dep->name())
                       << "\" [shape=ellipse, style=\"dashed,filled\", fillcolor=\"#fffde7\"];\n";
                    ss << "    \"" << escape_dot_label(t->name()) << "\" -> \"" << escape_dot_label(step_dep->name())
                       << "\" [style=dashed];\n";
                }
            }
        }

        ss << "}\n";
        return ss.str();
    }

    static int run(const Builder &b, const std::string &format, const std::string &target_filter,
                   std::ostream &os = std::cout) {
        terminal::init_console();
        if (!target_filter.empty() && !b.find_target(target_filter)) {
            std::cerr << "cxxgen error: target '" << target_filter << "' not found in build configuration.\n";
            std::cerr << "Available targets:";
            for (const auto &t : b.targets()) {
                std::cerr << " " << t->name();
            }
            std::cerr << "\n";
            return 1;
        }

        if (format == "mermaid") {
            os << to_mermaid(b, target_filter) << "\n";
        } else if (format == "dot") {
            os << to_dot(b, target_filter) << "\n";
        } else if (format == "tree" || format == "unicode") {
            os << to_tree(b, target_filter, /*force_ascii=*/false, /*force_unicode=*/true) << "\n";
        } else if (format == "pure-ascii" || format == "plain") {
            os << to_tree(b, target_filter, /*force_ascii=*/true, /*force_unicode=*/false) << "\n";
        } else {
            // Default "ascii" format: check if terminal supports UTF-8 cleanly
            bool force_ascii = !terminal::is_unicode_supported();
            os << to_tree(b, target_filter, force_ascii) << "\n";
        }
        return 0;
    }
};

// ============================================================================
// 8.18. Integrated Static Analyzer & Linter (cxxgen lint)
// ============================================================================

inline std::optional<std::string> find_clang_tidy() {
    namespace fs = std::filesystem;
    std::vector<std::string> candidates = {"clang-tidy"};
#if defined(_WIN32)
    candidates.push_back("clang-tidy.exe");
    const char *userprofile = std::getenv("USERPROFILE");
    if (userprofile) {
        fs::path scoop_tidy = fs::path(userprofile) / "scoop/apps/llvm/current/bin/clang-tidy.exe";
        if (fs::exists(scoop_tidy)) {
            candidates.insert(candidates.begin(), scoop_tidy.string());
        }
    }
    const char *pf = std::getenv("ProgramFiles");
    if (pf) {
        fs::path pf_tidy = fs::path(pf) / "LLVM/bin/clang-tidy.exe";
        if (fs::exists(pf_tidy)) {
            candidates.push_back(pf_tidy.string());
        }
    }
#endif
    auto clang = find_clang_toolchain();
    if (clang) {
        fs::path cdir = fs::path(clang->compiler_path()).parent_path();
#if defined(_WIN32)
        fs::path sibling = cdir / "clang-tidy.exe";
#else
        fs::path sibling = cdir / "clang-tidy";
#endif
        std::error_code ec;
        if (fs::exists(sibling, ec)) {
            candidates.insert(candidates.begin(), sibling.string());
        }
    }

    for (const auto &c : candidates) {
        std::error_code ec;
        if (fs::exists(c, ec) && !fs::is_directory(c, ec)) {
            return c;
        }
        auto in_p = SystemDoctor::find_in_path(c);
        if (in_p) {
            return *in_p;
        }
        auto res = ProcessRunner::run({c, "--version"});
        if (res.success()) {
            return c;
        }
    }
    return std::nullopt;
}

struct LintConfig {
    std::vector<std::filesystem::path> paths = {};
    bool fix = false;
    bool verbose = false;
    std::filesystem::path build_path = ".";
    std::string checks = "";
    std::vector<std::string> extra_args = {};
};

struct LintResult {
    bool success = true;
    size_t files_scanned = 0;
    size_t warnings_count = 0;
    size_t errors_count = 0;
    std::string report = "";
    std::string error_message = "";
};

class CodeLinter {
public:
    static bool is_lintable(const std::filesystem::path &p) {
        std::string ext = p.extension().string();
        for (char &c : ext) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return ext == ".cpp" || ext == ".cxx" || ext == ".cc" || ext == ".c";
    }

    static bool is_ignored_directory(const std::string &name) {
        return name == ".git" || name == ".cxxgen" || name == "bin" || name == "obj" || name == "build" ||
               name == "out" || name == ".vs" || name == ".cache" || name == "node_modules" || name == "scratch" ||
               name == "test_compile_commands_fixture" || name.rfind(".cxxgen_test_", 0) == 0;
    }

    static bool has_clang_tidy_file(const std::filesystem::path &start_dir) {
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path cur = fs::absolute(start_dir, ec);
        while (!cur.empty()) {
            if (fs::exists(cur / ".clang-tidy", ec)) {
                return true;
            }
            if (!cur.has_relative_path() || cur == cur.parent_path()) {
                break;
            }
            cur = cur.parent_path();
        }
        return false;
    }

    static LintResult lint_all(const std::string &clang_tidy_path, const LintConfig &config) {
        namespace fs = std::filesystem;
        LintResult result;
        std::error_code ec;

        std::vector<fs::path> target_files;
        for (const auto &input_path : config.paths) {
            fs::path p = input_path;
            if (!fs::exists(p, ec)) {
                result.success = false;
                result.error_message = "cxxgen error: path does not exist: " + p.string();
                return result;
            }

            if (fs::is_regular_file(p, ec)) {
                if (is_lintable(p)) {
                    target_files.push_back(p);
                }
            } else if (fs::is_directory(p, ec)) {
                for (auto it = fs::recursive_directory_iterator(p, fs::directory_options::skip_permission_denied, ec);
                     it != fs::recursive_directory_iterator(); ++it) {
                    if (it.depth() > 25) {
                        it.pop();
                        continue;
                    }
                    if (it->is_directory(ec)) {
                        std::string dirname = it->path().filename().string();
                        if (is_ignored_directory(dirname)) {
                            it.disable_recursion_pending();
                            continue;
                        }
                    } else if (it->is_regular_file(ec)) {
                        if (is_lintable(it->path())) {
                            target_files.push_back(it->path());
                        }
                    }
                }
            }
        }

        // Deduplicate target files
        std::vector<fs::path> unique_files;
        for (const auto &f : target_files) {
            auto norm = fs::absolute(f, ec).lexically_normal();
            bool duplicate = false;
            for (const auto &uf : unique_files) {
                if (fs::absolute(uf, ec).lexically_normal() == norm) {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) {
                unique_files.push_back(f);
            }
        }

        if (unique_files.empty()) {
            result.success = true;
            result.report = "[cxxgen lint] No C/C++ source files found to lint.";
            return result;
        }

        result.files_scanned = unique_files.size();
        std::ostringstream report_ss;
        bool color = terminal::color_enabled();

        fs::path comp_db = config.build_path / "compile_commands.json";
        if (!fs::exists(comp_db, ec)) {
            if (fs::exists("compile_commands.json", ec)) {
                comp_db = "compile_commands.json";
            } else if (fs::exists("build/compile_commands.json", ec)) {
                comp_db = "build/compile_commands.json";
            }
        }
        bool has_comp_db = fs::exists(comp_db, ec);

        for (const auto &file : unique_files) {
            std::vector<std::string> cmd;
            cmd.push_back(clang_tidy_path);

            if (has_comp_db) {
                cmd.push_back("-p");
                fs::path pdir = comp_db.parent_path();
                cmd.push_back(util::normalize_path(pdir.empty() ? "." : pdir.string(), false));
            }

            if (!config.checks.empty()) {
                cmd.push_back("--checks=" + config.checks);
            } else if (!has_clang_tidy_file(file.parent_path())) {
                cmd.push_back("--checks=clang-analyzer-*,bugprone-*");
            }

            if (config.fix) {
                cmd.push_back("--fix");
                cmd.push_back("--fix-errors");
            }

            cmd.push_back("--quiet");
            cmd.push_back(util::normalize_path(file.string(), false));

            // Append compiler flags
            cmd.push_back("--");
            cmd.push_back("-std=c++20");
            if (fs::exists("include", ec)) {
                cmd.push_back("-Iinclude");
            }
            for (const auto &arg : config.extra_args) {
                cmd.push_back(arg);
            }

            if (config.verbose) {
                std::cout << "[cxxgen lint] " << util::flatten_command(cmd) << "\n";
            }

            auto pres = ProcessRunner::run(cmd);
            std::string out = pres.combined_output();

            // Parse diagnostics
            std::istringstream stream(out);
            std::string line;
            while (std::getline(stream, line)) {
                if (line.empty())
                    continue;
                if (line.find(": warning:") != std::string::npos) {
                    result.warnings_count++;
                    if (color) {
                        size_t pos = line.find("warning:");
                        report_ss << line.substr(0, pos) << "\033[33mwarning:\033[0m" << line.substr(pos + 8) << "\n";
                    } else {
                        report_ss << line << "\n";
                    }
                } else if (line.find(": error:") != std::string::npos) {
                    result.errors_count++;
                    if (color) {
                        size_t pos = line.find("error:");
                        report_ss << line.substr(0, pos) << "\033[31merror:\033[0m" << line.substr(pos + 6) << "\n";
                    } else {
                        report_ss << line << "\n";
                    }
                } else if (config.verbose || line.find("clang-tidy") != std::string::npos ||
                           line.find("warnings generated") != std::string::npos) {
                    report_ss << line << "\n";
                }
            }

            if (!pres.success() && pres.exit_code != 0 && result.errors_count == 0) {
                result.errors_count++;
                report_ss << out << "\n";
            }
        }

        if (result.errors_count > 0) {
            result.success = false;
            if (result.error_message.empty()) {
                result.error_message = "Linting found " + std::to_string(result.errors_count) + " error(s).";
            }
        }

        result.report = report_ss.str();
        return result;
    }
};

// ============================================================================
// 8.19. Code Coverage Profiling & Instrumentation (cxxgen test --coverage)
// ============================================================================

inline std::optional<std::string> find_llvm_profdata() {
    namespace fs = std::filesystem;
    std::vector<std::string> candidates = {"llvm-profdata"};
#if defined(_WIN32)
    candidates.push_back("llvm-profdata.exe");
    const char *userprofile = std::getenv("USERPROFILE");
    if (userprofile) {
        fs::path scoop_p = fs::path(userprofile) / "scoop/apps/llvm/current/bin/llvm-profdata.exe";
        if (fs::exists(scoop_p)) {
            candidates.insert(candidates.begin(), scoop_p.string());
        }
    }
    const char *pf = std::getenv("ProgramFiles");
    if (pf) {
        fs::path pf_p = fs::path(pf) / "LLVM/bin/llvm-profdata.exe";
        if (fs::exists(pf_p)) {
            candidates.push_back(pf_p.string());
        }
    }
#endif
    auto clang = find_clang_toolchain();
    if (clang) {
        fs::path cdir = fs::path(clang->compiler_path()).parent_path();
#if defined(_WIN32)
        fs::path sibling = cdir / "llvm-profdata.exe";
#else
        fs::path sibling = cdir / "llvm-profdata";
#endif
        std::error_code ec;
        if (fs::exists(sibling, ec)) {
            candidates.insert(candidates.begin(), sibling.string());
        }
    }

    for (const auto &c : candidates) {
        std::error_code ec;
        if (fs::exists(c, ec) && !fs::is_directory(c, ec)) {
            return c;
        }
        auto in_p = SystemDoctor::find_in_path(c);
        if (in_p) {
            return *in_p;
        }
        auto res = ProcessRunner::run({c, "--version"});
        if (res.success()) {
            return c;
        }
    }
    return std::nullopt;
}

inline std::optional<std::string> find_llvm_cov() {
    namespace fs = std::filesystem;
    std::vector<std::string> candidates = {"llvm-cov"};
#if defined(_WIN32)
    candidates.push_back("llvm-cov.exe");
    const char *userprofile = std::getenv("USERPROFILE");
    if (userprofile) {
        fs::path scoop_p = fs::path(userprofile) / "scoop/apps/llvm/current/bin/llvm-cov.exe";
        if (fs::exists(scoop_p)) {
            candidates.insert(candidates.begin(), scoop_p.string());
        }
    }
    const char *pf = std::getenv("ProgramFiles");
    if (pf) {
        fs::path pf_p = fs::path(pf) / "LLVM/bin/llvm-cov.exe";
        if (fs::exists(pf_p)) {
            candidates.push_back(pf_p.string());
        }
    }
#endif
    auto clang = find_clang_toolchain();
    if (clang) {
        fs::path cdir = fs::path(clang->compiler_path()).parent_path();
#if defined(_WIN32)
        fs::path sibling = cdir / "llvm-cov.exe";
#else
        fs::path sibling = cdir / "llvm-cov";
#endif
        std::error_code ec;
        if (fs::exists(sibling, ec)) {
            candidates.insert(candidates.begin(), sibling.string());
        }
    }

    for (const auto &c : candidates) {
        std::error_code ec;
        if (fs::exists(c, ec) && !fs::is_directory(c, ec)) {
            return c;
        }
        auto in_p = SystemDoctor::find_in_path(c);
        if (in_p) {
            return *in_p;
        }
        auto res = ProcessRunner::run({c, "--version"});
        if (res.success()) {
            return c;
        }
    }
    return std::nullopt;
}

inline std::optional<std::string> find_gcov() {
    namespace fs = std::filesystem;
    std::vector<std::string> candidates = {"gcov"};
#if defined(_WIN32)
    candidates.push_back("gcov.exe");
    const char *userprofile = std::getenv("USERPROFILE");
    if (userprofile) {
        fs::path scoop_gcov = fs::path(userprofile) / "scoop/apps/gcc/current/bin/gcov.exe";
        if (fs::exists(scoop_gcov)) {
            candidates.insert(candidates.begin(), scoop_gcov.string());
        }
        fs::path msys_gcov = fs::path(userprofile) / "scoop/apps/msys2/current/ucrt64/bin/gcov.exe";
        if (fs::exists(msys_gcov)) {
            candidates.push_back(msys_gcov.string());
        }
    }
#endif
    auto gcc = find_gcc_toolchain();
    if (gcc) {
        fs::path cdir = fs::path(gcc->compiler_path()).parent_path();
#if defined(_WIN32)
        fs::path sibling = cdir / "gcov.exe";
#else
        fs::path sibling = cdir / "gcov";
#endif
        std::error_code ec;
        if (fs::exists(sibling, ec)) {
            candidates.insert(candidates.begin(), sibling.string());
        }
    }

    for (const auto &c : candidates) {
        std::error_code ec;
        if (fs::exists(c, ec) && !fs::is_directory(c, ec)) {
            return c;
        }
        auto in_p = SystemDoctor::find_in_path(c);
        if (in_p) {
            return *in_p;
        }
        auto res = ProcessRunner::run({c, "--version"});
        if (res.success()) {
            return c;
        }
    }
    return std::nullopt;
}

struct CoverageConfig {
    std::filesystem::path binary_path;
    std::vector<std::string> source_files = {};
    std::filesystem::path profraw_file = "default.profraw";
    std::filesystem::path profdata_file = "default.profdata";
    std::filesystem::path html_dir = "";
    std::string export_format = "";
    std::filesystem::path export_file = "";
    bool verbose = false;
};

struct CoverageResult {
    bool success = true;
    double line_coverage_percent = 0.0;
    double function_coverage_percent = 0.0;
    double branch_coverage_percent = 0.0;
    std::string report_output = "";
    std::string error_message = "";
};

class CoverageReporter {
public:
    static CoverageResult generate_report(const CoverageConfig &config) {
        namespace fs = std::filesystem;
        CoverageResult result;
        std::error_code ec;

        auto profdata_bin = find_llvm_profdata();
        auto cov_bin = find_llvm_cov();

        if (profdata_bin && cov_bin) {
            std::vector<fs::path> raw_files;
            if (fs::exists(config.profraw_file, ec)) {
                raw_files.push_back(config.profraw_file);
            }
            if (fs::exists(".cxxgen/coverage", ec)) {
                for (const auto &entry : fs::directory_iterator(".cxxgen/coverage", ec)) {
                    if (entry.path().extension() == ".profraw") {
                        bool dup = false;
                        for (const auto &rf : raw_files) {
                            if (fs::absolute(rf, ec) == fs::absolute(entry.path(), ec)) {
                                dup = true;
                                break;
                            }
                        }
                        if (!dup)
                            raw_files.push_back(entry.path());
                    }
                }
            }
            if (raw_files.empty() && fs::exists("default.profraw", ec)) {
                raw_files.push_back("default.profraw");
            }

            if (raw_files.empty()) {
                result.success = false;
                result.error_message =
                    "cxxgen error: no profile data (.profraw) found. Run test binary with coverage enabled.";
                return result;
            }

            fs::path out_data =
                config.profdata_file.empty() ? ".cxxgen/coverage/coverage.profdata" : config.profdata_file;
            if (util::is_system_protected_dir(out_data)) {
                result.success = false;
                result.error_message =
                    "cxxgen security error: refusing to write coverage profdata to protected system directory: " +
                    out_data.string();
                return result;
            }
            if (out_data.has_parent_path()) {
                fs::create_directories(out_data.parent_path(), ec);
            }

            std::vector<std::string> merge_cmd = {*profdata_bin, "merge", "-sparse"};
            for (const auto &rf : raw_files) {
                merge_cmd.push_back(util::normalize_path(rf.string(), false));
            }
            merge_cmd.push_back("-o");
            merge_cmd.push_back(util::normalize_path(out_data.string(), false));

            if (config.verbose) {
                std::cout << "[cxxgen coverage] " << util::flatten_command(merge_cmd) << "\n";
            }
            auto m_res = ProcessRunner::run(merge_cmd);
            if (!m_res.success()) {
                result.success = false;
                result.error_message = "llvm-profdata merge failed:\n" + m_res.combined_output();
                return result;
            }

            std::vector<std::string> report_cmd = {*cov_bin, "report",
                                                   util::normalize_path(config.binary_path.string(), false),
                                                   "--instr-profile=" + util::normalize_path(out_data.string(), false)};
            if (config.verbose) {
                std::cout << "[cxxgen coverage] " << util::flatten_command(report_cmd) << "\n";
            }
            auto r_res = ProcessRunner::run(report_cmd);
            if (!r_res.success()) {
                result.success = false;
                result.error_message = "llvm-cov report failed:\n" + r_res.combined_output();
                return result;
            }

            result.report_output = r_res.stdout_output;

            std::istringstream stream(r_res.stdout_output);
            std::string line;
            while (std::getline(stream, line)) {
                if (line.rfind("TOTAL", 0) == 0) {
                    std::vector<std::string> tokens;
                    std::istringstream line_stream(line);
                    std::string tok;
                    while (line_stream >> tok) {
                        tokens.push_back(tok);
                    }
                    std::vector<double> percentages;
                    for (const auto &t : tokens) {
                        if (!t.empty() && t.back() == '%') {
                            try {
                                double p = std::stod(t.substr(0, t.size() - 1));
                                percentages.push_back(p);
                            } catch (...) {}
                        }
                    }
                    if (percentages.size() >= 1) {
                        result.line_coverage_percent = percentages[0];
                    }
                    if (percentages.size() >= 2) {
                        result.function_coverage_percent = percentages[1];
                    }
                    if (percentages.size() >= 3) {
                        result.line_coverage_percent = percentages[2];
                    }
                    if (percentages.size() >= 4) {
                        result.branch_coverage_percent = percentages[3];
                    }
                    break;
                }
            }

            if (!config.html_dir.empty() || config.export_format == "html") {
                fs::path target_html_dir = config.html_dir.empty() ? ".cxxgen/coverage/html" : config.html_dir;
                if (!util::is_system_protected_dir(target_html_dir)) {
                    fs::create_directories(target_html_dir, ec);
                    std::vector<std::string> show_cmd = {
                        *cov_bin,
                        "show",
                        util::normalize_path(config.binary_path.string(), false),
                        "--instr-profile=" + util::normalize_path(out_data.string(), false),
                        "--format=html",
                        "--output-dir=" + util::normalize_path(target_html_dir.string(), false)};
                    ProcessRunner::run(show_cmd);
                }
            }

            if (config.export_format == "lcov") {
                fs::path target_lcov_file =
                    config.export_file.empty() ? ".cxxgen/coverage/coverage.lcov" : config.export_file;
                if (!util::is_system_protected_dir(target_lcov_file)) {
                    if (target_lcov_file.has_parent_path()) {
                        fs::create_directories(target_lcov_file.parent_path(), ec);
                    }
                    std::vector<std::string> export_cmd = {
                        *cov_bin, "export", util::normalize_path(config.binary_path.string(), false),
                        "--instr-profile=" + util::normalize_path(out_data.string(), false), "--format=lcov"};
                    auto exp_res = ProcessRunner::run(export_cmd);
                    if (exp_res.success()) {
                        std::ofstream out_lcov(target_lcov_file);
                        out_lcov << exp_res.stdout_output;
                    }
                }
            }

            result.success = true;
            return result;
        }

        auto gcov_bin = find_gcov();
        if (gcov_bin) {
            std::vector<std::string> gcov_cmd = {*gcov_bin};
            for (const auto &s : config.source_files) {
                gcov_cmd.push_back(util::normalize_path(s, false));
            }
            auto g_res = ProcessRunner::run(gcov_cmd);
            result.report_output = g_res.stdout_output;
            result.success = g_res.success();
            return result;
        }

        result.success = false;
        result.error_message = "cxxgen error: neither 'llvm-cov'/'llvm-profdata' nor 'gcov' was found on host system.";
        return result;
    }
};

// ============================================================================
// 8.20. Project Importer Engine (CMake & MSVC -> build.cpp)
// ============================================================================

struct ImportedTarget {
    std::string name;
    TargetType type = TargetType::Executable;
    std::vector<std::string> sources = {};
    std::vector<std::string> headers = {};
    std::vector<std::string> include_dirs = {};
    std::vector<std::string> defines = {};
    std::vector<std::string> compile_options = {};
    std::vector<std::string> linked_targets = {};
    std::vector<std::string> system_libs = {};
    std::vector<std::string> link_directories = {};
    CppVersion cpp_version = CppVersion::Cpp20;
    std::string pch_header = "";
    bool is_test = false;
};

struct ImportedProject {
    std::string project_name = "App";
    std::vector<ImportedTarget> targets = {};
    std::string default_target = "";
    CppVersion default_cpp_version = CppVersion::Cpp20;
};

class ProjectCodeSynthesizer {
public:
    static std::string sanitize_var(std::string_view name) {
        std::string res;
        for (char c : name) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                res.push_back(c);
            } else {
                res.push_back('_');
            }
        }
        if (res.empty() || std::isdigit(static_cast<unsigned char>(res[0]))) {
            res = "t_" + res;
        }
        return res;
    }

    static std::string cpp_version_str(CppVersion v) {
        switch (v) {
        case CppVersion::Cpp11:
            return "cxxgen::CppVersion::Cpp11";
        case CppVersion::Cpp14:
            return "cxxgen::CppVersion::Cpp14";
        case CppVersion::Cpp17:
            return "cxxgen::CppVersion::Cpp17";
        case CppVersion::Cpp20:
            return "cxxgen::CppVersion::Cpp20";
        case CppVersion::Cpp23:
            return "cxxgen::CppVersion::Cpp23";
        case CppVersion::Cpp26:
            return "cxxgen::CppVersion::Cpp26";
        }
        return "cxxgen::CppVersion::Cpp20";
    }

    static std::string synthesize(const ImportedProject &proj) {
        std::ostringstream out;
        out << "// ==============================================================================\n"
            << "// Generated by cxxgen import from existing build specification\n"
            << "// ==============================================================================\n\n"
            << "#include \"include/cxxgen.hpp\"\n\n"
            << "void build(cxxgen::Builder &b) {\n"
            << "    b.set_project_name(\"" << util::escape_c_string(proj.project_name) << "\");\n"
            << "    auto target = b.standard_target_options();\n"
            << "    auto optimize = b.standard_optimize_options();\n\n";

        // Map target names to unique variable names
        std::unordered_map<std::string, std::string> var_names;
        for (const auto &t : proj.targets) {
            std::string var = sanitize_var(t.name);
            int count = 1;
            while (true) {
                bool clash = false;
                for (const auto &[k, v] : var_names) {
                    if (v == var) {
                        clash = true;
                        break;
                    }
                }
                if (!clash && var != "b" && var != "target" && var != "optimize")
                    break;
                var = sanitize_var(t.name) + "_" + std::to_string(++count);
            }
            var_names[t.name] = var;
        }

        std::string primary_target = "";

        for (const auto &t : proj.targets) {
            std::string method = "add_executable";
            if (t.is_test) {
                method = "add_test";
            } else {
                switch (t.type) {
                case TargetType::Executable:
                    method = "add_executable";
                    if (primary_target.empty())
                        primary_target = var_names[t.name];
                    break;
                case TargetType::StaticLibrary:
                    method = "add_static_library";
                    break;
                case TargetType::SharedLibrary:
                    method = "add_shared_library";
                    break;
                case TargetType::Plugin:
                    method = "add_plugin";
                    break;
                case TargetType::Test:
                    method = "add_test";
                    break;
                case TargetType::Interface:
                    method = "add_interface_library";
                    break;
                }
            }

            out << "    auto &" << var_names[t.name] << " = b." << method << "({\n"
                << "        .name = \"" << util::escape_c_string(t.name) << "\",\n";

            if (!t.sources.empty()) {
                out << "        .sources = {\n";
                for (const auto &s : t.sources) {
                    out << "            \"" << util::escape_c_string(util::normalize_path(s, false)) << "\",\n";
                }
                out << "        },\n";
            }

            if (!t.include_dirs.empty()) {
                out << "        .include_dirs = {\n";
                for (const auto &inc : t.include_dirs) {
                    out << "            \"" << util::escape_c_string(util::normalize_path(inc, false)) << "\",\n";
                }
                out << "        },\n";
            }

            if (!t.defines.empty()) {
                out << "        .defines = {\n";
                for (const auto &d : t.defines) {
                    out << "            \"" << util::escape_c_string(d) << "\",\n";
                }
                out << "        },\n";
            }

            if (!t.compile_options.empty()) {
                out << "        .compile_options = {\n";
                for (const auto &f : t.compile_options) {
                    out << "            \"" << util::escape_c_string(f) << "\",\n";
                }
                out << "        },\n";
            }

            if (!t.link_directories.empty()) {
                out << "        .link_directories = {\n";
                for (const auto &ld : t.link_directories) {
                    out << "            \"" << util::escape_c_string(util::normalize_path(ld, false)) << "\",\n";
                }
                out << "        },\n";
            }

            if (t.cpp_version != CppVersion::Cpp20) {
                out << "        .cpp_version = " << cpp_version_str(t.cpp_version) << ",\n";
            }

            if (!t.pch_header.empty()) {
                out << "        .pch_header = \"" << util::escape_c_string(util::normalize_path(t.pch_header, false))
                    << "\",\n";
            }

            out << "        .target = target,\n"
                << "        .optimize = optimize.mode,\n"
                << "    });\n";

            // Linked targets
            for (const auto &dep : t.linked_targets) {
                if (var_names.count(dep)) {
                    out << "    " << var_names[t.name] << ".link(" << var_names[dep] << ");\n";
                }
            }

            // System libraries
            for (const auto &sys : t.system_libs) {
                out << "    " << var_names[t.name] << ".link_system_lib(\"" << util::escape_c_string(sys) << "\");\n";
            }

            out << "\n";
        }

        std::string install_target = primary_target;
        if (install_target.empty()) {
            for (const auto &t : proj.targets) {
                if (t.type != TargetType::Interface && !t.is_test) {
                    install_target = var_names[t.name];
                    break;
                }
            }
        }
        if (!install_target.empty()) {
            out << "    b.install_artifact(" << install_target << ");\n";
        }

        out << "}\n\n"
            << "CXXGEN_MAIN(build)\n";

        return out.str();
    }
};

class CMakeImporter {
public:
    struct Token {
        enum class Type { Word, OpenParen, CloseParen };
        Type type;
        std::string text;
        size_t line = 0;
    };

    struct Command {
        std::string name;
        std::vector<std::string> args;
        size_t line = 0;
    };

    static std::vector<Token> tokenize(std::string_view content) {
        std::vector<Token> tokens;
        size_t i = 0;
        size_t line = 1;
        size_t n = content.size();

        while (i < n) {
            char c = content[i];
            if (c == '\n') {
                line++;
                i++;
                continue;
            }
            if (std::isspace(static_cast<unsigned char>(c)) || c == ';') {
                i++;
                continue;
            }
            if (c == '#') {
                // Check for bracket comment #[[ ... ]]
                if (i + 2 < n && content[i + 1] == '[' && content[i + 2] == '[') {
                    i += 3;
                    while (i + 1 < n && !(content[i] == ']' && content[i + 1] == ']')) {
                        if (content[i] == '\n')
                            line++;
                        i++;
                    }
                    if (i + 1 < n)
                        i += 2;
                } else {
                    // Line comment
                    while (i < n && content[i] != '\n') {
                        i++;
                    }
                }
                continue;
            }
            if (c == '(') {
                tokens.push_back({Token::Type::OpenParen, "(", line});
                i++;
                continue;
            }
            if (c == ')') {
                tokens.push_back({Token::Type::CloseParen, ")", line});
                i++;
                continue;
            }
            // Word (quoted, unquoted, or compound such as APP_VER="1.0.0")
            std::string str;
            while (i < n) {
                char ch = content[i];
                if (ch == '\n' || std::isspace(static_cast<unsigned char>(ch)) || ch == ';') {
                    break;
                }
                if (ch == '(' || ch == ')') {
                    break;
                }
                if (ch == '#' && str.empty()) {
                    break;
                }
                if (ch == '"') {
                    bool is_start_quote = str.empty();
                    i++;
                    if (!is_start_quote) {
                        str.push_back('"');
                    }
                    while (i < n && content[i] != '"') {
                        if (content[i] == '\\' && i + 1 < n) {
                            i++;
                            char esc = content[i];
                            if (esc == 'n')
                                str.push_back('\n');
                            else if (esc == 't')
                                str.push_back('\t');
                            else if (esc == 'r')
                                str.push_back('\r');
                            else
                                str.push_back(esc);
                        } else {
                            if (content[i] == '\n')
                                line++;
                            str.push_back(content[i]);
                        }
                        i++;
                    }
                    if (i < n && content[i] == '"') {
                        i++;
                        if (!is_start_quote) {
                            str.push_back('"');
                        }
                    }
                } else {
                    str.push_back(ch);
                    i++;
                }
            }
            tokens.push_back({Token::Type::Word, str, line});
        }
        return tokens;
    }

    static std::vector<Command> parse_commands(const std::vector<Token> &tokens) {
        std::vector<Command> commands;
        size_t i = 0;
        while (i < tokens.size()) {
            if (tokens[i].type == Token::Type::Word && i + 1 < tokens.size() &&
                tokens[i + 1].type == Token::Type::OpenParen) {
                Command cmd;
                cmd.name = tokens[i].text;
                cmd.line = tokens[i].line;
                for (char &ch : cmd.name) {
                    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                }
                i += 2; // skip name and '('
                int depth = 1;
                while (i < tokens.size() && depth > 0) {
                    if (tokens[i].type == Token::Type::OpenParen) {
                        depth++;
                        i++;
                    } else if (tokens[i].type == Token::Type::CloseParen) {
                        depth--;
                        i++;
                    } else {
                        cmd.args.push_back(tokens[i].text);
                        i++;
                    }
                }
                commands.push_back(std::move(cmd));
            } else {
                i++;
            }
        }
        return commands;
    }

    static std::vector<std::string> expand_args(const std::vector<std::string> &args,
                                                const std::unordered_map<std::string, std::vector<std::string>> &vars,
                                                const std::string &project_name, const std::string &curr_dir) {
        std::vector<std::string> res;
        for (const auto &raw : args) {
            // Check if exact match for ${VAR}
            if (raw.size() >= 4 && raw.rfind("${", 0) == 0 && raw.back() == '}') {
                std::string var_name = raw.substr(2, raw.size() - 3);
                if (var_name == "PROJECT_NAME" || var_name == "CMAKE_PROJECT_NAME") {
                    res.push_back(project_name);
                    continue;
                }
                if (var_name == "CMAKE_CURRENT_SOURCE_DIR" || var_name == "CMAKE_SOURCE_DIR") {
                    res.push_back(curr_dir.empty() ? "." : curr_dir);
                    continue;
                }
                if (var_name == "CMAKE_CURRENT_BINARY_DIR" || var_name == "CMAKE_BINARY_DIR") {
                    auto it_b = vars.find(var_name);
                    if (it_b != vars.end() && !it_b->second.empty()) {
                        res.push_back(it_b->second.front());
                    } else {
                        res.push_back(curr_dir.empty() ? "." : curr_dir);
                    }
                    continue;
                }
                auto it = vars.find(var_name);
                if (it != vars.end()) {
                    for (const auto &v : it->second) {
                        res.push_back(v);
                    }
                    continue;
                }
            }

            // Substring replacement for ${VAR}
            std::string s = raw;
            size_t pos = 0;
            while ((pos = s.find("${", pos)) != std::string::npos) {
                size_t end_pos = s.find('}', pos);
                if (end_pos == std::string::npos)
                    break;
                std::string var_name = s.substr(pos + 2, end_pos - pos - 2);
                std::string rep = "";
                if (var_name == "PROJECT_NAME" || var_name == "CMAKE_PROJECT_NAME") {
                    rep = project_name;
                } else if (var_name == "CMAKE_CURRENT_SOURCE_DIR" || var_name == "CMAKE_SOURCE_DIR") {
                    rep = curr_dir.empty() ? "." : curr_dir;
                } else if (var_name == "CMAKE_CURRENT_BINARY_DIR" || var_name == "CMAKE_BINARY_DIR") {
                    auto it_b = vars.find(var_name);
                    if (it_b != vars.end() && !it_b->second.empty()) {
                        rep = it_b->second.front();
                    } else {
                        rep = curr_dir.empty() ? "." : curr_dir;
                    }
                } else {
                    auto it = vars.find(var_name);
                    if (it != vars.end() && !it->second.empty()) {
                        rep = it->second.front();
                    }
                }
                s.replace(pos, end_pos - pos + 1, rep);
                pos += rep.size();
            }

            // Clean generator expression $<...>
            while (s.size() >= 4 && s.rfind("$<", 0) == 0 && s.back() == '>') {
                if (s.rfind("$<BUILD_INTERFACE:", 0) == 0) {
                    s = s.substr(18, s.size() - 19);
                } else if (s.rfind("$<INSTALL_INTERFACE:", 0) == 0) {
                    s.clear();
                    break;
                } else {
                    size_t colon = s.rfind(':');
                    if (colon != std::string::npos && colon > 1) {
                        s = s.substr(colon + 1, s.size() - colon - 2);
                    } else {
                        s.clear();
                        break;
                    }
                }
            }

            if (!s.empty()) {
                res.push_back(s);
            }
        }
        return res;
    }

    static bool parse_cmake_content(std::string_view content, const std::filesystem::path &base_dir,
                                    const std::filesystem::path &curr_dir, ImportedProject &proj,
                                    std::unordered_map<std::string, std::vector<std::string>> &vars,
                                    std::string &error_msg, std::unordered_set<std::string> &visited,
                                    size_t depth = 0) {
        namespace fs = std::filesystem;
        std::error_code ec;

        auto tokens = tokenize(content);
        auto commands = parse_commands(tokens);

        auto find_target = [&](std::string_view name) -> ImportedTarget * {
            for (auto &t : proj.targets) {
                if (t.name == name)
                    return &t;
            }
            return nullptr;
        };

        std::string rel_prefix = "";
        if (!curr_dir.empty() && curr_dir != ".") {
            rel_prefix = util::normalize_path(curr_dir.string(), false);
            if (rel_prefix.rfind("./", 0) == 0)
                rel_prefix = rel_prefix.substr(2);
            if (!rel_prefix.empty())
                rel_prefix += "/";
        }

        if (!vars.count("CMAKE_CURRENT_BINARY_DIR")) {
            std::string default_bin = fs::exists(base_dir / "build") ? "build" : ".";
            vars["CMAKE_CURRENT_BINARY_DIR"] = {default_bin};
            vars["CMAKE_BINARY_DIR"] = {default_bin};
        }

        std::vector<bool> if_stack;
        auto is_active = [&]() {
            for (bool b : if_stack) {
                if (!b)
                    return false;
            }
            return true;
        };

        for (const auto &cmd : commands) {
            if (cmd.name == "if") {
                bool cond = true;
                if (!cmd.args.empty()) {
                    std::string cname = cmd.args[0];
                    if (cname == "MSVC") {
#if defined(_MSC_VER)
                        cond = true;
#else
                        cond = false;
#endif
                    } else if (cname == "WIN32" || cname == "_WIN32") {
#if defined(_WIN32)
                        cond = true;
#else
                        cond = false;
#endif
                    } else if (cname == "APPLE") {
#if defined(__APPLE__)
                        cond = true;
#else
                        cond = false;
#endif
                    } else if (cname == "UNIX") {
#if !defined(_WIN32)
                        cond = true;
#else
                        cond = false;
#endif
                    }
                }
                if_stack.push_back(cond);
                continue;
            } else if (cmd.name == "else") {
                if (!if_stack.empty()) {
                    if_stack.back() = !if_stack.back();
                }
                continue;
            } else if (cmd.name == "endif") {
                if (!if_stack.empty()) {
                    if_stack.pop_back();
                }
                continue;
            }

            if (!is_active()) {
                continue;
            }

            auto args = expand_args(cmd.args, vars, proj.project_name, curr_dir.string());
            if (cmd.name == "project" && !args.empty()) {
                proj.project_name = args[0];
                vars["PROJECT_NAME"] = {args[0]};
                if (!vars.count("CMAKE_PROJECT_NAME")) {
                    vars["CMAKE_PROJECT_NAME"] = {args[0]};
                }
            } else if (cmd.name == "find_package" && !args.empty()) {
                std::string pkg = args[0];
                if (pkg == "Vulkan") {
                    const char *vsdk = std::getenv("VULKAN_SDK");
                    if (vsdk) {
                        vars["Vulkan_INCLUDE_DIRS"] = {
                            util::normalize_path((fs::path(vsdk) / "Include").string(), false)};
                    }
                }
            } else if (cmd.name == "find_path" && args.size() >= 3) {
                std::string var_name = args[0];
                std::vector<std::string> names;
                bool reading_names = false;
                for (size_t a = 1; a < args.size(); ++a) {
                    if (args[a] == "NAMES") {
                        reading_names = true;
                        continue;
                    }
                    if (args[a] == "PATHS" || args[a] == "HINTS" || args[a] == "REQUIRED") {
                        reading_names = false;
                        continue;
                    }
                    if (reading_names)
                        names.push_back(args[a]);
                }
                fs::path vcpkg_inc = base_dir / "build/vcpkg_installed/x64-windows/include";
                if (!fs::exists(vcpkg_inc)) {
                    vcpkg_inc = base_dir / "vcpkg_installed/x64-windows/include";
                }
                if (fs::exists(vcpkg_inc)) {
                    for (const auto &fn : names) {
                        if (fs::exists(vcpkg_inc / fn)) {
                            vars[var_name] = {util::normalize_path(vcpkg_inc.string(), false)};
                            break;
                        }
                    }
                }
            } else if (cmd.name == "set" && args.size() >= 2) {
                if (args[0] == "CMAKE_CXX_STANDARD") {
                    std::string std_str = args[1];
                    if (std_str == "11")
                        proj.default_cpp_version = CppVersion::Cpp11;
                    else if (std_str == "14")
                        proj.default_cpp_version = CppVersion::Cpp14;
                    else if (std_str == "17")
                        proj.default_cpp_version = CppVersion::Cpp17;
                    else if (std_str == "20")
                        proj.default_cpp_version = CppVersion::Cpp20;
                    else if (std_str == "23")
                        proj.default_cpp_version = CppVersion::Cpp23;
                    else if (std_str == "26")
                        proj.default_cpp_version = CppVersion::Cpp26;
                } else {
                    std::vector<std::string> vals(args.begin() + 1, args.end());
                    vars[args[0]] = std::move(vals);
                }
            } else if (cmd.name == "add_executable" && !args.empty()) {
                std::string tname = args[0];
                ImportedTarget t;
                t.name = tname;
                t.type = TargetType::Executable;
                t.cpp_version = proj.default_cpp_version;
                fs::path vcpkg_lib = base_dir / "build/vcpkg_installed/x64-windows/lib";
                if (!fs::exists(vcpkg_lib)) {
                    vcpkg_lib = base_dir / "vcpkg_installed/x64-windows/lib";
                }
                if (fs::exists(vcpkg_lib)) {
                    t.link_directories.push_back(
                        util::normalize_path(fs::relative(vcpkg_lib, base_dir).string(), false));
                }
                const char *vsdk = std::getenv("VULKAN_SDK");
                if (vsdk) {
                    fs::path vsdk_lib = fs::path(vsdk) / "Lib";
                    if (fs::exists(vsdk_lib)) {
                        t.link_directories.push_back(util::normalize_path(vsdk_lib.string(), false));
                    }
                }
                for (size_t a = 1; a < args.size(); ++a) {
                    if (args[a] == "WIN32" || args[a] == "MACOSX_BUNDLE" || args[a] == "EXCLUDE_FROM_ALL")
                        continue;
                    std::string p = args[a];
                    if (p.rfind("./", 0) == 0)
                        p = p.substr(2);
                    t.sources.push_back(rel_prefix + p);
                }
                proj.targets.push_back(std::move(t));
            } else if (cmd.name == "add_library" && !args.empty()) {
                std::string tname = args[0];
                ImportedTarget t;
                t.name = tname;
                t.type = TargetType::StaticLibrary;
                t.cpp_version = proj.default_cpp_version;
                fs::path vcpkg_lib = base_dir / "build/vcpkg_installed/x64-windows/lib";
                if (!fs::exists(vcpkg_lib)) {
                    vcpkg_lib = base_dir / "vcpkg_installed/x64-windows/lib";
                }
                if (fs::exists(vcpkg_lib)) {
                    t.link_directories.push_back(
                        util::normalize_path(fs::relative(vcpkg_lib, base_dir).string(), false));
                }
                const char *vsdk = std::getenv("VULKAN_SDK");
                if (vsdk) {
                    fs::path vsdk_lib = fs::path(vsdk) / "Lib";
                    if (fs::exists(vsdk_lib)) {
                        t.link_directories.push_back(util::normalize_path(vsdk_lib.string(), false));
                    }
                }
                for (size_t a = 1; a < args.size(); ++a) {
                    if (args[a] == "STATIC") {
                        t.type = TargetType::StaticLibrary;
                        continue;
                    }
                    if (args[a] == "SHARED") {
                        t.type = TargetType::SharedLibrary;
                        continue;
                    }
                    if (args[a] == "MODULE") {
                        t.type = TargetType::Plugin;
                        continue;
                    }
                    if (args[a] == "INTERFACE") {
                        t.type = TargetType::Interface;
                        continue;
                    }
                    if (args[a] == "OBJECT" || args[a] == "EXCLUDE_FROM_ALL")
                        continue;
                    std::string p = args[a];
                    if (p.rfind("./", 0) == 0)
                        p = p.substr(2);
                    t.sources.push_back(rel_prefix + p);
                }
                proj.targets.push_back(std::move(t));
            } else if (cmd.name == "target_sources" && args.size() >= 2) {
                auto *t = find_target(args[0]);
                if (t) {
                    bool skipping_base_dirs = false;
                    for (size_t a = 1; a < args.size(); ++a) {
                        const std::string &arg = args[a];
                        if (arg == "PRIVATE" || arg == "PUBLIC" || arg == "INTERFACE")
                            continue;
                        if (arg == "FILE_SET" || arg == "CXX_MODULES" || arg == "HEADERS" || arg == "TYPE")
                            continue;
                        if (arg == "BASE_DIRS") {
                            skipping_base_dirs = true;
                            continue;
                        }
                        if (arg == "FILES") {
                            skipping_base_dirs = false;
                            continue;
                        }
                        if (skipping_base_dirs)
                            continue;

                        std::string ext = fs::path(arg).extension().string();
                        if (ext == ".spv" || arg.find(".spv") != std::string::npos)
                            continue;
                        if (ext == ".h" || ext == ".hpp" || ext == ".hxx") {
                            t->headers.push_back(rel_prefix + arg);
                            continue;
                        }

                        std::string p = arg;
                        if (p.rfind("./", 0) == 0)
                            p = p.substr(2);
                        if (!p.empty() && p != "/") {
                            if (p.rfind("build/", 0) == 0 || (!rel_prefix.empty() && p.rfind(rel_prefix, 0) == 0)) {
                                t->sources.push_back(p);
                            } else {
                                t->sources.push_back(rel_prefix + p);
                            }
                        }
                    }
                }
            } else if (cmd.name == "target_include_directories" && args.size() >= 2) {
                auto *t = find_target(args[0]);
                if (t) {
                    for (size_t a = 1; a < args.size(); ++a) {
                        if (args[a] == "PRIVATE" || args[a] == "PUBLIC" || args[a] == "INTERFACE")
                            continue;
                        std::string inc = args[a];
                        if (inc.empty() || inc == "/")
                            continue;
                        if (inc.rfind("./", 0) == 0)
                            inc = inc.substr(2);
                        if (inc == ".")
                            inc = rel_prefix.empty() ? "." : rel_prefix.substr(0, rel_prefix.size() - 1);
                        else
                            inc = rel_prefix + inc;
                        t->include_dirs.push_back(inc);
                    }
                }
            } else if (cmd.name == "target_compile_definitions" && args.size() >= 2) {
                auto *t = find_target(args[0]);
                if (t) {
                    for (size_t a = 1; a < args.size(); ++a) {
                        if (args[a] == "PRIVATE" || args[a] == "PUBLIC" || args[a] == "INTERFACE")
                            continue;
                        t->defines.push_back(args[a]);
                    }
                }
            } else if (cmd.name == "target_compile_options" && args.size() >= 2) {
                auto *t = find_target(args[0]);
                if (t) {
                    for (size_t a = 1; a < args.size(); ++a) {
                        if (args[a] == "PRIVATE" || args[a] == "PUBLIC" || args[a] == "INTERFACE")
                            continue;
                        std::string opt = args[a];
                        if (opt.empty() || (opt[0] != '-' && opt[0] != '/'))
                            continue;
                        t->compile_options.push_back(opt);
                    }
                }
            } else if (cmd.name == "target_compile_features" && args.size() >= 2) {
                auto *t = find_target(args[0]);
                if (t) {
                    for (size_t a = 1; a < args.size(); ++a) {
                        if (args[a] == "cxx_std_11")
                            t->cpp_version = CppVersion::Cpp11;
                        else if (args[a] == "cxx_std_14")
                            t->cpp_version = CppVersion::Cpp14;
                        else if (args[a] == "cxx_std_17")
                            t->cpp_version = CppVersion::Cpp17;
                        else if (args[a] == "cxx_std_20")
                            t->cpp_version = CppVersion::Cpp20;
                        else if (args[a] == "cxx_std_23")
                            t->cpp_version = CppVersion::Cpp23;
                        else if (args[a] == "cxx_std_26")
                            t->cpp_version = CppVersion::Cpp26;
                    }
                }
            } else if (cmd.name == "set_target_properties" && args.size() >= 3) {
                auto *t = find_target(args[0]);
                if (t) {
                    for (size_t a = 1; a + 1 < args.size(); ++a) {
                        if (args[a] == "CXX_STANDARD") {
                            std::string std_str = args[a + 1];
                            if (std_str == "11")
                                t->cpp_version = CppVersion::Cpp11;
                            else if (std_str == "14")
                                t->cpp_version = CppVersion::Cpp14;
                            else if (std_str == "17")
                                t->cpp_version = CppVersion::Cpp17;
                            else if (std_str == "20")
                                t->cpp_version = CppVersion::Cpp20;
                            else if (std_str == "23")
                                t->cpp_version = CppVersion::Cpp23;
                            else if (std_str == "26")
                                t->cpp_version = CppVersion::Cpp26;
                        }
                    }
                }
            } else if (cmd.name == "target_link_directories" && args.size() >= 2) {
                auto *t = find_target(args[0]);
                if (t) {
                    for (size_t a = 1; a < args.size(); ++a) {
                        if (args[a] == "PRIVATE" || args[a] == "PUBLIC" || args[a] == "INTERFACE")
                            continue;
                        t->link_directories.push_back(args[a]);
                    }
                }
            } else if (cmd.name == "target_link_libraries" && args.size() >= 2) {
                auto *t = find_target(args[0]);
                if (t) {
                    for (size_t a = 1; a < args.size(); ++a) {
                        if (args[a] == "PRIVATE" || args[a] == "PUBLIC" || args[a] == "INTERFACE")
                            continue;
                        std::string lib = args[a];
                        // Link target if target exists
                        bool is_target = false;
                        for (const auto &ot : proj.targets) {
                            if (ot.name == lib) {
                                is_target = true;
                                break;
                            }
                        }
                        if (is_target) {
                            t->linked_targets.push_back(lib);
                        } else {
                            if (lib == "Vulkan::Vulkan") {
                                lib = "vulkan-1";
                            } else {
                                size_t cpos = lib.find("::");
                                if (cpos != std::string::npos) {
                                    lib = lib.substr(cpos + 2);
                                }
                            }
                            if (lib == "glfw") {
                                fs::path vcpkg_lib = base_dir / "build/vcpkg_installed/x64-windows/lib";
                                if (!fs::exists(vcpkg_lib))
                                    vcpkg_lib = base_dir / "vcpkg_installed/x64-windows/lib";
                                if (fs::exists(vcpkg_lib / "glfw3dll.lib")) {
                                    lib = "glfw3dll";
                                } else if (fs::exists(vcpkg_lib / "glfw3.lib")) {
                                    lib = "glfw3";
                                }
                            }
                            if (std::find(t->system_libs.begin(), t->system_libs.end(), lib) == t->system_libs.end()) {
                                t->system_libs.push_back(lib);
                            }
                            if (lib == "spdlog") {
                                if (std::find(t->system_libs.begin(), t->system_libs.end(), "fmt") ==
                                    t->system_libs.end()) {
                                    t->system_libs.push_back("fmt");
                                }
                            }
                            if (lib == "ktx") {
                                if (std::find(t->system_libs.begin(), t->system_libs.end(), "zstd") ==
                                    t->system_libs.end()) {
                                    t->system_libs.push_back("zstd");
                                }
                            }
                        }
                    }
                }
            } else if (cmd.name == "add_test" && args.size() >= 2) {
                // add_test(NAME test_name COMMAND target_name ...)
                std::string target_name = "";
                for (size_t a = 0; a + 1 < args.size(); ++a) {
                    if (args[a] == "COMMAND") {
                        target_name = args[a + 1];
                        break;
                    }
                }
                if (target_name.empty() && args.size() == 2) {
                    target_name = args[1];
                }
                auto *t = find_target(target_name);
                if (t) {
                    t->is_test = true;
                    t->type = TargetType::Test;
                }
            } else if (cmd.name == "add_subdirectory" && !args.empty()) {
                if (depth >= 32) {
                    continue; // Recursion depth limit defense
                }
                fs::path subdir = args[0];
                fs::path next_dir = (curr_dir.empty() || curr_dir == ".") ? subdir : (curr_dir / subdir);
                fs::path sub_cmake = (base_dir / next_dir / "CMakeLists.txt").lexically_normal();
                if (util::is_system_protected_dir(sub_cmake)) {
                    continue; // Skip system protected directories
                }
                std::string canon = fs::weakly_canonical(sub_cmake, ec).string();
                if (ec)
                    canon = sub_cmake.string();
                if (visited.insert(canon).second) {
                    if (fs::exists(sub_cmake, ec)) {
                        std::ifstream in(sub_cmake);
                        if (in) {
                            std::string sub_content((std::istreambuf_iterator<char>(in)),
                                                    std::istreambuf_iterator<char>());
                            parse_cmake_content(sub_content, base_dir, next_dir, proj, vars, error_msg, visited,
                                                depth + 1);
                        }
                    }
                }
            }
        }
        return true;
    }

    static bool parse_cmake_content(std::string_view content, const std::filesystem::path &base_dir,
                                    const std::filesystem::path &curr_dir, ImportedProject &proj,
                                    std::unordered_map<std::string, std::vector<std::string>> &vars,
                                    std::string &error_msg) {
        std::unordered_set<std::string> visited;
        return parse_cmake_content(content, base_dir, curr_dir, proj, vars, error_msg, visited, 0);
    }

    static std::optional<ImportedProject> import_file(const std::filesystem::path &filepath, std::string &error_msg) {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (!fs::exists(filepath, ec)) {
            error_msg = "CMakeLists file does not exist: " + filepath.string();
            return std::nullopt;
        }

        std::ifstream in(filepath);
        if (!in) {
            error_msg = "Failed to open CMakeLists file: " + filepath.string();
            return std::nullopt;
        }
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        ImportedProject proj;
        proj.project_name = filepath.parent_path().stem().string();
        if (proj.project_name.empty() || proj.project_name == ".") {
            proj.project_name = "App";
        }

        std::unordered_map<std::string, std::vector<std::string>> vars;
        fs::path base_dir = filepath.parent_path().empty() ? fs::path(".") : filepath.parent_path();
        std::unordered_set<std::string> visited;
        std::string canon_main = fs::weakly_canonical(filepath, ec).string();
        if (ec)
            canon_main = filepath.string();
        visited.insert(canon_main);
        if (!parse_cmake_content(content, base_dir, ".", proj, vars, error_msg, visited, 0)) {
            return std::nullopt;
        }

        // Post-process linked targets: if a library was added after target_link_libraries was parsed
        for (auto &t : proj.targets) {
            auto sys_copy = t.system_libs;
            t.system_libs.clear();
            for (const auto &lib : sys_copy) {
                bool is_target = false;
                for (const auto &ot : proj.targets) {
                    if (ot.name == lib) {
                        is_target = true;
                        break;
                    }
                }
                if (is_target) {
                    if (std::find(t.linked_targets.begin(), t.linked_targets.end(), lib) == t.linked_targets.end()) {
                        t.linked_targets.push_back(lib);
                    }
                } else {
                    t.system_libs.push_back(lib);
                }
            }
        }

        return proj;
    }
};

class MsvcImporter {
public:
    static std::string extract_tag(std::string_view xml, std::string_view tag) {
        std::string open = "<" + std::string(tag);
        size_t p1 = xml.find(open);
        if (p1 == std::string::npos)
            return "";
        size_t close_bracket = xml.find('>', p1);
        if (close_bracket == std::string::npos)
            return "";
        if (xml[close_bracket - 1] == '/')
            return ""; // self-closing
        std::string close = "</" + std::string(tag) + ">";
        size_t p2 = xml.find(close, close_bracket);
        if (p2 == std::string::npos)
            return "";
        return std::string(xml.substr(close_bracket + 1, p2 - close_bracket - 1));
    }

    static std::vector<std::string> extract_all_attributes(std::string_view xml, std::string_view tag,
                                                           std::string_view attr) {
        std::vector<std::string> results;
        std::string tag_open = "<" + std::string(tag);
        std::string attr_lead = std::string(attr) + "=\"";
        size_t pos = 0;
        while ((pos = xml.find(tag_open, pos)) != std::string::npos) {
            size_t tag_end = xml.find('>', pos);
            if (tag_end == std::string::npos)
                break;
            size_t attr_pos = xml.find(attr_lead, pos);
            if (attr_pos != std::string::npos && attr_pos < tag_end) {
                size_t val_start = attr_pos + attr_lead.size();
                size_t val_end = xml.find('\"', val_start);
                if (val_end != std::string::npos && val_end <= tag_end) {
                    results.emplace_back(xml.substr(val_start, val_end - val_start));
                }
            }
            pos = tag_end + 1;
        }
        return results;
    }

    static std::optional<ImportedTarget> import_vcxproj(const std::filesystem::path &vcxproj_path,
                                                        const std::filesystem::path &root_dir, std::string &error_msg) {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (!fs::exists(vcxproj_path, ec)) {
            error_msg = "vcxproj file does not exist: " + vcxproj_path.string();
            return std::nullopt;
        }

        std::ifstream in(vcxproj_path);
        if (!in) {
            error_msg = "Failed to open vcxproj file: " + vcxproj_path.string();
            return std::nullopt;
        }
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        ImportedTarget t;
        std::string p_name = extract_tag(content, "ProjectName");
        if (p_name.empty()) {
            p_name = extract_tag(content, "RootNamespace");
        }
        if (p_name.empty()) {
            p_name = vcxproj_path.stem().string();
        }
        t.name = p_name;

        // ConfigurationType
        std::string cfg_type = extract_tag(content, "ConfigurationType");
        if (cfg_type == "StaticLibrary") {
            t.type = TargetType::StaticLibrary;
        } else if (cfg_type == "DynamicLibrary") {
            t.type = TargetType::SharedLibrary;
        } else {
            t.type = TargetType::Executable;
        }

        // LanguageStandard
        std::string lang_std = extract_tag(content, "LanguageStandard");
        if (lang_std == "stdcpp14")
            t.cpp_version = CppVersion::Cpp14;
        else if (lang_std == "stdcpp17")
            t.cpp_version = CppVersion::Cpp17;
        else if (lang_std == "stdcpp20")
            t.cpp_version = CppVersion::Cpp20;
        else if (lang_std == "stdcpplatest")
            t.cpp_version = CppVersion::Cpp23;

        // PCH
        std::string pch_file = extract_tag(content, "PrecompiledHeaderFile");
        std::string pch_use = extract_tag(content, "PrecompiledHeader");
        if (!pch_file.empty() && pch_use != "NotUsing") {
            t.pch_header = pch_file;
        }

        fs::path vcx_dir = vcxproj_path.parent_path();
        if (vcx_dir.empty())
            vcx_dir = ".";

        // Sources: ClCompile Include="..."
        auto compile_includes = extract_all_attributes(content, "ClCompile", "Include");
        for (const auto &inc : compile_includes) {
            fs::path src_p = (vcx_dir / inc).lexically_normal();
            fs::path rel = fs::relative(src_p, root_dir, ec);
            std::string norm = util::normalize_path(ec ? src_p.string() : rel.string(), false);
            if (norm.rfind("./", 0) == 0)
                norm = norm.substr(2);
            t.sources.push_back(norm);
        }

        // AdditionalIncludeDirectories
        std::string inc_tag = extract_tag(content, "AdditionalIncludeDirectories");
        if (!inc_tag.empty()) {
            std::istringstream stream(inc_tag);
            std::string item;
            while (std::getline(stream, item, ';')) {
                if (item.empty() || item.find("%(") != std::string::npos)
                    continue;
                fs::path inc_p = (vcx_dir / item).lexically_normal();
                fs::path rel = fs::relative(inc_p, root_dir, ec);
                std::string norm = util::normalize_path(ec ? inc_p.string() : rel.string(), false);
                if (norm.rfind("./", 0) == 0)
                    norm = norm.substr(2);
                t.include_dirs.push_back(norm);
            }
        }

        // PreprocessorDefinitions
        std::string def_tag = extract_tag(content, "PreprocessorDefinitions");
        if (!def_tag.empty()) {
            std::istringstream stream(def_tag);
            std::string item;
            while (std::getline(stream, item, ';')) {
                if (item.empty() || item.find("%(") != std::string::npos)
                    continue;
                // Filter out standard MSVC boilerplate
                if (item == "WIN32" || item == "_WINDOWS" || item == "_CONSOLE" || item == "_DEBUG" ||
                    item == "NDEBUG" || item == "_LIB" || item == "_USRDLL" || item == "_WINDLL" || item == "UNICODE" ||
                    item == "_UNICODE" || item == "_MBCS") {
                    continue;
                }
                t.defines.push_back(item);
            }
        }

        // AdditionalDependencies
        std::string lib_tag = extract_tag(content, "AdditionalDependencies");
        if (!lib_tag.empty()) {
            std::istringstream stream(lib_tag);
            std::string item;
            while (std::getline(stream, item, ';')) {
                if (item.empty() || item.find("%(") != std::string::npos)
                    continue;
                // Filter out standard Windows SDK libraries
                if (item == "kernel32.lib" || item == "user32.lib" || item == "gdi32.lib" || item == "winspool.lib" ||
                    item == "comdlg32.lib" || item == "advapi32.lib" || item == "shell32.lib" || item == "ole32.lib" ||
                    item == "oleaut32.lib" || item == "uuid.lib" || item == "odbc32.lib" || item == "odbccp32.lib") {
                    continue;
                }
                t.system_libs.push_back(item);
            }
        }

        // ProjectReference Include="..."
        auto project_refs = extract_all_attributes(content, "ProjectReference", "Include");
        for (const auto &ref : project_refs) {
            std::string dep_name = fs::path(ref).stem().string();
            t.linked_targets.push_back(dep_name);
        }

        return t;
    }

    static std::optional<ImportedProject> import_project(const std::filesystem::path &vcxproj_path,
                                                         std::string &error_msg) {
        namespace fs = std::filesystem;
        fs::path root_dir = vcxproj_path.parent_path().empty() ? fs::path(".") : vcxproj_path.parent_path();
        auto t_opt = import_vcxproj(vcxproj_path, root_dir, error_msg);
        if (!t_opt)
            return std::nullopt;

        ImportedProject proj;
        proj.project_name = t_opt->name;
        proj.targets.push_back(std::move(*t_opt));
        return proj;
    }

    static std::optional<ImportedProject> import_solution(const std::filesystem::path &sln_path,
                                                          std::string &error_msg) {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (!fs::exists(sln_path, ec)) {
            error_msg = "Solution file does not exist: " + sln_path.string();
            return std::nullopt;
        }

        std::ifstream in(sln_path);
        if (!in) {
            error_msg = "Failed to open solution file: " + sln_path.string();
            return std::nullopt;
        }

        ImportedProject proj;
        proj.project_name = sln_path.stem().string();
        fs::path sln_dir = sln_path.parent_path().empty() ? fs::path(".") : sln_path.parent_path();

        struct SlnProject {
            std::string name;
            fs::path rel_path;
            std::string guid;
        };
        std::vector<SlnProject> sln_projects;
        std::unordered_map<std::string, std::vector<std::string>> guid_deps;

        std::string line;
        std::string current_project_guid = "";
        bool in_dependencies = false;

        while (std::getline(in, line)) {
            if (line.rfind("Project(\"", 0) == 0) {
                size_t eq = line.find('=');
                if (eq != std::string::npos) {
                    std::vector<std::string> parts;
                    size_t p = eq + 1;
                    while (p < line.size()) {
                        size_t q1 = line.find('\"', p);
                        if (q1 == std::string::npos)
                            break;
                        size_t q2 = line.find('\"', q1 + 1);
                        if (q2 == std::string::npos)
                            break;
                        parts.push_back(line.substr(q1 + 1, q2 - q1 - 1));
                        p = q2 + 1;
                    }
                    if (parts.size() >= 3) {
                        std::string p_name = parts[0];
                        std::string p_path = parts[1];
                        std::string p_guid = parts[2];
                        if (p_path.find(".vcxproj") != std::string::npos) {
                            sln_projects.push_back({p_name, fs::path(p_path), p_guid});
                            current_project_guid = p_guid;
                        }
                    }
                }
            } else if (line.find("ProjectSection(ProjectDependencies)") != std::string::npos) {
                in_dependencies = true;
            } else if (line.find("EndProjectSection") != std::string::npos) {
                in_dependencies = false;
            } else if (in_dependencies && !current_project_guid.empty()) {
                size_t eq = line.find('=');
                if (eq != std::string::npos) {
                    auto trim = [](std::string s) {
                        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
                            s.erase(s.begin());
                        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
                            s.pop_back();
                        return s;
                    };
                    std::string dep_guid = trim(line.substr(eq + 1));
                    if (!dep_guid.empty()) {
                        guid_deps[current_project_guid].push_back(dep_guid);
                    }
                }
            }
        }

        for (const auto &sp : sln_projects) {
            fs::path vcx_full = sln_dir / sp.rel_path;
            std::string sub_err;
            auto t_opt = import_vcxproj(vcx_full, sln_dir, sub_err);
            if (t_opt) {
                if (t_opt->name.empty())
                    t_opt->name = sp.name;

                // Add GUID dependencies
                auto dit = guid_deps.find(sp.guid);
                if (dit != guid_deps.end()) {
                    for (const auto &dep_g : dit->second) {
                        for (const auto &other_sp : sln_projects) {
                            if (other_sp.guid == dep_g) {
                                if (std::find(t_opt->linked_targets.begin(), t_opt->linked_targets.end(),
                                              other_sp.name) == t_opt->linked_targets.end()) {
                                    t_opt->linked_targets.push_back(other_sp.name);
                                }
                                break;
                            }
                        }
                    }
                }
                proj.targets.push_back(std::move(*t_opt));
            }
        }

        if (proj.targets.empty()) {
            error_msg = "No C/C++ projects (.vcxproj) found in solution: " + sln_path.string();
            return std::nullopt;
        }

        return proj;
    }
};

struct ImportOptions {
    std::filesystem::path output_path = "build.cpp";
    bool force = false;
    bool dry_run = false;
    bool verbose = false;
    std::string format = "";
};

struct ImportResult {
    bool success = true;
    std::string generated_code = "";
    std::string error_message = "";
    size_t targets_imported = 0;
    std::string detected_format = "";
    std::filesystem::path source_path = "";
};

class ProjectImporter {
public:
    static ImportResult import_project(const std::filesystem::path &input_path, const ImportOptions &options = {}) {
        namespace fs = std::filesystem;
        ImportResult result;
        std::error_code ec;

        if (!options.dry_run && util::is_system_protected_dir(options.output_path)) {
            result.success = false;
            result.error_message =
                "cxxgen security error: refusing to write imported project to system protected directory '" +
                options.output_path.string() + "'";
            return result;
        }

        fs::path target_file = input_path;
        std::string format = options.format;

        // Auto-detect if directory or empty
        if (target_file.empty() || fs::is_directory(target_file, ec)) {
            fs::path dir = target_file.empty() ? fs::current_path() : target_file;
            if (format == "cmake" || (format.empty() && fs::exists(dir / "CMakeLists.txt", ec))) {
                target_file = dir / "CMakeLists.txt";
                format = "cmake";
            } else if (format == "msvc" || format.empty()) {
                // Search for .sln
                fs::path found_sln;
                fs::path found_vcxproj;
                for (const auto &entry : fs::directory_iterator(dir, ec)) {
                    if (entry.path().extension() == ".sln") {
                        found_sln = entry.path();
                        break;
                    } else if (entry.path().extension() == ".vcxproj") {
                        found_vcxproj = entry.path();
                    }
                }
                if (!found_sln.empty()) {
                    target_file = found_sln;
                    format = "msvc";
                } else if (!found_vcxproj.empty()) {
                    target_file = found_vcxproj;
                    format = "msvc";
                }
            }
        }

        if (!fs::exists(target_file, ec)) {
            result.success = false;
            result.error_message =
                "cxxgen error: could not locate build specification at '" + target_file.string() + "'";
            return result;
        }

        // Determine format if not yet known
        if (format.empty()) {
            std::string filename = target_file.filename().string();
            std::string ext = target_file.extension().string();
            if (filename == "CMakeLists.txt" || ext == ".cmake") {
                format = "cmake";
            } else if (ext == ".sln" || ext == ".vcxproj") {
                format = "msvc";
            } else {
                result.success = false;
                result.error_message = "cxxgen error: unrecognized file format '" + target_file.string() +
                                       "'. Expected CMakeLists.txt, .sln, or .vcxproj.";
                return result;
            }
        }

        result.detected_format = format;
        result.source_path = target_file;

        std::optional<ImportedProject> proj_opt;
        std::string parse_err;

        if (format == "cmake") {
            proj_opt = CMakeImporter::import_file(target_file, parse_err);
        } else if (format == "msvc") {
            if (target_file.extension() == ".sln") {
                proj_opt = MsvcImporter::import_solution(target_file, parse_err);
            } else {
                proj_opt = MsvcImporter::import_project(target_file, parse_err);
            }
        } else {
            result.success = false;
            result.error_message = "cxxgen error: unsupported import format '" + format + "'. Supported: cmake, msvc.";
            return result;
        }

        if (!proj_opt) {
            result.success = false;
            result.error_message = "cxxgen error: import failed: " + parse_err;
            return result;
        }

        result.targets_imported = proj_opt->targets.size();
        result.generated_code = ProjectCodeSynthesizer::synthesize(*proj_opt);

        if (options.dry_run) {
            result.success = true;
            return result;
        }

        // Check if output file already exists
        fs::path out_p = options.output_path;
        if (util::is_system_protected_dir(out_p)) {
            result.success = false;
            result.error_message =
                "cxxgen security error: refusing to write imported project to system protected directory '" +
                out_p.string() + "'";
            return result;
        }

        if (fs::exists(out_p, ec) && !options.force) {
            result.success = false;
            result.error_message =
                "cxxgen error: '" + out_p.string() +
                "' already exists. Use --force to overwrite or --out=<file> to specify a different path.";
            return result;
        }

        if (out_p.has_parent_path()) {
            fs::create_directories(out_p.parent_path(), ec);
        }

        std::ofstream out_file(out_p);
        if (!out_file) {
            result.success = false;
            result.error_message = "cxxgen error: failed to write output file: " + out_p.string();
            return result;
        }

        out_file << result.generated_code;
        result.success = true;
        return result;
    }
};

// ============================================================================
// 8.21. Header Self-Containment Linter (cxxgen check-headers)
// ============================================================================

struct HeaderCheckConfig {
    std::vector<std::filesystem::path> paths = {};
    std::vector<std::filesystem::path> include_dirs = {};
    std::vector<std::string> defines = {};
    std::vector<std::string> extra_flags = {};
    CppVersion cpp_version = CppVersion::Cpp20;
    bool verbose = false;
    size_t jobs = 0;
    std::filesystem::path work_dir = {};
};

struct HeaderCheckFailure {
    std::filesystem::path header_path;
    std::string error_message;
};

struct HeaderCheckResult {
    bool success = true;
    size_t headers_scanned = 0;
    size_t headers_passed = 0;
    size_t headers_failed = 0;
    std::vector<HeaderCheckFailure> failures = {};
    std::string report = "";
};

class HeaderChecker {
public:
    static bool is_header(const std::filesystem::path &p) {
        std::string ext = p.extension().string();
        for (char &c : ext) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return ext == ".h" || ext == ".hpp" || ext == ".hxx" || ext == ".hh" || ext == ".h++" || ext == ".inl";
    }

    static bool is_ignored_directory(std::string_view name) {
        return name == ".git" || name == ".cxxgen" || name == "build" || name == "bin" || name == "obj" ||
               name == "out" || name == "dist" || name == "node_modules" || name == ".vs" || name == ".idea";
    }

    static std::vector<std::filesystem::path> discover_headers(const HeaderCheckConfig &config) {
        namespace fs = std::filesystem;
        std::error_code ec;
        std::vector<fs::path> target_paths = config.paths;
        fs::path base = config.work_dir.empty() ? fs::current_path(ec) : config.work_dir;

        if (target_paths.empty()) {
            if (fs::exists(base / "include", ec)) {
                target_paths.push_back(base / "include");
            }
            if (fs::exists(base / "src", ec)) {
                target_paths.push_back(base / "src");
            }
            if (target_paths.empty()) {
                target_paths.push_back(base);
            }
        }

        std::vector<fs::path> discovered;
        for (const auto &p_raw : target_paths) {
            fs::path p = p_raw.is_absolute() ? p_raw : (base / p_raw);
            if (fs::is_regular_file(p, ec)) {
                if (is_header(p)) {
                    discovered.push_back(p.lexically_normal());
                }
            } else if (fs::is_directory(p, ec)) {
                for (fs::recursive_directory_iterator it(p, fs::directory_options::skip_permission_denied, ec), end;
                     it != end; ++it) {
                    if (ec) {
                        ec.clear();
                        continue;
                    }
                    if (it->is_directory(ec)) {
                        if (is_ignored_directory(it->path().filename().string())) {
                            it.disable_recursion_pending();
                            continue;
                        }
                    } else if (it->is_regular_file(ec)) {
                        if (is_header(it->path())) {
                            discovered.push_back(it->path().lexically_normal());
                        }
                    }
                }
            }
        }

        // Deduplicate
        std::vector<fs::path> unique_headers;
        for (const auto &h : discovered) {
            bool dup = false;
            for (const auto &uh : unique_headers) {
                if (uh == h) {
                    dup = true;
                    break;
                }
            }
            if (!dup) {
                unique_headers.push_back(h);
            }
        }
        return unique_headers;
    }

    static HeaderCheckResult check_all(const IToolchain &toolchain, const HeaderCheckConfig &config) {
        namespace fs = std::filesystem;
        std::error_code ec;
        HeaderCheckResult result;

        auto headers = discover_headers(config);
        if (headers.empty()) {
            result.success = true;
            result.report = "[cxxgen check-headers] No C/C++ header files found to check.\n";
            return result;
        }

        result.headers_scanned = headers.size();
        fs::path base = config.work_dir.empty() ? fs::current_path(ec) : config.work_dir;

        // Build list of include search directories
        std::vector<fs::path> inc_dirs = config.include_dirs;
        if (inc_dirs.empty()) {
            if (fs::exists(base / "include", ec))
                inc_dirs.push_back(base / "include");
            if (fs::exists(base / "src", ec))
                inc_dirs.push_back(base / "src");
            inc_dirs.push_back(base);
        }

        fs::path tmp_dir = base / ".cxxgen" / "tmp_check_headers";
        fs::create_directories(tmp_dir, ec);

        std::mutex res_mutex;
        size_t thread_count =
            config.jobs > 0 ? config.jobs : std::max<unsigned int>(1, std::thread::hardware_concurrency());
        ThreadPool pool(thread_count);

        for (size_t i = 0; i < headers.size(); ++i) {
            const auto &header = headers[i];
            pool.enqueue([&toolchain, &config, &inc_dirs, &tmp_dir, header, i, &result, &res_mutex]() {
                namespace fs = std::filesystem;
                std::error_code ec;

                if (header.string().find_first_of("\"\r\n") != std::string::npos) {
                    std::lock_guard<std::mutex> lock(res_mutex);
                    result.failures.push_back({header, "header path contains illegal quotes or control characters"});
                    result.headers_failed++;
                    result.success = false;
                    return;
                }

                fs::path snippet_file = tmp_dir / ("chk_" + std::to_string(i) + ".cpp");
                std::ofstream snip_out(snippet_file);
                snip_out << "#include \"" << util::normalize_path(header.string(), false) << "\"\n";
                snip_out.close();

                std::vector<std::string> cmd;
                cmd.push_back(toolchain.compiler_path());

                if (toolchain.family() == CompilerFamily::Msvc) {
                    cmd.push_back("/nologo");
                    cmd.push_back("/Zs");
                    cmd.push_back("/WX");
                    cmd.push_back("/EHsc");
                    switch (config.cpp_version) {
                    case CppVersion::Cpp14:
                        cmd.push_back("/std:c++14");
                        break;
                    case CppVersion::Cpp17:
                        cmd.push_back("/std:c++17");
                        break;
                    case CppVersion::Cpp20:
                        cmd.push_back("/std:c++20");
                        break;
                    case CppVersion::Cpp23:
                    case CppVersion::Cpp26:
                        cmd.push_back("/std:c++latest");
                        break;
                    default:
                        cmd.push_back("/std:c++20");
                        break;
                    }
                    for (const auto &inc : inc_dirs) {
                        cmd.push_back("/I" + util::normalize_path(inc.string(), true));
                    }
                    if (header.has_parent_path()) {
                        cmd.push_back("/I" + util::normalize_path(header.parent_path().string(), true));
                    }
                    for (const auto &def : config.defines) {
                        cmd.push_back("/D" + def);
                    }
                    for (const auto &flg : config.extra_flags) {
                        cmd.push_back(flg);
                    }
                    cmd.push_back(snippet_file.string());
                } else {
                    // Clang or GCC
                    cmd.push_back("-fsyntax-only");
                    cmd.push_back("-Werror");
                    switch (config.cpp_version) {
                    case CppVersion::Cpp11:
                        cmd.push_back("-std=c++11");
                        break;
                    case CppVersion::Cpp14:
                        cmd.push_back("-std=c++14");
                        break;
                    case CppVersion::Cpp17:
                        cmd.push_back("-std=c++17");
                        break;
                    case CppVersion::Cpp20:
                        cmd.push_back("-std=c++20");
                        break;
                    case CppVersion::Cpp23:
                        cmd.push_back("-std=c++23");
                        break;
                    case CppVersion::Cpp26:
                        cmd.push_back("-std=c++2c");
                        break;
                    }
                    for (const auto &inc : inc_dirs) {
                        cmd.push_back("-I" + util::normalize_path(inc.string(), false));
                    }
                    if (header.has_parent_path()) {
                        cmd.push_back("-I" + util::normalize_path(header.parent_path().string(), false));
                    }
                    for (const auto &def : config.defines) {
                        cmd.push_back("-D" + def);
                    }
                    for (const auto &flg : config.extra_flags) {
                        cmd.push_back(flg);
                    }
                    cmd.push_back(snippet_file.string());
                }

                ProcessOptions popts;
                popts.command = cmd;
                popts.working_directory = config.work_dir.empty() ? fs::current_path(ec) : config.work_dir;
                auto pres = ProcessRunner::run(popts);

                fs::remove(snippet_file, ec);

                std::lock_guard<std::mutex> lock(res_mutex);
                if (pres.success()) {
                    result.headers_passed++;
                    if (config.verbose) {
                        std::cout << "[OK]   " << header.string() << "\n";
                    }
                } else {
                    result.headers_failed++;
                    result.failures.push_back({header, pres.combined_output()});
                    if (config.verbose) {
                        std::cout << "[FAIL] " << header.string() << "\n";
                    }
                }
            });
        }

        pool.wait_all();
        fs::remove_all(tmp_dir, ec);

        std::ostringstream ss;
        if (result.headers_failed == 0) {
            result.success = true;
            ss << "[cxxgen check-headers] All " << result.headers_passed << " header(s) are self-contained.\n";
        } else {
            result.success = false;
            ss << "[cxxgen check-headers] " << result.headers_failed << " of " << result.headers_scanned
               << " header(s) failed self-containment check:\n\n";
            for (const auto &fail : result.failures) {
                ss << "  - " << fail.header_path.string() << ":\n";
                std::istringstream err_stream(fail.error_message);
                std::string err_line;
                while (std::getline(err_stream, err_line)) {
                    if (!err_line.empty()) {
                        ss << "      " << err_line << "\n";
                    }
                }
                ss << "\n";
            }
        }
        result.report = ss.str();
        return result;
    }
};

// ============================================================================
// 8.22. Enterprise Dependency Hygiene & Packaging Engine (Epic 14)
// ============================================================================

class CMakeConfigExporter {
public:
    static bool export_config(const Builder &builder, const CMakeConfigOptions &options, std::string *error = nullptr) {
        namespace fs = std::filesystem;
        std::error_code ec;

        std::string pkg_name = options.package_name.empty()
                                   ? (builder.project_name().empty() ? "Project" : builder.project_name())
                                   : options.package_name;
        std::string version = options.version.empty() ? "1.0.0" : options.version;
        std::string ns = options.namespace_name.empty() ? pkg_name : options.namespace_name;

        auto is_safe_cmake_str = [](std::string_view s) {
            if (s.empty())
                return false;
            for (char c : s) {
                if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-' && c != '.' && c != ':')
                    return false;
            }
            return true;
        };

        if (!is_safe_cmake_str(pkg_name) || !is_safe_cmake_str(version) || !is_safe_cmake_str(ns)) {
            if (error)
                *error = "cxxgen error: package name, version, or namespace contains illegal CMake characters";
            return false;
        }

        fs::path out_dir = options.output_dir.empty() ? (fs::path("lib") / "cmake" / pkg_name) : options.output_dir;
        fs::create_directories(out_dir, ec);

        // 1. <Package>ConfigVersion.cmake
        fs::path ver_file = out_dir / (pkg_name + "ConfigVersion.cmake");
        std::ofstream ver_out(ver_file);
        if (!ver_out.is_open()) {
            if (error)
                *error = "Failed to create " + ver_file.string();
            return false;
        }

        std::string major_ver = "1";
        auto dot_pos = version.find('.');
        if (dot_pos != std::string::npos) {
            major_ver = version.substr(0, dot_pos);
        }

        ver_out << "# Generated by cxxgen (C++ Generator & Build System)\n"
                << "set(PACKAGE_VERSION \"" << version << "\")\n\n"
                << "if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)\n"
                << "  set(PACKAGE_VERSION_COMPATIBLE FALSE)\n"
                << "else()\n"
                << "  set(PACKAGE_VERSION_COMPATIBLE TRUE)\n"
                << "  if(PACKAGE_FIND_VERSION_MAJOR EQUAL " << major_ver << ")\n"
                << "    set(PACKAGE_VERSION_EXACT TRUE)\n"
                << "  endif()\n"
                << "endif()\n";
        ver_out.close();

        // 2. <Package>Targets.cmake
        fs::path targets_file = out_dir / (pkg_name + "Targets.cmake");
        std::ofstream tgt_out(targets_file);
        if (!tgt_out.is_open()) {
            if (error)
                *error = "Failed to create " + targets_file.string();
            return false;
        }

        tgt_out << "# Generated by cxxgen (C++ Generator & Build System)\n"
                << "# Relocatable package target definitions\n\n"
                << "get_filename_component(_IMPORT_PREFIX \"${CMAKE_CURRENT_LIST_FILE}\" PATH)\n"
                << "get_filename_component(_IMPORT_PREFIX \"${_IMPORT_PREFIX}\" PATH)\n"
                << "get_filename_component(_IMPORT_PREFIX \"${_IMPORT_PREFIX}\" PATH)\n"
                << "if(_IMPORT_PREFIX STREQUAL \"/\")\n"
                << "  set(_IMPORT_PREFIX \"\")\n"
                << "endif()\n\n";

        std::vector<const Target *> targets_to_export;
        if (!options.target_names.empty()) {
            for (const auto &tname : options.target_names) {
                const Target *t = builder.find_target(tname);
                if (t)
                    targets_to_export.push_back(t);
            }
        } else {
            for (const auto &t : builder.targets()) {
                if (t->type() == TargetType::StaticLibrary || t->type() == TargetType::SharedLibrary ||
                    t->type() == TargetType::Interface) {
                    targets_to_export.push_back(t.get());
                }
            }
        }

        for (const Target *t : targets_to_export) {
            std::string target_type_str = "STATIC";
            if (t->type() == TargetType::SharedLibrary)
                target_type_str = "SHARED";
            else if (t->type() == TargetType::Interface)
                target_type_str = "INTERFACE";

            std::string cmake_target = ns + "::" + t->name();

            tgt_out << "if(NOT TARGET " << cmake_target << ")\n";
            tgt_out << "  add_library(" << cmake_target << " " << target_type_str << " IMPORTED)\n";
            tgt_out << "  set_target_properties(" << cmake_target << " PROPERTIES\n";
            tgt_out << "    INTERFACE_INCLUDE_DIRECTORIES \"${_IMPORT_PREFIX}/include\"\n";

            if (t->type() == TargetType::StaticLibrary) {
#if defined(_WIN32)
                std::string lib_name = t->name() + ".lib";
#else
                std::string lib_name = "lib" + t->name() + ".a";
#endif
                tgt_out << "    IMPORTED_LOCATION \"${_IMPORT_PREFIX}/lib/" << lib_name << "\"\n";
            } else if (t->type() == TargetType::SharedLibrary) {
#if defined(_WIN32)
                tgt_out << "    IMPORTED_IMPLIB \"${_IMPORT_PREFIX}/lib/" << t->name() << ".lib\"\n";
                tgt_out << "    IMPORTED_LOCATION \"${_IMPORT_PREFIX}/bin/" << t->name() << ".dll\"\n";
#elif defined(__APPLE__)
                tgt_out << "    IMPORTED_LOCATION \"${_IMPORT_PREFIX}/lib/lib" << t->name() << ".dylib\"\n";
#else
                tgt_out << "    IMPORTED_LOCATION \"${_IMPORT_PREFIX}/lib/lib" << t->name() << ".so\"\n";
#endif
            }

            if (!t->config().defines.empty()) {
                tgt_out << "    INTERFACE_COMPILE_DEFINITIONS \"";
                for (size_t d = 0; d < t->config().defines.size(); ++d) {
                    if (d > 0)
                        tgt_out << ";";
                    tgt_out << t->config().defines[d];
                }
                tgt_out << "\"\n";
            }

            tgt_out << "  )\n";
            tgt_out << "endif()\n\n";
        }
        tgt_out.close();

        // 3. <Package>Config.cmake
        fs::path cfg_file = out_dir / (pkg_name + "Config.cmake");
        std::ofstream cfg_out(cfg_file);
        if (!cfg_out.is_open()) {
            if (error)
                *error = "Failed to create " + cfg_file.string();
            return false;
        }

        cfg_out << "# Generated by cxxgen (C++ Generator & Build System)\n"
                << "# Package configuration for " << pkg_name << "\n\n"
                << "include(CMakeFindDependencyMacro)\n\n"
                << "if(EXISTS \"${CMAKE_CURRENT_LIST_DIR}/" << pkg_name << "Targets.cmake\")\n"
                << "  include(\"${CMAKE_CURRENT_LIST_DIR}/" << pkg_name << "Targets.cmake\")\n"
                << "endif()\n\n"
                << "if(EXISTS \"${CMAKE_CURRENT_LIST_DIR}/" << pkg_name << "ConfigVersion.cmake\")\n"
                << "  include(\"${CMAKE_CURRENT_LIST_DIR}/" << pkg_name << "ConfigVersion.cmake\")\n"
                << "endif()\n";
        cfg_out.close();

        return true;
    }
};

struct PackageResult {
    bool success{false};
    std::filesystem::path archive_path{};
    uintmax_t archive_size{0};
    std::string package_name{};
    std::string version{};
    std::string error_message{};
};

class PackageEngine {
public:
    static PackageResult package(Builder &builder, const IToolchain &toolchain, const PackageConfig &config,
                                 bool verbose = false) {
        (void)verbose;
        namespace fs = std::filesystem;
        PackageResult result;
        std::error_code ec;

        // 1. Determine package name and version
        std::string pkg_name = config.package_name;
        if (pkg_name.empty()) {
            pkg_name = builder.project_name();
        }
        if (pkg_name.empty()) {
            if (!builder.installed_artifacts().empty()) {
                pkg_name = builder.installed_artifacts()[0];
            } else if (!builder.targets().empty()) {
                pkg_name = builder.targets()[0]->name();
            } else {
                pkg_name = "app";
            }
        }
        std::string version = config.version.empty() ? "1.0.0" : config.version;

        // Security check: Validate package name and version to prevent directory traversal
        if (!util::is_safe_package_name(pkg_name)) {
            result.success = false;
            result.error_message = "cxxgen security error: invalid package name '" + pkg_name +
                                   "': traversal and illegal characters forbidden";
            return result;
        }
        if (!util::is_safe_package_name(version)) {
            result.success = false;
            result.error_message = "cxxgen security error: invalid package version '" + version +
                                   "': traversal and illegal characters forbidden";
            return result;
        }

        result.package_name = pkg_name;
        result.version = version;

        // 2. Identify platform & architecture
        std::string platform_str;
#if defined(_WIN32)
        platform_str = "windows";
#elif defined(__APPLE__)
        platform_str = "macos";
#else
        platform_str = "linux";
#endif
        std::string arch_str = "x64";
#if defined(_M_ARM64) || defined(__aarch64__)
        arch_str = "arm64";
#elif defined(_M_IX86) || defined(__i386__)
        arch_str = "x86";
#endif

        // 3. Determine archive format and filename
        std::string fmt = config.format;
        if (fmt.empty() || fmt == "auto") {
#if defined(_WIN32)
            fmt = "zip";
#else
            fmt = "tar.gz";
#endif
        }

        std::string archive_filename =
            pkg_name + "-" + version + "-" + platform_str + "-" + arch_str + (fmt == "zip" ? ".zip" : ".tar.gz");

        fs::path out_dir = config.output_dir.empty() ? "dist" : config.output_dir;
        if (util::is_system_protected_dir(out_dir)) {
            result.success = false;
            result.error_message =
                "cxxgen security error: refusing to write package to protected system directory: " + out_dir.string();
            return result;
        }
        fs::create_directories(out_dir, ec);
        fs::path final_archive = fs::absolute(out_dir / archive_filename, ec);

        // 4. Staging directory: .cxxgen/stage/<pkg_name>-<version>
        fs::path stage_base = fs::current_path(ec) / ".cxxgen" / "stage";
        fs::path stage_pkg = (stage_base / (pkg_name + "-" + version)).lexically_normal();
        fs::path norm_base = stage_base.lexically_normal();
        auto [b_it, m_it] = std::mismatch(norm_base.begin(), norm_base.end(), stage_pkg.begin(), stage_pkg.end());
        if (b_it != norm_base.end()) {
            result.success = false;
            result.error_message = "cxxgen security error: package staging directory escapes workspace";
            return result;
        }

        fs::remove_all(stage_pkg, ec);
        fs::create_directories(stage_pkg / "bin", ec);
        fs::create_directories(stage_pkg / "lib", ec);
        fs::create_directories(stage_pkg / "include", ec);

        // 5. Gather installed targets to package
        std::vector<const Target *> targets_to_pack;
        if (!config.artifacts.empty()) {
            for (const auto &aname : config.artifacts) {
                const Target *t = builder.find_target(aname);
                if (t)
                    targets_to_pack.push_back(t);
            }
        } else if (!builder.installed_artifacts().empty()) {
            for (const auto &aname : builder.installed_artifacts()) {
                const Target *t = builder.find_target(aname);
                if (t)
                    targets_to_pack.push_back(t);
            }
        } else {
            for (const auto &t : builder.targets()) {
                targets_to_pack.push_back(t.get());
            }
        }

        // Copy built target binaries
        for (const Target *t : targets_to_pack) {
            fs::path art_path = builder.get_artifact_path(*t, toolchain);
            if (fs::exists(art_path, ec)) {
                if (t->type() == TargetType::Executable || t->type() == TargetType::SharedLibrary ||
                    t->type() == TargetType::Plugin) {
                    fs::copy_file(art_path, stage_pkg / "bin" / art_path.filename(),
                                  fs::copy_options::overwrite_existing, ec);
                    if (t->type() == TargetType::SharedLibrary) {
                        fs::path implib = builder.get_import_library_path(*t, toolchain);
                        if (fs::exists(implib, ec)) {
                            fs::copy_file(implib, stage_pkg / "lib" / implib.filename(),
                                          fs::copy_options::overwrite_existing, ec);
                        }
                    }
                } else if (t->type() == TargetType::StaticLibrary) {
                    fs::copy_file(art_path, stage_pkg / "lib" / art_path.filename(),
                                  fs::copy_options::overwrite_existing, ec);
                }
            }
        }

        // Copy public include directories
        std::vector<std::string> inc_dirs = config.include_dirs;
        if (inc_dirs.empty()) {
            if (fs::exists("include", ec)) {
                inc_dirs.push_back("include");
            }
        }
        for (const auto &inc : inc_dirs) {
            fs::path p(inc);
            if (fs::exists(p, ec) && fs::is_directory(p, ec)) {
                fs::copy(p, stage_pkg / "include", fs::copy_options::recursive | fs::copy_options::overwrite_existing,
                         ec);
            }
        }

        // Copy extra files (README, LICENSE, etc.)
        std::vector<std::string> extras = config.extra_files;
        if (extras.empty()) {
            const char *candidates[] = {"README.md", "README", "LICENSE", "LICENSE.txt", "LICENSE.md", "NOTICE"};
            for (const char *cand : candidates) {
                if (fs::exists(cand, ec)) {
                    extras.push_back(cand);
                }
            }
        }
        for (const auto &ef : extras) {
            fs::path p(ef);
            if (fs::exists(p, ec)) {
                fs::copy_file(p, stage_pkg / p.filename(), fs::copy_options::overwrite_existing, ec);
            }
        }

        // Export CMake package configuration into stage
        CMakeConfigOptions copt;
        copt.package_name = pkg_name;
        copt.version = version;
        copt.output_dir = stage_pkg / "lib" / "cmake" / pkg_name;
        CMakeConfigExporter::export_config(builder, copt);

        // 6. Clean target archive if exists
        fs::remove(final_archive, ec);

        // 7. Compress stage directory into archive
        bool compress_success = false;
        if (fmt == "zip") {
            ProcessResult pres = ProcessRunner::run({"tar", "-a", "-c", "-f", final_archive.string(), "-C",
                                                     stage_base.string(), stage_pkg.filename().string()});
            if (pres.success() && fs::exists(final_archive, ec) && fs::file_size(final_archive, ec) > 0) {
                compress_success = true;
            } else {
#if defined(_WIN32)
                ProcessOptions ps_opts;
                ps_opts.command = {"powershell", "-NoProfile", "-NonInteractive", "-Command",
                                   "Compress-Archive -Path ($env:CXXGEN_STAGE_PATH + '/*') -DestinationPath "
                                   "$env:CXXGEN_ARCHIVE_PATH -Force"};
                ps_opts.extra_env["CXXGEN_STAGE_PATH"] = stage_pkg.string();
                ps_opts.extra_env["CXXGEN_ARCHIVE_PATH"] = final_archive.string();
                ProcessResult ps_res = ProcessRunner::run(ps_opts);
                if (ps_res.success() && fs::exists(final_archive, ec)) {
                    compress_success = true;
                } else {
                    result.error_message = "Failed to create zip archive: " + ps_res.stderr_output;
                }
#else
                ProcessResult zres = ProcessRunner::run(
                    {"zip", "-r", final_archive.string(), stage_pkg.filename().string()}, stage_base);
                if (zres.success() && fs::exists(final_archive, ec)) {
                    compress_success = true;
                } else {
                    result.error_message = "Failed to create zip archive: " + zres.stderr_output;
                }
#endif
            }
        } else {
            // tar.gz
            ProcessResult pres = ProcessRunner::run(
                {"tar", "-czf", final_archive.string(), "-C", stage_base.string(), stage_pkg.filename().string()});
            if (pres.success() && fs::exists(final_archive, ec)) {
                compress_success = true;
            } else {
                result.error_message = "Failed to create tar.gz archive: " + pres.stderr_output;
            }
        }

        // Clean stage pkg dir
        fs::remove_all(stage_pkg, ec);

        if (!compress_success || !fs::exists(final_archive, ec)) {
            result.success = false;
            if (result.error_message.empty()) {
                result.error_message = "Failed to produce distribution archive";
            }
            return result;
        }

        result.success = true;
        result.archive_path = final_archive;
        result.archive_size = fs::file_size(final_archive, ec);
        return result;
    }
};

inline bool Builder::export_cmake_config(const CMakeConfigOptions &options, std::string *error) const {
    return CMakeConfigExporter::export_config(*this, options, error);
}

// ============================================================================
// 9. CLI Runner & Standalone Entrypoint (CXXGEN_MAIN)
// ============================================================================

struct CliArgs {
    std::string command = "build";
    std::string target_or_step = "";
    std::optional<Optimization> optimize = std::nullopt;
    std::optional<CompilerFamily> compiler = std::nullopt;
    uint32_t jobs = 0;
    bool verbose = false;
    bool show_help = false;
    bool show_version = false;
    std::vector<std::string> passthrough_args = {};
    std::string generator_format = "";
    std::string init_type = "exe";
    bool force = false;
    bool no_color = false;
    bool check_only = false;
    std::vector<std::string> fmt_paths = {};
    std::string toolchain_action = "list";
    std::filesystem::path work_dir = "";
    std::string graph_format = "ascii";
    std::string graph_target = "";
    bool profile = false;
    std::filesystem::path profile_output = "build_trace.json";
    Sanitizer sanitizers = Sanitizer::None;
    bool coverage = false;
    std::vector<std::string> lint_paths = {};
    bool fix = false;
    std::string coverage_export = "";
    std::vector<std::string> import_args = {};
    std::filesystem::path import_output = "build.cpp";
    std::string import_format = "";
    bool dry_run = false;
    std::vector<std::string> check_headers_paths = {};
    bool locked = false;
    std::string package_format = "auto";
    std::filesystem::path package_output = "";
    std::vector<std::string> update_pkgs = {};
    std::string target_triple = "";
    std::filesystem::path sysroot = "";
    bool global_cache = true;
    std::string cache_action = "stats";
    std::string cache_param = "";
    bool enable_std_module = false;
};

inline int GlobalObjectCache::run_cli_cache(const CliArgs &args) {
    std::string action = args.cache_action;
    if (action.empty() || action == "stats") {
        auto stats = get_stats();
        std::cout << "[cxxgen] Global Object Cache Statistics:\n";
        std::cout << "  Cache Directory:    " << get_cache_dir().string() << "\n";
        std::cout << "  Total Objects:      " << stats.total_files << "\n";
        double pct_used =
            (stats.max_bytes > 0) ? (static_cast<double>(stats.total_bytes) / stats.max_bytes * 100.0) : 0.0;
        std::cout << "  Total Cache Size:   " << format_bytes(stats.total_bytes) << " / "
                  << format_bytes(stats.max_bytes) << " (" << std::fixed << std::setprecision(1) << pct_used
                  << "% used)\n";
        uint64_t total_requests = stats.hits + stats.misses;
        double hit_rate = (total_requests > 0) ? (static_cast<double>(stats.hits) / total_requests * 100.0) : 0.0;
        std::cout << "  Hits:               " << stats.hits << " (" << std::fixed << std::setprecision(1) << hit_rate
                  << "%)\n";
        std::cout << "  Direct Hits:        " << stats.direct_hits << "\n";
        std::cout << "  Misses:             " << stats.misses << "\n";
        std::cout << "  Files Stored:       " << stats.files_stored << "\n";
        std::cout << "  Reclaimed:          " << format_bytes(stats.reclaimed_bytes) << "\n";
        return 0;
    } else if (action == "clean") {
        auto res = clean();
        std::cout << "[cxxgen] Global Object Cache Clean:\n";
        std::cout << "  Cleaned " << res.files_removed << " file(s).\n";
        std::cout << "  Reclaimed " << format_bytes(res.bytes_reclaimed) << " of storage.\n";
        return 0;
    } else if (action == "max-size") {
        if (args.cache_param.empty()) {
            std::cout << "Current max cache size: " << format_bytes(get_max_size()) << "\n";
            return 0;
        }
        uint64_t new_size = parse_size(args.cache_param);
        if (new_size == 0) {
            std::cerr << "cxxgen error: invalid max cache size '" << args.cache_param
                      << "'. Example: '5G' or '500M'.\n";
            return 1;
        }
        uint64_t old_size = get_max_size();
        set_max_size(new_size);
        std::cout << "[cxxgen] Global Object Cache Configuration:\n";
        std::cout << "  Maximum cache size updated: " << format_bytes(new_size) << " (previously "
                  << format_bytes(old_size) << ").\n";
        uint64_t pruned = prune_to_max_size();
        if (pruned > 0) {
            std::cout << "  Pruned oldest objects, reclaimed " << format_bytes(pruned) << ".\n";
        }
        return 0;
    } else {
        std::cerr << "cxxgen error: unknown cache action '" << action << "'. Valid actions: stats, clean, max-size.\n";
        return 1;
    }
}

inline void print_help(std::ostream &os = std::cout) {
    os << "cxxgen (C++ Generator & Build System) build runner\n"
       << "Usage: cxxgen [command] [options] [-- <args>]\n\n"
       << "Commands:\n"
       << "  build [target/step]   Build artifacts (default: install step)\n"
       << "  run [target]          Build and run executable (passes arguments after --)\n"
       << "  test [target]         Build and execute unit tests\n"
       << "  package, pack         Bundle binaries, libraries, and headers into distributable archive\n"
       << "  update [packages...]  Update pinned dependency versions in cxxgen.lock\n"
       << "  clean                 Clean build artifacts and cache\n"
       << "  cache [action] [arg]  Manage global object cache (stats, clean, max-size <size>)\n"
       << "  generate <format>     Generate project files (cmake, msvc, ninja, compile_commands, cmake-config)\n"
       << "  import [format] [path] Import CMakeLists.txt or MSVC solution/project to build.cpp\n"
       << "  init [--type=...]     Scaffold new project (exe, lib, plugin)\n"
       << "  fmt [paths...]        Format source files using clang-format (like zig fmt)\n"
       << "  lint [paths...]       Run clang-tidy static analysis across translation units\n"
       << "  check-headers [paths] Compile headers in isolation to verify self-containment\n"
       << "  watch                 Watch project directories and rebuild on change\n"
       << "  toolchain [action]    Inspect available toolchains (list, detect)\n"
       << "  doctor                Inspect developer environment and installed toolchains\n"
       << "  graph                 Visualize target and step dependency graph (ascii, mermaid, dot)\n"
       << "  help, --help, -h      Display this help message\n"
       << "  version, --version    Display version information\n\n"
       << "Options:\n"
       << "  -O, --optimize <mode> Optimization mode (Debug, ReleaseFast, ReleaseSafe, ReleaseSmall)\n"
       << "  --compiler <name>     Compiler family (auto, msvc, clang, gcc, emscripten)\n"
       << "  --target <triple>     Cross-compilation target triple (e.g. wasm32, aarch64-linux-gnu, arm-none-eabi)\n"
       << "  --sysroot <path>      Sysroot path for cross-compilation headers and libraries\n"
       << "  --global-cache, --cache Enable global cross-project object cache (default: on)\n"
       << "  --no-cache, --no-global-cache Disable global cross-project compilation cache\n"
       << "  --std-module          Enable C++23 standard library module (import std;) support\n"
       << "  -j, --jobs <N>        Number of concurrent jobs (default: hardware concurrency)\n"
       << "  -v, --verbose         Enable verbose diagnostic logging\n"
       << "  --locked, --frozen    Strictly enforce cxxgen.lock consistency (fail if lockfile would change)\n"
       << "  --sanitize=<list>     Enable sanitizers (address, undefined, leak, thread)\n"
       << "  --coverage            Enable code coverage profiling and instrumentation\n"
       << "  --fix                 Automatically apply fixes (for lint)\n"
       << "  --profile[=<file>]    Record build trace JSON for chrome://tracing or Perfetto\n"
       << "  --no-color            Disable ANSI color output (also respects NO_COLOR env)\n"
       << "  --check               Check formatting without modifying files (for fmt)\n"
       << "  --type <type>         Project scaffolding type for init (exe, lib, plugin)\n"
       << "  --force               Force overwrite existing files (init, import)\n"
       << "  --dry-run             Preview generated build.cpp without writing to disk\n"
       << "  --out <path>          Output file path for import or package (default: build.cpp or dist/)\n"
       << "  --format <fmt>        Format for graph, import, or package (zip, tar.gz)\n"
       << "  -C, --work-dir <path> Switch to specified working directory\n";
}

inline void print_version(std::ostream &os = std::cout) {
    os << "cxxgen version 1.0.0 (C++20 / C++23)\n";
}

inline CliArgs parse_cli_args(int argc, char **argv) {
    CliArgs args;
    bool command_set = false;
    bool passthrough_mode = false;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);

        if (passthrough_mode) {
            args.passthrough_args.emplace_back(arg);
            continue;
        }

        if (arg == "--") {
            passthrough_mode = true;
            continue;
        }

        if (arg == "-h" || arg == "--help" || arg == "help") {
            args.show_help = true;
            return args;
        }

        if (arg == "-V" || arg == "--version" || arg == "version") {
            args.show_version = true;
            return args;
        }

        if (arg == "-v" || arg == "--verbose") {
            args.verbose = true;
            continue;
        }

        if (arg == "--no-color") {
            args.no_color = true;
            continue;
        }

        if (arg == "--global-cache" || arg == "--cache") {
            args.global_cache = true;
            continue;
        }

        if (arg == "--no-cache" || arg == "--no-global-cache") {
            args.global_cache = false;
            continue;
        }

        if (arg == "--std-module") {
            args.enable_std_module = true;
            continue;
        }

        if (arg == "--check") {
            args.check_only = true;
            continue;
        }

        if (arg == "--force") {
            args.force = true;
            continue;
        }

        if (arg.rfind("--type=", 0) == 0) {
            args.init_type = std::string(arg.substr(7));
            continue;
        }

        if (arg == "--type" && i + 1 < argc) {
            args.init_type = argv[++i];
            continue;
        }

        if (arg.rfind("--work-dir=", 0) == 0) {
            std::filesystem::path raw_p(arg.substr(11));
            std::error_code ec;
            args.work_dir = std::filesystem::absolute(raw_p, ec);
            if (ec)
                args.work_dir = raw_p;
            continue;
        }

        if ((arg == "--work-dir" || arg == "-C") && i + 1 < argc) {
            std::filesystem::path raw_p(argv[++i]);
            std::error_code ec;
            args.work_dir = std::filesystem::absolute(raw_p, ec);
            if (ec)
                args.work_dir = raw_p;
            continue;
        }

        if (arg.rfind("-O", 0) == 0) {
            std::string_view opt_str;
            if (arg.size() > 2 && arg[2] == '=') {
                opt_str = arg.substr(3);
            } else if (arg.size() > 2) {
                opt_str = arg.substr(2);
            } else if (i + 1 < argc) {
                opt_str = argv[++i];
            }
            if (!opt_str.empty()) {
                args.optimize = parse_optimization(opt_str);
            }
            continue;
        }

        if (arg.rfind("--optimize=", 0) == 0) {
            args.optimize = parse_optimization(arg.substr(11));
            continue;
        }

        if (arg == "--optimize" && i + 1 < argc) {
            args.optimize = parse_optimization(argv[++i]);
            continue;
        }

        if (arg.rfind("--compiler=", 0) == 0) {
            args.compiler = parse_compiler_family(arg.substr(11));
            continue;
        }

        if (arg == "--compiler" && i + 1 < argc) {
            args.compiler = parse_compiler_family(argv[++i]);
            continue;
        }

        if (arg.rfind("-j", 0) == 0) {
            std::string_view j_str;
            if (arg.size() > 2 && arg[2] == '=') {
                j_str = arg.substr(3);
            } else if (arg.size() > 2) {
                j_str = arg.substr(2);
            } else if (i + 1 < argc) {
                j_str = argv[++i];
            }
            if (!j_str.empty()) {
                try {
                    args.jobs = static_cast<uint32_t>(std::stoul(std::string(j_str)));
                } catch (...) {}
            }
            continue;
        }

        if (arg.rfind("--jobs=", 0) == 0) {
            try {
                args.jobs = static_cast<uint32_t>(std::stoul(std::string(arg.substr(7))));
            } catch (...) {}
            continue;
        }

        if (arg == "--jobs" && i + 1 < argc) {
            try {
                args.jobs = static_cast<uint32_t>(std::stoul(argv[++i]));
            } catch (...) {}
            continue;
        }

        if (arg == "--locked" || arg == "--frozen") {
            args.locked = true;
            continue;
        }

        if (arg == "--dry-run") {
            args.dry_run = true;
            continue;
        }

        if (arg.rfind("--out=", 0) == 0) {
            args.import_output = std::string(arg.substr(6));
            args.package_output = std::string(arg.substr(6));
            continue;
        }

        if (arg == "--out" && i + 1 < argc) {
            args.import_output = argv[++i];
            args.package_output = args.import_output;
            continue;
        }

        if (arg.rfind("--format=", 0) == 0) {
            args.graph_format = std::string(arg.substr(9));
            args.import_format = std::string(arg.substr(9));
            args.package_format = std::string(arg.substr(9));
            continue;
        }

        if (arg == "--format" && i + 1 < argc) {
            args.graph_format = argv[++i];
            args.import_format = args.graph_format;
            args.package_format = args.graph_format;
            continue;
        }

        if (arg.rfind("--sysroot=", 0) == 0) {
            args.sysroot = std::string(arg.substr(10));
            continue;
        }

        if (arg == "--sysroot" && i + 1 < argc) {
            args.sysroot = argv[++i];
            continue;
        }

        if (arg.rfind("--target-triple=", 0) == 0) {
            args.target_triple = std::string(arg.substr(16));
            continue;
        }

        if (arg == "--target-triple" && i + 1 < argc) {
            args.target_triple = argv[++i];
            continue;
        }

        if (arg.rfind("--triple=", 0) == 0) {
            args.target_triple = std::string(arg.substr(9));
            continue;
        }

        if (arg == "--triple" && i + 1 < argc) {
            args.target_triple = argv[++i];
            continue;
        }

        if (arg.rfind("--target=", 0) == 0) {
            std::string val = std::string(arg.substr(9));
            if (args.command == "graph" && val.find('-') == std::string::npos && val != "wasm32" && val != "wasm64") {
                args.graph_target = val;
            } else {
                args.target_triple = val;
                args.graph_target = val;
            }
            continue;
        }

        if (arg == "--target" && i + 1 < argc) {
            std::string val = argv[++i];
            if (args.command == "graph" && val.find('-') == std::string::npos && val != "wasm32" && val != "wasm64") {
                args.graph_target = val;
            } else {
                args.target_triple = val;
                args.graph_target = val;
            }
            continue;
        }

        if (arg.rfind("--profile=", 0) == 0) {
            args.profile = true;
            args.profile_output = std::string(arg.substr(10));
            continue;
        }

        if (arg == "--profile") {
            args.profile = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                args.profile_output = argv[++i];
            }
            continue;
        }

        if (arg.rfind("--sanitize=", 0) == 0) {
            args.sanitizers = args.sanitizers | parse_sanitizers(arg.substr(11));
            continue;
        }

        if (arg == "--sanitize" && i + 1 < argc) {
            args.sanitizers = args.sanitizers | parse_sanitizers(argv[++i]);
            continue;
        }

        if (arg == "--coverage") {
            args.coverage = true;
            continue;
        }

        if (arg == "--fix") {
            args.fix = true;
            continue;
        }

        if (arg.rfind("--export=", 0) == 0) {
            args.coverage_export = std::string(arg.substr(9));
            continue;
        }

        if (arg == "--export" && i + 1 < argc) {
            args.coverage_export = argv[++i];
            continue;
        }

        if (!command_set) {
            if (arg == "build" || arg == "run" || arg == "test" || arg == "clean" || arg == "generate" ||
                arg == "init" || arg == "fmt" || arg == "lint" || arg == "watch" || arg == "toolchain" ||
                arg == "doctor" || arg == "graph" || arg == "dump-manifest" || arg == "import" ||
                arg == "check-headers" || arg == "check_headers" || arg == "package" || arg == "pack" ||
                arg == "update" || arg == "cache") {
                args.command =
                    (arg == "check_headers") ? "check-headers" : (arg == "pack" ? "package" : std::string(arg));
                command_set = true;
                continue;
            }
        }

        // Positional argument
        if (args.command == "fmt") {
            args.fmt_paths.emplace_back(arg);
        } else if (args.command == "lint") {
            args.lint_paths.emplace_back(arg);
        } else if (args.command == "cache") {
            if (args.cache_action == "stats") {
                args.cache_action = std::string(arg);
            } else if (args.cache_param.empty()) {
                args.cache_param = std::string(arg);
            }
        } else if (args.command == "import") {
            args.import_args.emplace_back(arg);
        } else if (args.command == "check-headers") {
            args.check_headers_paths.emplace_back(arg);
        } else if (args.command == "update") {
            args.update_pkgs.emplace_back(arg);
        } else if (args.command == "package") {
            if (args.target_or_step.empty() && arg[0] != '-') {
                args.target_or_step = std::string(arg);
            }
        } else if (args.command == "generate" && args.generator_format.empty()) {
            args.generator_format = std::string(arg);
        } else if (args.command == "toolchain") {
            args.toolchain_action = std::string(arg);
        } else if (args.target_or_step.empty()) {
            args.target_or_step = std::string(arg);
        } else {
            args.passthrough_args.emplace_back(arg);
        }
    }

    return args;
}

template<typename BuildFn>
inline int run_cli(int argc, char **argv, BuildFn &&build_fn) {
    terminal::init_console();
    try {
        CliArgs args = parse_cli_args(argc, argv);

        if (args.show_help) {
            print_help(std::cout);
            return 0;
        }

        if (args.show_version) {
            print_version(std::cout);
            return 0;
        }

        GlobalObjectCache::instance().set_enabled(args.global_cache);

        if (args.command == "cache") {
            return GlobalObjectCache::instance().run_cli_cache(args);
        }

        if (!args.work_dir.empty()) {
            std::error_code ec;
            std::filesystem::current_path(args.work_dir, ec);
            if (ec) {
                std::cerr << "cxxgen error: failed to switch working directory to '" << args.work_dir.string()
                          << "': " << ec.message() << "\n";
                return 1;
            }
        }

        if (args.command == "toolchain") {
            std::cout << "[cxxgen] Toolchain Discovery:\n";
            auto clang = find_clang_toolchain();
            auto gcc = find_gcc_toolchain();
            auto msvc = find_msvc_toolchain();
            auto em = find_emscripten_toolchain();
            std::cout << "  Clang:      " << (clang ? clang->name() + " -> " + clang->compiler_path() : "Not detected")
                      << "\n";
            std::cout << "  GCC:        " << (gcc ? gcc->name() + " -> " + gcc->compiler_path() : "Not detected")
                      << "\n";
            std::cout << "  MSVC:       " << (msvc ? msvc->name() + " -> " + msvc->compiler_path() : "Not detected")
                      << "\n";
            std::cout << "  Emscripten: " << (em ? em->name() + " -> " + em->compiler_path() : "Not detected") << "\n";
            TargetOptions topt;
            topt.triple = args.target_triple;
            topt.sysroot = args.sysroot;
            auto def = find_toolchain(args.compiler.value_or(CompilerFamily::Auto), topt);
            if (def) {
                std::cout << "[cxxgen] Selected toolchain: " << def->name() << "\n";
            } else {
                std::cout << "[cxxgen] No compatible toolchain detected.\n";
            }
            return 0;
        }

        if (args.command == "init") {
            ProjectScaffolderConfig scfg;
            scfg.target_dir = args.work_dir.empty() ? std::filesystem::current_path() : args.work_dir;
            scfg.type = args.init_type;
            scfg.force = args.force;
            scfg.verbose = args.verbose;
            auto sres = ProjectScaffolder::scaffold(scfg);
            if (!sres.success) {
                std::cerr << sres.error_message << "\n";
                return 1;
            }
            std::cout << "[cxxgen] Initialized new " << args.init_type << " project in " << scfg.target_dir.string()
                      << "\n";
            return 0;
        }

        if (args.command == "fmt") {
            auto fmt_binary = find_clang_format();
            if (!fmt_binary) {
                std::cerr << "cxxgen error: 'clang-format' executable not found on PATH or alongside compiler.\n";
                std::cerr << "Please install clang-format (e.g. via LLVM or Scoop: 'scoop install llvm').\n";
                return 1;
            }

            FormatConfig fcfg;
            fcfg.paths = args.fmt_paths.empty()
                             ? std::vector<std::filesystem::path>{"."}
                             : std::vector<std::filesystem::path>(args.fmt_paths.begin(), args.fmt_paths.end());
            fcfg.check = args.check_only;
            fcfg.verbose = args.verbose;

            auto fres = CodeFormatter::format_all(*fmt_binary, fcfg);
            if (!fres.success) {
                std::cerr << fres.error_message << "\n";
                return 1;
            }
            if (args.check_only) {
                if (fres.mismatched_files.empty()) {
                    std::cout << "[cxxgen fmt] All " << fres.files_scanned << " file(s) are properly formatted.\n";
                    return 0;
                } else {
                    std::cerr << "[cxxgen fmt] " << fres.mismatched_files.size() << " file(s) need formatting:\n";
                    for (const auto &f : fres.mismatched_files) {
                        std::cerr << "  - " << f << "\n";
                    }
                    return 1;
                }
            } else {
                std::cout << "[cxxgen fmt] Formatted " << fres.files_formatted << " file(s) (" << fres.files_scanned
                          << " scanned).\n";
                return 0;
            }
        }

        if (args.command == "import") {
            ImportOptions iopts;
            iopts.output_path = args.import_output.empty() ? "build.cpp" : args.import_output;
            iopts.force = args.force;
            iopts.dry_run = args.dry_run;
            iopts.verbose = args.verbose;
            iopts.format = args.import_format;

            std::filesystem::path target_path = ".";
            if (!args.import_args.empty()) {
                std::string first = args.import_args[0];
                if (first == "cmake" || first == "msvc") {
                    iopts.format = first;
                    if (args.import_args.size() > 1) {
                        target_path = args.import_args[1];
                    }
                } else {
                    target_path = first;
                }
            }

            auto ires = ProjectImporter::import_project(target_path, iopts);
            if (!ires.success) {
                std::cerr << ires.error_message << "\n";
                return 1;
            }
            if (iopts.dry_run) {
                std::cout << ires.generated_code;
            } else {
                std::cout << "[cxxgen import] Successfully generated " << iopts.output_path.string() << " from "
                          << ires.detected_format << " (" << ires.targets_imported << " target(s) imported).\n";
            }
            return 0;
        }

        if (args.command == "doctor") {
            return SystemDoctor::run_doctor(args.verbose);
        }

        if (args.command == "lint") {
            auto tidy_binary = find_clang_tidy();
            if (!tidy_binary) {
                std::cerr << "cxxgen error: 'clang-tidy' executable not found on PATH or alongside compiler.\n";
                std::cerr << "Please install clang-tidy (e.g. via LLVM or Scoop: 'scoop install llvm').\n";
                return 1;
            }

            Builder b;
            if (!args.target_triple.empty()) {
                b.set_target_triple(args.target_triple);
            }
            if (!args.sysroot.empty()) {
                b.set_sysroot(args.sysroot);
            }
            if (args.optimize.has_value()) {
                b.set_standard_optimize_options({.mode = *args.optimize});
            }
            build_fn(b);

            std::shared_ptr<IToolchain> toolchain =
                find_toolchain(args.compiler.value_or(CompilerFamily::Auto), b.standard_target_options());
            CompileCommandsGenerator cc_gen(b, toolchain.get());
            cc_gen.generate_file();

            LintConfig lcfg;
            if (args.lint_paths.empty()) {
                for (const auto &t : b.targets()) {
                    if (t->type() == TargetType::Interface)
                        continue;
                    for (const auto &s : t->config().sources) {
                        lcfg.paths.push_back(s);
                    }
                }
                if (lcfg.paths.empty()) {
                    lcfg.paths.push_back(".");
                }
            } else {
                for (const auto &p : args.lint_paths) {
                    lcfg.paths.emplace_back(p);
                }
            }
            lcfg.fix = args.fix;
            lcfg.verbose = args.verbose;
            lcfg.build_path = ".";

            auto lres = CodeLinter::lint_all(*tidy_binary, lcfg);
            if (!lres.report.empty()) {
                std::cout << lres.report;
                if (lres.report.back() != '\n')
                    std::cout << "\n";
            }
            if (!lres.success && !lres.error_message.empty()) {
                std::cerr << lres.error_message << "\n";
                return 1;
            }
            std::cout << "[cxxgen lint] Scanned " << lres.files_scanned << " file(s): " << lres.warnings_count
                      << " warning(s), " << lres.errors_count << " error(s).\n";
            return (lres.errors_count > 0) ? 1 : 0;
        }

        if (args.command == "check-headers") {
            Builder b;
            if (!args.target_triple.empty()) {
                b.set_target_triple(args.target_triple);
            }
            if (!args.sysroot.empty()) {
                b.set_sysroot(args.sysroot);
            }
            if (args.optimize.has_value()) {
                b.set_standard_optimize_options({.mode = *args.optimize});
            }
            build_fn(b);

            auto tc = find_toolchain(args.compiler.value_or(CompilerFamily::Auto), b.standard_target_options());
            if (!tc) {
                std::cerr << "cxxgen error: no compatible toolchain found on system\n";
                return 1;
            }

            HeaderCheckConfig hcfg;
            hcfg.verbose = args.verbose;
            hcfg.jobs = args.jobs;
            for (const auto &p : args.check_headers_paths) {
                hcfg.paths.emplace_back(p);
            }
            for (const auto &t : b.targets()) {
                for (const auto &inc : t->config().include_dirs) {
                    hcfg.include_dirs.emplace_back(inc);
                }
                for (const auto &def : t->config().defines) {
                    hcfg.defines.push_back(def);
                }
            }
            auto hres = HeaderChecker::check_all(*tc, hcfg);
            if (!hres.report.empty()) {
                std::cout << hres.report;
                if (hres.report.back() != '\n')
                    std::cout << "\n";
            }
            return hres.success ? 0 : 1;
        }

        Builder b;
        b.set_working_directory(args.work_dir.empty() ? std::filesystem::current_path() : args.work_dir);
        b.set_locked(args.locked);
        b.set_update_packages(args.update_pkgs);

        if (!args.target_triple.empty()) {
            b.set_target_triple(args.target_triple);
        }
        if (!args.sysroot.empty()) {
            b.set_sysroot(args.sysroot);
        }
        if (args.optimize.has_value()) {
            b.set_standard_optimize_options({.mode = *args.optimize});
        }
        if (args.sanitizers != Sanitizer::None) {
            b.set_sanitizers(args.sanitizers);
        }
        if (args.coverage) {
            b.set_coverage(true);
        }

        // Execute user build function to configure build graph
        build_fn(b);

        if (args.command == "update") {
            std::cout << "[cxxgen update] Updated dependency lockfile (cxxgen.lock).\n";
            return 0;
        }

        if (args.sanitizers != Sanitizer::None) {
            for (auto &t : b.targets()) {
                t->sanitize(args.sanitizers);
            }
        }
        if (args.coverage) {
            for (auto &t : b.targets()) {
                t->coverage(true);
            }
        }

        if (args.command == "dump-manifest") {
            std::filesystem::path out_file =
                args.target_or_step.empty() ? ".cxxgen/manifest.json" : args.target_or_step;
            if (!b.dump_manifest_to_file(out_file)) {
                std::cerr << "cxxgen error: failed to dump manifest to " << out_file.string() << "\n";
                return 1;
            }
            return 0;
        }

        if (args.command == "graph") {
            std::string target_filter = args.graph_target.empty() ? args.target_or_step : args.graph_target;
            return BuildGraphVisualizer::run(b, args.graph_format, target_filter, std::cout);
        }

        if (args.command == "clean") {
            namespace fs = std::filesystem;
            std::error_code ec;
            fs::path cur_dir = fs::current_path(ec);
            if (util::is_system_protected_dir(cur_dir)) {
                std::cerr << "cxxgen security error: refusing to run clean in system protected directory '"
                          << cur_dir.string() << "'\n";
                return 1;
            }

            std::cout << "[cxxgen] Clean command received. Cleaning build artifacts...\n";
            fs::path cxxgen_dir = cur_dir / ".cxxgen";
            if (fs::is_symlink(cxxgen_dir, ec)) {
                fs::remove(cxxgen_dir, ec);
            } else if (fs::exists(cxxgen_dir, ec)) {
                fs::remove_all(cxxgen_dir, ec);
            }

            fs::path bin_dir = cur_dir / "bin";
            if (fs::is_symlink(bin_dir, ec)) {
                fs::remove(bin_dir, ec);
            } else if (fs::exists(bin_dir, ec)) {
                fs::remove_all(bin_dir, ec);
            }
            fs::path obj_dir = cur_dir / "obj";
            if (fs::is_symlink(obj_dir, ec)) {
                fs::remove(obj_dir, ec);
            } else if (fs::exists(obj_dir, ec)) {
                fs::remove_all(obj_dir, ec);
            }
            BuildStateDatabase::instance().clear();
            std::cout << "[cxxgen] Clean completed.\n";
            return 0;
        }

        if (args.command == "generate") {
            if (args.generator_format.empty()) {
                std::cerr
                    << "cxxgen error: missing generator format (cmake, msvc, ninja, compile_commands, cmake-config)\n";
                return 1;
            }
            if (args.generator_format == "cmake") {
                CMakeGenerator gen(b);
                if (!gen.generate_file()) {
                    std::cerr << "cxxgen error: failed to generate CMakeLists.txt\n";
                    return 1;
                }
                std::cout << "[cxxgen] Successfully generated CMakeLists.txt\n";
                return 0;
            }
            if (args.generator_format == "cmake-config" || args.generator_format == "cmake_config") {
                CMakeConfigOptions copt;
                copt.package_name = b.project_name();
                std::string err;
                if (!b.export_cmake_config(copt, &err)) {
                    std::cerr << "cxxgen error: " << err << "\n";
                    return 1;
                }
                std::cout << "[cxxgen] Successfully generated CMake package configuration files in lib/cmake/"
                          << (copt.package_name.empty() ? "Project" : copt.package_name) << "\n";
                return 0;
            }
            if (args.generator_format == "msvc") {
                MsvcGenerator gen(b);
                if (!gen.generate_all()) {
                    std::cerr << "cxxgen error: failed to generate MSVC solution and project files\n";
                    return 1;
                }
                std::cout << "[cxxgen] Successfully generated MSVC solution and project files\n";
                return 0;
            }
            if (args.generator_format == "ninja") {
                NinjaGenerator gen(b);
                if (!gen.generate_file()) {
                    std::cerr << "cxxgen error: failed to generate build.ninja\n";
                    return 1;
                }
                std::cout << "[cxxgen] Successfully generated build.ninja\n";
                return 0;
            }
            if (args.generator_format == "compile_commands") {
                CompileCommandsGenerator gen(b);
                if (!gen.generate_file()) {
                    std::cerr << "cxxgen error: failed to generate compile_commands.json\n";
                    return 1;
                }
                std::cout << "[cxxgen] Successfully generated compile_commands.json\n";
                return 0;
            }
            std::cerr << "cxxgen error: unknown generator format '" << args.generator_format << "'\n";
            return 1;
        }

        std::shared_ptr<IToolchain> toolchain =
            find_toolchain(args.compiler.value_or(CompilerFamily::Auto), b.standard_target_options());
        if (!toolchain) {
            std::cerr << "cxxgen error: no compatible toolchain found on system\n";
            return 1;
        }

        if (args.no_color) {
            terminal::set_color_override(false);
        }

        bool use_color = !args.no_color && terminal::color_enabled();
        auto reporter = std::make_shared<terminal::ProgressReporter>(0, std::cout, use_color);

        ExecutionContext ctx{
            .toolchain = *toolchain,
            .verbose = args.verbose,
            .thread_count = args.jobs,
            .no_color = args.no_color,
            .coverage = args.coverage,
            .progress_reporter = reporter,
        };

        if (args.profile) {
            ctx.profiler = std::make_shared<BuildProfiler>();
        }

        if (args.command == "build") {
            // Automatically emit/refresh compile_commands.json on every build
            CompileCommandsGenerator cc_gen(b, toolchain.get());
            cc_gen.generate_file();

            StepResult res = StepResult::ok();
            if (!args.target_or_step.empty()) {
                res = b.execute_step(args.target_or_step, ctx);
            } else {
                res = b.execute_all(ctx);
            }
            if (!res.success()) {
                std::cerr << "cxxgen error: build failed"
                          << (args.target_or_step.empty() ? "" : " for step/target '" + args.target_or_step + "'")
                          << ": " << res.error_message << "\n";
                return res.exit_code != 0 ? res.exit_code : 1;
            }
            if (!args.verbose) {
                reporter->print_summary();
            }
            if (ctx.profiler) {
                ctx.profiler->write_trace_file(args.profile_output);
                ctx.profiler->print_summary(std::cout, use_color);
            }
            return 0;
        }

        if (args.command == "package" || args.command == "pack") {
            PackageConfig pcfg = b.package_config();
            if (!args.package_format.empty() && args.package_format != "auto") {
                pcfg.format = args.package_format;
            }
            if (!args.package_output.empty()) {
                pcfg.output_dir = args.package_output;
            }
            if (!args.target_or_step.empty() && pcfg.package_name.empty()) {
                pcfg.package_name = args.target_or_step;
            }

            // Build targets to pack
            std::vector<std::string> pack_targets = pcfg.artifacts;
            if (pack_targets.empty()) {
                pack_targets = b.installed_artifacts();
            }
            if (pack_targets.empty()) {
                for (const auto &t : b.targets()) {
                    pack_targets.push_back(t->name());
                }
            }
            for (const auto &tname : pack_targets) {
                if (const Target *t = b.find_target(tname)) {
                    BuildResult bres = b.build_target(*t, *toolchain, args.verbose);
                    if (!bres.success) {
                        std::cerr << "cxxgen error: build failed before packaging: " << bres.error_message << "\n";
                        return 1;
                    }
                }
            }

            auto pres = PackageEngine::package(b, *toolchain, pcfg, args.verbose);
            if (!pres.success) {
                std::cerr << "cxxgen error: packaging failed: " << pres.error_message << "\n";
                return 1;
            }

            std::cout << "[cxxgen package] Created distribution package: " << pres.archive_path.string() << " ("
                      << (pres.archive_size / 1024) << " KB)\n";
            return 0;
        }

        if (args.command == "run") {
            // Automatically emit/refresh compile_commands.json on run
            CompileCommandsGenerator cc_gen(b, toolchain.get());
            cc_gen.generate_file();

            Target *target_to_run = nullptr;
            if (!args.target_or_step.empty()) {
                target_to_run = b.find_target(args.target_or_step);
            } else {
                for (const auto &t : b.targets()) {
                    if (t->type() == TargetType::Executable) {
                        target_to_run = t.get();
                        break;
                    }
                }
            }
            if (!target_to_run) {
                std::cerr << "cxxgen error: no executable target found to run\n";
                return 1;
            }

            TargetBuildStep build_step(b, *target_to_run);
            StepResult build_res = build_step.execute(ctx);
            if (!build_res.success()) {
                std::cerr << "cxxgen error: build failed for target '" << target_to_run->name()
                          << "': " << build_res.error_message << "\n";
                return build_res.exit_code != 0 ? build_res.exit_code : 1;
            }

            RunArtifactStep run_step(*target_to_run, args.passthrough_args);
            StepResult run_res = run_step.execute(ctx);
            if (ctx.profiler) {
                ctx.profiler->write_trace_file(args.profile_output);
                ctx.profiler->print_summary(std::cout, use_color);
            }
            return run_res.exit_code;
        }

        if (args.command == "test") {
            size_t test_count = 0;
            size_t passed_count = 0;
            std::vector<std::string> coverage_binaries;

            for (const auto &t : b.targets()) {
                if (t->type() == TargetType::Test) {
                    if (!args.target_or_step.empty() && t->name() != args.target_or_step) {
                        continue;
                    }
                    ++test_count;
                    TargetBuildStep build_step(b, *t);
                    StepResult b_res = build_step.execute(ctx);
                    if (!b_res.success()) {
                        std::cerr << "cxxgen error: build failed for test target '" << t->name()
                                  << "': " << b_res.error_message << "\n";
                        return b_res.exit_code != 0 ? b_res.exit_code : 1;
                    }
                    RunArtifactStep run_step(*t, args.passthrough_args);
                    StepResult r_res = run_step.execute(ctx);
                    if (r_res.success()) {
                        ++passed_count;
                        if (ctx.coverage || t->has_coverage()) {
                            coverage_binaries.push_back(t->artifact_path(ctx.toolchain).string());
                        }
                    } else {
                        std::cerr << "cxxgen error: test target '" << t->name() << "' failed\n";
                        return r_res.exit_code != 0 ? r_res.exit_code : 1;
                    }
                }
            }
            if (test_count == 0) {
                std::cout << "[cxxgen] No test targets found to run.\n";
            } else {
                std::cout << "[cxxgen] All " << passed_count << " test target(s) passed successfully.\n";
            }
            if (ctx.profiler) {
                ctx.profiler->write_trace_file(args.profile_output);
                ctx.profiler->print_summary(std::cout, use_color);
            }
            if (!coverage_binaries.empty()) {
                std::cout << "\n================================================================================\n";
                std::cout << "cxxgen test - Code Coverage Report\n";
                std::cout << "================================================================================\n";
                for (const auto &bin : coverage_binaries) {
                    std::string target_name = std::filesystem::path(bin).stem().string();
                    CoverageConfig ccfg;
                    ccfg.binary_path = bin;
                    ccfg.profraw_file = ".cxxgen/coverage/" + target_name + ".profraw";
                    if (!std::filesystem::exists(ccfg.profraw_file)) {
                        ccfg.profraw_file = "default.profraw";
                    }
                    ccfg.profdata_file = ".cxxgen/coverage/" + target_name + ".profdata";
                    ccfg.export_format = args.coverage_export;
                    if (args.coverage_export == "html") {
                        ccfg.html_dir = ".cxxgen/coverage/html";
                    }
                    ccfg.verbose = args.verbose;
                    auto cov_res = CoverageReporter::generate_report(ccfg);
                    if (cov_res.success && !cov_res.report_output.empty()) {
                        std::cout << cov_res.report_output;
                        if (cov_res.report_output.back() != '\n')
                            std::cout << "\n";
                        if (!args.coverage_export.empty()) {
                            std::cout << "[cxxgen coverage] Exported coverage in '" << args.coverage_export
                                      << "' format.\n";
                        }
                    } else if (!cov_res.error_message.empty()) {
                        std::cerr << cov_res.error_message << "\n";
                    }
                }
            }
            return 0;
        }

        if (args.command == "watch") {
            std::cout << "[cxxgen watch] Initializing live file watcher mode...\n";
            std::cout << "[cxxgen watch] Running initial build...\n";
            auto init_res = b.execute_all(ctx);
            if (init_res.success()) {
                std::cout << "[cxxgen watch] Build succeeded.\n";
            } else {
                std::cerr << "[cxxgen watch] Build failed: " << init_res.error_message << "\n";
            }

            WatcherConfig wcfg;
            if (!args.work_dir.empty()) {
                wcfg.watch_directories = {args.work_dir / "src", args.work_dir / "include", args.work_dir};
            } else {
                wcfg.watch_directories = {"src", "include", "."};
            }
            wcfg.debounce = std::chrono::milliseconds(150);

            FileWatcher watcher(wcfg);
            watcher.start([&](const std::vector<FileChangeEvent> &events) {
                std::cout << "\n[cxxgen watch] Change detected (" << events.size() << " file(s)):\n";
                for (const auto &ev : events) {
                    const char *type_str = ev.event == WatchEvent::Created    ? "[+]"
                                           : ev.event == WatchEvent::Modified ? "[*]"
                                                                              : "[-]";
                    std::cout << "  " << type_str << " " << ev.path.string() << "\n";
                }
                std::cout << "[cxxgen watch] Rebuilding...\n";
                auto start_time = std::chrono::steady_clock::now();
                auto res = b.execute_all(ctx);
                auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                                                 start_time);
                if (res.success()) {
                    std::cout << "[cxxgen watch] Build succeeded (" << dur.count() << " ms).\n";
                } else {
                    std::cerr << "[cxxgen watch] Build failed (" << dur.count() << " ms): " << res.error_message
                              << "\n";
                }
            });

            std::cout << "[cxxgen watch] Watching for file changes (150ms debounce). Press Enter to exit...\n";
            std::cin.get();
            watcher.stop();
            std::cout << "[cxxgen watch] Watcher stopped.\n";
            return 0;
        }

        std::cerr << "cxxgen error: unknown command '" << args.command << "'\n";
        return 1;

    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}

} // namespace cxxgen

#define CXXGEN_MAIN(BuildFn)                                                                                           \
    int main(int argc, char **argv) {                                                                                  \
        return ::cxxgen::run_cli(argc, argv, BuildFn);                                                                 \
    }
