#include "cxxgen.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

void test_doctor_diagnose() {
    auto report = cxxgen::SystemDoctor::diagnose(false);

    // There should be checks performed
    assert(!report.items.empty());
    assert(report.items.size() >= 5);

    // On our host system, at least one compiler must be found
    assert(report.has_compiler);
    assert(report.ok_count >= 1);

    // Check that categories exist
    bool found_compilers = false;
    bool found_tools = false;
    for (const auto &item : report.items) {
        if (item.category == "Compilers") {
            found_compilers = true;
        }
        if (item.category == "Developer Tools") {
            found_tools = true;
        }
        // If status is Ok, path and version shouldn't be empty
        if (item.status == cxxgen::DoctorStatus::Ok) {
            assert(!item.path.empty());
        }
    }
    assert(found_compilers);
    assert(found_tools);
    std::cout << "[PASS] test_doctor_diagnose\n";
}

void test_doctor_report_formatting() {
    auto report = cxxgen::SystemDoctor::diagnose(false);
    std::ostringstream oss;
    cxxgen::SystemDoctor::print_report(report, oss, false);

    std::string output = oss.str();
    assert(output.find("cxxgen doctor") != std::string::npos);
    assert(output.find("Compilers") != std::string::npos);
    assert(output.find("Summary:") != std::string::npos);
    assert(output.find("Status:") != std::string::npos);
    std::cout << "[PASS] test_doctor_report_formatting\n";
}

void test_doctor_run() {
    std::ostringstream oss;
    int exit_code = cxxgen::SystemDoctor::run_doctor(false, oss);
    assert(exit_code == 0); // Should succeed since host has a working compiler
    std::cout << "[PASS] test_doctor_run\n";
}

void test_doctor_cli_parsing() {
    const char *argv[] = {"cxxgen", "doctor"};
    auto args = cxxgen::parse_cli_args(2, const_cast<char **>(argv));
    assert(args.command == "doctor");
    std::cout << "[PASS] test_doctor_cli_parsing\n";
}

void test_doctor_find_in_path() {
#if defined(_WIN32)
    auto git_path = cxxgen::SystemDoctor::find_in_path("git");
    // git is installed on Windows test host
    assert(git_path.has_value());
    assert(git_path->find("git") != std::string::npos);
#else
    auto sh_path = cxxgen::SystemDoctor::find_in_path("sh");
    assert(sh_path.has_value());
#endif
    // Non-existent binary should return nullopt
    auto fake = cxxgen::SystemDoctor::find_in_path("non_existent_fake_binary_12345");
    assert(!fake.has_value());
    std::cout << "[PASS] test_doctor_find_in_path\n";
}

int main() {
    std::cout << "Running test_doctor...\n";
    test_doctor_diagnose();
    test_doctor_report_formatting();
    test_doctor_run();
    test_doctor_cli_parsing();
    test_doctor_find_in_path();
    std::cout << "All doctor tests passed successfully!\n";
    return 0;
}
