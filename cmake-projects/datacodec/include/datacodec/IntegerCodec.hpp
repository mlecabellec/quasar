#ifndef DATACODEC_INTEGERCODEC_HPP
#define DATACODEC_INTEGERCODEC_HPP

#include "datacodec/ICodec.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/coretypes/BitBufferSlice.hpp"
#include <memory>
#include <type_traits>
#include <stdexcept>

namespace datacodec {

/**
 * @class IntegerCodec
 * @brief Codec for encoding/decoding integer values.
 */
template <typename T> class IntegerCodec : public ICodec {
public:
  static_assert(std::is_integral<T>::value,
                "IntegerCodec only supports integral types");

  /**
   * @brief Constructs an IntegerCodec.
   * @param bitSize Number of bits for the integer.
   * @param isBigEndian True for Big Endian.
   */
  IntegerCodec(size_t bitSize, bool isBigEndian = true)
      : m_bitSize(bitSize), m_isBigEndian(isBigEndian) {}

  std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const override {
    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for IntegerCodec");
    }
    T value = 0;
    // Implementation placeholder
    return quasar::named::NamedInteger<T>::create("", value);
  }

  std::string getCodecType() const override {
      return "Integer";
  }

  void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
              quasar::coretypes::BitBufferSlice &buffer) const override {
    std::shared_ptr<quasar::named::NamedInteger<T>> namedInt =
        std::dynamic_pointer_cast<quasar::named::NamedInteger<T>>(value);
    if (!namedInt) {
      throw std::invalid_argument("Invalid type for IntegerCodec encoding");
    }
    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for IntegerCodec encoding");
    }
    // Implementation placeholder
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

#endif // DATACODEC_INTEGERCODEC_HPP
