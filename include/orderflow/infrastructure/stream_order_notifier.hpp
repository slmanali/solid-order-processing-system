#pragma once

#include "orderflow/ports/order_notifier.hpp"

#include <iosfwd>

namespace orderflow {

class StreamOrderNotifier final : public OrderNotifier {
public:
    explicit StreamOrderNotifier(std::ostream& output) noexcept;

    void orderConfirmed(const Order& order) override;

private:
    std::ostream& output_;
};

} // namespace orderflow
