#include "quasar/coretypes/Boolean.hpp"
#include <algorithm>
#include <cctype>

namespace quasar {
namespace coretypes {

Boolean::Boolean(bool value) : value_(value) {
  // Initialize the boolean value directly from the parameter.
}

Boolean::Boolean(const std::string &s) : value_(parseBoolean(s)) {
  // Initialize the boolean value by parsing the string parameter.
}

Boolean::Boolean(const char *s) {
  // Check for null pointer as per CS-0010.
  if (s == nullptr) {
    value_ = false;
  } else {
    value_ = parseBoolean(s);
  }
}

bool Boolean::booleanValue() const {
  // Return the internal primitive boolean value.
  return value_;
}

std::string Boolean::toString() const {
  // Return "true" if the value is true, "false" otherwise.
  return value_ ? "true" : "false";
}

bool Boolean::parseBoolean(const std::string &s) {
  // Check if the string length is 4. If not, it cannot be "true".
  if (s.length() != 4) {
    return false;
  }

  // Create a lowercase copy of the string to perform case-insensitive
  // comparison.
  std::string lower = s;
  std::transform(lower.begin(), lower.end(), lower.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  // Compare the lowercase string with "true".
  // If equal, return true; otherwise, return false.
  return lower == "true";
}

} // namespace coretypes
} // namespace quasar
