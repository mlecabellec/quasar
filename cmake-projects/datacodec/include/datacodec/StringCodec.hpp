/**
 * @file StringCodec.hpp
 * @brief Codec implementation for string types.
 */

#ifndef DATACODEC_STRINGCODEC_HPP
#define DATACODEC_STRINGCODEC_HPP

#include "datacodec/ICodec.hpp"
#include "quasar/named/NamedString.hpp"
#include <string>

namespace datacodec {

/**
 * @class StringCodec
 * @brief Codec for encoding/decoding strings.
 *
 * Supports both fixed-length strings (padded) and variable-length
 * (null-terminated or prefixed). For now, focusing on fixed-length for
 * simplicity in initial phase.
 */
class StringCodec : public ICodec {
public:
  /**
   * @brief Constructs a StringCodec.
   * @param maxByteLength Maximum length in bytes (including terminator if
   * applicable).
   * @param isFixedLength If true, always reads/writes maxByteLength.
   */
  StringCodec(size_t maxByteLength, bool isFixedLength = true)
      : m_maxBitSize(maxByteLength * 8), m_isFixedLength(isFixedLength) {}

  std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const override {

    // Placeholder: read bytes from buffer until null or max length
    std::string value = "";

    return quasar::named::NamedString::create("", value);
  }

  void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
              quasar::coretypes::BitBufferSlice &buffer) const override {

    std::shared_ptr<quasar::named::NamedString> namedStr =
        std::dynamic_pointer_cast<quasar::named::NamedString>(value);
    if (!namedStr) {
      throw std::invalid_argument("Invalid type for StringCodec encoding");
    }

    // Placeholder: write bytes to buffer
  }

  size_t getBitSize() const override {
    return m_isFixedLength ? m_maxBitSize : 0; // 0 indicates dynamic
  }

  size_t getEncodedBitSize(
      const std::shared_ptr<quasar::named::NamedObject> &value) const override {
    if (m_isFixedLength)
      return m_maxBitSize;

    std::shared_ptr<quasar::named::NamedString> namedStr =
        std::dynamic_pointer_cast<quasar::named::NamedString>(value);
    // Length + 1 for null terminator, * 8 for bits
    return namedStr ? (namedStr->getValue().length() + 1) * 8 : 0;
  }

private:
  size_t m_maxBitSize;
  bool m_isFixedLength;
};

} // namespace datacodec

#endif // DATACODEC_STRINGCODEC_HPP
