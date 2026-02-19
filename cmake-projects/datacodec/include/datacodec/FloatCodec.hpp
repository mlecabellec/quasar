/**
 * @file FloatCodec.hpp
 * @brief Codec implementation for floating-point types.
 */

#ifndef DATACODEC_FLOATCODEC_HPP
#define DATACODEC_FLOATCODEC_HPP

#include "datacodec/ICodec.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include <cstring>
#include <type_traits>

namespace datacodec {

/**
 * @class FloatCodec
 * @brief Codec for encoding/decoding floating-point values (float, double).
 */
template <typename T> class FloatCodec : public ICodec {
public:
  static_assert(std::is_floating_point<T>::value,
                "FloatCodec only supports floating-point types");

  /**
   * @brief Constructs a FloatCodec.
   * @param isBigEndian True for Big Endian, False for Little Endian.
   */
  FloatCodec(bool isBigEndian = true)
      : m_bitSize(sizeof(T) * 8), m_isBigEndian(isBigEndian) {}

  std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const override {

    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for FloatCodec");
    }

    // Placeholder for bit reading logic.
    // We would read sizeof(T) bytes/bits and reinterpret cast to T.
    // For now, assuming T initialized to 0.0
    T value = 0.0;

    // Return a NamedFloatingPoint unnamed
    return quasar::named::NamedFloatingPoint<T>::create("", value);
  }

  void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
              quasar::coretypes::BitBufferSlice &buffer) const override {

    auto namedFloat =
        std::dynamic_pointer_cast<quasar::named::NamedFloatingPoint<T>>(value);
    if (!namedFloat) {
      throw std::invalid_argument("Invalid type for FloatCodec encoding");
    }

    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for FloatCodec encoding");
    }

    // T rawValue = namedFloat->value();
    // Placeholder for bit writing logic.
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

#endif // DATACODEC_FLOATCODEC_HPP
