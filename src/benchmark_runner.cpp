#include "benchmark/benchmark_runner.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "concurrency/spsc_ring_buffer.hpp"

namespace loe {
BenchmarkResult run_spsc_benchmark(const std::vector<Command>& commands, std::size_t pool_capacity) {
    constexpr std::size_t queue_slots = 65'536;
    // Keep the multi-megabyte queue off the typical Windows thread stack.
    auto queue = std::make_unique<SpscRingBuffer<Command, queue_slots>>();
    MatchingEngine engine(pool_capacity);
    std::vector<std::uint64_t> latencies; latencies.reserve(commands.size());
    std::atomic<bool> producer_done{false};
    const auto started = std::chrono::steady_clock::now();
    std::thread producer([&] {
        for (const Command& command : commands) {
            while (!queue->try_push(command)) std::this_thread::yield(); // Backpressure: no commands are dropped.
        }
        producer_done.store(true, std::memory_order_release);
    });
    Command command{};
    while (!producer_done.load(std::memory_order_acquire) || !queue->empty()) {
        if (!queue->try_pop(command)) { std::this_thread::yield(); continue; }
        const auto before = std::chrono::steady_clock::now();
        engine.process(command);
        const auto after = std::chrono::steady_clock::now();
        latencies.push_back(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count()));
    }
    producer.join();
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    const auto latency = calculate_latency_statistics(std::move(latencies)).value_or(LatencyStatistics{});
    return BenchmarkResult{commands.size(), seconds, seconds > 0 ? commands.size() / seconds : 0.0, engine.metrics(), latency};
}
} // namespace loe
