#pragma once

#include "orderflow/domain/order.hpp"

namespace orderflow {

class DiscountPolicy {
public:
    virtual ~DiscountPolicy() = default;
    [[nodiscard]] virtual Money discountFor(const Order& order) const = 0;
};

} // namespace orderflow
