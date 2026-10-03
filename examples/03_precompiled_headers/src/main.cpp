#include "data_processor.hpp"
#include "report_generator.hpp"

int main() {
    analytics::DataProcessor processor;

    processor.add("Compute", 142.50);
    processor.add("Storage", 89.20);
    processor.add("Networking", 215.10);
    processor.add("Compute", 78.40);
    processor.add("Database", 310.00);

    std::cout << analytics::ReportGenerator::format_summary(processor);
    std::cout << "[cxxgen PCH] Executed successfully using precompiled headers!\n";
    return 0;
}
