#pragma once

#include "core/order.hpp"

namespace loe {
enum class CommandType : std::uint8_t { Submit, Cancel };

struct Command {
    CommandType type{CommandType::Submit};
    OrderRequest order{};
    OrderId cancel_id{};

    [[nodiscard]] static Command submit(OrderRequest request) {
        Command result{};
        result.type = CommandType::Submit;
        result.order = request;
        return result;
    }
    [[nodiscard]] static Command cancel(OrderId id) {
        Command result{};
        result.type = CommandType::Cancel;
        result.cancel_id = id;
        return result;
    }
};
} // namespace loe
