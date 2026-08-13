#pragma once

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <optional>
#include <vector>

namespace loe {
struct LatencyStatistics {
    std::uint64_t minimum_ns{}; std::uint64_t average_ns{}; std::uint64_t p50_ns{};
    std::uint64_t p90_ns{}; std::uint64_t p95_ns{}; std::uint64_t p99_ns{}; std::uint64_t maximum_ns{};
};
[[nodiscard]] inline std::optional<LatencyStatistics> calculate_latency_statistics(std::vector<std::uint64_t> samples) {
    if (samples.empty()) return std::nullopt;
    std::sort(samples.begin(), samples.end());
    const auto percentile = [&samples](unsigned percent) {
        const std::size_t index = (static_cast<std::size_t>(percent) * (samples.size() - 1) + 50) / 100;
        return samples[index];
    };
    const auto sum = std::accumulate(samples.begin(), samples.end(), std::uint64_t{0});
    return LatencyStatistics{samples.front(), sum / samples.size(), percentile(50), percentile(90),
                             percentile(95), percentile(99), samples.back()};
}
} // namespace loe
