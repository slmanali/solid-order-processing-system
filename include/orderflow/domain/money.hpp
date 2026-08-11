#pragma once

#include <compare>
#include <cstdint>
#include <string>

namespace orderflow {

class Money final {
public:
    Money() = default;

    [[nodiscard]] static Money fromCents(std::int64_t cents);
    [[nodiscard]] static Money dollars(std::int64_t wholeDollars);

    [[nodiscard]] std::int64_t cents() const noexcept;
    [[nodiscard]] bool isZero() const noexcept;
    [[nodiscard]] std::string format() const;
    [[nodiscard]] Money percentage(int basisPoints) const;
    [[nodiscard]] Money times(int quantity) const;

    [[nodiscard]] Money operator+(Money other) const;
    [[nodiscard]] Money operator-(Money other) const;
    Money& operator+=(Money other);

    auto operator<=>(const Money&) const = default;

private:
    explicit Money(std::int64_t cents) noexcept;

    std::int64_t cents_{};
};

} // namespace orderflow
