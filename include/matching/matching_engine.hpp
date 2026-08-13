#pragma once

#include <cstddef>
#include <vector>

#include "book/limit_order_book.hpp"
#include "core/command.hpp"

namespace loe {
struct EngineMetrics {
    std::size_t submitted_orders{};
    std::size_t cancelled_orders{};
    std::size_t rejected_orders{};
    std::size_t pool_exhausted_remainders{};
    std::size_t trades{};
    Quantity traded_quantity{};
};

class MatchingEngine {
public:
    explicit MatchingEngine(std::size_t pool_capacity = 1'000'000) : book_(pool_capacity) {}
    bool process(const Command& command);
    [[nodiscard]] const LimitOrderBook& book() const noexcept { return book_; }
    [[nodiscard]] const std::vector<Trade>& trades() const noexcept { return trades_; }
    [[nodiscard]] const EngineMetrics& metrics() const noexcept { return metrics_; }
    [[nodiscard]] bool validate_invariants(std::string* reason = nullptr) const { return book_.validate_invariants(reason); }

private:
    LimitOrderBook book_;
    std::vector<Trade> trades_;
    EngineMetrics metrics_;
};
} // namespace loe
