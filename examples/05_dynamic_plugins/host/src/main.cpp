#include "plugin_interface.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

class DynamicPluginLoader {
public:
    explicit DynamicPluginLoader(const fs::path &path) : path_(path) {
#if defined(_WIN32)
        handle_ = LoadLibraryA(path.string().c_str());
#else
        handle_ = dlopen(path.string().c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
    }

    ~DynamicPluginLoader() {
        if (handle_) {
#if defined(_WIN32)
            FreeLibrary(static_cast<HMODULE>(handle_));
#else
            dlclose(handle_);
#endif
        }
    }

    [[nodiscard]] bool is_loaded() const noexcept { return handle_ != nullptr; }

    [[nodiscard]] plugins::IPlugin *get_plugin() const {
        if (!handle_)
            return nullptr;
#if defined(_WIN32)
        FARPROC proc = GetProcAddress(static_cast<HMODULE>(handle_), "create_plugin");
        if (!proc)
            return nullptr;
        union {
            FARPROC fp;
            plugins::CreatePluginFn fn;
        } caster;
        caster.fp = proc;
        return caster.fn();
#else
        void *sym = dlsym(handle_, "create_plugin");
        if (!sym)
            return nullptr;
        auto fn = reinterpret_cast<plugins::CreatePluginFn>(sym);
        return fn();
#endif
    }

    [[nodiscard]] const fs::path &path() const noexcept { return path_; }

private:
    fs::path path_;
    void *handle_{nullptr};
};

int main(int argc, char **argv) {
    std::cout << "========================================\n";
    std::cout << "      cxxgen Dynamic Plugin Host          \n";
    std::cout << "========================================\n";

    std::string sample_text = (argc > 1) ? argv[1] : "The Quick Brown Fox Jumps Over The Lazy Dog!";
    std::cout << "Input Text: \"" << sample_text << "\"\n\n";

    // Search common plugin output locations
    std::vector<fs::path> search_paths = {"bin/plugins", "bin", "lib", "."};

    std::vector<std::string> plugin_names = {"plugin_rot13", "plugin_reverse"};

    int loaded_count = 0;

    for (const auto &pname : plugin_names) {
        fs::path found_path;
        for (const auto &sp : search_paths) {
#if defined(_WIN32)
            fs::path candidate = sp / (pname + ".dll");
#elif defined(__APPLE__)
            fs::path candidate = sp / (pname + ".dylib");
#else
            fs::path candidate = sp / (pname + ".so");
#endif
            if (fs::exists(candidate)) {
                found_path = candidate;
                break;
            }
        }

        if (found_path.empty()) {
            std::cout << "[WARN] Could not find dynamic binary for: " << pname << "\n";
            continue;
        }

        std::cout << "Loading: " << found_path.string() << " ...\n";
        DynamicPluginLoader loader(found_path);
        if (!loader.is_loaded()) {
            std::cout << "  -> [FAILED] Could not load dynamic library.\n";
            continue;
        }

        plugins::IPlugin *plugin = loader.get_plugin();
        if (!plugin) {
            std::cout << "  -> [FAILED] Entrypoint 'create_plugin' not found or returned null.\n";
            continue;
        }

        std::cout << "  -> [OK] Loaded Plugin: \"" << plugin->name() << "\"\n";
        std::cout << "  -> [RESULT]: " << plugin->process(sample_text) << "\n\n";
        loaded_count++;
    }

    std::cout << "Summary: Successfully executed " << loaded_count << " dynamic plugin(s)!\n";
    return 0;
}
