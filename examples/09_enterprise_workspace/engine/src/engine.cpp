#include "engine.hpp"

#include <iostream>

namespace enterprise {

void Logger::log(LogLevel level, std::string_view component, std::string_view message) {
    const char *lvl_str = "INFO";
    switch (level) {
    case LogLevel::Debug:
        lvl_str = "DEBUG";
        break;
    case LogLevel::Info:
        lvl_str = "INFO";
        break;
    case LogLevel::Warn:
        lvl_str = "WARN";
        break;
    case LogLevel::Error:
        lvl_str = "ERROR";
        break;
    }
    std::cout << "[" << lvl_str << "] [" << component << "] " << message << "\n";
}

SystemContext::SystemContext(std::string name) : name_(std::move(name)) {}

void SystemContext::start() {
    running_ = true;
    start_time_ = std::chrono::steady_clock::now();
    Logger::log(LogLevel::Info, name_, "Context initialized and started.");
}

void SystemContext::stop() {
    running_ = false;
    Logger::log(LogLevel::Info, name_, "Context stopped.");
}

std::string SystemContext::info() const {
    return "Enterprise Module [" + name_ + "] (v" + version() + ") - State: " + (running_ ? "Running" : "Idle");
}

std::string version() {
    return "2.4.0-enterprise";
}

} // namespace enterprise
