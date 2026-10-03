#pragma once

#include <chrono>
#include <string>
#include <string_view>

namespace enterprise {

enum class LogLevel { Debug, Info, Warn, Error };

class Logger {
public:
    static void log(LogLevel level, std::string_view component, std::string_view message);
};

class SystemContext {
public:
    explicit SystemContext(std::string name);
    void start();
    void stop();
    [[nodiscard]] std::string info() const;

private:
    std::string name_;
    std::chrono::steady_clock::time_point start_time_;
    bool running_{false};
};

std::string version();

} // namespace enterprise
