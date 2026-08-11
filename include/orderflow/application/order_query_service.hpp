#pragma once

#include "orderflow/ports/order_repository.hpp"

#include <optional>
#include <string_view>
#include <vector>

namespace orderflow {

class OrderQueryService final {
public:
    explicit OrderQueryService(const OrderReader& orders) noexcept;

    [[nodiscard]] std::optional<Order> findById(std::string_view id) const;
    [[nodiscard]] std::vector<Order> paidOrders() const;

private:
    const OrderReader& orders_;
};

} // namespace orderflow
