export module greeter;

import math;

export namespace greeter {

struct Stats {
    int calculations_performed = 0;
    int sum_result = 0;
};

Stats compute_stats(int a, int b) {
    Stats s;
    s.calculations_performed = 2;
    s.sum_result = math::add(a, b);
    return s;
}

} // namespace greeter
