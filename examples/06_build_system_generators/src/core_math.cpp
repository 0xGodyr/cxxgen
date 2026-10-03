#include "core_math.hpp"

namespace core {

int64_t fibonacci(uint32_t n) {
    if (n == 0)
        return 0;
    if (n == 1)
        return 1;
    int64_t a = 0, b = 1;
    for (uint32_t i = 2; i <= n; ++i) {
        int64_t c = a + b;
        a = b;
        b = c;
    }
    return b;
}

bool is_even(int64_t n) {
    return (n % 2) == 0;
}

} // namespace core
