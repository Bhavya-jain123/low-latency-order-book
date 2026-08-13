#pragma once

#include "core/types.hpp"

namespace loe {
struct OrderRequest {
    OrderId id{};
    Side side{};
    Price price{};
    Quantity quantity{};
    Timestamp timestamp{};
};

struct Order {
    OrderId id{};
    Side side{};
    Price price{};
    Quantity remaining{};
    Timestamp timestamp{};
    std::uint64_t arrival_sequence{};
};
} // namespace loe
