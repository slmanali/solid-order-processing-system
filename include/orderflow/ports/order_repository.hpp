#pragma once

#include "orderflow/domain/order.hpp"

#include <optional>
#include <string_view>
#include <vector>

namespace orderflow {

class OrderReader {
public:
    virtual ~OrderReader() = default;
    [[nodiscard]] virtual std::optional<Order> findById(std::string_view id) const = 0;
    [[nodiscard]] virtual std::vector<Order> all() const = 0;
};

class OrderWriter {
public:
    virtual ~OrderWriter() = default;
    virtual void save(const Order& order) = 0;
};

} // namespace orderflow
