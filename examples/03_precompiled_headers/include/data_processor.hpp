#pragma once

#if !defined(CXXGEN_PCH_INCLUDED)
#include "pch.hpp"
#endif

namespace analytics {

struct DataPoint {
    std::string category;
    double value;
};

class DataProcessor {
public:
    void add(std::string_view category, double value);
    [[nodiscard]] double sum() const;
    [[nodiscard]] double average() const;
    [[nodiscard]] std::map<std::string, double> category_breakdown() const;

private:
    std::vector<DataPoint> points_;
};

} // namespace analytics
