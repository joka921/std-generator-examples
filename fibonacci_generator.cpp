#include <generator>
#include <cstddef>
#include <ranges>

std::generator<size_t> fibonacci_gen() {
    size_t i = 0; j = 1;
    while (true) {
        co_yield j;
        i = std::exchange(j, i + j);
    }
}

int main() {
    for (auto fib : fibonacci_gen() | std::views::take(5)) {
        std::println("yielded {}", fib);
    }
}