#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <type_traits>

namespace loe {
// Bounded SPSC queue. One producer calls try_push and one consumer calls try_pop.
template <typename T, std::size_t Capacity>
class SpscRingBuffer {
    static_assert(Capacity >= 2 && (Capacity & (Capacity - 1)) == 0, "Capacity must be a power of two >= 2");
    static_assert(std::is_trivially_copyable_v<T>, "SPSC messages must be trivially copyable");
public:
    [[nodiscard]] bool try_push(const T& value) noexcept {
        const auto write = write_index_.load(std::memory_order_relaxed);
        const auto next = increment(write);
        if (next == read_index_.load(std::memory_order_acquire)) return false;
        buffer_[write] = value;
        write_index_.store(next, std::memory_order_release);
        return true;
    }
    [[nodiscard]] bool try_pop(T& value) noexcept {
        const auto read = read_index_.load(std::memory_order_relaxed);
        if (read == write_index_.load(std::memory_order_acquire)) return false;
        value = buffer_[read];
        read_index_.store(increment(read), std::memory_order_release);
        return true;
    }
    [[nodiscard]] bool empty() const noexcept {
        return read_index_.load(std::memory_order_acquire) == write_index_.load(std::memory_order_acquire);
    }
    static constexpr std::size_t capacity() noexcept { return Capacity - 1; }
private:
    static constexpr std::size_t increment(std::size_t index) noexcept { return (index + 1) & (Capacity - 1); }
    alignas(64) std::array<T, Capacity> buffer_{};
    alignas(64) std::atomic<std::size_t> write_index_{0};
    alignas(64) std::atomic<std::size_t> read_index_{0};
};
} // namespace loe
