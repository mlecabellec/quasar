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
 * This class is immutable and thread-safe.
 */
class Boolean {
public:
  /**
   * @brief Allocates a Boolean object representing the value argument.
   * @param value The value of the Boolean.
   */
  explicit Boolean(bool value);

  /**
   * @brief Allocates a Boolean object representing the value true if the string
   * argument is not null and is equal, ignoring case, to the string "true".
   * @param s The string to be converted to a Boolean.
   */
  explicit Boolean(const std::string &s);

  /**
   * @brief Allocates a Boolean object from a C-string.
   * @param s The string to be converted to a Boolean.
   */
  explicit Boolean(const char *s);

  /**
   * @brief Returns the value of this Boolean object as a boolean primitive.
   * @return the primitive boolean value of this object.
   */
  bool booleanValue() const;

  /**
   * @brief Returns a String object representing this Boolean's value.
   * @return a string representation of this object.
   */
  std::string toString() const;

  /**
   * @brief Parses the string argument as a boolean.
   * @param s the String containing the boolean representation to be parsed.
   * @return the boolean represented by the string argument.
   */
  static bool parseBoolean(const std::string &s);

  /**
   * @brief Converts a numeric value to a Boolean.
   *
   * Comparison with 0 is used: 0 is false, anything else is true.
   *
   * @tparam T The numeric type.
   * @param value The numeric value.
   * @return Boolean representation.
   */
  template <typename T> static Boolean fromNumeric(T value) {
    return Boolean(value != 0);
  }

private:
  /**
   * @brief The primitive value.
   */
  bool value_;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_BOOLEAN_HPP
