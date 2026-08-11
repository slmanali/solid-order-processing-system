#pragma once

#include "orderflow/ports/order_repository.hpp"

#include <vector>

namespace orderflow {

class InMemoryOrderRepository final : public OrderReader, public OrderWriter {
public:
    void save(const Order& order) override;
    [[nodiscard]] std::optional<Order> findById(std::string_view id) const override;
    [[nodiscard]] std::vector<Order> all() const override;

private:
    std::vector<Order> orders_;
};

} // namespace orderflow
