#include <benchmark/benchmark.h>
#include "../include/SquirtleFilter.h"

static void BM_Insert(benchmark::State& state) {
    BloomFilter bf(10000, 0.01, 3);
    std::string base = "key";
    for (auto _ : state) {
        bf.insert(base + std::to_string(state.iterations()));
    }
}
BENCHMARK(BM_Insert);

static void BM_Lookup(benchmark::State& state) {
    BloomFilter bf(10000, 0.01, 3);
    std::string base = "key";
    for (int i = 0; i < 10000; ++i) bf.insert(base + std::to_string(i));

    for (auto _ : state) {
        benchmark::DoNotOptimize(bf.contains(base + std::to_string(state.iterations() % 10000)));
    }
}
BENCHMARK(BM_Lookup);

BENCHMARK_MAIN();
