#include "orderflow/application/checkout_service.hpp"
#include "orderflow/application/order_query_service.hpp"
#include "orderflow/infrastructure/in_memory_order_repository.hpp"
#include "orderflow/infrastructure/stream_order_notifier.hpp"
#include "orderflow/strategies/discounts.hpp"
#include "orderflow/strategies/payments.hpp"
#include "orderflow/strategies/shipping.hpp"

#include <exception>
#include <iostream>
#include <string_view>
#include <vector>

namespace {

void printHelp(std::string_view executable) {
    std::cout << "OrderFlow - C++20 SOLID order-processing example\n\n"
              << "Usage:\n"
              << "  " << executable << " demo    Run a complete sample checkout\n"
              << "  " << executable << " --help  Show this help\n";
}

int runDemo() {
    using namespace orderflow;

    InMemoryOrderRepository orders;
    PercentageDiscountPolicy loyaltyDiscount{1'000}; // 10%
    FreeShippingOverPolicy shipping{Money::dollars(100), Money::fromCents(799)};
    SandboxCardProcessor payments{"tok_demo_approved"};
    StreamOrderNotifier notifier{std::cout};

    CheckoutService checkout{orders, loyaltyDiscount, shipping, payments, notifier};
    Order order{"ORD-1001",
                "customer@example.com",
                std::vector<OrderLine>{
                    OrderLine{"HEADSET-01", "Wireless Headset", Money::fromCents(8'999), 1},
                    OrderLine{"KEYBOARD-02", "Mechanical Keyboard", Money::fromCents(5'950), 1},
                }};

    const CheckoutReceipt receipt = checkout.checkout(std::move(order));
    std::cout << "\nCheckout receipt\n"
              << "  Order:    " << receipt.orderId << '\n'
              << "  Subtotal: " << receipt.pricing.subtotal.format() << '\n'
              << "  Discount: " << receipt.pricing.discount.format() << '\n'
              << "  Shipping: " << receipt.pricing.shipping.format() << '\n'
              << "  Total:    " << receipt.pricing.total.format() << '\n'
              << "  Payment:  " << receipt.paymentReference << '\n';

    const OrderQueryService queries{orders};
    std::cout << "  Stored paid orders: " << queries.paidOrders().size() << '\n';
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc == 2 && std::string_view{argv[1]} == "demo") {
            return runDemo();
        }
        if (argc == 2 && (std::string_view{argv[1]} == "--help" ||
                          std::string_view{argv[1]} == "-h")) {
            printHelp(argv[0]);
            return 0;
        }
        printHelp(argv[0]);
        return argc == 1 ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
