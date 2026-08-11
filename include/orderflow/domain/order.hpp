#pragma once

#include "orderflow/domain/money.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace orderflow {

enum class OrderStatus {
    pending,
    paid,
};

[[nodiscard]] std::string_view toString(OrderStatus status) noexcept;

class OrderLine final {
public:
    OrderLine(std::string sku, std::string name, Money unitPrice, int quantity);

    [[nodiscard]] const std::string& sku() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] Money unitPrice() const noexcept;
    [[nodiscard]] int quantity() const noexcept;
    [[nodiscard]] Money lineTotal() const;

private:
    std::string sku_;
    std::string name_;
    Money unitPrice_;
    int quantity_;
};

struct PricingSummary final {
    Money subtotal;
    Money discount;
    Money shipping;
    Money total;

    auto operator<=>(const PricingSummary&) const = default;
};

class Order final {
public:
    Order(std::string id, std::string customerEmail, std::vector<OrderLine> lines);

    [[nodiscard]] const std::string& id() const noexcept;
    [[nodiscard]] const std::string& customerEmail() const noexcept;
    [[nodiscard]] const std::vector<OrderLine>& lines() const noexcept;
    [[nodiscard]] OrderStatus status() const noexcept;
    [[nodiscard]] Money subtotal() const;
    [[nodiscard]] const std::optional<PricingSummary>& pricing() const noexcept;
    [[nodiscard]] const std::string& paymentReference() const noexcept;

    void markPaid(PricingSummary pricing, std::string paymentReference);

private:
    static void validateText(std::string_view value, std::string_view fieldName);

    std::string id_;
    std::string customerEmail_;
    std::vector<OrderLine> lines_;
    OrderStatus status_{OrderStatus::pending};
    std::optional<PricingSummary> pricing_;
    std::string paymentReference_;
};

} // namespace orderflow
