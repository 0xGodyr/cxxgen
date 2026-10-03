#include "plugin_interface.hpp"

#include <cctype>
#include <string>

namespace plugins {

class Rot13Plugin : public IPlugin {
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "ROT13 Cipher Plugin"; }

    [[nodiscard]] std::string process(std::string_view input) const override {
        std::string result(input);
        for (char &c : result) {
            if (c >= 'a' && c <= 'z') {
                c = static_cast<char>('a' + (c - 'a' + 13) % 26);
            } else if (c >= 'A' && c <= 'Z') {
                c = static_cast<char>('A' + (c - 'A' + 13) % 26);
            }
        }
        return result;
    }
};

static Rot13Plugin s_instance;

} // namespace plugins

extern "C" PLUGIN_EXPORT plugins::IPlugin *create_plugin() {
    return &plugins::s_instance;
}
