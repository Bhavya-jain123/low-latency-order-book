# Benchmarking methodology

`order_engine_bench` creates a seeded synthetic command sequence and sends it through one producer, a bounded SPSC queue, and one matching consumer. It reports commands processed, elapsed wall time, commands/second, engine counters, and min/average/P50/P90/P95/P99/max consumer processing time in nanoseconds. Percentiles are selected from sorted samples with a documented nearest-index calculation.

Run a reproducible workload:

```sh
./build-release/order_engine_bench --count 100000 --seed 42 --pattern mixed --cancel-probability 0.10 --csv results.csv
```

Supported patterns are `noncrossing`, `crossing`, `sameprice`, `mixed`, and `burst`. Counts of 10,000, 100,000, and 1,000,000 exercise the requested load scales. Results are intentionally not checked into the repository because they are machine-, compiler-, build-, and load-dependent; the CSV option emits actual results for each run.

Interpret latency carefully: per-command timing includes matching/book work and clock overhead, not queue wait. Use a Release build, report machine/compiler details with results, repeat runs, and do not compare results from different workload settings as if they were equivalent.
