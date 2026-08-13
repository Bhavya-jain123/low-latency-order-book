# Architecture and design notes

## Data flow

```text
deterministic synthetic generator -> one producer -> bounded SPSC ring -> one consumer
                                                                      -> MatchingEngine
                                                                      -> LimitOrderBook + trade history
```

The order book is only mutated by the consumer/matching thread. The CLI uses the engine directly because it is an interactive single-threaded tool. This ownership rule removes the need for book locks and makes price-time behaviour deterministic for a given command sequence.

## Book representation and complexity

`std::map` stores bid price levels in descending order and ask price levels in ascending order. Thus the best price is `begin()` for both sides. Inserting/removing a price level is O(log P), where P is active price levels. Each level is a `std::list<Order*>`; appending and erasing using its stored iterator are O(1), and preserves FIFO time priority. The order-ID index is an `std::unordered_map<OrderId, Location>`; cancellation lookup is average-case constant time, not an absolute O(1) guarantee.

The index holds side, price, and the list iterator, so cancellation never scans a book. A separate submitted-ID set rejects ID reuse after an order has filled or been cancelled. Debug/test code calls `validate_invariants()` to cross-check the index, maps, empty levels, quantities, and sequence order.

## Matching

An incoming buy repeatedly consumes the first order at the lowest ask while `buy_price >= best_ask`; a sell does the symmetric operation against the highest bid. Quantity is `min(incoming_remaining, resting_remaining)`. The execution price is the **resting order's price**. A completely filled resting order is erased from list/map/index and returned to the pool. If residual incoming quantity remains, it becomes the last order in its price level.

This engine only supports limit orders. It uses integer price ticks rather than floating point; CLI input such as `100.25` is 10,025 ticks.

## SPSC queue and memory ordering

`SpscRingBuffer<T, Capacity>` is bounded and only supports exactly one producer and one consumer. Its usable capacity is `Capacity - 1`; one slot distinguishes full from empty. The producer writes data then publishes `write_index` with release ordering. The consumer observes that index with acquire ordering before reading data. The consumer releases its updated read index; the producer acquires it before overwriting a slot. Each side modifies only its own index, and indices are cache-line aligned to reduce false sharing.

`try_push` returns false when full and `try_pop` returns false when empty. The benchmark's producer applies explicit backpressure by yielding and retrying, so it does not silently lose commands. This is not an MPMC queue, a wait-free system, or a claim of universally superior performance.

MSVC reports C4324 when a type is deliberately padded because of the queue's 64-byte index alignment. The CMake target suppresses only that warning (`/wd4324`); the padding is intentional and documented here rather than being treated as an accidental layout defect.

## Memory pool

`OrderPool` preallocates a fixed vector of `Order` objects and keeps a free pointer stack. Only resting orders occupy pool slots; incoming orders that fully execute do not need a pooled object. If a partially unfilled incoming order cannot be rested because the pool is exhausted, completed fills remain valid and the residual is explicitly reported as pool-exhausted/rejected. The pool removes per-resting-order heap allocation but the project intentionally does **not** claim zero allocation: maps, hash tables, list nodes, trade history, and benchmark vectors can allocate.

## Limitations

- Single symbol, fixed-point price scale of 100 for the CLI.
- No amend/replace, market orders, persistence, recovery, risk controls, audit-grade clock, exchange protocol, or live connectivity.
- SPSC benchmark latency measures consumer `engine.process()` duration, not end-to-end queue wait or scheduler latency.
- `std::map`, `std::list`, and `std::unordered_map` prioritize transparent correctness over a cache-optimized production layout.
