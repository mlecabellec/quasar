#ifndef SMP_UUID_H
#define SMP_UUID_H

#include <array>
#include <cstdint>
#include <functional>
#include <iosfwd>

namespace Smp {
/// Universally Unique Identifier (UUID) as defined by the Open Group.
struct Uuid final {
  /// First 8 hex nibbles (4 bytes).
  uint32_t Data1;

  /// Next 3 groups of 4 hex nibbles (3 x 2 bytes).
  std::array<uint16_t, 3> Data2;

  /// Final 6 groups of 2 hex nibbles (6 x 1 byte).
  std::array<uint8_t, 6> Data3;

  /// Default constructor.
  constexpr Uuid() noexcept : Uuid(0, 0, 0, 0, 0) {}

  /// Constructor with direct data initialization.
  constexpr Uuid(uint32_t data1, const std::array<uint16_t, 3> &data2,
                 const std::array<uint8_t, 6> &data3) noexcept
      : Data1(data1), Data2(data2), Data3(data3) {}

  /// Constructor with individual fields.
  constexpr Uuid(uint32_t data1, uint16_t data2_0, uint16_t data2_1,
                 uint16_t data2_2, uint64_t data3) noexcept
      : Uuid(data1, std::array<uint16_t, 3>{data2_0, data2_1, data2_2},
             std::array<uint8_t, 6>{static_cast<uint8_t>(data3 >> 40),
                                    static_cast<uint8_t>(data3 >> 32),
                                    static_cast<uint8_t>(data3 >> 24),
                                    static_cast<uint8_t>(data3 >> 16),
                                    static_cast<uint8_t>(data3 >> 8),
                                    static_cast<uint8_t>(data3)}) {}

  /// Constructor compatible with SMP2 string format representation.
  constexpr Uuid(uint32_t data1, uint16_t data2_0, uint16_t data2_1,
                 const std::array<char, 8> &data3) noexcept
      : Uuid(data1,
             std::array<uint16_t, 3>{
                 data2_0, data2_1,
                 static_cast<uint16_t>((static_cast<uint8_t>(data3[0]) << 8) |
                                       static_cast<uint8_t>(data3[1]))},
             std::array<uint8_t, 6>{
                 static_cast<uint8_t>(data3[2]), static_cast<uint8_t>(data3[3]),
                 static_cast<uint8_t>(data3[4]), static_cast<uint8_t>(data3[5]),
                 static_cast<uint8_t>(data3[6]),
                 static_cast<uint8_t>(data3[7])}) {}

  /// Construct from string representation.
  explicit Uuid(const char *value);

  constexpr Uuid(const Uuid &) noexcept = default;
  constexpr Uuid(Uuid &&) noexcept = default;
  Uuid &operator=(const Uuid &) noexcept = default;
  Uuid &operator=(Uuid &&) noexcept = default;

  bool operator==(const Uuid &other) const;
  bool operator!=(const Uuid &other) const;
  bool operator<(const Uuid &other) const;
};

std::ostream &operator<<(std::ostream &os, const Uuid &uuid);
} // namespace Smp

namespace std {
template <> struct hash<Smp::Uuid> {
  size_t operator()(const Smp::Uuid &uuid) const;
};
} // namespace std

#endif // SMP_UUID_H
