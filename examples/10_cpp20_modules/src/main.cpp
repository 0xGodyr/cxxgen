#include <iostream>

import math;
import greeter;

int main() {
    std::cout << "====================================================\n";
    std::cout << "   cxxgen: C++20 / C++23 Modules Architecture Demo   \n";
    std::cout << "====================================================\n\n";

    // Call functions exported from math module
    int sum = math::add(25, 17);
    int product = math::multiply(6, 7);
    auto fact5 = math::factorial(5);

    std::cout << "[math module]\n";
    std::cout << "  25 + 17       = " << sum << "\n";
    std::cout << "  6 * 7         = " << product << "\n";
    std::cout << "  5!            = " << fact5 << "\n";

    // Use exported struct from math module
    math::Vector2D v1(3.0, 4.0);
    math::Vector2D v2(1.0, 2.0);
    math::Vector2D v3 = v1 + v2;
    double dot_prod = v1.dot(v2);

    std::cout << "  v1            = (" << v1.x << ", " << v1.y << ")\n";
    std::cout << "  v2            = (" << v2.x << ", " << v2.y << ")\n";
    std::cout << "  v1 + v2       = (" << v3.x << ", " << v3.y << ")\n";
    std::cout << "  v1 . v2       = " << dot_prod << "\n\n";

    // Call function from greeter module which internally imported math
    auto stats = greeter::compute_stats(100, 200);
    std::cout << "[greeter module (imports math)]\n";
    std::cout << "  Calculations  = " << stats.calculations_performed << "\n";
    std::cout << "  Sum Result    = " << stats.sum_result << "\n\n";

    std::cout << "[SUCCESS] C++20 Modules executed cleanly via cxxgen!\n";
    return 0;
}
