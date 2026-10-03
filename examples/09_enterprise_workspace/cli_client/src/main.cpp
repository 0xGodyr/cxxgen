#include "engine.hpp"

#include <iostream>

int main(int argc, char **argv) {
    enterprise::Logger::log(enterprise::LogLevel::Info, "CLI", "Enterprise CLI utility launched");

    enterprise::SystemContext ctx("CLI-Tool");
    ctx.start();

    if (argc > 1) {
        enterprise::Logger::log(enterprise::LogLevel::Debug, "CLI",
                                "Command argument received: " + std::string(argv[1]));
    }

    std::cout << ctx.info() << "\n";
    ctx.stop();
    return 0;
}
