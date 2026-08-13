#pragma once

#include "core/types.hpp"

namespace loe {
struct Trade {
    TradeId id{};
    OrderId buy_order_id{};
    OrderId sell_order_id{};
    Price price{};       // Resting-order price.
    Quantity quantity{};
    Timestamp timestamp{};
};
} // namespace loe
