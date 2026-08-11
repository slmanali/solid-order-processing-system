#pragma once

#include "orderflow/ports/shipping_policy.hpp"

namespace orderflow {

class FlatRateShippingPolicy final : public ShippingPolicy {
public:
    explicit FlatRateShippingPolicy(Money rate) noexcept;

    [[nodiscard]] Money shippingCostFor(const Order& order,
                                        Money discountedSubtotal) const override;

private:
    Money rate_;
};

class FreeShippingOverPolicy final : public ShippingPolicy {
public:
    FreeShippingOverPolicy(Money threshold, Money fallbackRate) noexcept;

    [[nodiscard]] Money shippingCostFor(const Order& order,
                                        Money discountedSubtotal) const override;

private:
    Money threshold_;
    Money fallbackRate_;
};

} // namespace orderflow
