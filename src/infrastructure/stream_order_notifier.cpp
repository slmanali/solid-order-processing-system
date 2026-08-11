#include "orderflow/infrastructure/stream_order_notifier.hpp"

#include <ostream>
#include <stdexcept>

namespace orderflow {

StreamOrderNotifier::StreamOrderNotifier(std::ostream& output) noexcept : output_(output) {}

void StreamOrderNotifier::orderConfirmed(const Order& order) {
    if (!order.pricing().has_value()) {
        throw std::logic_error("Cannot confirm an order without pricing");
    }
    output_ << "Confirmation for " << order.customerEmail() << ": order " << order.id()
            << " paid " << order.pricing()->total.format() << " (reference "
            << order.paymentReference() << ")\n";
}

} // namespace orderflow
