#include "orderflow/infrastructure/in_memory_order_repository.hpp"

#include <algorithm>
#include <stdexcept>

namespace orderflow {

void InMemoryOrderRepository::save(const Order& order) {
    if (findById(order.id()).has_value()) {
        throw std::invalid_argument("Order ID already exists: " + order.id());
    }
    orders_.push_back(order);
}

std::optional<Order> InMemoryOrderRepository::findById(std::string_view id) const {
    const auto iterator = std::ranges::find_if(
        orders_, [id](const Order& order) { return order.id() == id; });
    if (iterator == orders_.end()) {
        return std::nullopt;
    }
    return *iterator;
}

std::vector<Order> InMemoryOrderRepository::all() const {
    return orders_;
}

} // namespace orderflow
