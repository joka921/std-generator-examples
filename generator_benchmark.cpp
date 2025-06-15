//
// Created by kalmbacj on 6/15/25.
//
#include <benchmark/benchmark.h>
#include <generator>
#include "./simple_generator.h"
#include "./IndirectIota.h"

template <typename F>
using R = std::remove_reference_t<std::invoke_result_t<F, size_t>>;

template <typename F = std::identity>
custom::generator<R<F>> iota_gen(F f = {}) {
    size_t i = 0;
    while (true) {
        co_yield f(i++);
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

template <typename F>
static void BM_IotaGen(benchmark::State& state){
    auto gen = iota_gen(F{});
    auto it = gen.begin();
    R<F> res{};
    for (auto _ : state) {
        benchmark::DoNotOptimize( res= std::move(*it));
        ++it;
    }
}

template <typename F>
static void BM_Iota(benchmark::State& state){
    auto gen = std::views::iota(size_t{0}) | std::views::transform(F{});
    auto it = gen.begin();
    R<F> res{};
    for (auto _ : state) {
        benchmark::DoNotOptimize( res= std::move(*it));
        ++it;
    }
}

static void BM_IndirectIota(benchmark::State& state){
    auto gen = IndirectIota{};
    size_t res = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize( res+= gen.get_next());
    }
}

static void BM_VirtualIota(benchmark::State& state){
    auto ptr = makeVirtualIota();
    auto& gen = *ptr;
    size_t res = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize( res+= gen.get_next());
    }
}

template <typename F>
static void BM_JoinVec(benchmark::State& state){
    auto gen = iota_vec_gen(F{}) | std::views::join;
    R<F> res{};
    auto it = gen.begin();
    for (auto _ : state) {
        static_assert(!std::is_const_v<decltype(*it)>);
        benchmark::DoNotOptimize( res= std::move(*it));
        ++it;
    }
}

struct NoCopy {
            NoCopy() = default;
            NoCopy(const NoCopy&) = delete;
            NoCopy& operator=(const NoCopy&) = delete;
    NoCopy(NoCopy&&) = default;
    NoCopy& operator=(NoCopy&&) = default;
        };

auto toNoCopy = [](size_t) {
    return NoCopy{};
};

auto toString = [](size_t i) {
    return std::to_string(i);
};

using ToString = decltype(toString);
//using ToString = decltype(toNoCopy);

BENCHMARK(BM_IotaGen<std::identity>);
BENCHMARK(BM_Iota<std::identity>);
BENCHMARK(BM_IndirectIota);
BENCHMARK(BM_VirtualIota);
BENCHMARK(BM_JoinVec<std::identity>);

BENCHMARK(BM_IotaGen<ToString>);
BENCHMARK(BM_Iota<ToString>);
BENCHMARK(BM_JoinVec<ToString>);
