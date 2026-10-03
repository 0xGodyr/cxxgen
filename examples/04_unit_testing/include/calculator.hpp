#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>

namespace calc {

int64_t add(int64_t a, int64_t b);
int64_t subtract(int64_t a, int64_t b);
int64_t multiply(int64_t a, int64_t b);
double divide(double a, double b);

int64_t power(int64_t base, uint32_t exp);
int64_t gcd(int64_t a, int64_t b);
bool is_prime(uint64_t n);

} // namespace calc
