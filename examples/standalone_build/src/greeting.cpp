#include "greeting.hpp"

namespace standalone {

std::string format_greeting(std::string_view name) {
    if (name.empty()) {
        return "Hello, World from cxxgen!";
    }
    return "Hello, " + std::string(name) + " from cxxgen!";
}

} // namespace standalone
