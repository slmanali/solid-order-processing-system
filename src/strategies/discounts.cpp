#include "orderflow/strategies/discounts.hpp"

#include <stdexcept>

namespace orderflow {

Money NoDiscountPolicy::discountFor(const Order&) const {
    return Money{};
}

PercentageDiscountPolicy::PercentageDiscountPolicy(int basisPoints) : basisPoints_(basisPoints) {
    if (basisPoints_ < 0 || basisPoints_ > 10'000) {
        throw std::invalid_argument("Discount must be between 0 and 10000 basis points");
    }
}

int PercentageDiscountPolicy::basisPoints() const noexcept {
    return basisPoints_;
}

Money PercentageDiscountPolicy::discountFor(const Order& order) const {
    return order.subtotal().percentage(basisPoints_);
}

} // namespace orderflow
