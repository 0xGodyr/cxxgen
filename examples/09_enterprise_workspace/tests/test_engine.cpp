#include "engine.hpp"

#include <cassert>
#include <iostream>

void test_version() {
    assert(enterprise::version() == "2.4.0-enterprise");
}

void test_context() {
    enterprise::SystemContext ctx("UnitTestCtx");
    assert(ctx.info().find("Idle") != std::string::npos);
    ctx.start();
    assert(ctx.info().find("Running") != std::string::npos);
    ctx.stop();
    assert(ctx.info().find("Idle") != std::string::npos);
}

int main() {
    std::cout << "[RUNNING] Enterprise Engine Test Suite...\n";
    test_version();
    test_context();
    std::cout << "[PASS] All enterprise engine unit tests passed!\n";
    return 0;
}
