#pragma once

#include "orderflow/ports/discount_policy.hpp"

namespace orderflow {

class NoDiscountPolicy final : public DiscountPolicy {
public:
    [[nodiscard]] Money discountFor(const Order& order) const override;
};

class PercentageDiscountPolicy final : public DiscountPolicy {
public:
    explicit PercentageDiscountPolicy(int basisPoints);

    [[nodiscard]] int basisPoints() const noexcept;
    [[nodiscard]] Money discountFor(const Order& order) const override;

private:
    int basisPoints_;
};

} // namespace orderflow
