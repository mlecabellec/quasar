/**
 * @file Number.hpp
 * @brief Definition of the Number abstract base class.
 */

#ifndef QUASAR_CORETYPES_NUMBER_HPP
#define QUASAR_CORETYPES_NUMBER_HPP

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
   * @return The numeric value represented by this object after conversion to 
   * type int.
   * @throws std::overflow_error If the value cannot fit into an int.
   */
  virtual int toInt() const = 0;

  /**
   * @brief Returns the value of this number as a long.
   * 
   * @return The numeric value represented by this object after conversion to 
   * type long.
   * @throws std::overflow_error If the value cannot fit into a long.
   */
  virtual long toLong() const = 0;

  /**
   * @brief Returns the value of this number as a float.
   * 
   * @return The numeric value represented by this object after conversion to 
   * type float.
   */
  virtual float toFloat() const = 0;

  /**
   * @brief Returns the value of this number as a double.
   * 
   * @return The numeric value represented by this object after conversion to 
   * type double.
   */
  virtual double toDouble() const = 0;

  /**
   * @brief Returns a string representation of the number.
   * @return A string representing the numeric value.
   */
  virtual std::string toString() const = 0;

  /**
   * @brief Compares this Number with another Number.
   * 
   * @param other The other Number to compare with.
   * @return -1 if this < other, 0 if this == other, or 1 if this > other.
   */
  virtual int compareTo(const Number &other) const = 0;

  /**
   * @brief Checks equality between this Number and another.
   * 
   * @param other The other Number to compare with.
   * @return true if both numbers represent the same value.
   */
  virtual bool equals(const Number &other) const = 0;

  /**
   * @brief Performs addition with another Number.
   * 
   * @param other The other Number to add.
   * @return A shared pointer to a new Number containing the sum.
   */
  virtual std::shared_ptr<Number> add(const Number &other) const = 0;

  /**
   * @brief Performs subtraction with another Number.
   * 
   * @param other The other Number to subtract.
   * @return A shared pointer to a new Number containing the difference.
   */
  virtual std::shared_ptr<Number> subtract(const Number &other) const = 0;

  /**
   * @brief Performs multiplication with another Number.
   * 
   * @param other The other Number to multiply by.
   * @return A shared pointer to a new Number containing the product.
   */
  virtual std::shared_ptr<Number> multiply(const Number &other) const = 0;

  /**
   * @brief Performs division by another Number.
   * 
   * @param other The other Number to divide by.
   * @return A shared pointer to a new Number containing the quotient.
   * @throws std::runtime_error If division by zero occurs.
   */
  virtual std::shared_ptr<Number> divide(const Number &other) const = 0;

  /**
   * @brief Performs addition with overflow/underflow checks.
   * 
   * @param other The other Number to add.
   * @return A shared pointer to a new Number containing the sum.
   * @throws std::overflow_error If the operation results in an overflow.
   */
  virtual std::shared_ptr<Number> safeAdd(const Number &other) const = 0;

  /**
   * @brief Performs subtraction with overflow/underflow checks.
   * 
   * @param other The other Number to subtract.
   * @return A shared pointer to a new Number containing the difference.
   * @throws std::overflow_error If the operation results in an overflow.
   */
  virtual std::shared_ptr<Number> safeSubtract(const Number &other) const = 0;

  /**
   * @brief Performs multiplication with overflow/underflow checks.
   * 
   * @param other The other Number to multiply by.
   * @return A shared pointer to a new Number containing the product.
   * @throws std::overflow_error If the operation results in an overflow.
   */
  virtual std::shared_ptr<Number> safeMultiply(const Number &other) const = 0;

  /**
   * @brief Performs division with safety checks.
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
   * @param other The other Number.
   * @return A shared pointer to a new Number containing the result.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseAnd(const Number &other) const = 0;

  /**
   * @brief Performs a bitwise OR operation.
   * 
   * @param other The other Number.
   * @return A shared pointer to a new Number containing the result.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseOr(const Number &other) const = 0;

  /**
   * @brief Performs a bitwise XOR operation.
   * 
   * @param other The other Number.
   * @return A shared pointer to a new Number containing the result.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseXor(const Number &other) const = 0;

  /**
   * @brief Performs a bitwise NOT operation.
   * 
   * @return A shared pointer to a new Number containing the bitwise complement.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseNot() const = 0;

  /**
   * @brief Performs a bitwise Left Shift operation.
   * 
   * @param amount The number of positions to shift.
   * @return A shared pointer to a new Number containing the result.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseLeftShift(int amount) const = 0;

  /**
   * @brief Performs a bitwise Right Shift operation.
   * 
   * @param amount The number of positions to shift.
   * @return A shared pointer to a new Number containing the result.
   * @throws std::runtime_error If the number type does not support bitwise operations.
   */
  virtual std::shared_ptr<Number> bitwiseRightShift(int amount) const = 0;

  /**
   * @brief Gets the type name of the number.
   * @return A string representing the type (e.g., "Integer", "FloatingPoint").
   */
  virtual std::string getType() const = 0;

  /**
   * @brief Checks if the number is an integral type.
   * @return true if the underlying type is an integer.
   */
  virtual bool isIntegerType() const = 0;

  /**
   * @brief Checks if the number is signed.
   * @return true if the underlying type can represent negative values.
   */
  virtual bool isSigned() const = 0;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_NUMBER_HPP
