#pragma once

#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#if defined(BUILDING_STRING_SHARED)
#define STRING_SHARED_API __declspec(dllexport)
#else
#define STRING_SHARED_API __declspec(dllimport)
#endif
#else
#define STRING_SHARED_API __attribute__((visibility("default")))
#endif

namespace strutil {

STRING_SHARED_API std::string to_upper(std::string_view input);
STRING_SHARED_API std::string to_lower(std::string_view input);
STRING_SHARED_API std::string join(const std::vector<std::string> &items, std::string_view delimiter);

} // namespace strutil
