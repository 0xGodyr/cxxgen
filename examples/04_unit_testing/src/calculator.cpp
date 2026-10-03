#include "calculator.hpp"

namespace calc {

int64_t add(int64_t a, int64_t b) {
    return a + b;
}

int64_t subtract(int64_t a, int64_t b) {
    return a - b;
}

int64_t multiply(int64_t a, int64_t b) {
    return a * b;
}

double divide(double a, double b) {
    if (b == 0.0) {
        throw std::invalid_argument("Division by zero");
    }
    return a / b;
}

int64_t power(int64_t base, uint32_t exp) {
    int64_t result = 1;
    int64_t current = base;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result *= current;
        }
        current *= current;
        exp /= 2;
    }
    return result;
}

int64_t gcd(int64_t a, int64_t b) {
    if (a < 0)
        a = -a;
    if (b < 0)
        b = -b;
    while (b != 0) {
        int64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

bool is_prime(uint64_t n) {
    if (n <= 1)
        return false;
    if (n <= 3)
        return true;
    if (n % 2 == 0 || n % 3 == 0)
        return false;
    for (uint64_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }
    return true;
}

} // namespace calc
