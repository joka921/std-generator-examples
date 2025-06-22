//
// Created by kalmbacj on 6/15/25.
//

#ifndef STD_GENERATOR_EXAMPLES_INDIRECTIOTA_H
#define STD_GENERATOR_EXAMPLES_INDIRECTIOTA_H

#include <cstddef>
#include <generator>
#include <vector>

#include "./simple_generator.h"
#include "./batched_generator.h"

template <typename F>
using R = std::remove_reference_t<std::invoke_result_t<F, size_t>>;

template <typename F = std::identity>
custom::generator<R<F>> iota_gen_std(F f = {}) {
    size_t i = 0;
    while (true) {
        co_yield f(i++);
    }
}

template <typename F = std::identity>
custom::generator<R<F>> iota_gen_simple(F f = {}) {
    size_t i = 0;
    while (true) {
        co_yield f(i++);
    }
}

template <typename F = std::identity>
batched::generator<R<F>> iota_gen_batched(F f = {}) {
    size_t i = 0;
    while (true) {
        co_yield f(i++);
    }
}

template <typename F = std::identity>
std::generator<std::vector<R<F>>&> iota_gen_batched_std(F f = {}) {
    std::vector<R<F>> batched;
    const size_t batchSize =  10000;
    batched.reserve(batchSize);
    size_t i = 0;
    while (true) {
        while (batched.size() < batchSize) {
            batched.push_back(f(i++));
        }
        co_yield batched;
    }
}

template<typename F = std::identity>
std::generator<std::vector<R<F>>> iota_vec_gen(F f = {}) {
    constexpr static size_t BufSize = 100;
    std::vector<R<F>> buffer;
    buffer.reserve(BufSize);
    size_t i = 0;
    while (true) {
        while (buffer.size() < BufSize) {
            buffer.emplace_back(f(i++));
        }
        co_yield std::move(buffer);
        buffer.clear();
    }
}
#include <memory>

class IndirectIota {
    size_t i = 0;
public:
    size_t get_next();
};

class Base {
public:
    virtual size_t get_next() = 0;
    virtual ~Base() = default;
};

std::unique_ptr<Base> makeVirtualIota();

int indirectFunction();


#endif //STD_GENERATOR_EXAMPLES_INDIRECTIOTA_H
