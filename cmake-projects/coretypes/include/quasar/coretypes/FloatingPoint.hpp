#ifndef QUASAR_CORETYPES_FLOATINGPOINT_HPP
#define QUASAR_CORETYPES_FLOATINGPOINT_HPP

#include "quasar/coretypes/Number.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace quasar {
namespace coretypes {

/**
 * @brief Templated Moving FloatingPoint class wrapping primitives.
 *
 * Supports float, double.
 * Immutable and thread-safe.
 */
template <typename T> class FloatingPoint : public Number {
  static_assert(std::is_floating_point<T>::value,
                "FloatingPoint class only supports floating point types");

public:
  /**
   * @brief Constructs a FloatingPoint object from a value.
   */
  explicit FloatingPoint(T value) : value_(value) {}

  /**
   * @brief Constructs from string.
   */
  explicit FloatingPoint(const std::string &s)
      : value_(parseFloatingPoint(s)) {}

  // Number interface implementation
  int toInt() const override {
    if (std::isnan(value_)) {
      throw std::runtime_error("Cannot convert NaN to int");
    }
    if (std::isinf(value_)) {
      throw std::overflow_error("Cannot convert Infinity to int");
    }
    // Safe range calculation
    constexpr double min_int =
        static_cast<double>(std::numeric_limits<int>::min());
    constexpr double max_int =
        static_cast<double>(std::numeric_limits<int>::max());

    // Check using double precision for both float and double T
    if (static_cast<double>(value_) < min_int ||
        static_cast<double>(value_) > max_int) {
      throw std::overflow_error("Floating point value out of integer range");
    }
    return static_cast<int>(value_);
  }

  long toLong() const override {
    if (std::isnan(value_)) {
      throw std::runtime_error("Cannot convert NaN to long");
    }
    if (std::isinf(value_)) {
      throw std::overflow_error("Cannot convert Infinity to long");
    }
    // Limits check
    // Note: double cannot represent full range of int64_t (long).
    // converting limits::max() to double rounds to 2^63 (approx) which is
    // slightly > max long. So 'value > limits::max()' logic works if value is
    // exactly 2^63 (overflow). If value is rounded 2^63 from user input, it
    // catches it.

    if (value_ > static_cast<T>(std::numeric_limits<long>::max()) ||
        value_ < static_cast<T>(std::numeric_limits<long>::min())) {
      throw std::overflow_error("Floating point value out of long range");
    }
    return static_cast<long>(value_);
  }

  float toFloat() const override {
    if constexpr (std::is_same<T, float>::value) {
      return value_;
    } else {
      // Check for overflow when casting double to float
      if (std::isinf(value_))
        return static_cast<float>(value_);
      if (std::abs(value_) > std::numeric_limits<float>::max()) {
        throw std::overflow_error("Value exceeds float range");
      }
      return static_cast<float>(value_);
    }
  }

  double toDouble() const override {
    // float to double is always safe
    return static_cast<double>(value_);
  }
  std::string toString() const override { return std::to_string(value_); }

  /**
   * @brief Returns the primitive value.
   */
  T value() const { return value_; }

  // Arithmetic
  FloatingPoint<T> add(const FloatingPoint<T> &other) const {
    return FloatingPoint<T>(value_ + other.value_);
  }
  FloatingPoint<T> subtract(const FloatingPoint<T> &other) const {
    return FloatingPoint<T>(value_ - other.value_);
  }
  FloatingPoint<T> multiply(const FloatingPoint<T> &other) const {
    return FloatingPoint<T>(value_ * other.value_);
  }
  FloatingPoint<T> divide(const FloatingPoint<T> &other) const {
    return FloatingPoint<T>(value_ / other.value_);
  }

  // Safe Arithmetic
  // Throw if Inf or NaN result.
  FloatingPoint<T> safeAdd(const FloatingPoint<T> &other) const {
    T res = value_ + other.value_;
    checkSafe(res);
    return FloatingPoint<T>(res);
  }

  FloatingPoint<T> safeSubtract(const FloatingPoint<T> &other) const {
    T res = value_ - other.value_;
    checkSafe(res);
    return FloatingPoint<T>(res);
  }

  FloatingPoint<T> safeMultiply(const FloatingPoint<T> &other) const {
    T res = value_ * other.value_;
    checkSafe(res);
    return FloatingPoint<T>(res);
  }

  FloatingPoint<T> safeDivide(const FloatingPoint<T> &other) const {
    if (other.value_ == 0.0) {
      throw std::runtime_error("Division by zero");
      // 0.0 division usually results in Inf, but safely should probably throw
      // runtime error or handle CheckSafe. Requirement says: "prevent unhandled
      // zero division" -> throw.
    }
    T res = value_ / other.value_;
    checkSafe(res);
    return FloatingPoint<T>(res);
  }

#include <memory>

  // --- Comparison (Virtual) ---
  int compareTo(const Number &other) const override {
    // General comparison via double
    double d1 = static_cast<double>(value_);
    double d2 = other.toDouble();
    // Handle NaN for total ordering
    if (std::isnan(d1) && std::isnan(d2))
      return 0;
    if (std::isnan(d1))
      return 1;
    if (std::isnan(d2))
      return -1;

    if (d1 < d2)
      return -1;
    if (d1 > d2)
      return 1;
    return 0;
  }

  bool equals(const Number &other) const override {
    if (std::isnan(value_) && std::isnan(other.toDouble()))
      return true; // Loose equality for NaNs?
    return compareTo(other) == 0;
  }

  // --- Arithmetic (Virtual) ---
  std::shared_ptr<Number> add(const Number &other) const override {
    return std::make_shared<FloatingPoint<T>>(value_ +
                                              static_cast<T>(other.toDouble()));
  }
  std::shared_ptr<Number> subtract(const Number &other) const override {
    return std::make_shared<FloatingPoint<T>>(value_ -
                                              static_cast<T>(other.toDouble()));
  }
  std::shared_ptr<Number> multiply(const Number &other) const override {
    return std::make_shared<FloatingPoint<T>>(value_ *
                                              static_cast<T>(other.toDouble()));
  }
  std::shared_ptr<Number> divide(const Number &other) const override {
    double d = other.toDouble();
    // Floating point division by zero usually Inf/NaN, but lets allow standard
    // behavior unless safe. Requirement: "Add methods for basic arithmetic".
    return std::make_shared<FloatingPoint<T>>(value_ / static_cast<T>(d));
  }

  // --- Safe Arithmetic (Virtual) ---
  std::shared_ptr<Number> safeAdd(const Number &other) const override {
    T res = value_ + static_cast<T>(other.toDouble());
    checkSafe(res);
    return std::make_shared<FloatingPoint<T>>(res);
  }
  std::shared_ptr<Number> safeSubtract(const Number &other) const override {
    T res = value_ - static_cast<T>(other.toDouble());
    checkSafe(res);
    return std::make_shared<FloatingPoint<T>>(res);
  }
  std::shared_ptr<Number> safeMultiply(const Number &other) const override {
    T res = value_ * static_cast<T>(other.toDouble());
    checkSafe(res);
    return std::make_shared<FloatingPoint<T>>(res);
  }
  std::shared_ptr<Number> safeDivide(const Number &other) const override {
    T d = static_cast<T>(other.toDouble());
    if (d == 0.0)
      throw std::runtime_error("Division by zero");
    T res = value_ / d;
    checkSafe(res);
    return std::make_shared<FloatingPoint<T>>(res);
  }

  // --- Bitwise (Dummy) ---
  std::shared_ptr<Number> bitwiseAnd(const Number &) const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }
  std::shared_ptr<Number> bitwiseOr(const Number &) const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }
  std::shared_ptr<Number> bitwiseXor(const Number &) const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }
  std::shared_ptr<Number> bitwiseNot() const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }
  std::shared_ptr<Number> bitwiseLeftShift(int) const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }
  std::shared_ptr<Number> bitwiseRightShift(int) const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }

  // --- Introspection ---
  std::string getType() const override { return "FloatingPoint"; }
  bool isIntegerType() const override { return false; }
  bool isSigned() const override { return true; }

  // Specific Comparison
  /**
   * @brief Compares with another FloatingPoint.
   * @param other The other FloatingPoint.
   * @return -1, 0, or 1.
   */
  int compareTo(const FloatingPoint<T> &other) const {
    if (value_ < other.value_)
      return -1;
    if (value_ > other.value_)
      return 1;
    // NaNs
    if (std::isnan(value_) && std::isnan(other.value_))
      return 0;
    if (std::isnan(value_))
      return 1; // Treat NaN as larger? Java behavior: NaN > POS_INF
    if (std::isnan(other.value_))
      return -1;
    return 0;
  }

  /**
   * @brief Checks equality.
   * @param other The other FloatingPoint.
   * @return true if equal.
   */
  bool equals(const FloatingPoint<T> &other) const {
    // Strict equality? or epsilon? Java Double.equals uses bit representation.
    // For C++, == is standard but handling NaN is tricky (NaN != NaN).
    if (std::isnan(value_) && std::isnan(other.value_))
      return true;
    return value_ == other.value_;
  }

  // String conversion
  // "methods shall support 10 base, or other arbitrary base"
  // C++ std::to_chars supports hex (base 16) for float.
  /**
   * @brief Converts val to string with optional radix (supports 16 for hex).
   * @param val The value.
   * @param radix The radix.
   * @return String representation.
   */
  static std::string toString(T val, int radix = 10) {
    if (radix == 16) {
      // Hex float format: %a
      std::stringstream ss;
      ss << std::hexfloat << val;
      return ss.str();
    }
    // Base 10 default
    return std::to_string(val);
  }

  /**
   * @brief Parses a floating point string.
   * @param s The string.
   * @return The parsed value.
   */
  static T parseFloatingPoint(const std::string &s) {
    if (s.empty())
      throw std::invalid_argument("Empty string");
    size_t idx = 0;
    try {
      T val;
      if constexpr (std::is_same<T, float>::value) {
        val = std::stof(s, &idx);
      } else {
        val = std::stod(s, &idx);
      }
      if (idx != s.length()) {
        throw std::invalid_argument("Not a full number");
      }
      return val;
    } catch (const std::exception &) {
      throw std::invalid_argument("Failed to parse floating point: " + s);
    }
  }

private:
  /**
   * @brief Checks if value is infinite or NaN and throws if so.
   */
  void checkSafe(T val) const {
    if (std::isinf(val))
      throw std::overflow_error("Floating point overflow (Infinity)");
    if (std::isnan(val))
      throw std::runtime_error("Floating point error (NaN)");
  }

  T value_;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_FLOATINGPOINT_HPP
