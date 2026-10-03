#include "calculator.hpp"

#include <cassert>
#include <iostream>

void test_power() {
    assert(calc::power(2, 0) == 1);
    assert(calc::power(2, 10) == 1024);
    assert(calc::power(3, 4) == 81);
    assert(calc::power(10, 5) == 100000);
}

void test_gcd() {
    assert(calc::gcd(54, 24) == 6);
    assert(calc::gcd(48, 18) == 6);
    assert(calc::gcd(101, 10) == 1);
    assert(calc::gcd(0, 5) == 5);
}

void test_is_prime() {
    assert(!calc::is_prime(0));
    assert(!calc::is_prime(1));
    assert(calc::is_prime(2));
    assert(calc::is_prime(3));
    assert(!calc::is_prime(4));
    assert(calc::is_prime(17));
    assert(!calc::is_prime(100));
    assert(calc::is_prime(10007));
}

int main() {
    std::cout << "[RUNNING] test_advanced suite...\n";
    test_power();
    test_gcd();
    test_is_prime();
    std::cout << "[PASS] All test_advanced checks passed!\n";
    return 0;
}
