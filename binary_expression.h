#ifndef STD_GENERATOR_EXAMPLES_BINARY_EXPRESSION_H
#define STD_GENERATOR_EXAMPLES_BINARY_EXPRESSION_H

#include <generator>
#include <vector>

using Arg = std::variant<double, std::vector<double>>;

using Exp = std::generator<Arg>;

auto getResultSize(const Arg& arg1, const Arg& arg2) {
    auto getSingleSize = []<typename T>(const T& arg) -> size_t {
        if constexpr (std::same_as<T, double>) {
            (void) arg;
            return 1;
        } else {
            return arg.size();
        }
    };

  // TODO Check that sizes match.
  return std::max(std::visit(getSingleSize, arg1), std::visit(getSingleSize, arg2));
}

auto yieldAll(double val, size_t n) {
    return std::views::repeat(val, n);
}
auto yieldAll(const std::vector<double>& vec, [[maybe_unused]] size_t n) -> decltype(auto) {
    return vec;
}

template <typename F>
Arg evaluateBinaryExpression(const Arg& arg1, const Arg& arg2, F f) {
    auto resultSize = getResultSize(arg1, arg2);
    auto impl = [f, resultSize](const auto& a, const auto& b) {
        return std::ranges::to<std::vector>(std::views::zip_transform(f, yieldAll(a, resultSize), yieldAll(b, resultSize)));
    };
    auto res = std::visit(impl, arg1, arg2);
    if (res.size() == 1) {
        return Arg{res.front()};
    } else {
        return res;
    }
}

template <typename F>
Exp binaryExpression(Exp exp1, Exp exp2, F f = {}) {
    for (const auto& [arg1, arg2] : std::views::zip(std::move(exp1), std::move(exp2))) {
        co_yield evaluateBinaryExpression(arg1, arg2, f);
    }
}


#endif //STD_GENERATOR_EXAMPLES_BINARY_EXPRESSION_H
