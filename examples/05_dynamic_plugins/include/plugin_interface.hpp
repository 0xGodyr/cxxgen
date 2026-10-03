#pragma once

#include <string>
#include <string_view>

#if defined(_WIN32)
#if defined(BUILDING_PLUGIN)
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __declspec(dllimport)
#endif
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

namespace plugins {

class IPlugin {
public:
    virtual ~IPlugin() = default;
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual std::string process(std::string_view input) const = 0;
};

using CreatePluginFn = IPlugin *(*)();

} // namespace plugins
