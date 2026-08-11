#pragma once

#include "orderflow/ports/discount_policy.hpp"
#include "orderflow/ports/order_notifier.hpp"
#include "orderflow/ports/order_repository.hpp"
#include "orderflow/ports/payment_processor.hpp"
#include "orderflow/ports/shipping_policy.hpp"

#include <stdexcept>
#include <string>

namespace orderflow {

class PaymentDeclinedError final : public std::runtime_error {
public:
    explicit PaymentDeclinedError(const std::string& reason);
};

struct CheckoutReceipt final {
    std::string orderId;
    PricingSummary pricing;
    std::string paymentReference;
};

class CheckoutService final {
public:
    CheckoutService(OrderWriter& orders,
                    const DiscountPolicy& discounts,
                    const ShippingPolicy& shipping,
                    PaymentProcessor& payments,
                    OrderNotifier& notifier) noexcept;

    [[nodiscard]] CheckoutReceipt checkout(Order order);

private:
    OrderWriter& orders_;
    const DiscountPolicy& discounts_;
    const ShippingPolicy& shipping_;
    PaymentProcessor& payments_;
    OrderNotifier& notifier_;
};

} // namespace orderflow
