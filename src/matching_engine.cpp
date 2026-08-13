#include "matching/matching_engine.hpp"

namespace loe {
bool MatchingEngine::process(const Command& command) {
    if (command.type == CommandType::Cancel) {
        const bool cancelled = book_.cancel(command.cancel_id);
        if (cancelled) ++metrics_.cancelled_orders;
        else ++metrics_.rejected_orders;
        return cancelled;
    }
    ++metrics_.submitted_orders;
    SubmitResult result = book_.submit(command.order);
    if (!result.accepted) { ++metrics_.rejected_orders; return false; }
    if (result.pool_exhausted) ++metrics_.pool_exhausted_remainders;
    for (const Trade& trade : result.trades) {
        ++metrics_.trades;
        metrics_.traded_quantity += trade.quantity;
        trades_.push_back(trade);
    }
    return !result.pool_exhausted;
}
} // namespace loe
