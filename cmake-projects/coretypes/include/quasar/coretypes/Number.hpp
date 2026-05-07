/**
 * @file Number.hpp
 * @brief Definition of the Number abstract base class.
 */

#ifndef QUASAR_CORETYPES_NUMBER_HPP
#define QUASAR_CORETYPES_NUMBER_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace quasar {
namespace coretypes {

/**
 * @brief Abstract base class for all numeric types in the Quasar framework.
 *
 * This class defines the common interface for all numeric objects, providing 
 * a unified way to handle different numeric representations such as integers 
 * (signed/unsigned, various sizes) and floating-point numbers.
 * 
 * **Compliance**:
 * - Fulfills [FE-0010.1.2] Each class related to a basic numeric type shall be derivated from a common "Number" base class.
 * 
 * The interface includes:
 * - **Conversion**: Methods to convert the numeric value to standard C++ primitive types.
 * - **Polymorphic Arithmetic**: Operations that return shared pointers to new Number instances,
 *   allowing for mixed-type arithmetic where the result type is determined by the implementation.
 * - **Safe Arithmetic**: Operations that include checks for overflow, underflow, and other
 *   arithmetic errors (like division by zero).
 * - **Comparison**: Standard equality and ordering comparisons.
 * - **Bitwise Operations**: Interface for bit-level manipulation, primarily for integer types.
 * - **Introspection**: Methods to query the underlying type properties at runtime.
 * 
 * Classes inheriting from Number should be immutable to ensure thread-safety and
 * predictable behavior when shared via std::shared_ptr.
 */
class Number {
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~Number() = default;

  /**
   * @brief Returns the value of this number as an int.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The numeric value represented by this object after conversion to 
   * type int.
   * @throws std::overflow_error If the value cannot fit into an int.
   */
  virtual int toInt() const = 0;

  /**
   * @brief Returns the value of this number as a long.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The numeric value represented by this object after conversion to 
   * type long.
   * @throws std::overflow_error If the value cannot fit into a long.
   */
  virtual long toLong() const = 0;

  /**
   * @brief Returns the value of this number as a float.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The numeric value represented by this object after conversion to 
   * type float.
   */
  virtual float toFloat() const = 0;

  /**
   * @brief Returns the value of this number as a double.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The numeric value represented by this object after conversion to 
   * type double.
   */
  virtual double toDouble() const = 0;

  /**
   * @brief Returns the value of this number as an int64_t.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The numeric value represented by this object after conversion to 
   * type int64_t.
   * @throws std::overflow_error If the value cannot fit into an int64_t.
   */
  virtual int64_t toInt64() const = 0;

  /**
   * @brief Returns the value of this number as a uint64_t.
   * 
   * Fulfills [FE-0010.1.5] Methods allowing explicit conversion to other numeric types.
   * 
   * @return The numeric value represented by this object after conversion to 
   * type uint64_t.
   * @throws std::overflow_error If the value cannot fit into a uint64_t.
   */
  virtual uint64_t toUInt64() const = 0;

  /**
   * @brief Returns the value of this number as an int32_t.
   * 
   * @return The numeric value.
   * @throws std::overflow_error If the value cannot fit.
   */
  virtual int32_t toInt32() const = 0;

  /**
   * @brief Returns the value of this number as a uint32_t.
   * 
   * @return The numeric value.
   * @throws std::overflow_error If the value cannot fit.
   */
  virtual uint32_t toUInt32() const = 0;

  /**
   * @brief Returns the value of this number as an int16_t.
   * 
   * @return The numeric value.
   * @throws std::overflow_error If the value cannot fit.
   */
  virtual int16_t toInt16() const = 0;

  /**
   * @brief Returns the value of this number as a uint16_t.
   * 
   * @return The numeric value.
   * @throws std::overflow_error If the value cannot fit.
   */
  virtual uint16_t toUInt16() const = 0;

  /**
   * @brief Returns the value of this number as an int8_t.
   * 
   * @return The numeric value.
   * @throws std::overflow_error If the value cannot fit.
   */
  virtual int8_t toInt8() const = 0;

  /**
   * @brief Returns the value of this number as a uint8_t.
   * 
   * @return The numeric value.
   * @throws std::overflow_error If the value cannot fit.
   */
  virtual uint8_t toUInt8() const = 0;

  /**
   * @brief Returns the value of this number as a size_t.
   * 
   * @return The numeric value.
   * @throws std::overflow_error If the value cannot fit.
   */
  virtual size_t toSizeT() const = 0;

  /**
   * @brief Returns the value of this number as a ptrdiff_t.
   * 
   * @return The numeric value.
   * @throws std::overflow_error If the value cannot fit.
   */
  virtual ptrdiff_t toPtrDiffT() const = 0;

