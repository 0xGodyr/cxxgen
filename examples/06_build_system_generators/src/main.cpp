#include "core_math.hpp"

#include <iostream>

int main() {
    std::cout << "=== Foreign Generator Compatibility Demo ===\n";
    std::cout << "Fibonacci sequence:\n";
    for (uint32_t i = 0; i <= 10; ++i) {
        std::cout << "  fib(" << i << ") = " << core::fibonacci(i)
                  << (core::is_even(core::fibonacci(i)) ? " [even]" : " [odd]") << "\n";
    }
    std::cout << "Built successfully via cxxgen multi-generator backend!\n";
    return 0;
}
