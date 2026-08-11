#include "orderflow/application/order_query_service.hpp"

#include <algorithm>

namespace orderflow {

OrderQueryService::OrderQueryService(const OrderReader& orders) noexcept : orders_(orders) {}

std::optional<Order> OrderQueryService::findById(std::string_view id) const {
    return orders_.findById(id);
}

std::vector<Order> OrderQueryService::paidOrders() const {
    std::vector<Order> orders = orders_.all();
    std::erase_if(orders, [](const Order& order) { return order.status() != OrderStatus::paid; });
    return orders;
}

} // namespace orderflow
