#ifndef QUASAR_CORETYPES_STRING_HPP
#define QUASAR_CORETYPES_STRING_HPP

#include <string>
#include <vector>

namespace quasar::coretypes {

/**
 * @brief Wrapper class for std::string.
 */
class String {
public:
  /**
   * @brief Default constructor.
   */
  String();

  /**
   * @brief Constructs from std::string.
   * @param s The string value.
   */
  String(const std::string &s);

  /**
   * @brief Constructs from C-string.
   * @param s The C-string value.
   */
  String(const char *s);

  /**
   * @brief Destructor.
   */
  virtual ~String() = default;

  /**
   * @brief Gets the string value.
   * @return The std::string value.
   */
  std::string toString() const;

  /**
   * @brief Gets reference to internal string value.
   * @return Const reference to std::string.
   */
  const std::string &value() const;

  /**
   * @brief Gets the length of the string.
   * @return Length.
   */
  size_t length() const;

  /**
   * @brief Checks if string is empty.
   * @return true if empty.
   */
  bool empty() const;

  /**
   * @brief Checks equality.
   * @param other The other String.
   * @return true if equal.
   */
  bool equals(const String &other) const;

  /**
   * @brief Compares lexicographically.
   * @param other The other String.
   * @return Comparison result.
   */
  int compareTo(const String &other) const;

private:
  /**
   * @brief The internal string value.
   */
  std::string value_;
};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_STRING_HPP
