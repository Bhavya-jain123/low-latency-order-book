#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include "benchmark/benchmark_runner.hpp"
#include "benchmark/workload.hpp"

int main(int argc, char** argv) {
    using namespace loe;
    WorkloadConfig config{}; std::string csv_path;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--count" && i + 1 < argc) config.order_count = std::stoull(argv[++i]);
            else if (arg == "--seed" && i + 1 < argc) config.seed = std::stoull(argv[++i]);
            else if (arg == "--cancel-probability" && i + 1 < argc) config.cancellation_probability = std::stod(argv[++i]);
            else if (arg == "--pattern" && i + 1 < argc) config.pattern = parse_workload_pattern(argv[++i]);
            else if (arg == "--csv" && i + 1 < argc) csv_path = argv[++i];
            else if (arg == "--help") { std::cout << "--count N --seed N --pattern noncrossing|crossing|sameprice|mixed|burst --cancel-probability P --csv path\n"; return 0; }
            else throw std::invalid_argument("unknown or incomplete argument: " + arg);
        }
        const auto commands = generate_workload(config);
        const auto result = run_spsc_benchmark(commands, config.order_count + 1);
        std::cout << std::fixed << std::setprecision(2)
                  << "commands=" << result.commands << " elapsed_seconds=" << result.elapsed_seconds
                  << " commands_per_second=" << result.commands_per_second << '\n'
                  << "trades=" << result.engine_metrics.trades << " traded_quantity=" << result.engine_metrics.traded_quantity
                  << " cancelled=" << result.engine_metrics.cancelled_orders << " rejected=" << result.engine_metrics.rejected_orders
                  << " pool_exhausted_remainders=" << result.engine_metrics.pool_exhausted_remainders << '\n'
                  << "latency_ns min=" << result.latency.minimum_ns << " avg=" << result.latency.average_ns
                  << " p50=" << result.latency.p50_ns << " p90=" << result.latency.p90_ns << " p95=" << result.latency.p95_ns
                  << " p99=" << result.latency.p99_ns << " max=" << result.latency.maximum_ns << '\n';
        if (!csv_path.empty()) {
            std::ofstream out(csv_path);
            if (!out) throw std::runtime_error("unable to write CSV");
            out << "commands,elapsed_seconds,commands_per_second,trades,traded_quantity,cancelled,rejected,pool_exhausted_remainders,min_ns,avg_ns,p50_ns,p90_ns,p95_ns,p99_ns,max_ns\n";
            out << result.commands << ',' << result.elapsed_seconds << ',' << result.commands_per_second << ',' << result.engine_metrics.trades << ',' << result.engine_metrics.traded_quantity << ',' << result.engine_metrics.cancelled_orders << ',' << result.engine_metrics.rejected_orders << ',' << result.engine_metrics.pool_exhausted_remainders << ',' << result.latency.minimum_ns << ',' << result.latency.average_ns << ',' << result.latency.p50_ns << ',' << result.latency.p90_ns << ',' << result.latency.p95_ns << ',' << result.latency.p99_ns << ',' << result.latency.maximum_ns << '\n';
        }
    } catch (const std::exception& error) { std::cerr << "benchmark error: " << error.what() << '\n'; return 1; }
}
