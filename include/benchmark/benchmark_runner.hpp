#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "benchmark/latency_statistics.hpp"
#include "core/command.hpp"
#include "matching/matching_engine.hpp"

namespace loe {
struct BenchmarkResult {
    std::size_t commands{};
    double elapsed_seconds{};
    double commands_per_second{};
    EngineMetrics engine_metrics{};
    LatencyStatistics latency{};
};
[[nodiscard]] BenchmarkResult run_spsc_benchmark(const std::vector<Command>& commands, std::size_t pool_capacity);
} // namespace loe
