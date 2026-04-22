#ifndef DATACODEC_STRINGCODEC_HPP
#define DATACODEC_STRINGCODEC_HPP

#include "datacodec/ICodec.hpp"
#include "quasar/named/NamedString.hpp"
#include <string>
#include <stdexcept>

namespace datacodec {

/**
 * @class StringCodec
 * @brief Codec for encoding/decoding strings.
 */
class StringCodec : public ICodec {
public:
  /**
   * @brief Default constructor for dynamic length strings.
   */
  StringCodec() : m_maxBitSize(1024 * 8), m_isFixedLength(false) {}

  /**
   * @brief Constructs a StringCodec with limits.
   */
  StringCodec(size_t maxByteLength, bool isFixedLength = true)
      : m_maxBitSize(maxByteLength * 8), m_isFixedLength(isFixedLength) {}

  std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const override {
    (void)buffer;
    return quasar::named::NamedString::create("", "");
  }

  std::string getCodecType() const override {
      return "String";
  }

  void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
              quasar::coretypes::BitBufferSlice &buffer) const override {
    std::shared_ptr<quasar::named::NamedString> namedStr =
        std::dynamic_pointer_cast<quasar::named::NamedString>(value);
    if (!namedStr) {
      throw std::invalid_argument("Invalid type for StringCodec encoding");
    }
    (void)buffer;
  }

  size_t getBitSize() const override {
    return m_isFixedLength ? m_maxBitSize : 0;
  }

  size_t getEncodedBitSize(
      const std::shared_ptr<quasar::named::NamedObject> &value) const override {
    if (m_isFixedLength)
      return m_maxBitSize;

    std::shared_ptr<quasar::named::NamedString> namedStr =
        std::dynamic_pointer_cast<quasar::named::NamedString>(value);
    return namedStr ? (namedStr->toString().length() + 1) * 8 : 0;
  }

private:
  size_t m_maxBitSize;
  bool m_isFixedLength;
};

} // namespace datacodec

#endif // DATACODEC_STRINGCODEC_HPP
