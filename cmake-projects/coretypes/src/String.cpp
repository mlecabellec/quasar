#include "quasar/coretypes/String.hpp"

namespace quasar::coretypes {

String::String() : value_("") {
  // Fulfills [FE-0030.3] Add a String class as a wrapper around std::string.
  // Default constructor initializes the internal storage as an empty string.
}

String::String(const std::string &s) : value_(s) {
  // Fulfills [FE-0030.3] Add a String class as a wrapper around std::string.
  // Wraps an existing std::string object.
}

String::String(const char *s) : value_(s ? s : "") {
  // Fulfills [FE-0030.3] Add a String class as a wrapper around std::string.
  // Defensive check: if a null C-string pointer is provided, initialize as an empty string.
  // This prevents potential crashes when constructed with NULL.
}

std::string String::toString() const {
  // Returns a deep copy of the internal std::string.
  return value_;
}

const std::string &String::value() const { 
  // Provides direct read-only access to the internal representation.
  // Useful for efficiency to avoid string copies.
  return value_; 
}

size_t String::length() const { 
  // Delegates to the underlying std::string length() method.
  // Returns the number of characters in the string.
  return value_.length(); 
}

bool String::empty() const { 
  // Efficiently checks if the string contains no characters.
  return value_.empty(); 
}

bool String::equals(const String &other) const {
  // Direct equality comparison of the underlying byte sequences.
  return value_ == other.value_;
}

int String::compareTo(const String &other) const {
  // Performs a standard lexicographical comparison.
  // Returns < 0 if this is less than other, 0 if equal, and > 0 if greater.
  return value_.compare(other.value_);
}

} // namespace quasar::coretypes
