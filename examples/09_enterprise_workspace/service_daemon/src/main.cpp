#include "engine.hpp"

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    enterprise::Logger::log(enterprise::LogLevel::Info, "Daemon", "Background service daemon initiating...");

    enterprise::SystemContext ctx("Background-Service");
    ctx.start();

    enterprise::Logger::log(enterprise::LogLevel::Info, "Daemon", "Simulating 1 heartbeat tick...");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::cout << ctx.info() << "\n";
    ctx.stop();
    return 0;
}
