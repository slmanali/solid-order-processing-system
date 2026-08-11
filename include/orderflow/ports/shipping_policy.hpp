#pragma once

#include "orderflow/domain/order.hpp"

namespace orderflow {

class ShippingPolicy {
public:
    virtual ~ShippingPolicy() = default;
    [[nodiscard]] virtual Money shippingCostFor(const Order& order,
                                                Money discountedSubtotal) const = 0;
};

} // namespace orderflow
