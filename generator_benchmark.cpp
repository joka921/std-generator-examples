//
// Created by kalmbacj on 6/15/25.
//
#include <benchmark/benchmark.h>
#include <generator>
#include <numeric>
#include <iostream>
#include "./simple_generator.h"
//#include "./batched_generator.h"
#include "./IndirectIota.h"


template <typename F>
static void BM_IotaGenStd(benchmark::State& state){
    auto gen = iota_gen_std(F{});
    auto it = gen.begin();
    R<F> res{};
    for (auto _ : state) {
        benchmark::DoNotOptimize( res= std::move(*it));
        ++it;
    }
}

template <typename F>
static void BM_IotaGenSimple(benchmark::State& state){
    auto gen = iota_gen_simple(F{});
    auto it = gen.begin();
    R<F> res{};
    for (auto _ : state) {
        benchmark::DoNotOptimize( res= std::move(*it));
        ++it;
    }
}

template <typename F>
static void BM_IotaGenBatchedStdNested(benchmark::State& state){
    auto gen = iota_gen_batched_std(F{});
    auto it = gen.begin();
    R<F> res{};
    for (auto _ : state) {
        for (auto& el : *it) {
            benchmark::DoNotOptimize( res=std::move(el));
        }
        ++it;
    }
}

template <typename F>
static void BM_IotaGenBatchedStdJoin(benchmark::State& state){
    auto gen = iota_gen_batched_std(F{}) | std::views::join;
    auto it = gen.begin();
    R<F> res{};
    for (auto _ : state) {
        benchmark::DoNotOptimize( res=std::move(*it));
        ++it;
    }
}

template <typename F>
static void BM_IotaGenBatchedNested(benchmark::State& state){
  auto gen = iota_gen_batched(F{});
  auto it = gen.begin();
  R<F> res{};
  for (auto _ : state) {
    for (auto& el : *it) {
      benchmark::DoNotOptimize( res=std::move(el));
    }
    ++it;
  }
}

template <typename F>
static void BM_IotaGenBatchedJoin(benchmark::State& state){
    auto gen = iota_gen_batched(F{}) | std::views::join;
    auto it = gen.begin();
    R<F> res{};
    for (auto _ : state) {
        benchmark::DoNotOptimize( res=std::move(*it));
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

static void BM_IndirectFunction(benchmark::State& state){
  size_t res = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize( res+= indirectFunction());
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

//using ToString = decltype(toString);
using ToString = decltype(toNoCopy);

BENCHMARK(BM_IotaGenStd<std::identity>);
BENCHMARK(BM_IotaGenSimple<std::identity>);
BENCHMARK(BM_Iota<std::identity>);
BENCHMARK(BM_IndirectIota);
BENCHMARK(BM_VirtualIota);
BENCHMARK(BM_IndirectFunction);
BENCHMARK(BM_IotaGenBatchedStdJoin<std::identity>);
BENCHMARK(BM_IotaGenBatchedStdNested<std::identity>);
BENCHMARK(BM_IotaGenBatchedJoin<std::identity>);
BENCHMARK(BM_IotaGenBatchedNested<std::identity>);

BENCHMARK(BM_IotaGenStd<ToString>);
BENCHMARK(BM_IotaGenSimple<ToString>);
BENCHMARK(BM_IotaGenBatchedJoin<ToString>);
BENCHMARK(BM_Iota<ToString>);
