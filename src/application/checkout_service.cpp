#include "orderflow/application/checkout_service.hpp"

#include <stdexcept>
#include <utility>

namespace orderflow {

PaymentDeclinedError::PaymentDeclinedError(const std::string& reason)
    : std::runtime_error("Payment declined: " + reason) {}

CheckoutService::CheckoutService(OrderWriter& orders,
                                 const DiscountPolicy& discounts,
                                 const ShippingPolicy& shipping,
                                 PaymentProcessor& payments,
                                 OrderNotifier& notifier) noexcept
    : orders_(orders),
      discounts_(discounts),
      shipping_(shipping),
      payments_(payments),
      notifier_(notifier) {}

CheckoutReceipt CheckoutService::checkout(Order order) {
    const Money subtotal = order.subtotal();
    const Money discount = discounts_.discountFor(order);
    if (discount > subtotal) {
        throw std::logic_error("A discount policy returned more than the order subtotal");
    }
    const Money discountedSubtotal = subtotal - discount;
    const Money shipping = shipping_.shippingCostFor(order, discountedSubtotal);
    const Money total = discountedSubtotal + shipping;

    PaymentResult payment = payments_.charge(order.id(), total);
    if (!payment.approved) {
        throw PaymentDeclinedError{payment.message.empty() ? "no reason provided" : payment.message};
    }
    if (payment.transactionId.empty()) {
        throw std::logic_error("An approved payment must include a transaction ID");
    }

    const PricingSummary pricing{subtotal, discount, shipping, total};
    order.markPaid(pricing, payment.transactionId);
    orders_.save(order);
    notifier_.orderConfirmed(order);

    return CheckoutReceipt{order.id(), pricing, std::move(payment.transactionId)};
}

} // namespace orderflow
