#include "orderflow/domain/money.hpp"

#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace orderflow {

Money::Money(std::int64_t cents) noexcept : cents_(cents) {}

Money Money::fromCents(std::int64_t cents) {
    if (cents < 0) {
        throw std::invalid_argument("Money cannot be negative");
    }
    return Money{cents};
}

Money Money::dollars(std::int64_t wholeDollars) {
    if (wholeDollars < 0 ||
        wholeDollars > std::numeric_limits<std::int64_t>::max() / 100) {
        throw std::invalid_argument("Dollar amount is outside the supported range");
    }
    return Money{wholeDollars * 100};
}

std::int64_t Money::cents() const noexcept {
    return cents_;
}

bool Money::isZero() const noexcept {
    return cents_ == 0;
}

std::string Money::format() const {
    std::ostringstream output;
    output << '$' << cents_ / 100 << '.' << std::setw(2) << std::setfill('0') << cents_ % 100;
    return output.str();
}

Money Money::percentage(int basisPoints) const {
    if (basisPoints < 0 || basisPoints > 10'000) {
        throw std::invalid_argument("Percentage must be between 0 and 10000 basis points");
    }
    if (basisPoints != 0 &&
        cents_ > (std::numeric_limits<std::int64_t>::max() - 5'000) / basisPoints) {
        throw std::overflow_error("Percentage calculation overflow");
    }
    return Money{(cents_ * basisPoints + 5'000) / 10'000};
}

Money Money::times(int quantity) const {
    if (quantity <= 0) {
        throw std::invalid_argument("Quantity must be positive");
    }
    if (cents_ > std::numeric_limits<std::int64_t>::max() / quantity) {
        throw std::overflow_error("Money multiplication overflow");
    }
    return Money{cents_ * quantity};
}

Money Money::operator+(Money other) const {
    if (cents_ > std::numeric_limits<std::int64_t>::max() - other.cents_) {
        throw std::overflow_error("Money addition overflow");
    }
    return Money{cents_ + other.cents_};
}

Money Money::operator-(Money other) const {
    if (other.cents_ > cents_) {
        throw std::invalid_argument("Money result cannot be negative");
    }
    return Money{cents_ - other.cents_};
}

Money& Money::operator+=(Money other) {
    *this = *this + other;
    return *this;
}

} // namespace orderflow
