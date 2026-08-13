#include "benchmark/workload.hpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string_view>

namespace loe {
WorkloadPattern parse_workload_pattern(const char* value) {
    const std::string_view input{value};
    if (input == "noncrossing") return WorkloadPattern::NonCrossing;
    if (input == "crossing") return WorkloadPattern::Crossing;
    if (input == "sameprice") return WorkloadPattern::SamePrice;
    if (input == "mixed") return WorkloadPattern::Mixed;
    if (input == "burst") return WorkloadPattern::Burst;
    throw std::invalid_argument("pattern must be noncrossing, crossing, sameprice, mixed, or burst");
}

std::vector<Command> generate_workload(const WorkloadConfig& config) {
    if (config.order_count == 0 || config.buy_probability < 0.0 || config.buy_probability > 1.0 ||
        config.cancellation_probability < 0.0 || config.cancellation_probability > 1.0 ||
        config.minimum_price > config.maximum_price || config.minimum_quantity == 0 || config.minimum_quantity > config.maximum_quantity)
        throw std::invalid_argument("invalid workload configuration");
    std::mt19937_64 rng(config.seed);
    std::bernoulli_distribution buy(config.buy_probability), cancel(config.cancellation_probability);
    std::uniform_int_distribution<Price> price(config.minimum_price, config.maximum_price);
    std::uniform_int_distribution<Quantity> quantity(config.minimum_quantity, config.maximum_quantity);
    std::vector<Command> result; result.reserve(config.order_count + config.order_count / 4);
    std::vector<OrderId> prior_ids; prior_ids.reserve(config.order_count);
    const Price midpoint = config.minimum_price + (config.maximum_price - config.minimum_price) / 2;
    for (std::size_t i = 0; i < config.order_count; ++i) {
        const Side side = buy(rng) ? Side::Buy : Side::Sell;
        Price chosen = price(rng);
        switch (config.pattern) {
            case WorkloadPattern::NonCrossing: chosen = side == Side::Buy ? std::min(chosen, midpoint - 1) : std::max(chosen, midpoint + 1); break;
            case WorkloadPattern::Crossing: chosen = side == Side::Buy ? config.maximum_price : config.minimum_price; break;
            case WorkloadPattern::SamePrice: chosen = midpoint; break;
            case WorkloadPattern::Burst: chosen = (i % 200 < 160) ? (side == Side::Buy ? config.maximum_price : config.minimum_price) : chosen; break;
            case WorkloadPattern::Mixed: break;
        }
        const OrderId id = static_cast<OrderId>(i + 1);
        result.push_back(Command::submit({id, side, chosen, quantity(rng), static_cast<Timestamp>(i + 1)}));
        prior_ids.push_back(id);
        if (i > 4 && cancel(rng)) {
            std::uniform_int_distribution<std::size_t> pick(0, prior_ids.size() - 1);
            result.push_back(Command::cancel(prior_ids[pick(rng)]));
        }
    }
    return result;
}
} // namespace loe
