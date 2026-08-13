#pragma once

#include <charconv>
#include <cmath>
#include <optional>
#include <limits>
#include <string>
#include <string_view>

#include "core/types.hpp"

namespace loe {
[[nodiscard]] inline std::optional<Price> parse_price(std::string_view text) {
    if (text.empty() || text.front() == '-') return std::nullopt;
    const auto dot = text.find('.');
    const auto whole = text.substr(0, dot);
    std::uint64_t units{};
    if (whole.empty()) return std::nullopt;
    const auto whole_result = std::from_chars(whole.data(), whole.data() + whole.size(), units);
    if (whole_result.ec != std::errc{} || whole_result.ptr != whole.data() + whole.size()) return std::nullopt;
    std::uint64_t fraction{};
    if (dot != std::string_view::npos) {
        const auto decimals = text.substr(dot + 1);
        if (decimals.empty() || decimals.size() > 2) return std::nullopt;
        const auto decimal_result = std::from_chars(decimals.data(), decimals.data() + decimals.size(), fraction);
        if (decimal_result.ec != std::errc{} || decimal_result.ptr != decimals.data() + decimals.size()) return std::nullopt;
        if (decimals.size() == 1) fraction *= 10;
    }
    constexpr auto max = static_cast<std::uint64_t>(std::numeric_limits<Price>::max());
    if (units > (max - fraction) / 100) return std::nullopt;
    return static_cast<Price>(units * 100 + fraction);
}
[[nodiscard]] inline std::string format_price(Price ticks) {
    const auto units = ticks / 100; const auto fraction = ticks % 100;
    return std::to_string(units) + "." + (fraction < 10 ? "0" : "") + std::to_string(fraction);
}
} // namespace loe