  /**
   * @brief Returns a string representation of the number.
   * 
   * Fulfills [FE-0010.1.7] Methods for encoding values to a string.
   * 
   * @return A string representing the numeric value.
   */
  virtual std::string toString() const = 0;

  /**
   * @brief Compares this Number with another Number.
   * 
   * Fulfills [FE-0030.1.1] Add methods for comparison with other Number objects.
   * 
   * @param other The other Number to compare with.
   * @return -1 if this < other, 0 if this == other, or 1 if this > other.
   */
  virtual int compareTo(const Number &other) const = 0;

  /**
   * @brief Checks equality between this Number and another.
   * 
   * Fulfills [FE-0030.1.1] Add methods for comparison with other Number objects.
   * 
   * @param other The other Number to compare with.
   * @return true if both numbers represent the same value.
   */
  virtual bool equals(const Number &other) const = 0;

  // Polymorphic arithmetic variations for all base integer types.
  // Fulfills [TSK-20260308-001.3] Provide explicit method overrides or specializations for each base integer type.

#define DEFINE_POLYMORPHIC_OP(OP_NAME, RETURN_TYPE) \
  virtual RETURN_TYPE OP_NAME(signed char val) const = 0; \
  virtual RETURN_TYPE OP_NAME(unsigned char val) const = 0; \
  virtual RETURN_TYPE OP_NAME(short val) const = 0; \
  virtual RETURN_TYPE OP_NAME(unsigned short val) const = 0; \
  virtual RETURN_TYPE OP_NAME(int val) const = 0; \
  virtual RETURN_TYPE OP_NAME(unsigned int val) const = 0; \
  virtual RETURN_TYPE OP_NAME(long val) const = 0; \
  virtual RETURN_TYPE OP_NAME(unsigned long val) const = 0; \
  virtual RETURN_TYPE OP_NAME(long long val) const = 0; \
  virtual RETURN_TYPE OP_NAME(unsigned long long val) const = 0;

  // Arithmetic
  DEFINE_POLYMORPHIC_OP(add, std::shared_ptr<Number>)
  DEFINE_POLYMORPHIC_OP(subtract, std::shared_ptr<Number>)
  DEFINE_POLYMORPHIC_OP(multiply, std::shared_ptr<Number>)
  DEFINE_POLYMORPHIC_OP(divide, std::shared_ptr<Number>)

  // Safe Arithmetic
  DEFINE_POLYMORPHIC_OP(safeAdd, std::shared_ptr<Number>)
  DEFINE_POLYMORPHIC_OP(safeSubtract, std::shared_ptr<Number>)
  DEFINE_POLYMORPHIC_OP(safeMultiply, std::shared_ptr<Number>)
  DEFINE_POLYMORPHIC_OP(safeDivide, std::shared_ptr<Number>)

  // Bitwise
  DEFINE_POLYMORPHIC_OP(bitwiseAnd, std::shared_ptr<Number>)
  DEFINE_POLYMORPHIC_OP(bitwiseOr, std::shared_ptr<Number>)
  DEFINE_POLYMORPHIC_OP(bitwiseXor, std::shared_ptr<Number>)
  DEFINE_POLYMORPHIC_OP(bitwiseLeftShift, std::shared_ptr<Number>)
  DEFINE_POLYMORPHIC_OP(bitwiseRightShift, std::shared_ptr<Number>)

  // Comparison
  DEFINE_POLYMORPHIC_OP(compareTo, int)
  DEFINE_POLYMORPHIC_OP(equals, bool)

#undef DEFINE_POLYMORPHIC_OP

  /**
   * @brief Performs addition with another Number.
   * 
   * Fulfills [FE-0030.1.3] Add methods for basic arithmetic operations.
   * 
   * @param other The other Number to add.
   * @return A shared pointer to a new Number containing the sum.
   */
  virtual std::shared_ptr<Number> add(const Number &other) const = 0;

  /**
   * @brief Performs subtraction with another Number.
   * 
   * Fulfills [FE-0030.1.3] Add methods for basic arithmetic operations.
   * 
   * @param other The other Number to subtract.
   * @return A shared pointer to a new Number containing the difference.
   */
  virtual std::shared_ptr<Number> subtract(const Number &other) const = 0;

  /**
   * @brief Performs multiplication with another Number.
   * 
   * Fulfills [FE-0030.1.3] Add methods for basic arithmetic operations.
   * 
   * @param other The other Number to multiply by.
   * @return A shared pointer to a new Number containing the product.
   */
  virtual std::shared_ptr<Number> multiply(const Number &other) const = 0;

