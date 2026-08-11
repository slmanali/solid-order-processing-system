#pragma once

#include "orderflow/domain/order.hpp"

namespace orderflow {

class OrderNotifier {
public:
    virtual ~OrderNotifier() = default;
    virtual void orderConfirmed(const Order& order) = 0;
};

} // namespace orderflow
