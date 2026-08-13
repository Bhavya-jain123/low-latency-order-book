#include <gtest/gtest.h>

#include "concurrency/spsc_ring_buffer.hpp"

namespace loe {
TEST(SpscRingBuffer, IsFifoAndBounded) {
    SpscRingBuffer<int, 4> queue;
    EXPECT_TRUE(queue.try_push(1)); EXPECT_TRUE(queue.try_push(2)); EXPECT_TRUE(queue.try_push(3));
    EXPECT_FALSE(queue.try_push(4)); int value{};
    EXPECT_TRUE(queue.try_pop(value)); EXPECT_EQ(value, 1);
    EXPECT_TRUE(queue.try_push(4));
    EXPECT_TRUE(queue.try_pop(value)); EXPECT_EQ(value, 2);
    EXPECT_TRUE(queue.try_pop(value)); EXPECT_EQ(value, 3);
    EXPECT_TRUE(queue.try_pop(value)); EXPECT_EQ(value, 4);
    EXPECT_FALSE(queue.try_pop(value));
}
} // namespace loe
