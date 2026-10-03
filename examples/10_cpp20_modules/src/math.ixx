export module math;

export namespace math {

struct Vector2D {
    double x = 0.0;
    double y = 0.0;

    constexpr Vector2D() = default;
    constexpr Vector2D(double x_, double y_) : x(x_), y(y_) {}

    constexpr Vector2D operator+(const Vector2D &other) const noexcept { return Vector2D(x + other.x, y + other.y); }

    constexpr Vector2D operator*(double scalar) const noexcept { return Vector2D(x * scalar, y * scalar); }

    constexpr double dot(const Vector2D &other) const noexcept { return (x * other.x) + (y * other.y); }
};

constexpr int add(int a, int b) noexcept {
    return a + b;
}

constexpr int multiply(int a, int b) noexcept {
    return a * b;
}

constexpr unsigned long long factorial(unsigned int n) noexcept {
    unsigned long long result = 1;
    for (unsigned int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

} // namespace math
