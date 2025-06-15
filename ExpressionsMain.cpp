//
// Created by kalmbacj on 6/15/25.
//

#include "./binary_expression.h"

#include <print>

int main() {
    auto gen1 = []() -> std::generator<Arg>{ co_yield Arg{3.0};};
    auto gen2 = []() -> std::generator<Arg>{ co_yield Arg{4.0};};

    auto binary = binaryExpression(gen1(), gen2(), std::plus{});
    for (const auto& result : binary) {
        std::println("result: {}", std::get<double>(result));
    }
}
