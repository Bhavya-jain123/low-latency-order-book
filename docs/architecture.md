# Architecture

## Data Flow

```text
Synthetic Order Generator
          │
          ▼
    Producer Thread
          │
          ▼
   SPSC Ring Buffer
          │
          ▼
    Consumer Thread
          │
          ▼
   Matching Engine
       ┌──┴──┐
       ▼     ▼
 Order Book  Trade History
```

The matching engine is the **single owner of the order book**. The CLI bypasses the queue and interacts directly with the engine for interactive use.

---

## Order Book

The book maintains separate **BID** and **ASK** sides using ordered price levels.

```text
std::map<Price, PriceLevel>
        │
        └── std::list<Order*>
```

* `std::map` keeps price levels ordered.
* `std::list` preserves FIFO time priority within a price level.
* `std::unordered_map<OrderId, Location>` provides average-case constant-time order lookup for cancellation.

Prices are stored as **integer ticks** rather than floating point.

---

## Matching Engine

For a BUY:

```text
match while buy_price >= best_ask
```

For a SELL:

```text
match while sell_price <= best_bid
```

Each execution uses:

```text
trade_quantity = min(incoming, resting)
```

The simulator uses the **resting order's price** as the execution price.

Remaining quantity is added back to the appropriate book.

---

## Concurrency

The ingestion path uses a bounded **SPSC (Single-Producer, Single-Consumer) ring buffer**.

```text
Producer → SPSC Queue → Matching Engine
```

The producer and consumer use atomic indices with **acquire/release ordering**. The queue applies backpressure when full rather than dropping orders.

The order book itself is not shared between threads, avoiding locking in the matching path.

---

## Memory Management

`OrderPool` preallocates reusable `Order` objects for active resting orders.

This reduces repeated heap allocation, but the system does **not** claim to be zero-allocation because other standard-library containers may still allocate.

---

## Complexity

| Operation                     | Complexity     |
| ----------------------------- | -------------- |
| Price-level insertion/removal | `O(log P)`     |
| Order-ID lookup               | Average `O(1)` |
| FIFO insertion                | `O(1)`         |
| Known-order removal           | `O(1)`         |

`P` represents the number of active price levels.

---

## Design Trade-offs

The implementation favors **correctness, deterministic behavior, and explainability** over a highly specialized production-market-data layout.

It currently uses:

* `std::map`
* `std::list`
* `std::unordered_map`
* Standard C++ atomics

This keeps the core architecture understandable while still allowing concurrency and performance experiments.
