# Project brief: extensible order-processing system

## Assignment

Design and implement an order-processing application for an online store. The system receives an
order, calculates a discount and shipping cost, collects payment, stores the completed order, and
sends a confirmation.

The important requirement is changeability. Business stakeholders expect new discount campaigns,
delivery rules, payment providers, databases, and notification channels. Adding one of these must
not require rewriting the checkout workflow.

## Functional requirements

1. An order contains a unique ID, customer email, and one or more product lines.
2. A product line contains a SKU, name, unit price, and positive quantity.
3. The system calculates subtotal, discount, shipping, and final total using fixed-point money.
4. A declined payment must not store the order or send a confirmation.
5. An approved payment must provide a transaction reference.
6. A successfully paid order is saved and then confirmed to the customer.
7. Users can query stored orders without receiving write capabilities.

## Design requirements

- Use C++20 and CMake.
- Demonstrate every SOLID principle in production code.
- Keep high-level application services independent of concrete infrastructure.
- Use small, client-specific interfaces.
- Make payment processors substitutable under one documented contract.
- Allow new policies and adapters without modifying the checkout service.
- Include automated positive, negative, and contract tests.
- Compile cleanly with strict GCC and Clang warnings.

## Acceptance criteria

- The sample application completes a checkout and prints a receipt.
- Tests verify money and order invariants.
- Tests verify discount and shipping policies.
- Tests prove that payment rejection prevents persistence and notification.
- At least two payment implementations pass the same contract test.
- A read-only query service can operate with an object that does not implement the write interface.
- CI configures, builds, tests, and runs the demo.
