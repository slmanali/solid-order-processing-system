#pragma once

#include "orderflow/ports/payment_processor.hpp"

#include <string>

namespace orderflow {

class SandboxCardProcessor final : public PaymentProcessor {
public:
    explicit SandboxCardProcessor(std::string cardToken);

    [[nodiscard]] PaymentResult charge(std::string_view orderId, Money amount) override;

private:
    std::string cardToken_;
};

class CashOnDeliveryProcessor final : public PaymentProcessor {
public:
    [[nodiscard]] PaymentResult charge(std::string_view orderId, Money amount) override;
};

} // namespace orderflow
