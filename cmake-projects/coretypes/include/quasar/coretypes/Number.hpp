#ifndef QUASAR_CORETYPES_NUMBER_HPP
#define QUASAR_CORETYPES_NUMBER_HPP

#include <memory>
#include <string>

namespace quasar {
namespace coretypes {

/**
 * @brief Abstract base class for all numeric types.
 *
 * This class defines the common interface for numeric types, mainly focusing on
 * conversion capabilities to primitive C++ types.
 */
class Number {
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~Number() = default;

  /**
   * @brief Returns the value of the specified number as an int.
   * @return The numeric value represented by this object after conversion to
   * type int.
   */
  virtual int toInt() const = 0;

  /**
   * @brief Returns the value of the specified number as a long.
   * @return The numeric value represented by this object after conversion to
   * type long.
   */
  virtual long toLong() const = 0;

  /**
   * @brief Returns the value of the specified number as a float.
   * @return The numeric value represented by this object after conversion to
   * type float.
   */
  virtual float toFloat() const = 0;

  /**
   * @brief Returns the value of the specified number as a double.
   * @return The numeric value represented by this object after conversion to
   * type double.
   */
  virtual double toDouble() const = 0;

  /**
   * @brief Returns a string representation of the number.
   * @return Variable represented as a string.
   */
  virtual std::string toString() const = 0;

  // --- Comparison ---
  // --- Comparison ---
  /**
   * @brief Compares this Number with another.
   * @param other The other Number.
   * @return -1, 0, or 1.
   */
  virtual int compareTo(const Number &other) const = 0;

  /**
   * @brief Checks equality.
   * @param other The other Number.
   * @return true if equal.
   */
  virtual bool equals(const Number &other) const = 0;

  // --- Arithmetic (Polymorphic) ---
  // --- Arithmetic (Polymorphic) ---
  /**
   * @brief Adds another Number to this one.
   * @param other The other Number.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> add(const Number &other) const = 0;

  /**
   * @brief Subtracts another Number from this one.
   * @param other The other Number.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> subtract(const Number &other) const = 0;

  /**
   * @brief Multiplies this Number by another.
   * @param other The other Number.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> multiply(const Number &other) const = 0;

  /**
   * @brief Divides this Number by another.
   * @param other The other Number.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> divide(const Number &other) const = 0;

  // --- Safe Arithmetic ---
  // --- Safe Arithmetic ---
  /**
   * @brief Safely adds another Number (overflow check).
   * @param other The other Number.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> safeAdd(const Number &other) const = 0;

  /**
   * @brief Safely subtracts another Number.
   * @param other The other Number.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> safeSubtract(const Number &other) const = 0;

  /**
   * @brief Safely multiplies by another Number.
   * @param other The other Number.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> safeMultiply(const Number &other) const = 0;

  /**
   * @brief Safely divides by another Number.
   * @param other The other Number.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> safeDivide(const Number &other) const = 0;

  // --- Bitwise ---
  // --- Bitwise ---
  /**
   * @brief Bitwise AND.
   * @param other The other Number.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> bitwiseAnd(const Number &other) const = 0;

  /**
   * @brief Bitwise OR.
   * @param other The other Number.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> bitwiseOr(const Number &other) const = 0;

  /**
   * @brief Bitwise XOR.
   * @param other The other Number.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> bitwiseXor(const Number &other) const = 0;

  /**
   * @brief Bitwise NOT.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> bitwiseNot() const = 0;

  /**
   * @brief Bitwise Left Shift.
   * @param amount Shift amount.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> bitwiseLeftShift(int amount) const = 0;

  /**
   * @brief Bitwise Right Shift.
   * @param amount Shift amount.
   * @return Result as new Number.
   */
  virtual std::shared_ptr<Number> bitwiseRightShift(int amount) const = 0;

  // --- Introspection ---
  // --- Introspection ---
  /**
   * @brief Gets the type name of the number.
   * @return Type name string.
   */
  virtual std::string getType() const = 0;

  /**
   * @brief Checks if the number is an integer type.
   * @return true if integer type.
   */
  virtual bool isIntegerType() const = 0;

  /**
   * @brief Checks if the number is signed.
   * @return true if signed.
   */
  virtual bool isSigned() const = 0;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_NUMBER_HPP
