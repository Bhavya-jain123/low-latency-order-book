#include <gtest/gtest.h>

#include "benchmark/latency_statistics.hpp"
#include "benchmark/workload.hpp"
#include "matching/matching_engine.hpp"

namespace loe {
TEST(Workload, IsDeterministicAndMaintainsInvariants) {
    WorkloadConfig config{}; config.order_count = 100'000; config.seed = 7; config.cancellation_probability = 0.1; config.pattern = WorkloadPattern::Mixed;
    const auto first = generate_workload(config); const auto second = generate_workload(config);
    ASSERT_EQ(first.size(), second.size());
    MatchingEngine engine(config.order_count + 1);
    for (std::size_t i = 0; i < first.size(); ++i) {
        EXPECT_EQ(first[i].type, second[i].type); engine.process(first[i]);
    }
    EXPECT_TRUE(engine.validate_invariants());
}
TEST(LatencyStatistics, CalculatesNearestRankIndex) {
    const auto result = calculate_latency_statistics({5, 1, 3, 2, 4});
    ASSERT_TRUE(result); EXPECT_EQ(result->minimum_ns, 1U); EXPECT_EQ(result->average_ns, 3U); EXPECT_EQ(result->p50_ns, 3U); EXPECT_EQ(result->p99_ns, 5U);
}
} // namespace loe
