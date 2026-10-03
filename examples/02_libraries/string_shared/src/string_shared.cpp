#include "string_shared.hpp"

#include <algorithm>
#include <cctype>

namespace strutil {

std::string to_upper(std::string_view input) {
    std::string s(input);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return s;
}

std::string to_lower(std::string_view input) {
    std::string s(input);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string join(const std::vector<std::string> &items, std::string_view delimiter) {
    if (items.empty())
        return "";
    std::string result = items[0];
    for (size_t i = 1; i < items.size(); ++i) {
        result += delimiter;
        result += items[i];
    }
    return result;
}

} // namespace strutil
