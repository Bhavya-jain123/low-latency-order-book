# Benchmarking

The `order_engine_bench` executable generates a **deterministic synthetic workload** and processes it through the producer → SPSC queue → matching-engine pipeline.

## What is Measured

The benchmark reports:

* Total commands processed
* Elapsed time
* Commands per second
* Trade/cancellation counters
* Minimum, average, P50, P90, P95, P99, and maximum engine-processing latency

Latency is measured for the matching-engine processing step using nanoseconds.

## Running a Benchmark

Use a Release build:

```sh
./build-release/order_engine_bench \
  --count 100000 \
  --seed 42 \
  --pattern mixed \
  --cancel-probability 0.10 \
  --csv results.csv
```

The fixed seed makes a workload reproducible.

## Workloads

Supported patterns:

```text
noncrossing
crossing
sameprice
mixed
burst
```

The benchmark can be run at different scales, including:

```text
10,000
100,000
1,000,000 orders
```

## Interpreting Results

Latency measures the time spent processing a command by the matching engine. It **does not include time waiting in the SPSC queue**.

For meaningful comparisons:

* Use the same workload and seed.
* Use a Release build.
* Repeat runs.
* Record the machine and compiler configuration.

Benchmark results are not committed to the repository because they depend on the execution environment.
