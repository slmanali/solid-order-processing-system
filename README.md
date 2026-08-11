# OrderFlow — SOLID Order Processing in C++

[![CI](https://github.com/slmanali/solid-order-processing-system/actions/workflows/ci.yml/badge.svg)](https://github.com/slmanali/solid-order-processing-system/actions/workflows/ci.yml)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C.svg)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

OrderFlow is a dependency-free C++20 application that processes customer orders through
discount, shipping, payment, persistence, and notification stages. Its architecture is an
intentional, testable demonstration of all five SOLID principles—not five disconnected toy
examples.

## The problem

An online store needs a checkout system that can evolve without repeatedly rewriting its core
workflow. It must support new discounts, delivery rules, payment providers, storage backends,
and notification channels while keeping business rules testable and independent of infrastructure.

OrderFlow solves this by keeping the domain at the center and connecting replaceable policies and
adapters through small interfaces.

## Features

- Validated `Money`, `OrderLine`, and `Order` domain objects.
- Fixed-point monetary calculations with no floating-point rounding errors.
- Percentage and no-discount policies.
- Flat-rate and threshold-based free-shipping policies.
- Sandbox card and cash-on-delivery payment processors.
- In-memory repository implementing segregated read/write interfaces.
- Stream-based confirmation adapter.
- Constructor-injected checkout orchestration.
- Dependency-free automated tests, including payment-provider contract tests.
- GCC and Clang CI builds with warnings treated as errors.

## Where each SOLID principle appears

| Principle | Concrete implementation |
|---|---|
| **S — Single Responsibility** | `Order` protects order state, `CheckoutService` orchestrates checkout, policies calculate one concern, repositories store orders, and notifiers communicate results. |
| **O — Open/Closed** | New `DiscountPolicy`, `ShippingPolicy`, `PaymentProcessor`, or `OrderNotifier` implementations can be added without modifying `CheckoutService`. |
| **L — Liskov Substitution** | `SandboxCardProcessor` and `CashOnDeliveryProcessor` satisfy the same `PaymentProcessor` contract and pass the same contract test. |
| **I — Interface Segregation** | `OrderReader` and `OrderWriter` are separate. `OrderQueryService` depends only on reading; `CheckoutService` depends only on writing. |
| **D — Dependency Inversion** | High-level services receive abstract ports through their constructors. Concrete strategies and adapters are selected only in the CLI composition root. |

See [docs/architecture.md](docs/architecture.md) for the dependency diagram and a deeper design
walkthrough.

## Build and test

Requirements:

- CMake 3.20+
- A C++20 compiler: GCC 11+, Clang 14+, or recent MSVC

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

To apply the same strict compilation policy used by CI:

```bash
cmake -S . -B build -DORDERFLOW_WARNINGS_AS_ERRORS=ON
```

## Run the demo

```bash
./build/orderflow demo
```

Example output:

```text
Confirmation for customer@example.com: order ORD-1001 paid $134.54 (reference card-ORD-1001)

Checkout receipt
  Order:    ORD-1001
  Subtotal: $149.49
  Discount: $14.95
  Shipping: $0.00
  Total:    $134.54
  Payment:  card-ORD-1001
  Stored paid orders: 1
```

## Architecture at a glance

```text
include/orderflow/
├── domain/          # Business entities and value objects
├── ports/           # Small interfaces owned by the application core
├── application/     # Checkout and query use cases
├── strategies/      # Replaceable business policies
└── infrastructure/  # Storage and output adapters
```

Dependencies point toward the domain and ports. `app/main.cpp` is the composition root: it is the
only place that chooses concrete implementations and wires them together.

## Extension examples

To add a coupon system, implement `DiscountPolicy` and inject it into `CheckoutService`. To add
Stripe, a bank gateway, or another provider, implement `PaymentProcessor`. To persist orders in
SQLite, implement `OrderReader` and `OrderWriter`. None of those changes require editing the
checkout algorithm.

The repository includes a detailed [project brief](docs/project-brief.md) that can be used as an
interview assignment or public programming exercise.

## Project structure

```text
.
├── .github/workflows/ci.yml
├── app/main.cpp
├── docs/
├── include/orderflow/
├── src/
├── tests/test_main.cpp
└── CMakeLists.txt
```

## License

Released under the [MIT License](LICENSE).
