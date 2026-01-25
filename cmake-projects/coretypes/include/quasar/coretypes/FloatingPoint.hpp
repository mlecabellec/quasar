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
  int toInt() const override { return static_cast<int>(value_); }
  long toLong() const override { return static_cast<long>(value_); }
  float toFloat() const override { return static_cast<float>(value_); }
  double toDouble() const override { return static_cast<double>(value_); }
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

  // Comparison
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
