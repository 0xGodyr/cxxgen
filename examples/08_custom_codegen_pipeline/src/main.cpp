#include "ui_strings.hpp"

#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  Custom Codegen DAG Pipeline Demo      \n";
    std::cout << "========================================\n\n";

    std::cout << "Displaying Strings Generated at Build Time:\n";
    std::cout << "  [WELCOME]   : " << strings::MSG_WELCOME() << "\n";
    std::cout << "  [NEW GAME]  : " << strings::MSG_NEW_GAME() << "\n";
    std::cout << "  [LOAD GAME] : " << strings::MSG_LOAD_GAME() << "\n";
    std::cout << "  [OPTIONS]   : " << strings::MSG_OPTIONS() << "\n";
    std::cout << "  [QUIT]      : " << strings::MSG_QUIT() << "\n\n";

    std::cout << "[SUCCESS] DAG code generator pipeline completed successfully!\n";
    return 0;
}
