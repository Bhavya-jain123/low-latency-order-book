#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "core/order.hpp"

namespace loe {
// Fixed-capacity pool. Objects have stable addresses until released.
class OrderPool {
public:
    explicit OrderPool(std::size_t capacity) : storage_(capacity), free_() {
        free_.reserve(capacity);
        for (auto& slot : storage_) free_.push_back(&slot);
    }

    [[nodiscard]] Order* acquire(const OrderRequest& request, std::uint64_t sequence) {
        if (free_.empty()) return nullptr;
        Order* order = free_.back();
        free_.pop_back();
        *order = Order{request.id, request.side, request.price, request.quantity,
                       request.timestamp, sequence};
        return order;
    }
    void release(Order* order) { free_.push_back(order); }
    [[nodiscard]] std::size_t capacity() const noexcept { return storage_.size(); }
    [[nodiscard]] std::size_t available() const noexcept { return free_.size(); }

private:
    std::vector<Order> storage_;
    std::vector<Order*> free_;
};
} // namespace loe
