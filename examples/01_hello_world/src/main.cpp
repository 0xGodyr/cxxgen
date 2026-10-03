#include <iostream>
#include <string_view>
#include <vector>

void print_banner(std::string_view title) {
    std::cout << "========================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "========================================\n";
}

int main(int argc, char **argv) {
    print_banner("Welcome to cxxgen!");

    std::cout << "C++ Standard : C++20\n";
    std::cout << "Architecture : Native Fast Build\n\n";

    if (argc > 1) {
        std::cout << "Received arguments:\n";
        for (int i = 1; i < argc; ++i) {
            std::cout << "  [" << i << "] " << argv[i] << "\n";
        }
    } else {
        std::cout << "Tip: Run with arguments using `cxxgen run hello_world -- <your-args>`\n";
    }

    std::cout << "\nHello, Modern C++ World from cxxgen!\n";
    return 0;
}
