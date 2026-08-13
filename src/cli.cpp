#include <charconv>
#include <iostream>
#include <sstream>
#include <string>

#include "matching/matching_engine.hpp"
#include "utils/price.hpp"

namespace {
using namespace loe;
template <typename T> bool parse_unsigned(const std::string& text, T& value) {
    return std::from_chars(text.data(), text.data() + text.size(), value).ec == std::errc{};
}
void show_book(const MatchingEngine& engine) {
    const auto book = engine.book().snapshot();
    std::cout << "BIDS\n";
    for (const auto& level : book.bids) std::cout << format_price(level.price) << " qty=" << level.quantity << " orders=" << level.order_count << '\n';
    std::cout << "ASKS\n";
    for (const auto& level : book.asks) std::cout << format_price(level.price) << " qty=" << level.quantity << " orders=" << level.order_count << '\n';
}
void show_trades(const MatchingEngine& engine) {
    for (const auto& trade : engine.trades()) std::cout << trade.id << " buy=" << trade.buy_order_id << " sell=" << trade.sell_order_id << " price=" << format_price(trade.price) << " qty=" << trade.quantity << '\n';
}
} // namespace

int main() {
    using namespace loe;
    MatchingEngine engine;
    Timestamp timestamp{1};
    std::cout << "Offline limit order book. Type HELP for commands.\n";
    for (std::string line; std::cout << "> " && std::getline(std::cin, line); ++timestamp) {
        std::istringstream input(line); std::string command; input >> command;
        if (command == "QUIT" || command == "EXIT") break;
        if (command == "HELP") { std::cout << "BUY|SELL <id> <price> <quantity> | CANCEL <id> | BOOK | TRADES | QUIT\n"; continue; }
        if (command == "BOOK") { show_book(engine); continue; }
        if (command == "TRADES") { show_trades(engine); continue; }
        if (command == "CANCEL") {
            std::string id; if (!(input >> id)) { std::cout << "ERROR usage: CANCEL <id>\n"; continue; }
            OrderId parsed{}; if (!parse_unsigned(id, parsed)) { std::cout << "ERROR invalid id\n"; continue; }
            std::cout << (engine.process(Command::cancel(parsed)) ? "CANCELLED\n" : "REJECTED unknown/completed id\n"); continue;
        }
        if (command == "BUY" || command == "SELL") {
            std::string id, price, quantity; if (!(input >> id >> price >> quantity)) { std::cout << "ERROR usage: BUY|SELL <id> <price> <quantity>\n"; continue; }
            OrderId order_id{}; Quantity order_quantity{}; const auto parsed_price = parse_price(price);
            if (!parse_unsigned(id, order_id) || !parse_unsigned(quantity, order_quantity) || !parsed_price || order_quantity == 0) { std::cout << "ERROR invalid order\n"; continue; }
            const auto side = command == "BUY" ? Side::Buy : Side::Sell;
            const bool completed = engine.process(Command::submit({order_id, side, *parsed_price, order_quantity, timestamp}));
            std::cout << (completed ? "ACCEPTED\n" : "REJECTED (duplicate/invalid ID or pool exhaustion)\n"); continue;
        }
        std::cout << "ERROR unknown command\n";
    }
}
