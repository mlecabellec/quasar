#include "quasar/coretypes/String.hpp"

namespace quasar::coretypes {

String::String() : value_("") {}

String::String(const std::string &s) : value_(s) {}

String::String(const char *s) : value_(s ? s : "") {}

std::string String::toString() const {
  // Return a copy of the internal string value.
  return value_;
}

const std::string &String::value() const { return value_; }

size_t String::length() const { return value_.length(); }

bool String::empty() const { return value_.empty(); }

bool String::equals(const String &other) const {
  // Compare internal string values.
  return value_ == other.value_;
}

int String::compareTo(const String &other) const {
  // Use std::string::compare for lexicographical comparison.
  return value_.compare(other.value_);
}

} // namespace quasar::coretypes
