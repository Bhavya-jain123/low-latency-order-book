#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/command.hpp"

namespace loe {
enum class WorkloadPattern { NonCrossing, Crossing, SamePrice, Mixed, Burst };
struct WorkloadConfig {
    std::size_t order_count{10'000};
    std::uint64_t seed{42};
    double buy_probability{0.5};
    Price minimum_price{9'900};
    Price maximum_price{10'100};
    Quantity minimum_quantity{1};
    Quantity maximum_quantity{100};
    double cancellation_probability{0.0};
    WorkloadPattern pattern{WorkloadPattern::Mixed};
};
[[nodiscard]] std::vector<Command> generate_workload(const WorkloadConfig& config);
[[nodiscard]] WorkloadPattern parse_workload_pattern(const char* value);
} // namespace loe
