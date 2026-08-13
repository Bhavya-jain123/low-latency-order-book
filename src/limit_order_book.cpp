#include "book/limit_order_book.hpp"

#include <algorithm>
#include <sstream>

namespace loe {
LimitOrderBook::LimitOrderBook(std::size_t pool_capacity) : pool_(pool_capacity) {}

bool LimitOrderBook::crosses(const OrderRequest& request) const {
    if (request.side == Side::Buy)
        return !asks_.empty() && request.price >= asks_.begin()->first;
    return !bids_.empty() && request.price <= bids_.begin()->first;
}

Order* LimitOrderBook::best_opposing(Side incoming_side) {
    if (incoming_side == Side::Buy) {
        if (asks_.empty()) return nullptr;
        return asks_.begin()->second.orders.front();
    }
    if (bids_.empty()) return nullptr;
    return bids_.begin()->second.orders.front();
}

Trade LimitOrderBook::make_trade(const OrderRequest& incoming, const Order& resting, Quantity quantity) {
    const bool incoming_is_buy = incoming.side == Side::Buy;
    return Trade{next_trade_id_++, incoming_is_buy ? incoming.id : resting.id,
                 incoming_is_buy ? resting.id : incoming.id, resting.price, quantity,
                 incoming.timestamp};
}

void LimitOrderBook::rest(Order* order) {
    if (order->side == Side::Buy) {
        auto& level = bids_[order->price];
        level.orders.push_back(order);
        index_.emplace(order->id, Location{order->side, order->price, std::prev(level.orders.end())});
    } else {
        auto& level = asks_[order->price];
        level.orders.push_back(order);
        index_.emplace(order->id, Location{order->side, order->price, std::prev(level.orders.end())});
    }
}

void LimitOrderBook::remove_resting(Order* order) {
    const auto indexed = index_.find(order->id);
    if (indexed == index_.end()) return; // Internal callers only; defensive for release builds.
    const Location location = indexed->second;
    if (location.side == Side::Buy) {
        auto level = bids_.find(location.price);
        level->second.orders.erase(location.position);
        if (level->second.orders.empty()) bids_.erase(level);
    } else {
        auto level = asks_.find(location.price);
        level->second.orders.erase(location.position);
        if (level->second.orders.empty()) asks_.erase(level);
    }
    index_.erase(indexed);
    pool_.release(order);
}

SubmitResult LimitOrderBook::submit(const OrderRequest& request) {
    SubmitResult result{};
    if (request.id == 0 || request.quantity == 0 || request.price < 0 || used_ids_.find(request.id) != used_ids_.end()) return result;
    used_ids_.insert(request.id);
    result.accepted = true;
    Quantity remaining = request.quantity;
    while (remaining != 0 && crosses(OrderRequest{request.id, request.side, request.price, remaining, request.timestamp})) {
        Order* resting = best_opposing(request.side);
        const Quantity executed = std::min(remaining, resting->remaining);
        result.trades.push_back(make_trade(request, *resting, executed));
        remaining -= executed;
        resting->remaining -= executed;
        if (resting->remaining == 0) remove_resting(resting);
    }
    if (remaining != 0) {
        OrderRequest residual = request;
        residual.quantity = remaining;
        Order* order = pool_.acquire(residual, next_arrival_sequence_++);
        if (order == nullptr) {
            // Valid fills already happened; only the unfilled residual is rejected.
            result.pool_exhausted = true;
            return result;
        }
        rest(order);
        result.rested = true;
    }
    return result;
}

bool LimitOrderBook::cancel(OrderId id) {
    const auto found = index_.find(id);
    if (found == index_.end()) return false;
    Order* order = *found->second.position;
    remove_resting(order);
    return true;
}

bool LimitOrderBook::contains(OrderId id) const { return index_.find(id) != index_.end(); }

BookSnapshot LimitOrderBook::snapshot(std::size_t max_levels) const {
    BookSnapshot result{};
    const auto copy_levels = [max_levels](const auto& source, auto& target) {
        for (const auto& [price, level] : source) {
            if (target.size() == max_levels) break;
            Quantity total{};
            for (const Order* order : level.orders) total += order->remaining;
            target.push_back({price, total, level.orders.size()});
        }
    };
    copy_levels(bids_, result.bids);
    copy_levels(asks_, result.asks);
    return result;
}

bool LimitOrderBook::validate_invariants(std::string* reason) const {
    std::size_t seen{};
    const auto check = [&](const auto& source, Side side) -> bool {
        for (const auto& [price, level] : source) {
            if (level.orders.empty()) { if (reason) *reason = "empty price level"; return false; }
            std::uint64_t previous_sequence{};
            for (const Order* order : level.orders) {
                ++seen;
                if (!order || order->side != side || order->price != price || order->remaining == 0) {
                    if (reason) *reason = "invalid resting order"; return false;
                }
                if (previous_sequence > order->arrival_sequence) {
                    if (reason) *reason = "FIFO sequence violation"; return false;
                }
                previous_sequence = order->arrival_sequence;
                if (index_.find(order->id) == index_.end()) { if (reason) *reason = "book entry absent from index"; return false; }
            }
        }
        return true;
    };
    if (!check(bids_, Side::Buy) || !check(asks_, Side::Sell)) return false;
    if (seen != index_.size()) { if (reason) *reason = "index/book cardinality mismatch"; return false; }
    for (const auto& [id, location] : index_) {
        const Order* order = *location.position;
        if (order->id != id || order->side != location.side || order->price != location.price) {
            if (reason) *reason = "stale index location"; return false;
        }
    }
    return true;
}
} // namespace loe
