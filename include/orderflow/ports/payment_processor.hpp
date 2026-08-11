#pragma once

#include "orderflow/domain/money.hpp"

#include <string>
#include <string_view>

namespace orderflow {

struct PaymentResult final {
    bool approved{};
    std::string transactionId;
    std::string message;
};

class PaymentProcessor {
public:
    virtual ~PaymentProcessor() = default;

    // Implementations must return a non-empty transaction ID when approved.
    [[nodiscard]] virtual PaymentResult charge(std::string_view orderId, Money amount) = 0;
};

} // namespace orderflow
