#ifndef QUASAR_CORETYPES_INTEGER_HPP
#define QUASAR_CORETYPES_INTEGER_HPP

#include "quasar/coretypes/Number.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace quasar {
namespace coretypes {

/**
 * @brief Templated Integer class wrapping a primitive integer type.
 *
 * Supports int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t,
 * uint64_t. Immutable and thread-safe.
 */
template <typename T> class Integer : public Number {
  static_assert(std::is_integral<T>::value,
                "Integer class only supports integral types");

public:
  /**
   * @brief Constructs an Integer object from a value.
   */
  explicit Integer(T value) : value_(value) {}

  /**
   * @brief Constructs from string with radix.
   */
  explicit Integer(const std::string &s, int radix = 10)
      : value_(parseInt(s, radix, 0)) {}

  // Number interface implementation
  int toInt() const override {
    if constexpr (std::numeric_limits<T>::max() >
                  std::numeric_limits<int>::max()) {
      if (value_ > static_cast<T>(std::numeric_limits<int>::max()))
        throw std::overflow_error("Integer overflow in toInt");
    }
    if constexpr (std::is_signed<T>::value &&
                  std::numeric_limits<T>::min() <
                      std::numeric_limits<int>::min()) {
      if (value_ < static_cast<T>(std::numeric_limits<int>::min()))
        throw std::overflow_error("Integer underflow in toInt");
    }
    // Handle Unsigned T to Signed Int conversion where value > INT_MAX
    // e.g. uint32 to int32. generic check above covers it if T::max > int::max
    // (uint32 max > int32 max) But check strictness. uint32::max (4G) >
    // int32::max (2G). Yes.
    return static_cast<int>(value_);
  }

  long toLong() const override {
    if constexpr (std::numeric_limits<T>::max() >
                  std::numeric_limits<long>::max()) {
      if (value_ > static_cast<T>(std::numeric_limits<long>::max()))
        throw std::overflow_error("Integer overflow in toLong");
    }
    if constexpr (std::is_signed<T>::value &&
                  std::numeric_limits<T>::min() <
                      std::numeric_limits<long>::min()) {
      if (value_ < static_cast<T>(std::numeric_limits<long>::min()))
        throw std::overflow_error("Integer underflow in toLong");
    }
    return static_cast<long>(value_);
  }

  float toFloat() const override {
    return static_cast<float>(value_);
  } // Precision loss allowed
  double toDouble() const override { return static_cast<double>(value_); }
  std::string toString() const override { return toString(value_, 10); }

  /**
   * @brief Returns the primitive value.
   */
  T value() const { return value_; }

  // Arithmetic
  Integer<T> add(const Integer<T> &other) const {
    return Integer<T>(value_ + other.value_);
  }
  Integer<T> subtract(const Integer<T> &other) const {
    return Integer<T>(value_ - other.value_);
  }
  Integer<T> multiply(const Integer<T> &other) const {
    return Integer<T>(value_ * other.value_);
  }
  Integer<T> divide(const Integer<T> &other) const {
    if (other.value_ == 0)
      throw std::runtime_error("Division by zero");
    return Integer<T>(value_ / other.value_);
  }

  // Safe Arithmetic
  Integer<T> safeAdd(const Integer<T> &other) const {
    T res;
    if (__builtin_add_overflow(value_, other.value_, &res))
      throw std::overflow_error("Overflow in add");
    return Integer<T>(res);
  }

  Integer<T> safeSubtract(const Integer<T> &other) const {
    T res;
    if (__builtin_sub_overflow(value_, other.value_, &res))
      throw std::overflow_error("Overflow in subtract");
    return Integer<T>(res);
  }

  Integer<T> safeMultiply(const Integer<T> &other) const {
    T res;
    if (__builtin_mul_overflow(value_, other.value_, &res))
      throw std::overflow_error("Overflow in multiply");
    return Integer<T>(res);
  }

  Integer<T> safeDivide(const Integer<T> &other) const {
    if (other.value_ == 0)
      throw std::runtime_error("Division by zero");
    // Handle signed min / -1 check
    if constexpr (std::is_signed<T>::value) {
      if (value_ == std::numeric_limits<T>::min() && other.value_ == -1) {
        throw std::overflow_error("Overflow in divide");
      }
    }
    return Integer<T>(value_ / other.value_);
  }

  // Comparison
  int compareTo(const Number &other) const override {
    // Try to compare as same type T
    // Simplification: convert both to double for generic comparison
    // This handles Int vs Float.
    // For Int vs Int (large), double might lose precision.
    // Optimization: if other is integer...
    double d1 = static_cast<double>(value_);
    double d2 = other.toDouble();
    if (d1 < d2)
      return -1;
    if (d1 > d2)
      return 1;
    return 0;
  }

  bool equals(const Number &other) const override {
    return compareTo(other) == 0;
  }

  // Existing method for exact type match (function hiding? No, overloading)
  // But wait, the existing compareTo takes 'const Integer<T>&'.
  // The new one takes 'const Number&'.
  // We can keep specific one for performance if used directly on Integer<T>.
  /**
   * @brief Compares with another Integer.
   * @param other The other Integer.
   * @return -1 if this < other, 1 if this > other, 0 if equal.
   */
  int compareTo(const Integer<T> &other) const {
    if (value_ < other.value_)
      return -1;
    if (value_ > other.value_)
      return 1;
    return 0;
  }

  /**
   * @brief Checks equality with another Integer.
   * @param other The other Integer.
   * @return true if equal.
   */
  bool equals(const Integer<T> &other) const { return value_ == other.value_; }

  // --- Arithmetic (Virtual from Number) ---
  std::shared_ptr<Number> add(const Number &other) const override {
    return std::make_shared<Integer<T>>(value_ +
                                        static_cast<T>(other.toDouble()));
  }
  std::shared_ptr<Number> subtract(const Number &other) const override {
    return std::make_shared<Integer<T>>(value_ -
                                        static_cast<T>(other.toDouble()));
  }
  std::shared_ptr<Number> multiply(const Number &other) const override {
    return std::make_shared<Integer<T>>(value_ *
                                        static_cast<T>(other.toDouble()));
  }
  std::shared_ptr<Number> divide(const Number &other) const override {
    double d = other.toDouble();
    if (d == 0.0)
      throw std::runtime_error("Division by zero");
    return std::make_shared<Integer<T>>(value_ / static_cast<T>(d));
  }

  // --- Safe Arithmetic (Virtual) ---
  std::shared_ptr<Number> safeAdd(const Number &other) const override {
    // DoubleT o = static_cast<DoubleT>(other.toDouble());
    // Using built-in check requires T.
    // Let's cast other to T.
    T otherVal = static_cast<T>(other.toDouble());
    T res;
    if (__builtin_add_overflow(value_, otherVal, &res))
      throw std::overflow_error("Overflow in safeAdd");
    return std::make_shared<Integer<T>>(res);
  }
  std::shared_ptr<Number> safeSubtract(const Number &other) const override {
    T otherVal = static_cast<T>(other.toDouble());
    T res;
    if (__builtin_sub_overflow(value_, otherVal, &res))
      throw std::overflow_error("Overflow in safeSubtract");
    return std::make_shared<Integer<T>>(res);
  }
  std::shared_ptr<Number> safeMultiply(const Number &other) const override {
    T otherVal = static_cast<T>(other.toDouble());
    T res;
    if (__builtin_mul_overflow(value_, otherVal, &res))
      throw std::overflow_error("Overflow in safeMultiply");
    return std::make_shared<Integer<T>>(res);
  }
  std::shared_ptr<Number> safeDivide(const Number &other) const override {
    T otherVal = static_cast<T>(other.toDouble());
    if (otherVal == 0)
      throw std::runtime_error("Division by zero");
    // Signed min check
    if constexpr (std::is_signed<T>::value) {
      if (value_ == std::numeric_limits<T>::min() && otherVal == -1) {
        throw std::overflow_error("Overflow in safeDivide");
      }
    }
    return std::make_shared<Integer<T>>(value_ / otherVal);
  }

  // --- Bitwise ---
  std::shared_ptr<Number> bitwiseAnd(const Number &other) const override {
    T otherVal = static_cast<T>(other.toLong());
    return std::make_shared<Integer<T>>(value_ & otherVal);
  }
  std::shared_ptr<Number> bitwiseOr(const Number &other) const override {
    T otherVal = static_cast<T>(other.toLong());
    return std::make_shared<Integer<T>>(value_ | otherVal);
  }
  std::shared_ptr<Number> bitwiseXor(const Number &other) const override {
    T otherVal = static_cast<T>(other.toLong());
    return std::make_shared<Integer<T>>(value_ ^ otherVal);
  }
  std::shared_ptr<Number> bitwiseNot() const override {
    return std::make_shared<Integer<T>>(~value_);
  }
  std::shared_ptr<Number> bitwiseLeftShift(int amount) const override {
    return std::make_shared<Integer<T>>(value_ << amount);
  }
  std::shared_ptr<Number> bitwiseRightShift(int amount) const override {
    return std::make_shared<Integer<T>>(value_ >> amount);
  }

  // --- Introspection ---
  std::string getType() const override { return "Integer"; }
  bool isIntegerType() const override { return true; }
  bool isSigned() const override { return std::is_signed<T>::value; }

  // Endianness
  /**
   * @brief Swaps bytes (endianness swap).
   * @return New Integer with swapped bytes.
   */
  Integer<T> swapBytes() const {
    T val = value_;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(&val);
    std::reverse(ptr, ptr + sizeof(T));
    return Integer<T>(val);
  }

  // Static Helpers
  /**
   * @brief Converts value to string with radix.
   * @param val The value.
   * @param radix The radix (2-36).
   * @return String representation.
   */
  static std::string toString(T val, int radix) {
    if (radix < 2 || radix > 36)
      radix = 10;
    if (radix == 10)
      return std::to_string(val);

    // Custom radix
    bool negative = (val < 0) && std::is_signed<T>::value;
    // unsigned equivalent for math
    using UT = typename std::make_unsigned<T>::type;
    UT u_val;
    if (negative) {
      // For signed min, -val is UB. Cast to unsigned first then negate
      // implementation defined/2's comp Just cast to unsigned.
      u_val = static_cast<UT>(val);
      // Wait. If val = -5. u_val = MAX - 4.
      // We want abs value.
      // safe abs:
      if (val == std::numeric_limits<T>::min()) {
        u_val = static_cast<UT>(val); // treat as 2's comp large num?
        // Wait. toString(-128, 16) -> "-80".
        // In Java, Integer.toString(-5, 16) -> "-5".
        // We need abs(val).
        // abs(INT_MIN) -> (unsigned)INT_MIN.
        u_val = static_cast<UT>(-(val + 1)) + 1;
      } else {
        u_val = static_cast<UT>(-val);
      }
    } else {
      u_val = static_cast<UT>(val);
    }

    std::string res;
    if (u_val == 0)
      return "0";
    while (u_val > 0) {
      UT digit = u_val % radix;
      char c = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
      res += c;
      u_val /= radix;
    }
    if (negative)
      res += '-';
    std::reverse(res.begin(), res.end());
    return res;
  }

  /**
   * @brief Parses a string to a primitive integer.
   * @param s The string.
   * @param radix The radix.
   * @param out_idx Optional pointer to update with parsed length.
   * @return The parsed value.
   * @throws std::invalid_argument if invalid format.
   * @throws std::out_of_range if value out of range.
   */
  static T parseInt(const std::string &s, int radix,
                    size_t *out_idx = nullptr) {
    if (s.empty())
      throw std::invalid_argument("Empty string");

    size_t local_idx = 0;
    size_t *idx_ptr = out_idx ? out_idx : &local_idx;

    try {
      if constexpr (std::is_signed<T>::value) {
        long long val = std::stoll(s, idx_ptr, radix);
        if (!out_idx && *idx_ptr != s.length()) {
          throw std::invalid_argument("Not a full number");
        }
        // Range check
        if (val < std::numeric_limits<T>::min() ||
            val > std::numeric_limits<T>::max()) {
          throw std::out_of_range("Value out of range for type");
        }
        return static_cast<T>(val);
      } else {
        unsigned long long val = std::stoull(s, idx_ptr, radix);
        if (!out_idx && *idx_ptr != s.length()) {
          throw std::invalid_argument("Not a full number");
        }
        if (val > std::numeric_limits<T>::max()) {
          throw std::out_of_range("Value out of range for type");
        }
        return static_cast<T>(val);
      }
    } catch (const std::exception &e) {
      // If it was our own throw, rethrow.
      // But stoll throws invalid_argument too.
      // We want to wrap or just let it propagate?
      // The catch block below wraps it.
      // But wait. If 'Not a full number' is thrown (invalid_argument),
      // the catch below catches it and throws "Failed to parse: ...".
      // That is fine. std::invalid_argument is what we want.
      throw std::invalid_argument("Failed to parse integer: " + s);
    }
  }

  /**
   * @brief Creates an Integer object from string.
   * @param s The string.
   * @param radix The radix.
   * @return The Integer object.
   */
  static Integer<T> valueOf(const std::string &s, int radix = 10) {
    return Integer<T>(parseInt(s, radix));
  }

private:
  using DoubleT = double;
  T value_;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_INTEGER_HPP
