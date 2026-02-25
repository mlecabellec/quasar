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
 * This codec is responsible for handling string data, which can appear in various
 * EtherCAT contexts. This includes device names obtained from Slave Information
 * Interface (SII) data during network discovery (FE-0040.3.3), potentially as part
 * of mailbox protocol data (e.g., FoE file names), or other text-based
 * configurations. It supports both fixed-length strings (padded) and variable-length
 * (null-terminated or prefixed). For now, focusing on fixed-length for
 * simplicity in initial phase.
 *
 * Contribution to FE-0020: Implements the ICodec interface by creating and returning
 * `quasar::named::NamedString` objects, directly fulfilling FE-0020.4. This
 * extends the named object framework to handle textual data, which is essential
 * for device identification and configuration within EtherCAT systems.
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

    // Return a NamedString unnamed (name to be assigned by parent
    // schema/container), fulfilling FE-0020.4.
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
