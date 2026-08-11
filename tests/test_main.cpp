#include "orderflow/application/checkout_service.hpp"
#include "orderflow/application/order_query_service.hpp"
#include "orderflow/infrastructure/in_memory_order_repository.hpp"
#include "orderflow/infrastructure/stream_order_notifier.hpp"
#include "orderflow/strategies/discounts.hpp"
#include "orderflow/strategies/payments.hpp"
#include "orderflow/strategies/shipping.hpp"

#include <concepts>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

int assertions{};
int failures{};

void check(bool condition, std::string_view expression, int line) {
    ++assertions;
    if (!condition) {
        ++failures;
        std::cerr << "FAIL line " << line << ": " << expression << '\n';
    }
}

#define CHECK(expression) check(static_cast<bool>(expression), #expression, __LINE__)

template <typename Exception, typename Callable>
void checkThrows(Callable&& callable, std::string_view expression, int line) {
    ++assertions;
    try {
        std::invoke(std::forward<Callable>(callable));
    } catch (const Exception&) {
        return;
    } catch (...) {
        ++failures;
        std::cerr << "FAIL line " << line << ": " << expression << " threw the wrong exception\n";
        return;
    }
    ++failures;
    std::cerr << "FAIL line " << line << ": " << expression << " did not throw\n";
}

