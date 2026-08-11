#include "orderflow/domain/order.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace orderflow {
namespace {

bool isBlank(std::string_view value) {
    return value.empty() || std::ranges::all_of(value, [](unsigned char character) {
               return std::isspace(character) != 0;
           });
}

} // namespace

std::string_view toString(OrderStatus status) noexcept {
    switch (status) {
    case OrderStatus::pending:
        return "pending";
    case OrderStatus::paid:
        return "paid";
    }
    return "unknown";
}

OrderLine::OrderLine(std::string sku, std::string name, Money unitPrice, int quantity)
    : sku_(std::move(sku)), name_(std::move(name)), unitPrice_(unitPrice), quantity_(quantity) {
    if (isBlank(sku_)) {
        throw std::invalid_argument("SKU cannot be blank");
    }
    if (isBlank(name_)) {
        throw std::invalid_argument("Product name cannot be blank");
    }
    if (unitPrice_.isZero()) {
        throw std::invalid_argument("Unit price must be positive");
    }
    if (quantity_ <= 0) {
        throw std::invalid_argument("Quantity must be positive");
    }
}

const std::string& OrderLine::sku() const noexcept {
    return sku_;
}

const std::string& OrderLine::name() const noexcept {
    return name_;
}

Money OrderLine::unitPrice() const noexcept {
    return unitPrice_;
}

int OrderLine::quantity() const noexcept {
    return quantity_;
}

Money OrderLine::lineTotal() const {
    return unitPrice_.times(quantity_);
}

Order::Order(std::string id, std::string customerEmail, std::vector<OrderLine> lines)
    : id_(std::move(id)), customerEmail_(std::move(customerEmail)), lines_(std::move(lines)) {
    validateText(id_, "Order ID");
    validateText(customerEmail_, "Customer email");
    if (customerEmail_.find('@') == std::string::npos) {
        throw std::invalid_argument("Customer email must contain @");
    }
    if (lines_.empty()) {
        throw std::invalid_argument("An order must contain at least one line");
    }
}

const std::string& Order::id() const noexcept {
    return id_;
}

const std::string& Order::customerEmail() const noexcept {
    return customerEmail_;
}

const std::vector<OrderLine>& Order::lines() const noexcept {
    return lines_;
}

OrderStatus Order::status() const noexcept {
    return status_;
}

Money Order::subtotal() const {
    Money result;
    for (const OrderLine& line : lines_) {
        result += line.lineTotal();
    }
    return result;
}

const std::optional<PricingSummary>& Order::pricing() const noexcept {
    return pricing_;
}

const std::string& Order::paymentReference() const noexcept {
    return paymentReference_;
}

void Order::markPaid(PricingSummary pricing, std::string paymentReference) {
    if (status_ != OrderStatus::pending) {
        throw std::logic_error("Only a pending order can be marked as paid");
    }
    validateText(paymentReference, "Payment reference");
    const Money expectedSubtotal = subtotal();
    if (pricing.subtotal != expectedSubtotal) {
        throw std::invalid_argument("Pricing subtotal does not match the order");
    }
    if (pricing.discount > pricing.subtotal) {
        throw std::invalid_argument("Discount cannot exceed the subtotal");
    }
    const Money expectedTotal = pricing.subtotal - pricing.discount + pricing.shipping;
    if (pricing.total != expectedTotal) {
        throw std::invalid_argument("Pricing total is inconsistent");
    }
    pricing_ = pricing;
    paymentReference_ = std::move(paymentReference);
    status_ = OrderStatus::paid;
}

void Order::validateText(std::string_view value, std::string_view fieldName) {
    if (isBlank(value)) {
        throw std::invalid_argument(std::string{fieldName} + " cannot be blank");
    }
}

} // namespace orderflow
