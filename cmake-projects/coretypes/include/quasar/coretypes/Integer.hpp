/**
 * @file Integer.hpp
 * @brief Definition of the templated Integer class.
 */

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
 * This class provides an object-oriented wrapper for primitive integral types 
 * (int8_t, uint8_t, ..., int64_t, uint64_t). It inherits from Number and 
 * implements polymorphic arithmetic, bitwise operations, and conversions.
 * These types are essential for representing and manipulating numerical fields
 * within protocol headers and data structures, as required for implementing
 * protocols like those described in [FE-0100]. For example, they can be used
 * to represent port numbers, IP addresses, lengths, sequence numbers, and other
 * critical numerical data within network packets and data formats.
 * 
 * **Compliance**:
 * - Fulfills [FE-0010.1] Provide, for each basic numeric type, a class which is assignable to its basic type.
 * - Fulfills [FE-0010.1.2] Each class related to a basic numeric type shall be derivated from a common "Number" base class.
 * - Fulfills [FE-0030.8] All methods are thread safe (inherent due to immutability).
 * 
 * The class is designed to be immutable, meaning any operation that modifies
 * the value returns a new instance. This makes it inherently thread-safe.
 *
 * @tparam T The underlying integral primitive type.
 */
template <typename T> class Integer : public Number {
  static_assert(std::is_integral<T>::value,
                "Integer class only supports integral types");

public:
  /**
   * @brief Constructs an Integer object from a primitive value.
   * 
   * Fulfills [FE-0010.1.1] Constructor which takes a value of the basic type.
   * 
   * @param value The primitive integer value.
   */
  explicit Integer(T value) : value_(value) {}

  /**
   * @brief Constructs an Integer object from a string with a specified radix.
   * 
   * Fulfills [FE-0010.1.7] Methods for decoding values from a string.
   * 
   * @param s The string containing the numeric value.
   * @param radix The base to use for parsing (defaults to 10).
   * @throws std::invalid_argument If the string cannot be parsed.
   * @throws std::out_of_range If the value is outside the representable range of type T.
   */
  explicit Integer(const std::string &s, int radix = 10)
      : value_(parseInt(s, radix, 0)) {}

  /**
   * @brief Converts the integer to a standard 32-bit int.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The integer representation.
   * @throws std::overflow_error If the value is outside the range of int.
   */
  int toInt() const override {
    // Check if the current type's maximum value exceeds the capacity of a standard int.
    // This is a compile-time check using if constexpr.
    if constexpr (std::numeric_limits<T>::max() >
                  std::numeric_limits<int>::max()) {
      // If the actual value exceeds the maximum value of 'int', throw an overflow exception.
      if (value_ > static_cast<T>(std::numeric_limits<int>::max()))
        throw std::overflow_error("Integer overflow in toInt");
    }
    // For signed types, also check for underflow against the minimum int value.
    if constexpr (std::is_signed<T>::value) {
      if constexpr (std::numeric_limits<T>::min() <
                    std::numeric_limits<int>::min()) {
        // If the actual value is less than the minimum value of 'int', throw an underflow exception.
        if (value_ < static_cast<T>(std::numeric_limits<int>::min()))
          throw std::overflow_error("Integer underflow in toInt");
      }
    }
    // Safely cast to the target type as the range has been validated.
    return static_cast<int>(value_);
  }

  /**
   * @brief Converts the integer to a standard 64-bit long.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The long representation.
   * @throws std::overflow_error If the value is outside the range of long.
   */
  long toLong() const override {
    // Perform similar range checks as in toInt, but against long's limits.
    if constexpr (std::numeric_limits<T>::max() >
                  std::numeric_limits<long>::max()) {
      // Check for overflow against the upper limit of 'long'.
      if (value_ > static_cast<T>(std::numeric_limits<long>::max()))
        throw std::overflow_error("Integer overflow in toLong");
    }
    if constexpr (std::is_signed<T>::value) {
      if constexpr (std::numeric_limits<T>::min() <
                    std::numeric_limits<long>::min()) {
        // Check for underflow against the lower limit of 'long'.
        if (value_ < static_cast<T>(std::numeric_limits<long>::min()))
          throw std::overflow_error("Integer underflow in toLong");
      }
    }
    // Return the value cast to long.
    return static_cast<long>(value_);
  }

  /**
   * @brief Converts the integer to a float.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The float representation. Precision loss may occur for large values.
   */
  float toFloat() const override {
    // Conversion to float is generally safe, although precision loss may occur for large integers.
    return static_cast<float>(value_);
  } 

  /**
   * @brief Converts the integer to a double.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The double representation.
   */
  double toDouble() const override { 
    // Conversion to double provides better precision for larger integers compared to float.
    return static_cast<double>(value_); 
  }

  /**
   * @brief Returns a string representation of the integer in base 10.
   * 
   * Fulfills [FE-0010.1.7] Methods for encoding values to a string.
   * 
   * @return String representation.
   */
  std::string toString() const override { return toString(value_, 10); }

  /**
   * @brief Returns the primitive integer value.
   * @return The value of type T.
   */
  T value() const { return value_; }

  /**
   * @brief Adds another Integer of the same type.
   * 
   * Performs standard addition. Note that for signed types, overflow 
   * results in undefined behavior in C++, while for unsigned types, 
   * it follows modulo arithmetic (wrap-around).
   * 
   * Fulfills [FE-0010.1.3] Methods for addition.
   *
   * @param other The value to add.
   * @return A new Integer instance containing the sum.
   */
  Integer<T> add(const Integer<T> &other) const {
    // Standard addition with potential wrap-around (defined behavior for unsigned, UB for signed).
    return Integer<T>(value_ + other.value_);
  }

  /**
   * @brief Subtracts another Integer of the same type.
   * 
   * Performs standard subtraction. Similar to addition, behavior on 
   * underflow/overflow depends on whether T is signed or unsigned.
   * 
   * Fulfills [FE-0010.1.3] Methods for subtraction.
   *
   * @param other The value to subtract.
   * @return A new Integer instance containing the difference.
   */
  Integer<T> subtract(const Integer<T> &other) const {
    return Integer<T>(value_ - other.value_);
  }

  /**
   * @brief Multiplies by another Integer of the same type.
   * 
   * Performs standard multiplication.
   * 
   * Fulfills [FE-0010.1.3] Methods for multiplication.
   *
   * @param other The value to multiply by.
   * @return A new Integer instance containing the product.
   */
  Integer<T> multiply(const Integer<T> &other) const {
    return Integer<T>(value_ * other.value_);
  }

  /**
   * @brief Divides by another Integer of the same type.
   * 
   * Performs integer division (truncation towards zero).
   * 
   * Fulfills [FE-0010.1.3] Methods for division.
   *
   * @param other The value to divide by.
   * @return A new Integer instance containing the quotient.
   * @throws std::runtime_error If division by zero occurs.
   */
  Integer<T> divide(const Integer<T> &other) const {
    // Check for division by zero before performing the operation to avoid
    // hardware exceptions or crashes.
    if (other.value_ == 0)
      throw std::runtime_error("Division by zero");
    return Integer<T>(value_ / other.value_);
  }

  /**
   * @brief Adds another Integer with overflow detection.
   * 
   * Uses compiler intrinsics to perform addition and check if the result 
   * fits within the limits of type T. This provides a safe way to 
   * perform arithmetic without relying on undefined behavior.
   * 
   * Fulfills [FE-0010.1.4] Methods for safe addition.
   * 
   * @param other The value to add.
   * @return A new Integer instance containing the sum.
   * @throws std::overflow_error If an overflow or underflow occurs.
   */
  Integer<T> safeAdd(const Integer<T> &other) const {
    T res;
    // Use compiler intrinsics (__builtin_add_overflow) for efficient overflow 
    // detection. This is supported by GCC and Clang and translates to 
    // checking the processor's overflow flag after the addition.
    if (__builtin_add_overflow(value_, other.value_, &res))
      throw std::overflow_error("Overflow in add");
    return Integer<T>(res);
  }

  /**
   * @brief Subtracts another Integer with overflow detection.
   * 
   * Uses compiler intrinsics to perform subtraction and check for overflow 
   * or underflow conditions.
   * 
   * Fulfills [FE-0010.1.4] Methods for safe subtraction.
   * 
   * @param other The value to subtract.
   * @return A new Integer instance containing the difference.
   * @throws std::overflow_error If an overflow or underflow occurs.
   */
  Integer<T> safeSubtract(const Integer<T> &other) const {
    T res;
    // Use compiler intrinsics for efficient overflow detection.
    if (__builtin_sub_overflow(value_, other.value_, &res))
      throw std::overflow_error("Overflow in subtract");
    return Integer<T>(res);
  }

  /**
   * @brief Multiplies by another Integer with overflow detection.
   * 
   * Uses compiler intrinsics to perform multiplication and check if the 
   * product is representable in type T.
   * 
   * Fulfills [FE-0010.1.4] Methods for safe multiplication.
   * 
   * @param other The value to multiply by.
   * @return A new Integer instance containing the product.
   * @throws std::overflow_error If an overflow or underflow occurs.
   */
  Integer<T> safeMultiply(const Integer<T> &other) const {
    T res;
    // Use compiler intrinsics for efficient overflow detection.
    if (__builtin_mul_overflow(value_, other.value_, &res))
      throw std::overflow_error("Overflow in multiply");
    return Integer<T>(res);
  }

  /**
   * @brief Divides by another Integer with overflow detection.
   * 
   * Performs division with checks for both division by zero and the 
   * edge case of dividing the minimum signed value by -1.
   * 
   * Fulfills [FE-0010.1.4] Methods for safe division.
   * 
   * @param other The value to divide by.
   * @return A new Integer instance containing the quotient.
   * @throws std::runtime_error If division by zero occurs.
   * @throws std::overflow_error If an overflow occurs (e.g., INT_MIN / -1).
   */
  Integer<T> safeDivide(const Integer<T> &other) const {
    // Explicitly check for division by zero.
    if (other.value_ == 0)
      throw std::runtime_error("Division by zero");

    // Handle the specific case of INT_MIN / -1 which causes an overflow in two's complement
    // systems because the result (abs(INT_MIN)) cannot be represented in the same signed type.
    // For example, in 8-bit signed: -128 / -1 = 128, but 128 is not representable as int8_t (max is 127).
    if constexpr (std::is_signed<T>::value) {
      if (value_ == std::numeric_limits<T>::min() && other.value_ == -1) {
        throw std::overflow_error("Overflow in divide");
      }
    }
    return Integer<T>(value_ / other.value_);
  }

  /**
   * @brief Compares this Integer with another Number.
   * 
   * Fulfills [FE-0030.1.1] Methods for comparison with other Number objects.
   * 
   * @param other The number to compare with.
   * @return -1 if this < other, 0 if equal, 1 if this > other.
   */
  int compareTo(const Number &other) const override {
    // Generic comparison across different Number implementations.
    // We use double as a common denominator for comparison across different numeric types.
    // While this handles most cases, be aware that precision loss can occur for very large 
    // 64-bit integers (above 2^53).
    double d1 = static_cast<double>(value_);
    double d2 = other.toDouble();
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
   * @return true if both represent the same numeric value.
   */
  bool equals(const Number &other) const override {
    return compareTo(other) == 0;
  }

  /**
   * @brief Compares with another Integer of the same type T.
   * 
   * Fulfills [FE-0010.1.3] Methods for comparison.
   * 
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
   * @brief Checks equality with another Integer of the same type T.
   * @param other The other Integer.
   * @return true if equal.
   */
  bool equals(const Integer<T> &other) const { return value_ == other.value_; }

  /**
   * @brief Checks equality with a primitive value.
   * 
   * Fulfills [FE-0030.1.2] Methods for comparison with primitive types.
   * 
   * @param other The primitive value.
   * @return true if equal.
   */
  bool equals(T other) const { return value_ == other; }

  /**
   * @brief Compares with a primitive value.
   * 
   * Fulfills [FE-0030.1.2] Methods for comparison with primitive types.
   * 
   * @param other The primitive value.
   * @return -1, 0, or 1.
   */
  int compareTo(T other) const {
      if (value_ < other) return -1;
      if (value_ > other) return 1;
      return 0;
  }

  /**
   * @brief Equality operator.
   */
  bool operator==(T other) const { return value_ == other; }

  /**
   * @brief Inequality operator.
   */
  bool operator!=(T other) const { return value_ != other; }

  /**
   * @brief Less than operator.
   */
  bool operator<(T other) const { return value_ < other; }

  /**
   * @brief Greater than operator.
   */
  bool operator>(T other) const { return value_ > other; }

  /**
   * @brief Less than or equal operator.
   */
  bool operator<=(T other) const { return value_ <= other; }

  /**
   * @brief Greater than or equal operator.
   */
  bool operator>=(T other) const { return value_ >= other; }

  /**
   * @brief Polymorphic addition.
   * 
   * Fulfills [FE-0030.1.3] Methods for basic arithmetic operations.
   */
  std::shared_ptr<Number> add(const Number &other) const override {
    return std::make_shared<Integer<T>>(value_ +
                                        static_cast<T>(other.toDouble()));
  }

  /**
   * @brief Polymorphic subtraction.
   * 
   * Fulfills [FE-0030.1.3] Methods for basic arithmetic operations.
   */
  std::shared_ptr<Number> subtract(const Number &other) const override {
    return std::make_shared<Integer<T>>(value_ -
                                        static_cast<T>(other.toDouble()));
  }

  /**
   * @brief Polymorphic multiplication.
   * 
   * Fulfills [FE-0030.1.3] Methods for basic arithmetic operations.
   */
  std::shared_ptr<Number> multiply(const Number &other) const override {
    return std::make_shared<Integer<T>>(value_ *
                                        static_cast<T>(other.toDouble()));
  }

  /**
   * @brief Polymorphic division.
   * 
   * Fulfills [FE-0030.1.3] Methods for basic arithmetic operations.
   */
  std::shared_ptr<Number> divide(const Number &other) const override {
    double d = other.toDouble();
    if (d == 0.0)
      throw std::runtime_error("Division by zero");
    return std::make_shared<Integer<T>>(value_ / static_cast<T>(d));
  }

  /**
   * @brief Polymorphic safe addition.
   * 
   * Fulfills [FE-0030.1.4] Methods for safe arithmetic operations.
   */
  std::shared_ptr<Number> safeAdd(const Number &other) const override {
    T otherVal = static_cast<T>(other.toDouble());
    T res;
    if (__builtin_add_overflow(value_, otherVal, &res))
      throw std::overflow_error("Overflow in safeAdd");
    return std::make_shared<Integer<T>>(res);
  }

  /**
   * @brief Polymorphic safe subtraction.
   * 
   * Fulfills [FE-0030.1.4] Methods for safe arithmetic operations.
   */
  std::shared_ptr<Number> safeSubtract(const Number &other) const override {
    T otherVal = static_cast<T>(other.toDouble());
    T res;
    if (__builtin_sub_overflow(value_, otherVal, &res))
      throw std::overflow_error("Overflow in safeSubtract");
    return std::make_shared<Integer<T>>(res);
  }

  /**
   * @brief Polymorphic safe multiplication.
   * 
   * Fulfills [FE-0030.1.4] Methods for safe arithmetic operations.
   */
  std::shared_ptr<Number> safeMultiply(const Number &other) const override {
    T otherVal = static_cast<T>(other.toDouble());
    T res;
    if (__builtin_mul_overflow(value_, otherVal, &res))
      throw std::overflow_error("Overflow in safeMultiply");
    return std::make_shared<Integer<T>>(res);
  }

  /**
   * @brief Polymorphic safe division.
   * 
   * Fulfills [FE-0030.1.4] Methods for safe arithmetic operations.
   */
  std::shared_ptr<Number> safeDivide(const Number &other) const override {
    T otherVal = static_cast<T>(other.toDouble());
    if (otherVal == 0)
      throw std::runtime_error("Division by zero");
    if constexpr (std::is_signed<T>::value) {
      if (value_ == std::numeric_limits<T>::min() && otherVal == -1) {
        throw std::overflow_error("Overflow in safeDivide");
      }
    }
    return std::make_shared<Integer<T>>(value_ / otherVal);
  }

  /**
   * @brief Performs a bitwise AND operation.
   * 
   * Each bit of the result is 1 if the corresponding bits of both 
   * operands are 1; otherwise, the result bit is 0.
   * 
   * Fulfills [FE-0030.1.5] Methods for bitwise operations.
   * 
   * @param other The other number to perform bitwise AND with.
   * @return A shared pointer to a new Integer containing the result.
   */
  std::shared_ptr<Number> bitwiseAnd(const Number &other) const override {
    // We convert the other number to long to perform the bitwise operation.
    // This allows for interoperability between different integer types.
    T otherVal = static_cast<T>(other.toLong());
    return std::make_shared<Integer<T>>(value_ & otherVal);
  }

  /**
   * @brief Performs a bitwise OR operation.
   * 
   * Each bit of the result is 1 if at least one of the corresponding bits 
   * of the operands is 1; otherwise, the result bit is 0.
   * 
   * Fulfills [FE-0030.1.5] Methods for bitwise operations.
   * 
   * @param other The other number to perform bitwise OR with.
   * @return A shared pointer to a new Integer containing the result.
   */
  std::shared_ptr<Number> bitwiseOr(const Number &other) const override {
    T otherVal = static_cast<T>(other.toLong());
    return std::make_shared<Integer<T>>(value_ | otherVal);
  }

  /**
   * @brief Performs a bitwise XOR operation.
   * 
   * Each bit of the result is 1 if exactly one of the corresponding bits 
   * of the operands is 1; otherwise, the result bit is 0.
   * 
   * Fulfills [FE-0030.1.5] Methods for bitwise operations.
   * 
   * @param other The other number to perform bitwise XOR with.
   * @return A shared pointer to a new Integer containing the result.
   */
  std::shared_ptr<Number> bitwiseXor(const Number &other) const override {
    T otherVal = static_cast<T>(other.toLong());
    return std::make_shared<Integer<T>>(value_ ^ otherVal);
  }

  /**
   * @brief Performs a bitwise NOT operation.
   * 
   * Flips all bits of the integer: 0 becomes 1, and 1 becomes 0.
   * 
   * Fulfills [FE-0030.1.5] Methods for bitwise operations.
   * 
   * @return A shared pointer to a new Integer containing the bitwise complement.
   */
  std::shared_ptr<Number> bitwiseNot() const override {
    // The ~ operator performs bitwise NOT in C++.
    return std::make_shared<Integer<T>>(~value_);
  }

  /**
   * @brief Performs a bitwise Left Shift operation.
   * 
   * Shifts the bits of the integer to the left by the specified amount. 
   * The new bits on the right are filled with zeros.
   * 
   * Fulfills [FE-0030.1.5] Methods for bitwise operations.
   * 
   * @param amount The number of bits to shift.
   * @return A shared pointer to a new Integer containing the result.
   */
  std::shared_ptr<Number> bitwiseLeftShift(int amount) const override {
    // Left shifting is equivalent to multiplying by 2 raised to the power of amount.
    return std::make_shared<Integer<T>>(value_ << amount);
  }

  /**
   * @brief Performs a bitwise Right Shift operation.
   * 
   * Shifts the bits of the integer to the right by the specified amount. 
   * For signed types, the behavior depends on the compiler (usually 
   * arithmetic shift, preserving the sign bit). For unsigned types, 
   * it's a logical shift (filled with zeros).
   * 
   * Fulfills [FE-0030.1.5] Methods for bitwise operations.
   * 
   * @param amount The number of bits to shift.
   * @return A shared pointer to a new Integer containing the result.
   */
  std::shared_ptr<Number> bitwiseRightShift(int amount) const override {
    // Right shifting is equivalent to integer division by 2 raised to the power of amount.
    return std::make_shared<Integer<T>>(value_ >> amount);
  }

  /**
   * @brief Returns the type name.
   * 
   * Fulfills [FE-0030.1.9] Methods allowing some form of reflection and introspection.
   * 
   * @return "Integer"
   */
  std::string getType() const override { return "Integer"; }

  /**
   * @brief Checks if this is an integer type.
   * 
   * Fulfills [FE-0030.1.9] Methods allowing some form of reflection and introspection.
   * 
   * @return true
   */
  bool isIntegerType() const override { return true; }

  /**
   * @brief Checks if this is a signed integer type.
   * 
   * Fulfills [FE-0030.1.9] Methods allowing some form of reflection and introspection.
   * 
   * @return true if signed, false otherwise.
   */
  bool isSigned() const override { return std::is_signed<T>::value; }

  /**
   * @brief Swaps bytes to change endianness.
   * 
   * Useful for network protocol processing or cross-platform data exchange.
   * @return A new Integer instance with swapped byte order.
   */
  Integer<T> swapBytes() const {
    T val = value_;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(&val);
    // Reverse the byte array in-place.
    std::reverse(ptr, ptr + sizeof(T));
    return Integer<T>(val);
  }

  /**
   * @brief Converts an integer value to a string using a specified radix.
   * 
   * @param val The numeric value to convert.
   * @param radix The base to use (2 through 36).
   * @return A string representation of the value in the chosen base.
   */
  static std::string toString(T val, int radix) {
    // Validate radix range. Standard bases are 2 to 36 (using digits '0'-'9' and 'a'-'z').
    if (radix < 2 || radix > 36)
      radix = 10;
    
    // Optimization for the most common case: base 10.
    if (radix == 10)
      return std::to_string(val);

    // Custom radix conversion logic for non-decimal bases.
    // Determine if the value is negative to handle the sign separately.
    bool negative = (val < 0) && std::is_signed<T>::value;
    
    // Use an unsigned type for the actual conversion to avoid issues with 
    // negative numbers and two's complement representation.
    using UT = typename std::make_unsigned<T>::type;
    UT u_val;
    if (negative) {
      // Special handling for the minimum possible signed value (e.g., -128 for int8_t),
      // as its absolute value cannot be represented in the same signed type.
      if (val == std::numeric_limits<T>::min()) {
        u_val = static_cast<UT>(-(val + 1)) + 1;
      } else {
        u_val = static_cast<UT>(-val);
      }
    } else {
      u_val = static_cast<UT>(val);
    }

    std::string res;
    // Zero is a special case in any radix.
    if (u_val == 0)
      return "0";
    
    // Successive division algorithm to extract digits in reverse order.
    while (u_val > 0) {
      UT digit = u_val % radix;
      // Convert digit value to character: '0'-'9' for 0-9, 'a'-'z' for 10-35.
      char c = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
      res += c;
      u_val /= radix;
    }
    
    // Add the sign if the original value was negative.
    if (negative)
      res += '-';
    
    // Since digits were added by successive divisions, they are in reverse order (LSB first).
    // Reverse the string to get the correct representation.
    std::reverse(res.begin(), res.end());
    return res;
  }

  /**
   * @brief Parses a string into a primitive integer value using a specified radix.
   * 
   * @param s The string to parse.
   * @param radix The base of the number in the string.
   * @param out_idx Optional pointer to store the index of the first character after the number.
   * @return The parsed primitive integer value.
   * @throws std::invalid_argument If the string is empty or contains an invalid format.
   * @throws std::out_of_range If the parsed value is outside the representable range of type T.
   */
  static T parseInt(const std::string &s, int radix,
                    size_t *out_idx = nullptr) {
    // Basic validation: an empty string cannot be a number.
    if (s.empty())
      throw std::invalid_argument("Empty string");

    size_t local_idx = 0;
    size_t *idx_ptr = out_idx ? out_idx : &local_idx;

    try {
      // Use standard library long long parsing as a base, then check limits.
      if constexpr (std::is_signed<T>::value) {
        // Parse as a 64-bit signed integer.
        long long val = std::stoll(s, idx_ptr, radix);
        
        // If no external index was provided, ensure the whole string was consumed.
        if (!out_idx && *idx_ptr != s.length()) {
          throw std::invalid_argument("Trailing characters in numeric string");
        }
        
        // Bounds check: ensure the 64-bit value fits into the target type T.
        if (val < std::numeric_limits<T>::min() ||
            val > std::numeric_limits<T>::max()) {
          throw std::out_of_range("Value out of range for signed integer type");
        }
        return static_cast<T>(val);
      } else {
        // Parse as a 64-bit unsigned integer.
        unsigned long long val = std::stoull(s, idx_ptr, radix);
        
        // Consistency check for string consumption.
        if (!out_idx && *idx_ptr != s.length()) {
          throw std::invalid_argument("Trailing characters in numeric string");
        }
        
        // Bounds check against the target unsigned type's capacity.
        if (val > std::numeric_limits<T>::max()) {
          throw std::out_of_range("Value out of range for unsigned integer type");
        }
        return static_cast<T>(val);
      }
    } catch (const std::out_of_range &e) {
      // Re-throw out_of_range with more context if possible, otherwise let it bubble up.
      throw;
    } catch (const std::exception &) {
      // Catch any other parsing errors (like invalid characters) and wrap them.
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
