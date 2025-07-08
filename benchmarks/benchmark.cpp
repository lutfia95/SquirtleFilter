#include <benchmark/benchmark.h>
#include "../include/SquirtleFilter.h"
#include "../include/SFilters.h"
#include <string>

// === BloomFilter Benchmarks ===

static void BM_BloomFilter_Insert(benchmark::State& state) {
    BloomFilter bf(1000000, 0.01, 5);
    std::string base = "key";
    for (auto _ : state) {
        bf.insert((base + std::to_string(state.iterations())).data(),
                  (base + std::to_string(state.iterations())).size());
    }
}
BENCHMARK(BM_BloomFilter_Insert);

static void BM_BloomFilter_Contains(benchmark::State& state) {
    BloomFilter bf(1000000, 0.01, 5);
    std::string base = "key";
    for (int i = 0; i < 1000000; ++i) {
        auto str = base + std::to_string(i);
        bf.insert(str.data(), str.size());
    }
    for (auto _ : state) {
        auto str = base + std::to_string(state.iterations() % 1000000);
        benchmark::DoNotOptimize(bf.contains(str.data(), str.size()));
    }
}
BENCHMARK(BM_BloomFilter_Contains);

// === SFilters Benchmarks ===

static void BM_SFilters_Insert(benchmark::State& state) {
    SFilters filters;
    filters.initialize(100, 100000, 0.01, 5);
    std::string base = "sfkey";
    size_t idx = state.range(0) % 10;

    for (auto _ : state) {
        filters.insert(idx, base + std::to_string(state.iterations()));
    }
}
BENCHMARK(BM_SFilters_Insert)->Arg(0);

static void BM_SFilters_Contains(benchmark::State& state) {
    SFilters filters;
    filters.initialize(100, 100000, 0.01, 5);
    std::string base = "sfkey";
    for (int i = 0; i < 100000; ++i) {
        filters.insert(i % 100, base + std::to_string(i));
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(filters.contains(base + std::to_string(state.iterations() % 100000)));
    }
}
BENCHMARK(BM_SFilters_Contains);

static void BM_SFilters_MatchBitVector(benchmark::State& state) {
    SFilters filters;
    filters.initialize(100, 100000, 0.01, 5);
    std::string base = "match";
    for (int i = 0; i < 100000; ++i) {
        filters.insert(i % 100, base + std::to_string(i));
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(filters.matchBitVector(base + std::to_string(state.iterations() % 100000)));
    }
}
BENCHMARK(BM_SFilters_MatchBitVector);

BENCHMARK_MAIN();
