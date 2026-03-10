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
 * (int8_t, uint8_t, ..., int64_t, uint64_t, size_t, ptrdiff_t). It inherits from Number and 
 * implements polymorphic arithmetic, bitwise operations, and conversions.
 * 
 * **Compliance**:
 * - Fulfills [FE-0010.1] Provide, for each basic numeric type, a class which is assignable to its basic type.
 * - Fulfills [FE-0010.1.2] Each class related to a basic numeric type shall be derivated from a common "Number" base class.
 * - Fulfills [FE-0030.8] All methods are thread safe (inherent due to immutability).
 * - Fulfills [TSK-20260308-001.1] Full Polymorphic Dispatch.
 * - Fulfills [TSK-20260308-001.2] Precision Safety (uses common_type_t).
 * 
 * @tparam T The underlying integral primitive type.
 */
template <typename T> class Integer : public Number {
  static_assert(std::is_integral<T>::value,
                "Integer class only supports integral types");

public:
  /**
   * @brief Constructs an Integer object from a primitive value.
   * @param value The primitive integer value.
   */
  explicit Integer(T value) : value_(value) {}

  /**
   * @brief Constructs an Integer object from a string with a specified radix.
   * @param s The string containing the numeric value.
   * @param radix The base to use for parsing (defaults to 10).
   */
  explicit Integer(const std::string &s, int radix = 10)
      : value_(parseInt(s, radix, 0)) {}

  // --- Conversions ---

  int toInt() const override { return toType<int>(); }
  long toLong() const override { return toType<long>(); }
  float toFloat() const override { return static_cast<float>(value_); }
  double toDouble() const override { return static_cast<double>(value_); }

  int8_t toInt8() const override { return toType<int8_t>(); }
  int16_t toInt16() const override { return toType<int16_t>(); }
  int32_t toInt32() const override { return toType<int32_t>(); }
  int64_t toInt64() const override { return toType<int64_t>(); }
  uint8_t toUInt8() const override { return toType<uint8_t>(); }
  uint16_t toUInt16() const override { return toType<uint16_t>(); }
  uint32_t toUInt32() const override { return toType<uint32_t>(); }
  uint64_t toUInt64() const override { return toType<uint64_t>(); }
  size_t toSizeT() const override { return toType<size_t>(); }
  ptrdiff_t toPtrDiffT() const override { return toType<ptrdiff_t>(); }

  template <typename TargetT> TargetT toType() const {
    if constexpr (std::is_same_v<T, TargetT>) {
      return value_;
    }

    // Check upper bound
    if constexpr (std::numeric_limits<T>::max() > std::numeric_limits<TargetT>::max()) {
      if (value_ > static_cast<T>(std::numeric_limits<TargetT>::max()))
        throw std::overflow_error("Integer overflow");
    }

    // Check lower bound
    if constexpr (std::is_signed_v<T>) {
      if constexpr (std::is_signed_v<TargetT>) {
        if constexpr (std::numeric_limits<T>::min() < std::numeric_limits<TargetT>::min()) {
          if (value_ < static_cast<T>(std::numeric_limits<TargetT>::min()))
            throw std::overflow_error("Integer underflow");
        }
      } else {
        // T is signed, TargetT is unsigned. TargetT min is 0.
        if (value_ < 0)
          throw std::overflow_error("Integer underflow (negative to unsigned)");
      }
    }
    return static_cast<TargetT>(value_);
  }

  std::string toString() const override { return toString(value_, 10); }

  T value() const { return value_; }

  /**
   * @brief Compares with another Integer of the same type T.
   * @param other The other Integer.
   * @return -1 if this < other, 1 if this > other, 0 if equal.
   */
  int compareTo(const Integer<T> &other) const {
    if (value_ < other.value_) return -1;
    if (value_ > other.value_) return 1;
    return 0;
  }

  /**
   * @brief Checks equality with another Integer of the same type T.
   * @param other The other Integer.
   * @return true if equal.
   */
  bool equals(const Integer<T> &other) const { return value_ == other.value_; }

  /** @brief Equality operator. */
  bool operator==(T other) const { return value_ == other; }
  /** @brief Inequality operator. */
  bool operator!=(T other) const { return value_ != other; }
  /** @brief Less than operator. */
  bool operator<(T other) const { return value_ < other; }
  /** @brief Greater than operator. */
  bool operator>(T other) const { return value_ > other; }
  /** @brief Less than or equal operator. */
  bool operator<=(T other) const { return value_ <= other; }
  /** @brief Greater than or equal operator. */
  bool operator>=(T other) const { return value_ >= other; }

  // Same-type overloads for convenience and compatibility
  Integer<T> add(const Integer<T> &other) const { return Integer<T>(value_ + other.value_); }
  Integer<T> subtract(const Integer<T> &other) const { return Integer<T>(value_ - other.value_); }
  Integer<T> multiply(const Integer<T> &other) const { return Integer<T>(value_ * other.value_); }
  Integer<T> divide(const Integer<T> &other) const { 
      if (other.value_ == 0) throw std::runtime_error("Division by zero");
      return Integer<T>(value_ / other.value_); 
  }

  Integer<T> safeAdd(const Integer<T> &other) const {
      T res;
      if (__builtin_add_overflow(value_, other.value_, &res)) throw std::overflow_error("Overflow");
      return Integer<T>(res);
  }
  Integer<T> safeSubtract(const Integer<T> &other) const {
      T res;
      if (__builtin_sub_overflow(value_, other.value_, &res)) throw std::overflow_error("Overflow");
      return Integer<T>(res);
  }
  Integer<T> safeMultiply(const Integer<T> &other) const {
      T res;
      if (__builtin_mul_overflow(value_, other.value_, &res)) throw std::overflow_error("Overflow");
      return Integer<T>(res);
  }
  Integer<T> safeDivide(const Integer<T> &other) const {
      if (other.value_ == 0) throw std::runtime_error("Division by zero");
      if constexpr (std::is_signed_v<T>) {
          if (value_ == std::numeric_limits<T>::min() && other.value_ == -1) throw std::overflow_error("Overflow");
      }
      return Integer<T>(value_ / other.value_);
  }

  // --- Polymorphic Dispatch (Double Dispatch) ---

  std::shared_ptr<Number> add(const Number &other) const override { return other.add(value_); }
  std::shared_ptr<Number> subtract(const Number &other) const override { return other.subtract(value_); }
  std::shared_ptr<Number> multiply(const Number &other) const override { return other.multiply(value_); }
  std::shared_ptr<Number> divide(const Number &other) const override { return other.divide(value_); }

  std::shared_ptr<Number> safeAdd(const Number &other) const override { return other.safeAdd(value_); }
  std::shared_ptr<Number> safeSubtract(const Number &other) const override { return other.safeSubtract(value_); }
  std::shared_ptr<Number> safeMultiply(const Number &other) const override { return other.safeMultiply(value_); }
  std::shared_ptr<Number> safeDivide(const Number &other) const override { return other.safeDivide(value_); }

  std::shared_ptr<Number> bitwiseAnd(const Number &other) const override { return other.bitwiseAnd(value_); }
  std::shared_ptr<Number> bitwiseOr(const Number &other) const override { return other.bitwiseOr(value_); }
  std::shared_ptr<Number> bitwiseXor(const Number &other) const override { return other.bitwiseXor(value_); }
  
  std::shared_ptr<Number> bitwiseNot() const override { return std::make_shared<Integer<T>>(~value_); }
  std::shared_ptr<Number> bitwiseLeftShift(const Number &other) const override { return other.bitwiseLeftShift(value_); }
  std::shared_ptr<Number> bitwiseRightShift(const Number &other) const override { return other.bitwiseRightShift(value_); }

  int compareTo(const Number &other) const override { return -other.compareTo(value_); }
  bool equals(const Number &other) const override { return other.equals(value_); }

  // --- Implementation of virtual methods from Number ---

#define IMPLEMENT_INT_OP(OP_NAME, IMPL) \
  std::shared_ptr<Number> OP_NAME(signed char val) const override { return IMPL(val); } \
  std::shared_ptr<Number> OP_NAME(unsigned char val) const override { return IMPL(val); } \
  std::shared_ptr<Number> OP_NAME(short val) const override { return IMPL(val); } \
  std::shared_ptr<Number> OP_NAME(unsigned short val) const override { return IMPL(val); } \
  std::shared_ptr<Number> OP_NAME(int val) const override { return IMPL(val); } \
  std::shared_ptr<Number> OP_NAME(unsigned int val) const override { return IMPL(val); } \
  std::shared_ptr<Number> OP_NAME(long val) const override { return IMPL(val); } \
  std::shared_ptr<Number> OP_NAME(unsigned long val) const override { return IMPL(val); } \
  std::shared_ptr<Number> OP_NAME(long long val) const override { return IMPL(val); } \
  std::shared_ptr<Number> OP_NAME(unsigned long long val) const override { return IMPL(val); }

#define IMPLEMENT_INT_CMP_OP(OP_NAME, RETURN_TYPE, IMPL) \
  RETURN_TYPE OP_NAME(signed char val) const override { return IMPL(val); } \
  RETURN_TYPE OP_NAME(unsigned char val) const override { return IMPL(val); } \
  RETURN_TYPE OP_NAME(short val) const override { return IMPL(val); } \
  RETURN_TYPE OP_NAME(unsigned short val) const override { return IMPL(val); } \
  RETURN_TYPE OP_NAME(int val) const override { return IMPL(val); } \
  RETURN_TYPE OP_NAME(unsigned int val) const override { return IMPL(val); } \
  RETURN_TYPE OP_NAME(long val) const override { return IMPL(val); } \
  RETURN_TYPE OP_NAME(unsigned long val) const override { return IMPL(val); } \
  RETURN_TYPE OP_NAME(long long val) const override { return IMPL(val); } \
  RETURN_TYPE OP_NAME(unsigned long long val) const override { return IMPL(val); }

  template<typename U> std::shared_ptr<Number> addImpl(U val) const {
      using CommonT = std::common_type_t<T, U>;
      return std::make_shared<Integer<CommonT>>(static_cast<CommonT>(val) + static_cast<CommonT>(value_));
  }
  IMPLEMENT_INT_OP(add, addImpl)

  template<typename U> std::shared_ptr<Number> subtractImpl(U val) const {
      using CommonT = std::common_type_t<T, U>;
      return std::make_shared<Integer<CommonT>>(static_cast<CommonT>(val) - static_cast<CommonT>(value_));
  }
  IMPLEMENT_INT_OP(subtract, subtractImpl)

  template<typename U> std::shared_ptr<Number> multiplyImpl(U val) const {
      using CommonT = std::common_type_t<T, U>;
      return std::make_shared<Integer<CommonT>>(static_cast<CommonT>(val) * static_cast<CommonT>(value_));
  }
  IMPLEMENT_INT_OP(multiply, multiplyImpl)

  template<typename U> std::shared_ptr<Number> divideImpl(U val) const {
      using CommonT = std::common_type_t<T, U>;
      if (value_ == 0) throw std::runtime_error("Division by zero");
      return std::make_shared<Integer<CommonT>>(static_cast<CommonT>(val) / static_cast<CommonT>(value_));
  }
  IMPLEMENT_INT_OP(divide, divideImpl)

  template<typename U> std::shared_ptr<Number> safeAddImpl(U val) const {
      using CommonT = std::common_type_t<T, U>;
      CommonT res;
      if (__builtin_add_overflow(static_cast<CommonT>(val), static_cast<CommonT>(value_), &res))
          throw std::overflow_error("Overflow in safeAdd");
      return std::make_shared<Integer<CommonT>>(res);
  }
  IMPLEMENT_INT_OP(safeAdd, safeAddImpl)

  template<typename U> std::shared_ptr<Number> safeSubtractImpl(U val) const {
      using CommonT = std::common_type_t<T, U>;
      CommonT res;
      if (__builtin_sub_overflow(static_cast<CommonT>(val), static_cast<CommonT>(value_), &res))
          throw std::overflow_error("Overflow in safeSubtract");
      return std::make_shared<Integer<CommonT>>(res);
  }
  IMPLEMENT_INT_OP(safeSubtract, safeSubtractImpl)

  template<typename U> std::shared_ptr<Number> safeMultiplyImpl(U val) const {
      using CommonT = std::common_type_t<T, U>;
      CommonT res;
      if (__builtin_mul_overflow(static_cast<CommonT>(val), static_cast<CommonT>(value_), &res))
          throw std::overflow_error("Overflow in safeMultiply");
      return std::make_shared<Integer<CommonT>>(res);
  }
  IMPLEMENT_INT_OP(safeMultiply, safeMultiplyImpl)

  template<typename U> std::shared_ptr<Number> safeDivideImpl(U val) const {
      using CommonT = std::common_type_t<T, U>;
      if (value_ == 0) throw std::runtime_error("Division by zero");
      if constexpr (std::is_signed_v<CommonT>) {
          if (static_cast<CommonT>(val) == std::numeric_limits<CommonT>::min() && static_cast<CommonT>(value_) == -1)
              throw std::overflow_error("Overflow in safeDivide");
      }
      return std::make_shared<Integer<CommonT>>(static_cast<CommonT>(val) / static_cast<CommonT>(value_));
  }
  IMPLEMENT_INT_OP(safeDivide, safeDivideImpl)

  template<typename U> std::shared_ptr<Number> bitwiseAndImpl(U val) const {
      using CommonT = std::common_type_t<T, U>;
      return std::make_shared<Integer<CommonT>>(static_cast<CommonT>(val) & static_cast<CommonT>(value_));
  }
  IMPLEMENT_INT_OP(bitwiseAnd, bitwiseAndImpl)

  template<typename U> std::shared_ptr<Number> bitwiseOrImpl(U val) const {
      using CommonT = std::common_type_t<T, U>;
      return std::make_shared<Integer<CommonT>>(static_cast<CommonT>(val) | static_cast<CommonT>(value_));
  }
  IMPLEMENT_INT_OP(bitwiseOr, bitwiseOrImpl)

  template<typename U> std::shared_ptr<Number> bitwiseXorImpl(U val) const {
      using CommonT = std::common_type_t<T, U>;
      return std::make_shared<Integer<CommonT>>(static_cast<CommonT>(val) ^ static_cast<CommonT>(value_));
  }
  IMPLEMENT_INT_OP(bitwiseXor, bitwiseXorImpl)

  template<typename U> std::shared_ptr<Number> bitwiseLeftShiftImpl(U val) const {
      return std::make_shared<Integer<U>>(static_cast<U>(val) << value_);
  }
  IMPLEMENT_INT_OP(bitwiseLeftShift, bitwiseLeftShiftImpl)

  template<typename U> std::shared_ptr<Number> bitwiseRightShiftImpl(U val) const {
      return std::make_shared<Integer<U>>(static_cast<U>(val) >> value_);
  }
  IMPLEMENT_INT_OP(bitwiseRightShift, bitwiseRightShiftImpl)

  template<typename U> int compareToPolymorphic(U val) const {
      if constexpr (std::is_signed_v<U> == std::is_signed_v<T>) {
          if (value_ < val) return -1;
          if (value_ > val) return 1;
          return 0;
      } else {
          if constexpr (std::is_signed_v<T>) {
              if (value_ < 0) return -1;
              if (static_cast<std::make_unsigned_t<T>>(value_) < val) return -1;
              if (static_cast<std::make_unsigned_t<T>>(value_) > val) return 1;
              return 0;
          } else {
              if (val < 0) return 1;
              if (value_ < static_cast<std::make_unsigned_t<U>>(val)) return -1;
              if (value_ > static_cast<std::make_unsigned_t<U>>(val)) return 1;
              return 0;
          }
      }
  }
  IMPLEMENT_INT_CMP_OP(compareTo, int, compareToPolymorphic)

  template<typename U> bool equalsPolymorphic(U val) const {
      return compareToPolymorphic(val) == 0;
  }
  IMPLEMENT_INT_CMP_OP(equals, bool, equalsPolymorphic)

#undef IMPLEMENT_INT_OP
#undef IMPLEMENT_INT_CMP_OP


  // --- Reflection and Introspection ---

  std::string getType() const override { return "Integer"; }
  bool isIntegerType() const override { return true; }
  bool isSigned() const override { return std::is_signed<T>::value; }

  // --- Helper Methods ---

  Integer<T> swapBytes() const {
    T val = value_;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(&val);
    std::reverse(ptr, ptr + sizeof(T));
    return Integer<T>(val);
  }

  static std::string toString(T val, int radix) {
    if (radix < 2 || radix > 36) radix = 10;
    if (radix == 10) return std::to_string(val);
    bool negative = (val < 0) && std::is_signed<T>::value;
    using UT = typename std::make_unsigned<T>::type;
    UT u_val;
    if (negative) {
      if (val == std::numeric_limits<T>::min()) {
        u_val = static_cast<UT>(-(val + 1)) + 1;
      } else {
        u_val = static_cast<UT>(-val);
      }
    } else {
      u_val = static_cast<UT>(val);
    }
    std::string res;
    if (u_val == 0) return "0";
    while (u_val > 0) {
      UT digit = u_val % radix;
      char c = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
      res += c;
      u_val /= radix;
    }
    if (negative) res += '-';
    std::reverse(res.begin(), res.end());
    return res;
  }

  static T parseInt(const std::string &s, int radix, size_t *out_idx = nullptr) {
    if (s.empty()) throw std::invalid_argument("Empty string");
    size_t local_idx = 0;
    size_t *idx_ptr = out_idx ? out_idx : &local_idx;
    try {
      if constexpr (std::is_signed<T>::value) {
        long long val = std::stoll(s, idx_ptr, radix);
        if (!out_idx && *idx_ptr != s.length()) throw std::invalid_argument("Trailing characters");
        if (val < std::numeric_limits<T>::min() || val > std::numeric_limits<T>::max())
          throw std::out_of_range("Value out of range");
        return static_cast<T>(val);
      } else {
        unsigned long long val = std::stoull(s, idx_ptr, radix);
        if (!out_idx && *idx_ptr != s.length()) throw std::invalid_argument("Trailing characters");
        if (val > std::numeric_limits<T>::max()) throw std::out_of_range("Value out of range");
        return static_cast<T>(val);
      }
    } catch (const std::out_of_range &) {
      throw;
    } catch (const std::exception &) {
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
  T value_;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_INTEGER_HPP
