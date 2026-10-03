#include "greeting.hpp"

#include <iostream>

int main(int argc, char **argv) {
    std::string name = (argc > 1) ? argv[1] : "Developer";
    std::cout << standalone::format_greeting(name) << std::endl;
    return 0;
}