  /**
   * @brief Performs division by another Number.
   * 
   * Fulfills [FE-0030.1.3] Add methods for basic arithmetic operations.
   * 
   * @param other The other Number to divide by.
   * @return A shared pointer to a new Number containing the quotient.
   * @throws std::runtime_error If division by zero occurs.
   */
  virtual std::shared_ptr<Number> divide(const Number &other) const = 0;

  /**
   * @brief Performs addition with overflow/underflow checks.
   * 
   * Fulfills [FE-0030.1.4] Add methods for safe arithmetic operations.
   * 
   * @param other The other Number to add.
   * @return A shared pointer to a new Number containing the sum.
   * @throws std::overflow_error If the operation results in an overflow.
   */
  virtual std::shared_ptr<Number> safeAdd(const Number &other) const = 0;

  /**
   * @brief Performs subtraction with overflow/underflow checks.
   * 
   * Fulfills [FE-0030.1.4] Add methods for safe arithmetic operations.
   * 
   * @param other The other Number to subtract.
   * @return A shared pointer to a new Number containing the difference.
   * @throws std::overflow_error If the operation results in an overflow.
   */
  virtual std::shared_ptr<Number> safeSubtract(const Number &other) const = 0;

  /**
   * @brief Performs multiplication with overflow/underflow checks.
   * 
   * Fulfills [FE-0030.1.4] Add methods for safe arithmetic operations.
   * 
   * @param other The other Number to multiply by.
   * @return A shared pointer to a new Number containing the product.
   * @throws std::overflow_error If the operation results in an overflow.
   */
  virtual std::shared_ptr<Number> safeMultiply(const Number &other) const = 0;

  /**
   * @brief Performs division with safety checks.
   * 
   * Fulfills [FE-0030.1.4] Add methods for safe arithmetic operations.
   * 
   * @param other The other Number to divide by.
   * @return A shared pointer to a new Number containing the quotient.
   * @throws std::runtime_error If division by zero occurs.
   * @throws std::overflow_error If the operation results in an overflow (e.g., INT_MIN / -1).
   */
  virtual std::shared_ptr<Number> safeDivide(const Number &other) const = 0;

  /**
   * @brief Performs a bitwise AND operation.
   * 
   * Fulfills [FE-0030.1.5] Add methods for bitwise operations.
   * 
   * @param other The other Number.
   * @return A shared pointer to a new Number containing the result.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseAnd(const Number &other) const = 0;

  /**
   * @brief Performs a bitwise OR operation.
   * 
   * Fulfills [FE-0030.1.5] Add methods for bitwise operations.
   * 
   * @param other The other Number.
   * @return A shared pointer to a new Number containing the result.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseOr(const Number &other) const = 0;

  /**
   * @brief Performs a bitwise XOR operation.
   * 
   * Fulfills [FE-0030.1.5] Add methods for bitwise operations.
   * 
   * @param other The other Number.
   * @return A shared pointer to a new Number containing the result.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseXor(const Number &other) const = 0;

  /**
   * @brief Performs a bitwise NOT operation.
   * 
   * Fulfills [FE-0030.1.5] Add methods for bitwise operations.
   * 
   * @return A shared pointer to a new Number containing the bitwise complement.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseNot() const = 0;

  /**
   * @brief Performs a bitwise Left Shift operation.
   * 
   * Fulfills [FE-0030.1.5] Add methods for bitwise operations.
   * 
   * @param other The number of positions to shift.
   * @return A shared pointer to a new Number containing the result.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseLeftShift(const Number &other) const = 0;

  /**
   * @brief Performs a bitwise Right Shift operation.
   * 
   * Fulfills [FE-0030.1.5] Add methods for bitwise operations.
   * 
   * @param other The number of positions to shift.
   * @return A shared pointer to a new Number containing the result.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseRightShift(const Number &other) const = 0;

  /**
   * @brief Gets the type name of the number.
   * 
   * Fulfills [FE-0030.1.9] Add methods allowing some form of reflection and introspection.
   * 
   * @return A string representing the type (e.g., "Integer", "FloatingPoint").
   */
  virtual std::string getType() const = 0;

  /**
   * @brief Checks if the number is an integral type.
   * 
   * Fulfills [FE-0030.1.9] Add methods allowing some form of reflection and introspection.
   * 
   * @return true if the underlying type is an integer.
   */
  virtual bool isIntegerType() const = 0;

  /**
   * @brief Checks if the number is signed.
   * 
   * Fulfills [FE-0030.1.9] Add methods allowing some form of reflection and introspection.
   * 
   * @return true if the underlying type can represent negative values.
   */
  virtual bool isSigned() const = 0;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_NUMBER_HPP
