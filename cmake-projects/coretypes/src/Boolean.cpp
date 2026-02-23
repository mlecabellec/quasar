#include "quasar/coretypes/Boolean.hpp"
#include <algorithm>
#include <cctype>

namespace quasar {
namespace coretypes {

Boolean::Boolean(bool value) : value_(value) {
  // Fulfills [FE-0010.2.1] Encoding and decoding values to and from a basic boolean type.
  // Initialize the boolean value directly from the primitive parameter.
  // This class is immutable, so the value won't change after construction.
}

Boolean::Boolean(const std::string &s) : value_(parseBoolean(s)) {
  // Fulfills [FE-0010.2.2] Encoding and decoding values to and from a string.
  // Initialize the boolean value by parsing the string parameter.
  // parseBoolean handles the case-insensitive logic.
}

Boolean::Boolean(const char *s) {
  // Defensive check for null pointers.
  // According to specification (and common Java-like behavior), null strings parse to false.
  if (s == nullptr) {
    value_ = false;
  } else {
    // Convert the C-style string to a std::string and then delegate to parseBoolean.
    value_ = parseBoolean(std::string(s));
  }
}

bool Boolean::booleanValue() const {
  // Fulfills [FE-0010.2.1] Encoding and decoding values to and from a basic boolean type.
  // Returns the internal primitive bool state.
  return value_;
}

std::string Boolean::toString() const {
  // Fulfills [FE-0010.2.2] Encoding and decoding values to and from a string.
  // Standard string representation: returns "true" or "false".
  return value_ ? "true" : "false";
}

bool Boolean::parseBoolean(const std::string &s) {
  // Fulfills [FE-0010.2.2] Encoding and decoding values to and from a string.
  // Fast path: only strings with exactly 4 characters can be "true".
  // This optimization avoids unnecessary string copies or transformations for other values.
  if (s.length() != 4) {
    return false;
  }

  // Perform a case-insensitive comparison by converting a copy of the input to lowercase.
  std::string lower = s;
  std::transform(lower.begin(), lower.end(), lower.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  // Only the exact lowercase string "true" results in a true boolean value.
  // All other strings (including "false", "maybe", etc.) result in false.
  return lower == "true";
}

} // namespace coretypes
} // namespace quasar
