# Testing

The project uses **GoogleTest** to validate both the core matching logic and the supporting components.

## What is Tested

The test suite covers:

- Order validation and ID handling
- Price priority and FIFO time priority
- Full and partial fills
- Cancellation and trade generation
- Order-pool exhaustion
- Bounded SPSC queue behavior
- Deterministic synthetic-order generation
- Latency-statistic ordering
- A 100,000-order integration/stress test

## Invariant Validation

The order book provides `validate_invariants()` for debug and test builds.

It checks:

- Valid price levels and quantities
- Correct side/price information
- FIFO sequence
- Consistency between the book and Order-ID index
- Matching entry counts

The matching tests also verify that trades occur only at compatible prices and that the simulator uses the **resting order's price**.

## Running Tests

Configure and build a Debug version:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
