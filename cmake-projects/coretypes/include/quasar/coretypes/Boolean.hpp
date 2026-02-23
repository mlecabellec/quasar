/**
 * @file Boolean.hpp
 * @brief Definition of the Boolean wrapper class.
 */

#ifndef QUASAR_CORETYPES_BOOLEAN_HPP
#define QUASAR_CORETYPES_BOOLEAN_HPP

#include <string>

namespace quasar {
namespace coretypes {

/**
 * @brief The Boolean class wraps a value of the primitive type boolean in an
 * object.
 *
 * An object of type Boolean contains a single field whose type is boolean.
 * This class provides a set of methods for converting between boolean, String, 
 * and numeric types. 
 * 
 * **Compliance**:
 * - Fulfills [FE-0010.2] Provide a Boolean class.
 * - Fulfills [FE-0050.1.3] Mapping for SMP "Bool" Primitive Type.
 * 
 * This class is immutable and thread-safe. It follows the Java-style Boolean
 * wrapper conventions.
 */
class Boolean {
public:
  /**
   * @brief Allocates a Boolean object representing the value argument.
   * 
   * Fulfills [FE-0010.2.1] Encoding and decoding values to and from a basic boolean type.
   * 
   * @param value The value of the Boolean.
   */
  explicit Boolean(bool value);

  /**
   * @brief Allocates a Boolean object representing the value true if the string
   * argument is not null and is equal, ignoring case, to the string "true".
   *
   * Otherwise, allocates a Boolean object representing the value false.
   * 
   * Fulfills [FE-0010.2.2] Encoding and decoding values to and from a string.
   *
   * @param s The string to be converted to a Boolean.
   */
  explicit Boolean(const std::string &s);

  /**
   * @brief Allocates a Boolean object from a C-string.
   *
   * Similar to the std::string constructor, it checks for case-insensitive "true".
   *
   * @param s The C-string to be converted to a Boolean.
   */
  explicit Boolean(const char *s);

  /**
   * @brief Returns the value of this Boolean object as a boolean primitive.
   * @return the primitive boolean value of this object.
   */
  bool booleanValue() const;

  /**
   * @brief Returns a String object representing this Boolean's value.
   *
   * Returns "true" if the value is true, and "false" otherwise.
   * 
   * Fulfills [FE-0010.2.2] Encoding and decoding values to and from a string.
   *
   * @return a string representation of this object.
   */
  std::string toString() const;

  /**
   * @brief Parses the string argument as a boolean.
   *
   * The boolean returned represents the value true if the string argument 
   * is not null and is equal, ignoring case, to the string "true".
   *
   * @param s the String containing the boolean representation to be parsed.
   * @return the boolean represented by the string argument.
   */
  static bool parseBoolean(const std::string &s);

  /**
   * @brief Converts a numeric value to a Boolean.
   *
   * Comparison with 0 is used: 0 is false, anything else is true.
   * 
   * Fulfills [FE-0010.2.3] Methods for conversion from numeric types.
   *
   * @tparam T The numeric type.
   * @param value The numeric value.
   * @return A Boolean instance representing the numeric value.
   */
  template <typename T> static Boolean fromNumeric(T value) {
    // Standard C convention: 0 is false, everything else is true.
    return Boolean(value != 0);
  }

  /**
   * @brief Checks if this Boolean's value is equal to a primitive boolean value.
   * @param other The primitive boolean to compare with.
   * @return true if both values are the same.
   */
  bool equals(bool other) const { 
    // Direct comparison with the internal primitive value.
    return value_ == other; 
  }

  /**
   * @brief Compares this Boolean value with a primitive boolean value.
   *
   * @param other The primitive boolean to compare with.
   * @return 0 if equal, 1 if this is true and other is false, -1 if this is false and other is true.
   */
  int compareTo(bool other) const { 
      // Return 0 if the values are identical.
      if (value_ == other) return 0;
      // If not equal, 'true' is considered greater than 'false'.
      return value_ ? 1 : -1; 
  }

  /**
   * @brief Equality operator comparison with a primitive boolean.
   * @param other The primitive boolean to compare with.
   * @return true if equal.
   */
  bool operator==(bool other) const { return value_ == other; }

  /**
   * @brief Inequality operator comparison with a primitive boolean.
   * @param other The primitive boolean to compare with.
   * @return true if not equal.
   */
  bool operator!=(bool other) const { return value_ != other; }

private:
  /**
   * @brief The internal primitive boolean value.
   */
  bool value_;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_BOOLEAN_HPP
