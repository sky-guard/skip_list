#include "skip_list.hpp"

#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>

namespace skip_list {

    // Search Benchmark (50% hits, 50% misses, pre-generated)
    static void BM_SkipListSearch(benchmark::State& state) {
        // Generate N, Seed
        const auto N{static_cast<size_t>(state.range(0))};
        const auto seed{static_cast<seed_t>(state.range(1))};

        // Initialize Skip List with N even Keys
        SkipList sl{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
        std::vector<key_t> inserted_keys;
        inserted_keys.reserve(N);
        for (size_t key_idx{0}; key_idx < N; ++key_idx) {
            inserted_keys.push_back(static_cast<key_t>(key_idx * 2));
            sl.insert(inserted_keys[key_idx]);
        }

        // Pre-generate queries with 50% hits (even Keys) and 50% misses (odd Keys)
        std::vector<key_t> queries;
        queries.reserve(N);
        for (size_t query_idx{0}; query_idx < N / 2; ++query_idx) {
            // Hit
            queries.push_back(inserted_keys[query_idx * 2]);

            // Miss
            queries.push_back(static_cast<key_t>(query_idx * 2 + 1));
        }

        // Shuffle queries
        rng_t rng{seed + 1};
        std::ranges::shuffle(queries, rng);

        // Run queries
        size_t query_idx{0};
        auto query{queries[query_idx]};
        for (auto _ : state) {
            // Time Search
            auto start{std::chrono::high_resolution_clock::now()};
            auto res{sl.search(query)};
            auto end{std::chrono::high_resolution_clock::now()};

            // Prevent compiler optimizations
            benchmark::DoNotOptimize(res);

            // Record elapsed time manually
            std::chrono::duration<double> elapsed{end - start};
            state.SetIterationTime(elapsed.count());

            // Set up next iteration
            query_idx = (query_idx + 1) % queries.size();
            query = queries[query_idx];
        }

        // Set State Complexity
        state.SetComplexityN(static_cast<int64_t>(N));
    }

    // Insert Benchmark (absent Keys)
    static void BM_SkipListInsert(benchmark::State& state) {
        // Generate N, Seed
        const auto N{static_cast<size_t>(state.range(0))};
        const auto seed{static_cast<seed_t>(state.range(1))};

        // Initialize Skip List with N even Keys
        SkipList sl{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
        for (size_t key_idx = 0; key_idx < N; ++key_idx) {
            sl.insert(static_cast<key_t>(key_idx * 2));
        }

        // Pre-generate absent odd Keys
        std::vector<key_t> absent_keys;
        absent_keys.reserve(N);
        for (size_t key_idx{0}; key_idx < N; ++key_idx) {
            absent_keys.push_back(static_cast<key_t>(key_idx * 2 + 1));
        }

        // Shuffle absent Keys
        rng_t rng{seed + 1};
        std::ranges::shuffle(absent_keys, rng);

        // Run Insertions
        size_t key_idx{0};
        auto absent_key{absent_keys[key_idx]};
        for (auto _ : state) {
            // Time Insert
            auto start{std::chrono::high_resolution_clock::now()};
            auto res{sl.insert(absent_key)};
            auto end{std::chrono::high_resolution_clock::now()};

            // Prevent compiler optimizations
            benchmark::DoNotOptimize(res);

            // Record elapsed time manually
            std::chrono::duration<double> elapsed{end - start};
            state.SetIterationTime(elapsed.count());

            // Set up next iteration
            sl.erase(absent_key);
            key_idx = (key_idx + 1) % absent_keys.size();
            absent_key = absent_keys[key_idx];
        }

        // Set State Complexity
        state.SetComplexityN(static_cast<int64_t>(N));
    }

    // Ordered Union Benchmark (disjoint ranges, |A| = |B| = N/2)
    static void BM_SkipListOrderedUnion(benchmark::State& state) {
        // Generate N, Seed, Pivot
        const auto N{static_cast<size_t>(state.range(0))};
        const auto seed{static_cast<seed_t>(state.range(1))};
        const auto pivot{N / 2};

        // Initialize Skip Lists
        SkipList A{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
        SkipList B{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed + 1};
        for (size_t key_idx{0}; key_idx < N / 2; ++key_idx)
            A.insert(static_cast<key_t>(key_idx));
        for (size_t key_idx{0}; key_idx < N / 2; ++key_idx)
            B.insert(static_cast<key_t>(N + key_idx));

        // Run Ordered Unions
        for (auto _ : state) {
            // Time Ordered Union
            auto start{std::chrono::high_resolution_clock::now()};
            auto res{SkipList::ordered_union(std::move(A), std::move(B))};
            auto end{std::chrono::high_resolution_clock::now()};

            // Prevent compiler optimizations
            benchmark::DoNotOptimize(res);

            // Record elapsed time manually
            std::chrono::duration<double> elapsed{end - start};
            state.SetIterationTime(elapsed.count());

            // Set up next iteration
            auto [L, R]{res.split(static_cast<key_t>(pivot))};
            A = std::move(L);
            B = std::move(R);
        }

        // Set State Complexity
        state.SetComplexityN(static_cast<int64_t>(N));
    }

    // Rebuilding Ordered Union Benchmark (disjoint ranges, |A| = |B| = N/2)
    static void BM_SkipListOrderedUnionRebuilding(benchmark::State& state) {
        // Generate N, Seed, Pivot
        const auto N{static_cast<size_t>(state.range(0))};
        const auto seed{static_cast<seed_t>(state.range(1))};
        const auto pivot{N / 2};

        // Initialize Skip Lists
        SkipList A{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
        SkipList B{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed + 1};
        for (size_t key_idx{0}; key_idx < N / 2; ++key_idx)
            A.insert(static_cast<key_t>(key_idx));
        for (size_t key_idx{0}; key_idx < N / 2; ++key_idx)
            B.insert(static_cast<key_t>(N + key_idx));

        // Run Ordered Unions
        for (auto _ : state) {
            // Time Ordered Union
            auto start{std::chrono::high_resolution_clock::now()};
            auto res{SkipList::rebuilding_ordered_union(std::move(A), std::move(B))};
            auto end{std::chrono::high_resolution_clock::now()};

            // Prevent compiler optimizations
            benchmark::DoNotOptimize(res);

            // Record elapsed time manually
            std::chrono::duration<double> elapsed{end - start};
            state.SetIterationTime(elapsed.count());

            // Set up next iteration
            auto [L, R]{res.split(static_cast<key_t>(pivot))};
            A = std::move(L);
            B = std::move(R);
        }

        // Set State Complexity
        state.SetComplexityN(static_cast<int64_t>(N));
    }

    // Split Benchmark (Pivot giving approximately equal output sizes)
    static void BM_SkipListSplit(benchmark::State& state) {
        // Generate N, Seed, Pivot
        const auto N{static_cast<size_t>(state.range(0))};
        const auto seed{static_cast<seed_t>(state.range(1))};
        const auto pivot{N};

        // Initialize Skip List
        SkipList sl{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
        for (size_t key_idx{0}; key_idx < N; ++key_idx) {
            sl.insert(static_cast<key_t>(key_idx * 2));
        }

        for (auto _ : state) {
            // Time Split
            auto start{std::chrono::high_resolution_clock::now()};
            auto res{sl.split(static_cast<key_t>(pivot))};
            auto end{std::chrono::high_resolution_clock::now()};

            // Prevent compiler optimizations
            benchmark::DoNotOptimize(res);

            // Record elapsed time manually
            std::chrono::duration<double> elapsed{end - start};
            state.SetIterationTime(elapsed.count());

            // Set up next iteration
            sl = SkipList::ordered_union(std::move(res.first), std::move(res.second));
        }

        // Set State Complexity
        state.SetComplexityN(static_cast<int64_t>(N));
    }

    // Rebuilding Split Benchmark (Pivot giving approximately equal output sizes)
    static void BM_SkipListSplitRebuilding(benchmark::State& state) {
        const auto N{static_cast<size_t>(state.range(0))};
        const auto seed{static_cast<seed_t>(state.range(1))};
        const auto pivot{N};

        // Initialize Skip List
        SkipList sl{MAX_HEIGHT_DEFAULT, PROB_DEFAULT, seed};
        for (size_t key_idx{0}; key_idx < N; ++key_idx) {
            sl.insert(static_cast<key_t>(key_idx * 2));
        }

        for (auto _ : state) {
            // Time Rebuilding Split
            auto start{std::chrono::high_resolution_clock::now()};
            auto res{sl.rebuilding_split(static_cast<key_t>(pivot))};
            auto end{std::chrono::high_resolution_clock::now()};

            // Prevent compiler optimizations
            benchmark::DoNotOptimize(res);

            // Record elapsed time manually
            std::chrono::duration<double> elapsed{end - start};
            state.SetIterationTime(elapsed.count());

            // Set up next iteration
            sl = SkipList::rebuilding_ordered_union(std::move(res.first), std::move(res.second));
        }

        // Set State Complexity
        state.SetComplexityN(static_cast<int64_t>(N));
    }

    // Register benchmarks with geometric sizes and multiple Seeds
    // Sizes: 2^10 to 2^20 (6 sizes), Seeds: {17, 42, 1337} (3 seeds)
    #define BENCHMARK_CONFIG(func) \
        BENCHMARK(func) \
            ->Args({1 << 10, 17})->Args({1 << 10, 42})->Args({1 << 10, 1337}) \
            ->Args({1 << 12, 17})->Args({1 << 12, 42})->Args({1 << 12, 1337}) \
            ->Args({1 << 14, 17})->Args({1 << 14, 42})->Args({1 << 14, 1337}) \
            ->Args({1 << 16, 17})->Args({1 << 16, 42})->Args({1 << 16, 1337}) \
            ->Args({1 << 18, 17})->Args({1 << 18, 42})->Args({1 << 18, 1337}) \
            ->Args({1 << 20, 17})->Args({1 << 20, 42})->Args({1 << 20, 1337}) \
            ->UseManualTime() \
            ->Complexity(benchmark::oLogN)

    // Register benchmarks
    BENCHMARK_CONFIG(BM_SkipListSearch);
    BENCHMARK_CONFIG(BM_SkipListInsert);
    BENCHMARK_CONFIG(BM_SkipListOrderedUnion);
    BENCHMARK_CONFIG(BM_SkipListSplit);
    BENCHMARK_CONFIG(BM_SkipListSplitRebuilding);
    BENCHMARK_CONFIG(BM_SkipListOrderedUnionRebuilding);

} // namespace skip_list

BENCHMARK_MAIN();