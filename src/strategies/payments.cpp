#include "orderflow/strategies/payments.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace orderflow {
namespace {

void validateOrderId(std::string_view orderId) {
    const bool blank = orderId.empty() || std::ranges::all_of(orderId, [](unsigned char character) {
                           return std::isspace(character) != 0;
                       });
    if (blank) {
        throw std::invalid_argument("Order ID cannot be blank");
    }
}

} // namespace

SandboxCardProcessor::SandboxCardProcessor(std::string cardToken)
    : cardToken_(std::move(cardToken)) {
    if (cardToken_.empty()) {
        throw std::invalid_argument("Card token cannot be empty");
    }
}

PaymentResult SandboxCardProcessor::charge(std::string_view orderId, Money amount) {
    validateOrderId(orderId);
    if (amount.isZero()) {
        return PaymentResult{true, "card-free-" + std::string{orderId}, "No charge required"};
    }
    if (cardToken_ == "declined") {
        return PaymentResult{false, {}, "Sandbox card was declined"};
    }
    return PaymentResult{true, "card-" + std::string{orderId}, "Approved"};
}

PaymentResult CashOnDeliveryProcessor::charge(std::string_view orderId, Money) {
    validateOrderId(orderId);
    return PaymentResult{true, "cod-" + std::string{orderId}, "Pay on delivery"};
}

} // namespace orderflow
