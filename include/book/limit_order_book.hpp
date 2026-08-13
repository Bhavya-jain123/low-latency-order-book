#pragma once

#include <functional>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/order.hpp"
#include "core/trade.hpp"
#include "memory/order_pool.hpp"

namespace loe {
struct BookLevelView { Price price{}; Quantity quantity{}; std::size_t order_count{}; };
struct BookSnapshot { std::vector<BookLevelView> bids; std::vector<BookLevelView> asks; };
struct SubmitResult { bool accepted{}; bool rested{}; bool pool_exhausted{}; std::vector<Trade> trades; };

class LimitOrderBook {
public:
    explicit LimitOrderBook(std::size_t pool_capacity = 1'000'000);
    SubmitResult submit(const OrderRequest& request);
    bool cancel(OrderId id);
    [[nodiscard]] bool contains(OrderId id) const;
    [[nodiscard]] BookSnapshot snapshot(std::size_t max_levels = 10) const;
    [[nodiscard]] std::size_t active_order_count() const noexcept { return index_.size(); }
    [[nodiscard]] std::size_t pool_available() const noexcept { return pool_.available(); }
    [[nodiscard]] bool validate_invariants(std::string* reason = nullptr) const;

private:
    struct PriceLevel { std::list<Order*> orders; };
    using BidMap = std::map<Price, PriceLevel, std::greater<Price>>;
    using AskMap = std::map<Price, PriceLevel, std::less<Price>>;
    struct Location { Side side; Price price; std::list<Order*>::iterator position; };

    bool crosses(const OrderRequest& request) const;
    Order* best_opposing(Side incoming_side);
    void remove_resting(Order* order);
    void rest(Order* order);
    Trade make_trade(const OrderRequest& incoming, const Order& resting, Quantity quantity);

    BidMap bids_;
    AskMap asks_;
    std::unordered_map<OrderId, Location> index_;
    // Retains submitted identities so an ID cannot be reused after fill/cancel.
    std::unordered_set<OrderId> used_ids_;
    OrderPool pool_;
    TradeId next_trade_id_{1};
    std::uint64_t next_arrival_sequence_{1};
};
} // namespace loe
