#include "data_processor.hpp"

namespace analytics {

void DataProcessor::add(std::string_view category, double value) {
    points_.push_back({std::string(category), value});
}

double DataProcessor::sum() const {
    return std::accumulate(points_.begin(), points_.end(), 0.0,
                           [](double acc, const DataPoint &dp) { return acc + dp.value; });
}

double DataProcessor::average() const {
    if (points_.empty())
        return 0.0;
    return sum() / static_cast<double>(points_.size());
}

std::map<std::string, double> DataProcessor::category_breakdown() const {
    std::map<std::string, double> res;
    for (const auto &p : points_) {
        res[p.category] += p.value;
    }
    return res;
}

} // namespace analytics
