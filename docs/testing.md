# Testing strategy

GoogleTest covers order validity, price priority, FIFO at a price, full and partial fills, cancellation, trade records, active-ID behavior, pool exhaustion, bounded FIFO SPSC behaviour, deterministic generation, a 100,000-order stress/invariant run, and latency-statistic ordering.

The book exposes `validate_invariants()` for debug/test checks. It verifies non-empty levels, side/price/quantity consistency, FIFO arrival sequence, index-to-book membership in both directions, and matching cardinalities. The matching loop itself enforces compatible prices before trade creation; its tests assert resting-price execution and priority ordering.

Run `ctest --test-dir build --output-on-failure` after configuring a Debug build. The 100K synthetic test is deliberately an integration/stress test rather than a benchmark and makes no throughput assertion.
