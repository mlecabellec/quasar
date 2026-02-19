/**
 * @file IntegerCodec.hpp
 * @brief Codec implementation for integer types.
 */

#ifndef DATACODEC_INTEGERCODEC_HPP
#define DATACODEC_INTEGERCODEC_HPP

#include "datacodec/ICodec.hpp"
#include "quasar/coretypes/Integer.hpp"
#include "quasar/named/NamedInteger.hpp"
#include <algorithm>
#include <stdexcept>
#include <type_traits>

namespace datacodec {

/**
 * @class IntegerCodec
 * @brief Codec for encoding/decoding integer values.
 *
 * Supports different integer types (T), endianness, and bit counts.
 * Note: Bit counting logic should ideally be handled by the BitBufferSlice's
 * getBits/setBits methods, or we manually pack/unpack if not supported.
 * For this initial implementation, we assume byte-aligned operations or
 * simple bit operations provided by coretypes.
 */
template <typename T> class IntegerCodec : public ICodec {
public:
  static_assert(std::is_integral<T>::value,
                "IntegerCodec only supports integral types");

  /**
   * @brief Constructs an IntegerCodec.
   * @param bitSize Number of bits for the integer (usually sizeof(T) * 8).
   * @param isBigEndian True for Big Endian, False for Little Endian.
   */
  IntegerCodec(size_t bitSize, bool isBigEndian = true)
      : m_bitSize(bitSize), m_isBigEndian(isBigEndian) {}

  std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const override {

    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for IntegerCodec");
    }

    // Simplification: construct value from bits.
    // Real implementation depends heavily on BitBufferSlice API capabilities.
    // Assuming we extract to a T type.
    T value = 0;

    // TODO: Replace with actual BitBufferSlice extraction logic
    // This is a placeholder for the bit extraction logic
    // value = buffer.readBits<T>(0, m_bitSize, m_isBigEndian);

    // Return a NamedInteger unnamed (name to be assigned by parent
    // schema/container)
    return quasar::named::NamedInteger<T>::create("", value);
  }

  void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
              quasar::coretypes::BitBufferSlice &buffer) const override {

    auto namedInt =
        std::dynamic_pointer_cast<quasar::named::NamedInteger<T>>(value);
    if (!namedInt) {
      throw std::invalid_argument("Invalid type for IntegerCodec encoding");
    }

    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for IntegerCodec encoding");
    }

    T rawValue = namedInt->value();

    // TODO: Replace with actual BitBufferSlice insertion logic
    // buffer.writeBits(0, rawValue, m_bitSize, m_isBigEndian);
  }

  size_t getBitSize() const override { return m_bitSize; }

  size_t getEncodedBitSize(
      const std::shared_ptr<quasar::named::NamedObject> &value) const override {
    return m_bitSize;
  }

private:
  size_t m_bitSize;
  bool m_isBigEndian;
};

} // namespace datacodec

#endif // DATACODEC_INTEGERCODEC_HPP
