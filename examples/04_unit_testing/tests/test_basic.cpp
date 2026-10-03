#include "calculator.hpp"

#include <cassert>
#include <iostream>

void test_addition() {
    assert(calc::add(0, 0) == 0);
    assert(calc::add(10, 20) == 30);
    assert(calc::add(-5, 5) == 0);
    assert(calc::add(-10, -20) == -30);
}

void test_subtraction() {
    assert(calc::subtract(10, 5) == 5);
    assert(calc::subtract(5, 10) == -5);
    assert(calc::subtract(0, 0) == 0);
}

void test_multiplication() {
    assert(calc::multiply(6, 7) == 42);
    assert(calc::multiply(-6, 7) == -42);
    assert(calc::multiply(0, 100) == 0);
}

void test_division() {
    assert(calc::divide(10.0, 2.0) == 5.0);
    bool threw = false;
    try {
        calc::divide(10.0, 0.0);
    } catch (const std::invalid_argument &) {
        threw = true;
    }
    assert(threw);
}

int main() {
    std::cout << "[RUNNING] test_basic suite...\n";
    test_addition();
    test_subtraction();
    test_multiplication();
    test_division();
    std::cout << "[PASS] All test_basic checks passed!\n";
    return 0;
}
