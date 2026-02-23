/**
 * @file FloatingPoint.hpp
 * @brief Definition of the templated FloatingPoint class.
 */

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
 * @brief Templated FloatingPoint class wrapping primitive floating point types.
 *
 * This class provides an object-oriented wrapper for primitive types like float 
 * and double. It inherits from Number and implements polymorphic arithmetic, 
 * comparison, and conversion methods. 
 * 
 * **Compliance**:
 * - Fulfills [FE-0010.1] Provide, for each basic numeric type, a class which is assignable to its basic type.
 * - Fulfills [FE-0010.1.2] Each class related to a basic numeric type shall be derivated from a common "Number" base class.
 * - Fulfills [FE-0030.8] All methods are thread safe.
 * 
 * The class is immutable and thread-safe. It handles special floating-point 
 * values like NaN (Not-a-Number) and Infinity according to IEEE 754 standards,
 * while providing "safe" versions of arithmetic operations that throw exceptions
 * instead of returning these special values.
 *
 * @tparam T The underlying floating point primitive type (float or double).
 */
template <typename T> class FloatingPoint : public Number {
  static_assert(std::is_floating_point<T>::value,
                "FloatingPoint class only supports floating point types");

public:
  /**
   * @brief Constructs a FloatingPoint object from a primitive value.
   * 
   * Fulfills [FE-0010.1.1] Constructor which takes a value of the basic type.
   * 
   * @param value The primitive floating point value.
   */
  explicit FloatingPoint(T value) : value_(value) {}

  /**
   * @brief Constructs a FloatingPoint object by parsing a string.
   * 
   * Fulfills [FE-0010.1.7] Methods for decoding values from a string.
   * 
   * @param s The string containing the numeric value to parse.
   * @throws std::invalid_argument If the string cannot be parsed as a floating point number.
   */
  explicit FloatingPoint(const std::string &s)
      : value_(parseFloatingPoint(s)) {}

  /**
   * @brief Converts the floating point value to a 32-bit integer.
   * 
   * Truncates the fractional part. Performs bounds checking.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The integer representation.
   * @throws std::runtime_error If the value is NaN.
   * @throws std::overflow_error If the value is Infinite or outside the range of int.
   */
  int toInt() const override {
    // NaN (Not-a-Number) cannot be converted to a meaningful integer because
    // it represents an undefined or unrepresentable value.
    if (std::isnan(value_)) {
      throw std::runtime_error("Cannot convert NaN to int");
    }
    // Infinite values are outside the representable range of fixed-width integers.
    if (std::isinf(value_)) {
      throw std::overflow_error("Cannot convert Infinity to int");
    }
    
    // Bounds check using double precision to ensure the value fits in a 32-bit int.
    // We use numeric_limits to get the exact range of 'int' on the current platform.
    constexpr double min_int =
        static_cast<double>(std::numeric_limits<int>::min());
    constexpr double max_int =
        static_cast<double>(std::numeric_limits<int>::max());

    if (static_cast<double>(value_) < min_int ||
        static_cast<double>(value_) > max_int) {
      throw std::overflow_error("Floating point value out of integer range");
    }
    // Perform the actual truncation and cast.
    return static_cast<int>(value_);
  }

  /**
   * @brief Converts the floating point value to a 64-bit long integer.
   * 
   * Truncates the fractional part. Performs bounds checking.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The long representation.
   * @throws std::runtime_error If the value is NaN.
   * @throws std::overflow_error If the value is Infinite or outside the range of long.
   */
  long toLong() const override {
    if (std::isnan(value_)) {
      throw std::runtime_error("Cannot convert NaN to long");
    }
    if (std::isinf(value_)) {
      throw std::overflow_error("Cannot convert Infinity to long");
    }

    // Check if the value is within the representable range of a 64-bit long.
    if (value_ > static_cast<T>(std::numeric_limits<long>::max()) ||
        value_ < static_cast<T>(std::numeric_limits<long>::min())) {
      throw std::overflow_error("Floating point value out of long range");
    }
    return static_cast<long>(value_);
  }

  /**
   * @brief Converts the floating point value to a float.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The float representation.
   * @throws std::overflow_error If converting from double and the value exceeds float range.
   */
  float toFloat() const override {
    if constexpr (std::is_same<T, float>::value) {
      // If the underlying type is already float, return it directly.
      return value_;
    } else {
      // When converting from double to float, check if the value exceeds float's maximum capacity.
      if (std::isinf(value_))
        return static_cast<float>(value_);
      if (std::abs(value_) > std::numeric_limits<float>::max()) {
        throw std::overflow_error("Double value exceeds float range");
      }
      return static_cast<float>(value_);
    }
  }

  /**
   * @brief Converts the floating point value to a double.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The double representation.
   */
  double toDouble() const override {
    // Promotion from float to double is always safe and lossless.
    return static_cast<double>(value_);
  }
  
  /**
   * @brief Returns a string representation of the floating point value.
   * 
   * Fulfills [FE-0010.1.7] Methods for encoding values to a string.
   * 
   * @return A string representing the value.
   */
  std::string toString() const override { 
    // Uses standard conversion to string representation.
    return std::to_string(value_); 
  }

  /**
   * @brief Returns the primitive floating point value.
   * @return The value of type T.
   */
  T value() const { return value_; }

  /**
   * @brief Adds another FloatingPoint object of the same type.
   * 
   * Fulfills [FE-0010.1.3] Methods for addition.
   * 
   * @param other The value to add.
   * @return A new FloatingPoint instance containing the sum.
   */
  FloatingPoint<T> add(const FloatingPoint<T> &other) const {
    return FloatingPoint<T>(value_ + other.value_);
  }

  /**
   * @brief Subtracts another FloatingPoint object of the same type.
   * 
   * Fulfills [FE-0010.1.3] Methods for subtraction.
   * 
   * @param other The value to subtract.
   * @return A new FloatingPoint instance containing the difference.
   */
  FloatingPoint<T> subtract(const FloatingPoint<T> &other) const {
    return FloatingPoint<T>(value_ - other.value_);
  }

  /**
   * @brief Multiplies by another FloatingPoint object of the same type.
   * 
   * Fulfills [FE-0010.1.3] Methods for multiplication.
   * 
   * @param other The value to multiply by.
   * @return A new FloatingPoint instance containing the product.
   */
  FloatingPoint<T> multiply(const FloatingPoint<T> &other) const {
    return FloatingPoint<T>(value_ * other.value_);
  }

  /**
   * @brief Divides by another FloatingPoint object of the same type.
   * 
   * Fulfills [FE-0010.1.3] Methods for division.
   * 
   * @param other The value to divide by.
   * @return A new FloatingPoint instance containing the quotient.
   */
  FloatingPoint<T> divide(const FloatingPoint<T> &other) const {
    return FloatingPoint<T>(value_ / other.value_);
  }

  /**
   * @brief Adds another FloatingPoint object with safety checks.
   * 
   * Fulfills [FE-0010.1.4] Methods for safe addition.
   * 
   * @param other The value to add.
   * @return A new FloatingPoint instance containing the sum.
   * @throws std::overflow_error If the result is Infinite.
   * @throws std::runtime_error If the result is NaN.
   */
  FloatingPoint<T> safeAdd(const FloatingPoint<T> &other) const {
    T res = value_ + other.value_;
    checkSafe(res);
    return FloatingPoint<T>(res);
  }

  /**
   * @brief Subtracts another FloatingPoint object with safety checks.
   * 
   * Fulfills [FE-0010.1.4] Methods for safe subtraction.
   * 
   * @param other The value to subtract.
   * @return A new FloatingPoint instance containing the difference.
   * @throws std::overflow_error If the result is Infinite.
   * @throws std::runtime_error If the result is NaN.
   */
  FloatingPoint<T> safeSubtract(const FloatingPoint<T> &other) const {
    T res = value_ - other.value_;
    checkSafe(res);
    return FloatingPoint<T>(res);
  }

  /**
   * @brief Multiplies by another FloatingPoint object with safety checks.
   * 
   * Fulfills [FE-0010.1.4] Methods for safe multiplication.
   * 
   * @param other The value to multiply by.
   * @return A new FloatingPoint instance containing the product.
   * @throws std::overflow_error If the result is Infinite.
   * @throws std::runtime_error If the result is NaN.
   */
  FloatingPoint<T> safeMultiply(const FloatingPoint<T> &other) const {
    T res = value_ * other.value_;
    checkSafe(res);
    return FloatingPoint<T>(res);
  }

  /**
   * @brief Divides by another FloatingPoint object with safety checks.
   * 
   * Fulfills [FE-0010.1.4] Methods for safe division.
   * 
   * @param other The value to divide by.
   * @return A new FloatingPoint instance containing the quotient.
   * @throws std::runtime_error If division by zero occurs or if the result is NaN.
   * @throws std::overflow_error If the result is Infinite.
   */
  FloatingPoint<T> safeDivide(const FloatingPoint<T> &other) const {
    // Explicitly prevent division by exactly zero to avoid Inf/NaN results.
    if (other.value_ == 0.0) {
      throw std::runtime_error("Division by zero");
    }
    T res = value_ / other.value_;
    checkSafe(res);
    return FloatingPoint<T>(res);
  }

  /**
   * @brief Compares this FloatingPoint with another Number.
   * 
   * Fulfills [FE-0030.1.1] Methods for comparison with other Number objects.
   * 
   * Implements total ordering:
   * - -1 if this < other
   * - 0 if this == other
   * - 1 if this > other
   * NaNs are considered equal to each other and greater than any other value.
   * 
   * @param other The number to compare with.
   * @return Comparison result (-1, 0, 1).
   */
  int compareTo(const Number &other) const override {
    // Convert both to double for a common comparison base.
    double d1 = static_cast<double>(value_);
    double d2 = other.toDouble();
    
    // Total ordering: handle NaN cases explicitly.
    // Standard IEEE 754 comparisons with NaN always return false. 
    // For object-oriented collections and sorting, we need a consistent total order.
    // Following convention (like Java), NaN is considered greater than all other 
    // values including Positive Infinity.
    if (std::isnan(d1) && std::isnan(d2))
      return 0; // Both are NaN, treat as equal for ordering.
    if (std::isnan(d1))
      return 1; // This is NaN, other is not, so this is "greater".
    if (std::isnan(d2))
      return -1; // Other is NaN, this is not, so this is "smaller".

    // Standard comparison for finite and infinite numbers.
    if (d1 < d2)
      return -1;
    if (d1 > d2)
      return 1;
    return 0;
  }

  /**
   * @brief Checks equality with another Number.
   * 
   * Fulfills [FE-0010.1.3] Methods for comparison.
   * 
   * @param other The number to compare with.
   * @return true if both values represent the same numeric value (or both are NaN).
   */
  bool equals(const Number &other) const override {
    // Treat NaNs as equal for the purpose of object equality, even though 
    // primitive NaN != NaN according to IEEE 754.
    if (std::isnan(value_) && std::isnan(other.toDouble()))
      return true;
    return compareTo(other) == 0;
  }

  /**
   * @brief Polymorphic addition.
   * 
   * Fulfills [FE-0030.1.3] Methods for basic arithmetic operations.
   * 
   * @param other The number to add.
   * @return Shared pointer to a new FloatingPoint containing the sum.
   */
  std::shared_ptr<Number> add(const Number &other) const override {
    return std::make_shared<FloatingPoint<T>>(value_ +
                                              static_cast<T>(other.toDouble()));
  }

  /**
   * @brief Polymorphic subtraction.
   * 
   * Fulfills [FE-0030.1.3] Methods for basic arithmetic operations.
   * 
   * @param other The number to subtract.
   * @return Shared pointer to a new FloatingPoint containing the difference.
   */
  std::shared_ptr<Number> subtract(const Number &other) const override {
    return std::make_shared<FloatingPoint<T>>(value_ -
                                              static_cast<T>(other.toDouble()));
  }

  /**
   * @brief Polymorphic multiplication.
   * 
   * Fulfills [FE-0030.1.3] Methods for basic arithmetic operations.
   * 
   * @param other The number to multiply by.
   * @return Shared pointer to a new FloatingPoint containing the product.
   */
  std::shared_ptr<Number> multiply(const Number &other) const override {
    return std::make_shared<FloatingPoint<T>>(value_ *
                                              static_cast<T>(other.toDouble()));
  }

  /**
   * @brief Polymorphic division.
   * 
   * Fulfills [FE-0030.1.3] Methods for basic arithmetic operations.
   * 
   * @param other The number to divide by.
   * @return Shared pointer to a new FloatingPoint containing the quotient.
   */
  std::shared_ptr<Number> divide(const Number &other) const override {
    double d = other.toDouble();
    return std::make_shared<FloatingPoint<T>>(value_ / static_cast<T>(d));
  }

  /**
   * @brief Polymorphic safe addition.
   * 
   * Fulfills [FE-0030.1.4] Methods for safe arithmetic operations.
   * 
   * @param other The number to add.
   * @return Shared pointer to a new FloatingPoint.
   * @throws std::overflow_error If result is Infinite.
   * @throws std::runtime_error If result is NaN.
   */
  std::shared_ptr<Number> safeAdd(const Number &other) const override {
    T res = value_ + static_cast<T>(other.toDouble());
    checkSafe(res);
    return std::make_shared<FloatingPoint<T>>(res);
  }

  /**
   * @brief Polymorphic safe subtraction.
   * 
   * Fulfills [FE-0030.1.4] Methods for safe arithmetic operations.
   * 
   * @param other The number to subtract.
   * @return Shared pointer to a new FloatingPoint.
   */
  std::shared_ptr<Number> safeSubtract(const Number &other) const override {
    T res = value_ - static_cast<T>(other.toDouble());
    checkSafe(res);
    return std::make_shared<FloatingPoint<T>>(res);
  }

  /**
   * @brief Polymorphic safe multiplication.
   * 
   * Fulfills [FE-0030.1.4] Methods for safe arithmetic operations.
   * 
   * @param other The number to multiply by.
   * @return Shared pointer to a new FloatingPoint.
   */
  std::shared_ptr<Number> safeMultiply(const Number &other) const override {
    T res = value_ * static_cast<T>(other.toDouble());
    checkSafe(res);
    return std::make_shared<FloatingPoint<T>>(res);
  }

  /**
   * @brief Polymorphic safe division.
   * 
   * Fulfills [FE-0030.1.4] Methods for safe arithmetic operations.
   * 
   * @param other The number to divide by.
   * @return Shared pointer to a new FloatingPoint.
   * @throws std::runtime_error If division by zero occurs.
   */
  std::shared_ptr<Number> safeDivide(const Number &other) const override {
    T d = static_cast<T>(other.toDouble());
    if (d == 0.0)
      throw std::runtime_error("Division by zero");
    T res = value_ / d;
    checkSafe(res);
    return std::make_shared<FloatingPoint<T>>(res);
  }

  // --- Bitwise Operations (Unsupported for Floating Point) ---
  /**
   * @brief Performs a bitwise AND operation.
   * Floating point types do not support bitwise operations.
   * @throws std::runtime_error Always.
   */
  std::shared_ptr<Number> bitwiseAnd(const Number &) const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }

  /**
   * @brief Performs a bitwise OR operation.
   * Floating point types do not support bitwise operations.
   * @throws std::runtime_error Always.
   */
  std::shared_ptr<Number> bitwiseOr(const Number &) const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }

  /**
   * @brief Performs a bitwise XOR operation.
   * Floating point types do not support bitwise operations.
   * @throws std::runtime_error Always.
   */
  std::shared_ptr<Number> bitwiseXor(const Number &) const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }

  /**
   * @brief Performs a bitwise NOT operation.
   * Floating point types do not support bitwise operations.
   * @throws std::runtime_error Always.
   */
  std::shared_ptr<Number> bitwiseNot() const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }

  /**
   * @brief Performs a bitwise Left Shift operation.
   * Floating point types do not support bitwise operations.
   * @throws std::runtime_error Always.
   */
  std::shared_ptr<Number> bitwiseLeftShift(int) const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }

  /**
   * @brief Performs a bitwise Right Shift operation.
   * Floating point types do not support bitwise operations.
   * @throws std::runtime_error Always.
   */
  std::shared_ptr<Number> bitwiseRightShift(int) const override {
    throw std::runtime_error(
        "Bitwise operations not supported on FloatingPoint");
  }

  // --- Introspection ---
  /**
   * @brief Returns the type name.
   * 
   * Fulfills [FE-0030.1.9] Methods allowing some form of reflection and introspection.
   * 
   * @return "FloatingPoint"
   */
  std::string getType() const override { return "FloatingPoint"; }

  /**
   * @brief Checks if this is an integer type.
   * 
   * Fulfills [FE-0030.1.9] Methods allowing some form of reflection and introspection.
   * 
   * @return false
   */
  bool isIntegerType() const override { return false; }

  /**
   * @brief Checks if this is a signed type.
   * 
   * Fulfills [FE-0030.1.9] Methods allowing some form of reflection and introspection.
   * 
   * @return true (floating point types are signed).
   */
  bool isSigned() const override { return true; }

  // Specific Comparison for same-type FloatingPoint objects.
  int compareTo(const FloatingPoint<T> &other) const {
    if (value_ < other.value_)
      return -1;
    if (value_ > other.value_)
      return 1;
    // Total ordering for NaNs.
    if (std::isnan(value_) && std::isnan(other.value_))
      return 0;
    if (std::isnan(value_))
      return 1;
    if (std::isnan(other.value_))
      return -1;
    return 0;
  }

  bool equals(const FloatingPoint<T> &other) const {
    if (std::isnan(value_) && std::isnan(other.value_))
      return true;
    return value_ == other.value_;
  }

  // Primitive comparison
  /**
   * @brief Checks equality with a primitive value.
   * 
   * Fulfills [FE-0030.1.2] Methods for comparison with primitive types.
   * 
   * @param other The primitive value.
   * @return true if equal.
   */
  bool equals(T other) const {
    if (std::isnan(value_) && std::isnan(other)) return true;
    return value_ == other;
  }

  /**
   * @brief Compares with a primitive value.
   * 
   * Fulfills [FE-0030.1.2] Methods for comparison with primitive types.
   * 
   * @param other The primitive value.
   * @return -1, 0, or 1.
   */
  int compareTo(T other) const {
      if (std::isnan(value_) && std::isnan(other)) return 0;
      if (std::isnan(value_)) return 1;
      if (std::isnan(other)) return -1;
      if (value_ < other) return -1;
      if (value_ > other) return 1;
      return 0;
  }

  /**
   * @brief Equality operator.
   */
  bool operator==(T other) const { return equals(other); }

  /**
   * @brief Inequality operator.
   */
  bool operator!=(T other) const { return !equals(other); }

  /**
   * @brief Less than operator.
   */
  bool operator<(T other) const { return compareTo(other) < 0; }

  /**
   * @brief Greater than operator.
   */
  bool operator>(T other) const { return compareTo(other) > 0; }

  // Helper static methods.
  static std::string toString(T val, int radix = 10) {
    if (radix == 16) {
      // Use hexfloat manipulator for C++ standard hexadecimal floating point representation.
      std::stringstream ss;
      ss << std::hexfloat << val;
      return ss.str();
    }
    return std::to_string(val);
  }

  static T parseFloatingPoint(const std::string &s) {
    // Basic validation: an empty string cannot represent a number.
    if (s.empty())
      throw std::invalid_argument("Empty string");
    
    size_t idx = 0;
    try {
      T val;
      // Use compile-time dispatching to call the correct standard library function 
      // (stof for float, stod for double).
      if constexpr (std::is_same<T, float>::value) {
        val = std::stof(s, &idx);
      } else {
        val = std::stod(s, &idx);
      }
      
      // Verification: ensure the entire string was consumed. 
      // If idx is less than length, there are non-numeric trailing characters.
      if (idx != s.length()) {
        throw std::invalid_argument("Trailing characters in numeric string");
      }
      return val;
    } catch (const std::out_of_range &) {
      // Re-throw out_of_range which occurs if the value is too large for T.
      throw;
    } catch (const std::exception &) {
      // Wrap any other exceptions (like invalid_argument from stof/stod) with 
      // additional context.
      throw std::invalid_argument("Failed to parse floating point: " + s);
    }
  }

private:
  /**
   * @brief Checks if a value is safe (finite).
   * Throws exceptions for Infinite or NaN values.
   */
  void checkSafe(T val) const {
    // If the result is Infinite, it indicates an overflow occurred during the operation.
    if (std::isinf(val))
      throw std::overflow_error("Floating point overflow (Infinity)");
    
    // If the result is NaN, it indicates an undefined operation (like 0/0 or Inf-Inf).
    if (std::isnan(val))
      throw std::runtime_error("Floating point error (NaN)");
  }

  T value_;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_FLOATINGPOINT_HPP
