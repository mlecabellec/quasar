#include "Smp/Uuid.h"
#include "Smp/Exception.h"
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

namespace Smp {
namespace {
class UuidException final : public Exception {
public:
  UuidException(String8 message) : m_message(message) {}

  const char *what() const noexcept override { return m_message.c_str(); }

  String8 GetName() const noexcept override { return "UuidException"; }

  String8 GetDescription() const noexcept override {
    return "Exception thrown when parsing an invalid UUID string.";
  }

  String8 GetMessage() const noexcept override { return m_message.c_str(); }

  const IObject *GetSender() const noexcept override { return nullptr; }

private:
  std::string m_message;
};

int HexCharToInt(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}
} // namespace

Uuid::Uuid(const char *value) {
  if (!value || std::strlen(value) != 36) {
    throw UuidException("Invalid UUID string length (must be 36 characters)");
  }

  // Expected format: 8-4-4-4-12 hex digits
  // indices: 0-7, 9-12, 14-17, 19-22, 24-35. dashes at 8, 13, 18, 23

  if (value[8] != '-' || value[13] != '-' || value[18] != '-' ||
      value[23] != '-') {
    throw UuidException("Invalid UUID string format (missing dashes)");
  }

  auto ParseHex = [&](int start, int length, uint64_t &out) {
    out = 0;
    for (int i = 0; i < length; ++i) {
      int val = HexCharToInt(value[start + i]);
      if (val < 0)
        throw UuidException("Invalid hex character in UUID");
      out = (out << 4) | val;
    }
  };

  uint64_t temp = 0;

  ParseHex(0, 8, temp);
  Data1 = static_cast<uint32_t>(temp);

  ParseHex(9, 4, temp);
  Data2[0] = static_cast<uint16_t>(temp);

  ParseHex(14, 4, temp);
  Data2[1] = static_cast<uint16_t>(temp);

  ParseHex(19, 4, temp);
  Data2[2] = static_cast<uint16_t>(temp);

  ParseHex(24, 2, temp);
  Data3[0] = static_cast<uint8_t>(temp);
  ParseHex(26, 2, temp);
  Data3[1] = static_cast<uint8_t>(temp);
  ParseHex(28, 2, temp);
  Data3[2] = static_cast<uint8_t>(temp);
  ParseHex(30, 2, temp);
  Data3[3] = static_cast<uint8_t>(temp);
  ParseHex(32, 2, temp);
  Data3[4] = static_cast<uint8_t>(temp);
  ParseHex(34, 2, temp);
  Data3[5] = static_cast<uint8_t>(temp);
}

bool Uuid::operator==(const Uuid &other) const {
  return Data1 == other.Data1 && Data2 == other.Data2 && Data3 == other.Data3;
}

bool Uuid::operator!=(const Uuid &other) const { return !(*this == other); }

bool Uuid::operator<(const Uuid &other) const {
  if (Data1 != other.Data1)
    return Data1 < other.Data1;
  if (Data2 != other.Data2)
    return Data2 < other.Data2;
  return Data3 < other.Data3;
}

std::ostream &operator<<(std::ostream &os, const Uuid &uuid) {
  auto flags = os.flags();
  os << std::hex << std::setfill('0');
  os << std::setw(8) << uuid.Data1 << '-';
  os << std::setw(4) << uuid.Data2[0] << '-';
  os << std::setw(4) << uuid.Data2[1] << '-';
  os << std::setw(4) << uuid.Data2[2] << '-';
  os << std::setw(2) << static_cast<int>(uuid.Data3[0]);
  os << std::setw(2) << static_cast<int>(uuid.Data3[1]);
  os << std::setw(2) << static_cast<int>(uuid.Data3[2]);
  os << std::setw(2) << static_cast<int>(uuid.Data3[3]);
  os << std::setw(2) << static_cast<int>(uuid.Data3[4]);
  os << std::setw(2) << static_cast<int>(uuid.Data3[5]);
  os.flags(flags);
  return os;
}
} // namespace Smp

namespace std {
size_t hash<Smp::Uuid>::operator()(const Smp::Uuid &uuid) const {
  // Simple hash combination
  size_t h1 = std::hash<uint32_t>{}(uuid.Data1);
  size_t h2 = 0;
  for (auto v : uuid.Data2)
    h2 ^= std::hash<uint16_t>{}(v) + 0x9e3779b9 + (h2 << 6) + (h2 >> 2);
  size_t h3 = 0;
  for (auto v : uuid.Data3)
    h3 ^= std::hash<uint8_t>{}(v) + 0x9e3779b9 + (h3 << 6) + (h3 >> 2);

  return h1 ^ (h2 << 1) ^ (h3 << 2);
}
} // namespace std
