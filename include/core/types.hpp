#pragma once

#include <cstdint>
#include <string>

namespace loe {
using OrderId = std::uint64_t;
using TradeId = std::uint64_t;
using Quantity = std::uint64_t;
using Price = std::int64_t;       // Integer ticks; the CLI uses 100 ticks per unit.
using Timestamp = std::uint64_t; // Synthetic monotonic sequence/timestamp.

enum class Side : std::uint8_t { Buy, Sell };

[[nodiscard]] inline std::string to_string(Side side) {
    return side == Side::Buy ? "BUY" : "SELL";
}
} // namespace loe
