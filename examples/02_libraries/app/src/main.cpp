#include "math_static.hpp"
#include "string_shared.hpp"

#include <iostream>
#include <vector>

int main() {
    std::cout << "=== Testing Static Library (math_static) ===\n";
    int64_t a = 12, b = 25;
    std::cout << a << " + " << b << " = " << math::add(a, b) << "\n";
    std::cout << a << " * " << b << " = " << math::multiply(a, b) << "\n";
    std::cout << "5! = " << math::factorial(5) << "\n\n";

    std::cout << "=== Testing Shared Library (string_shared) ===\n";
    std::string text = "Hello from dynamic cxxgen shared library!";
    std::cout << "Upper : " << strutil::to_upper(text) << "\n";
    std::cout << "Lower : " << strutil::to_lower(text) << "\n";

    std::vector<std::string> parts = {"cxxgen", "modern", "fast", "c++20"};
    std::cout << "Joined: " << strutil::join(parts, " -> ") << "\n";

    return 0;
}
