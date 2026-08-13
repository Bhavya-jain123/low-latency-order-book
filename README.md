# Low-Latency C++ Limit Order Book & Matching Engine

A **C++20 backend simulation of an electronic trading system** that receives BUY and SELL limit orders, maintains an in-memory order book, matches compatible orders using **price-time priority**, and records executed trades.

The system is designed as a small, deterministic, offline environment for exploring the engineering problems behind **low-latency trading systems** — data structures, concurrency, memory management, correctness, and performance measurement.

> **This is an educational trading-system simulation. It does not connect to exchanges, brokers, live market data, or real-money trading systems.**

---

## What I Built

The system takes an incoming order and moves it through the following pipeline:

```text
          Order Input
               │
               ▼
      ┌─────────────────┐
      │ Matching Engine │
      └────────┬────────┘
               │
        ┌──────┴──────┐
        ▼             ▼
     BID BOOK      ASK BOOK
        │             │
        └──────┬──────┘
               ▼
        Trade Execution
               │
               ▼
         Trade History
```

The core idea is simple:

**If the best BUY price is high enough to meet the best SELL price, execute a trade.**

The engine continues matching until the incoming order is fully filled or no compatible order remains.

---

## Key Features

### 📖 Limit Order Book

Maintains separate **BID** and **ASK** books with ordered price levels.

### ⚡ Price-Time Matching

Orders are matched using:

* **Price priority** — better prices execute first.
* **Time priority** — orders at the same price execute FIFO.

### 🔄 Partial Fills

Orders do not need to match completely. Remaining quantities stay active in the book.

### ❌ Cancellation

Active orders can be cancelled by Order ID without scanning the entire book.

### 💱 Trade Recording

Every execution records the participating orders, execution price, quantity, and timestamp.

### 🔀 Single-Writer Matching Architecture

The matching engine owns the order book, keeping the core market state under a single consumer thread.

### 🚀 SPSC Communication

An advanced **Single-Producer/Single-Consumer ring buffer** separates order ingestion from the matching engine.

### 🧠 Memory Reuse

A fixed-size object-pool approach is included as an advanced optimization for reusing order objects.

### 📊 Performance Benchmarking

Synthetic workloads can be used to measure **throughput and latency**, including P50/P95/P99 latency, under different market conditions.

---

## Example

The engine can be controlled directly from the terminal:

```text
> BUY 1001 100.00 500
ACCEPTED

> SELL 2001 101.00 200
ACCEPTED

> SELL 2002 99.00 300
ACCEPTED

> BOOK
BIDS
100.00 qty=200 orders=1

ASKS
101.00 qty=200 orders=1

> TRADES
1 buy=1001 sell=2002 price=99.00 qty=300
```

Here, the BUY at `100.00` matches the SELL at `99.00` because:

```text
100.00 >= 99.00
```

A trade for `300` units is executed at the simulator's **resting-order price**, leaving `200` units of the BUY order in the book.

---

## Technical Design

The order book is built around three important structures:

```text
Ordered price levels
        +
FIFO orders at each price
        +
Order-ID index
```

This provides:

* ordered access to the best bid/ask,
* FIFO price-time priority,
* efficient average-case lookup of active orders during cancellation.

The matching engine is intentionally kept separate from the CLI, benchmarking, and ingestion components so that the **core matching logic remains deterministic and independently testable**.

---

## Performance & Concurrency

For larger synthetic workloads, the architecture becomes:

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
     Matching Engine
          │
          ▼
       Order Book
          │
          ▼
    Trade / Metrics
```

The benchmark framework supports reproducible workloads ranging from thousands to millions of orders and measures actual results rather than assuming a particular performance number.

---

## Testing

Correctness is tested around the behavior that matters most:

* price priority
* FIFO ordering
* full and partial fills
* cancellation
* trade generation
* order-index consistency
* queue behavior
* large synthetic workloads

Run the test suite with:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

---

## Benchmarking

For performance experiments, build in Release mode:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel
```

A benchmark can then generate synthetic orders and export measured results to CSV.

The project is intended to investigate questions such as:

* How many orders can the engine process?
* What is the average latency?
* What do P50/P95/P99 latency look like?
* How does workload shape affect performance?
* Does a concurrency or memory optimization actually help?

**All reported performance results should come from real benchmark runs.**

---

## Tech Stack

**C++20** · **CMake** · **GoogleTest** · **`std::atomic`** · **`std::map`** · **`std::unordered_map`** · **SPSC Ring Buffer** · **Synthetic Workloads**


---
## Repository Structure

```text
low-latency-order-book/
│
├── benchmarks/
├── docs/
│   ├── architecture.md
│   ├── benchmarking.md
│   └── testing.md
│
├── include/
├── scripts/
├── src/
├── tests/
│
├── .gitignore
├── CMakeLists.txt
├── LICENSE
└── README.md

```
---

## Scope

This project focuses on the engineering core of a trading system:

**order matching + data structures + concurrency + memory management + performance measurement.**

The system is an **offline educational simulation** that processes deterministic synthetic orders. It does not connect to live markets, brokers, or exchanges.

For deeper design and implementation details, see:

* [`docs/architecture.md`](docs/architecture.md)
* [`docs/benchmarking.md`](docs/benchmarking.md)
* [`docs/testing.md`](docs/testing.md)
