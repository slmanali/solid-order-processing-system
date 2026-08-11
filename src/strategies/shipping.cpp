#include "orderflow/strategies/shipping.hpp"

namespace orderflow {

FlatRateShippingPolicy::FlatRateShippingPolicy(Money rate) noexcept : rate_(rate) {}

Money FlatRateShippingPolicy::shippingCostFor(const Order&, Money) const {
    return rate_;
}

FreeShippingOverPolicy::FreeShippingOverPolicy(Money threshold, Money fallbackRate) noexcept
    : threshold_(threshold), fallbackRate_(fallbackRate) {}

Money FreeShippingOverPolicy::shippingCostFor(const Order&, Money discountedSubtotal) const {
    return discountedSubtotal >= threshold_ ? Money{} : fallbackRate_;
}

} // namespace orderflow
