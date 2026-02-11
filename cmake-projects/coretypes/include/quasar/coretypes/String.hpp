/**
 * @file String.hpp
 * @brief Definition of the String wrapper class.
 */

#ifndef QUASAR_CORETYPES_STRING_HPP
#define QUASAR_CORETYPES_STRING_HPP

#include <string>
#include <vector>

namespace quasar::coretypes {

/**
 * @brief Wrapper class for std::string providing a consistent API within the framework.
 *
 * This class provides a bridge between standard C++ strings and the Quasar core
 * type system. It is designed to be immutable, ensuring that string data can
 * be safely shared across threads without explicit locking for read operations.
 * 
 * It provides common string operations like length retrieval, empty checks,
 * and lexicographical comparison, while maintaining a clear path to the 
 * underlying std::string for compatibility with other libraries.
 */
class String {
public:
  /**
   * @brief Default constructor.
   * Initializes an empty string.
   */
  String();

  /**
   * @brief Constructs a String object from a std::string.
   * @param s The std::string value to wrap.
   */
  String(const std::string &s);

  /**
   * @brief Constructs a String object from a C-style string.
   * @param s The null-terminated C-string value to wrap.
   */
  String(const char *s);

  /**
   * @brief Virtual destructor.
   */
  virtual ~String() = default;

  /**
   * @brief Returns a copy of the underlying string value.
   * @return The std::string value.
   */
  std::string toString() const;

  /**
   * @brief Provides a constant reference to the internal string value.
   * 
   * Useful for avoiding unnecessary copies when only read access is needed.
   * @return Constant reference to the internal std::string.
   */
  const std::string &value() const;

  /**
   * @brief Returns the number of characters in the string.
   * @return The length of the string in characters.
   */
  size_t length() const;

  /**
   * @brief Checks if the string contains no characters.
   * @return true if the string length is 0, false otherwise.
   */
  bool empty() const;

  /**
   * @brief Checks if this String is identical to another String.
   * 
   * @param other The other String instance to compare with.
   * @return true if both strings have the same content.
   */
  bool equals(const String &other) const;

  /**
   * @brief Compares this String lexicographically with another String.
   * 
   * @param other The other String instance to compare with.
   * @return A negative value if this < other, zero if they are equal, 
   *         or a positive value if this > other.
   */
  int compareTo(const String &other) const;

private:
  /**
   * @brief The internal std::string storage.
   */
  std::string value_;
};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_STRING_HPP
