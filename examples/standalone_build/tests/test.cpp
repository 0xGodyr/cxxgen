#include "greeting.hpp"

#include <cassert>
#include <iostream>

int main() {
    assert(standalone::format_greeting("").find("World") != std::string::npos);
    assert(standalone::format_greeting("Alice") == "Hello, Alice from cxxgen!");
    std::cout << "[PASS] Standalone greeting tests passed!" << std::endl;
    return 0;
}
