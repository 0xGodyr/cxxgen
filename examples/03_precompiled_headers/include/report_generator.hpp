#pragma once

#if !defined(CXXGEN_PCH_INCLUDED)
#include "pch.hpp"
#endif

#include "data_processor.hpp"

namespace analytics {

class ReportGenerator {
public:
    static std::string format_summary(const DataProcessor &proc);
};

} // namespace analytics
