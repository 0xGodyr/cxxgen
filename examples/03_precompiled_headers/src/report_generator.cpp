#include "report_generator.hpp"

namespace analytics {

std::string ReportGenerator::format_summary(const DataProcessor &proc) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "========================================\n";
    oss << "        ANALYTICS SUMMARY REPORT        \n";
    oss << "========================================\n";
    oss << "Total Sum : " << proc.sum() << "\n";
    oss << "Average   : " << proc.average() << "\n";
    oss << "\nBreakdown by Category:\n";
    for (const auto &[cat, val] : proc.category_breakdown()) {
        oss << "  - " << std::left << std::setw(15) << cat << ": " << val << "\n";
    }
    oss << "========================================\n";
    return oss.str();
}

} // namespace analytics
