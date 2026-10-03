#include "math_static.hpp"

namespace math {

int64_t add(int64_t a, int64_t b) {
    return a + b;
}

int64_t multiply(int64_t a, int64_t b) {
    return a * b;
}

int64_t factorial(uint32_t n) {
    if (n <= 1)
        return 1;
    int64_t res = 1;
    for (uint32_t i = 2; i <= n; ++i) {
        res *= i;
    }
    return res;
}

} // namespace math