#define CHECK_THROWS(exception, expression)                                                          \
    checkThrows<exception>([&] { static_cast<void>(expression); }, #expression, __LINE__)

orderflow::Order sampleOrder(std::string id = "ORD-1") {
    using namespace orderflow;
    return Order{std::move(id),
                 "buyer@example.com",
                 std::vector<OrderLine>{
                     OrderLine{"SKU-A", "Keyboard", Money::dollars(100), 1},
                     OrderLine{"SKU-B", "Mouse", Money::dollars(25), 2},
                 }};
}

class FixedDiscount final : public orderflow::DiscountPolicy {
public:
    explicit FixedDiscount(orderflow::Money amount) noexcept : amount_(amount) {}
    orderflow::Money discountFor(const orderflow::Order&) const override { return amount_; }

private:
    orderflow::Money amount_;
};

class FixedShipping final : public orderflow::ShippingPolicy {
public:
    explicit FixedShipping(orderflow::Money amount) noexcept : amount_(amount) {}
    orderflow::Money shippingCostFor(const orderflow::Order&, orderflow::Money) const override {
        return amount_;
    }

private:
    orderflow::Money amount_;
};

class RecordingPayment final : public orderflow::PaymentProcessor {
public:
    explicit RecordingPayment(bool approved) : approved_(approved) {}

    orderflow::PaymentResult charge(std::string_view orderId, orderflow::Money amount) override {
        called = true;
        charged = amount;
        return approved_ ? orderflow::PaymentResult{true, "tx-" + std::string{orderId}, "approved"}
                         : orderflow::PaymentResult{false, {}, "test decline"};
    }

    bool called{};
    orderflow::Money charged{};

private:
    bool approved_;
};

class RecordingWriter final : public orderflow::OrderWriter {
public:
    void save(const orderflow::Order& order) override { saved = order; }
    std::optional<orderflow::Order> saved;
};

class RecordingNotifier final : public orderflow::OrderNotifier {
public:
    void orderConfirmed(const orderflow::Order& order) override {
        called = true;
        orderId = order.id();
    }

    bool called{};
    std::string orderId;
};

class ReaderOnly final : public orderflow::OrderReader {
public:
    explicit ReaderOnly(orderflow::Order order) : order_(std::move(order)) {}

    std::optional<orderflow::Order> findById(std::string_view id) const override {
        return id == order_.id() ? std::optional<orderflow::Order>{order_} : std::nullopt;
    }
    std::vector<orderflow::Order> all() const override { return {order_}; }

private:
    orderflow::Order order_;
};

static_assert(std::derived_from<orderflow::InMemoryOrderRepository, orderflow::OrderReader>);
static_assert(std::derived_from<orderflow::InMemoryOrderRepository, orderflow::OrderWriter>);
static_assert(!std::derived_from<ReaderOnly, orderflow::OrderWriter>);

void testMoneyValueObject() {
    using orderflow::Money;
    CHECK(Money::fromCents(1'234).format() == "$12.34");
    CHECK(Money::dollars(2).times(3) == Money::dollars(6));
    CHECK(Money::fromCents(1'234).percentage(1'000) == Money::fromCents(123));
    CHECK(Money::dollars(10) + Money::fromCents(50) == Money::fromCents(1'050));
    CHECK(Money::dollars(10) - Money::dollars(3) == Money::dollars(7));
    CHECK_THROWS(std::invalid_argument, Money::fromCents(-1));
    CHECK_THROWS(std::invalid_argument, Money::dollars(1) - Money::dollars(2));
    CHECK_THROWS(std::invalid_argument, Money::dollars(1).percentage(10'001));
}

void testOrderInvariants() {
    using namespace orderflow;
    Order order = sampleOrder();
    CHECK(order.id() == "ORD-1");
    CHECK(order.status() == OrderStatus::pending);
    CHECK(order.subtotal() == Money::dollars(150));
    CHECK(!order.pricing().has_value());

    const PricingSummary pricing{
        Money::dollars(150), Money::dollars(10), Money::dollars(5), Money::dollars(145)};
    order.markPaid(pricing, "tx-1");
    CHECK(order.status() == OrderStatus::paid);
    CHECK(order.pricing() == pricing);
    CHECK(order.paymentReference() == "tx-1");
    CHECK_THROWS(std::logic_error, order.markPaid(pricing, "tx-2"));
    CHECK_THROWS(std::invalid_argument,
                 Order("", "buyer@example.com", {OrderLine{"A", "A", Money::dollars(1), 1}}));
    CHECK_THROWS(std::invalid_argument,
                 Order("O", "invalid-email", {OrderLine{"A", "A", Money::dollars(1), 1}}));
    CHECK_THROWS(std::invalid_argument, Order("O", "buyer@example.com", {}));
}

void testOpenClosedPolicies() {
    using namespace orderflow;
    const Order order = sampleOrder();
    const NoDiscountPolicy noDiscount;
    const PercentageDiscountPolicy loyalty{1'000};
    const FlatRateShippingPolicy flatRate{Money::fromCents(799)};
    const FreeShippingOverPolicy freeOver100{Money::dollars(100), Money::fromCents(799)};

    CHECK(noDiscount.discountFor(order).isZero());
    CHECK(loyalty.discountFor(order) == Money::dollars(15));
    CHECK(flatRate.shippingCostFor(order, Money::dollars(135)) == Money::fromCents(799));
    CHECK(freeOver100.shippingCostFor(order, Money::dollars(135)).isZero());
    CHECK(freeOver100.shippingCostFor(order, Money::dollars(90)) == Money::fromCents(799));
}

void testDependencyInjectedCheckout() {
    using namespace orderflow;
    RecordingWriter writer;
    FixedDiscount discount{Money::dollars(10)};
    FixedShipping shipping{Money::dollars(5)};
    RecordingPayment payment{true};
    RecordingNotifier notifier;
    CheckoutService service{writer, discount, shipping, payment, notifier};

    const CheckoutReceipt receipt = service.checkout(sampleOrder());
    CHECK(payment.called);
    CHECK(payment.charged == Money::dollars(145));
    CHECK(receipt.pricing.total == Money::dollars(145));
    CHECK(receipt.paymentReference == "tx-ORD-1");
    CHECK(writer.saved.has_value());
    CHECK(writer.saved->status() == OrderStatus::paid);
    CHECK(notifier.called);
    CHECK(notifier.orderId == "ORD-1");
}

void testDeclinedPaymentStopsSideEffects() {
    using namespace orderflow;
    RecordingWriter writer;
    FixedDiscount discount{Money{}};
    FixedShipping shipping{Money{}};
    RecordingPayment payment{false};
    RecordingNotifier notifier;
    CheckoutService service{writer, discount, shipping, payment, notifier};

    CHECK_THROWS(PaymentDeclinedError, service.checkout(sampleOrder()));
    CHECK(payment.called);
    CHECK(!writer.saved.has_value());
    CHECK(!notifier.called);
}

void verifyPaymentProcessorContract(orderflow::PaymentProcessor& processor,
                                    std::string_view orderId) {
    const orderflow::PaymentResult result =
        processor.charge(orderId, orderflow::Money::dollars(25));
    CHECK(result.approved);
    CHECK(!result.transactionId.empty());
}

void testLiskovSubstitution() {
    orderflow::SandboxCardProcessor card{"tok_approved"};
    orderflow::CashOnDeliveryProcessor cash;
    verifyPaymentProcessorContract(card, "CARD-ORDER");
    verifyPaymentProcessorContract(cash, "COD-ORDER");

    orderflow::SandboxCardProcessor declined{"declined"};
    const orderflow::PaymentResult result =
        declined.charge("DECLINED", orderflow::Money::dollars(25));
    CHECK(!result.approved);
    CHECK(result.transactionId.empty());
}

void testSegregatedReaderAndRepository() {
    using namespace orderflow;
    ReaderOnly reader{sampleOrder("READ-1")};
    const OrderQueryService readOnlyQueries{reader};
    CHECK(readOnlyQueries.findById("READ-1").has_value());
    CHECK(!readOnlyQueries.findById("missing").has_value());
    CHECK(readOnlyQueries.paidOrders().empty());

    InMemoryOrderRepository repository;
    repository.save(sampleOrder("STORED-1"));
    CHECK(repository.findById("STORED-1").has_value());
    CHECK(repository.all().size() == 1U);
    CHECK_THROWS(std::invalid_argument, repository.save(sampleOrder("STORED-1")));
}

void testStreamNotifier() {
    using namespace orderflow;
    Order order = sampleOrder("NOTICE-1");
    const Money subtotal = order.subtotal();
    order.markPaid(PricingSummary{subtotal, Money{}, Money{}, subtotal}, "tx-notice");
    std::ostringstream output;
    StreamOrderNotifier notifier{output};
    notifier.orderConfirmed(order);
    CHECK(output.str().find("NOTICE-1") != std::string::npos);
    CHECK(output.str().find("$150.00") != std::string::npos);
}

} // namespace

int main() {
    testMoneyValueObject();
    testOrderInvariants();
    testOpenClosedPolicies();
    testDependencyInjectedCheckout();
    testDeclinedPaymentStopsSideEffects();
    testLiskovSubstitution();
    testSegregatedReaderAndRepository();
    testStreamNotifier();

    if (failures == 0) {
        std::cout << "All " << assertions << " assertions passed.\n";
        return 0;
    }
    std::cerr << failures << " of " << assertions << " assertions failed.\n";
    return 1;
}
