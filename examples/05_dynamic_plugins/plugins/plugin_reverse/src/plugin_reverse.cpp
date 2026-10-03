#include "plugin_interface.hpp"

#include <algorithm>
#include <string>

namespace plugins {

class ReversePlugin : public IPlugin {
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "String Reverser Plugin"; }

    [[nodiscard]] std::string process(std::string_view input) const override {
        std::string result(input);
        std::reverse(result.begin(), result.end());
        return result;
    }
};

static ReversePlugin s_instance;

} // namespace plugins

extern "C" PLUGIN_EXPORT plugins::IPlugin *create_plugin() {
    return &plugins::s_instance;
}
