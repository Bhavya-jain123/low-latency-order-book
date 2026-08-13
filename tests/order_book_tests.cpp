#include <gtest/gtest.h>

#include "matching/matching_engine.hpp"

namespace loe {
TEST(OrderBook, UsesPriceThenFifoPriority) {
    MatchingEngine engine(10);
    EXPECT_TRUE(engine.process(Command::submit({1, Side::Sell, 10100, 5, 1})));
    EXPECT_TRUE(engine.process(Command::submit({2, Side::Sell, 10000, 5, 2})));
    EXPECT_TRUE(engine.process(Command::submit({3, Side::Sell, 10000, 5, 3})));
    EXPECT_TRUE(engine.process(Command::submit({4, Side::Buy, 10200, 12, 4})));
    ASSERT_EQ(engine.trades().size(), 3U);
    EXPECT_EQ(engine.trades()[0].sell_order_id, 2U);
    EXPECT_EQ(engine.trades()[1].sell_order_id, 3U);
    EXPECT_EQ(engine.trades()[2].sell_order_id, 1U);
    EXPECT_EQ(engine.trades()[0].price, 10000);
    EXPECT_TRUE(engine.validate_invariants());
}
TEST(OrderBook, PreservesPartialFillOnBook) {
    MatchingEngine engine(10);
    engine.process(Command::submit({1, Side::Sell, 10000, 10, 1}));
    engine.process(Command::submit({2, Side::Buy, 10000, 4, 2}));
    ASSERT_EQ(engine.trades().size(), 1U);
    const auto snapshot = engine.book().snapshot();
    ASSERT_EQ(snapshot.asks.size(), 1U);
    EXPECT_EQ(snapshot.asks[0].quantity, 6U);
    EXPECT_TRUE(engine.book().contains(1));
    EXPECT_FALSE(engine.book().contains(2));
}
TEST(OrderBook, CancellingOutstandingOrderRemovesIt) {
    MatchingEngine engine(10);
    engine.process(Command::submit({1, Side::Buy, 10000, 10, 1}));
    EXPECT_TRUE(engine.process(Command::cancel(1)));
    EXPECT_FALSE(engine.process(Command::cancel(1)));
    EXPECT_FALSE(engine.book().contains(1));
    EXPECT_TRUE(engine.book().snapshot().bids.empty());
    EXPECT_TRUE(engine.validate_invariants());
}
TEST(OrderBook, RejectsDuplicateActiveAndInvalidOrders) {
    MatchingEngine engine(10);
    EXPECT_TRUE(engine.process(Command::submit({1, Side::Buy, 100, 1, 1})));
    EXPECT_FALSE(engine.process(Command::submit({1, Side::Sell, 100, 1, 2})));
    EXPECT_FALSE(engine.process(Command::submit({0, Side::Buy, 100, 1, 3})));
    EXPECT_FALSE(engine.process(Command::submit({2, Side::Buy, 100, 0, 4})));
}
TEST(OrderBook, DoesNotReuseAnOrderIdAfterItIsFilled) {
    MatchingEngine engine(10);
    EXPECT_TRUE(engine.process(Command::submit({1, Side::Sell, 100, 1, 1})));
    EXPECT_TRUE(engine.process(Command::submit({2, Side::Buy, 100, 1, 2})));
    EXPECT_FALSE(engine.process(Command::submit({1, Side::Buy, 99, 1, 3})));
}
TEST(OrderBook, ReportsPoolExhaustionWithoutCorruptingBook) {
    MatchingEngine engine(1);
    EXPECT_TRUE(engine.process(Command::submit({1, Side::Buy, 100, 1, 1})));
    EXPECT_FALSE(engine.process(Command::submit({2, Side::Buy, 99, 1, 2})));
    EXPECT_TRUE(engine.book().contains(1));
    EXPECT_FALSE(engine.book().contains(2));
    EXPECT_TRUE(engine.validate_invariants());
}
} // namespace loe
