#ifndef DATACODEC_FLOATCODEC_HPP
#define DATACODEC_FLOATCODEC_HPP

#include "datacodec/ICodec.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <string>

namespace datacodec {

/**
 * @class FloatCodec
 * @brief Codec for encoding/decoding floating-point values.
 */
template <typename T> class FloatCodec : public ICodec {
public:
  static_assert(std::is_floating_point<T>::value,
                "FloatCodec only supports floating-point types");

  /**
   * @brief Constructs a FloatCodec.
   * @param isBigEndian True for Big Endian.
   */
  FloatCodec(bool isBigEndian = true)
      : m_bitSize(sizeof(T) * 8), m_isBigEndian(isBigEndian) {}

  std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const override {
    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for FloatCodec");
    }
    T value = 0.0;
    return quasar::named::NamedFloatingPoint<T>::create("", value);
  }

  std::string getCodecType() const override {
      return "FloatingPoint";
  }

  void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
              quasar::coretypes::BitBufferSlice &buffer) const override {
    std::shared_ptr<quasar::named::NamedFloatingPoint<T>> namedFloat =
        std::dynamic_pointer_cast<quasar::named::NamedFloatingPoint<T>>(value);
    if (!namedFloat) {
      throw std::invalid_argument("Invalid type for FloatCodec encoding");
    }
    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for FloatCodec encoding");
    }
  }

  size_t getBitSize() const override { return m_bitSize; }

  size_t getEncodedBitSize(
      const std::shared_ptr<quasar::named::NamedObject> &value) const override {
    (void)value;
    return m_bitSize;
  }

private:
  size_t m_bitSize;
  bool m_isBigEndian;
};

} // namespace datacodec

#endif // DATACODEC_FLOATCODEC_HPP
