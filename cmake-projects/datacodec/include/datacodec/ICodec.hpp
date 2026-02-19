/**
 * @file ICodec.hpp
 * @brief Interface for binary data codecs.
 */

#ifndef DATACODEC_ICODEC_HPP
#define DATACODEC_ICODEC_HPP

#include "quasar/coretypes/BitBufferSlice.hpp"
#include "quasar/named/NamedObject.hpp"
#include <memory>

namespace datacodec {

/**
 * @class ICodec
 * @brief Abstract interface for encoding and decoding values to/from binary
 * buffers.
 *
 * This interface defines the contract for transforming between raw binary
 * representations (accessed via BitBufferSlice) and high-level object
 * representations (NamedObject instances).
 */
class ICodec {
public:
  virtual ~ICodec() = default;

  /**
   * @brief Decodes a value from the buffer.
   *
   * Reads bits from the provided buffer slice and constructs a corresponding
   * NamedObject. The type of the returned object depends on the specific codec
   * implementation (e.g., NamedInteger).
   *
   * @param buffer The buffer slice containing the binary data.
   * @return A shared pointer to the decoded NamedObject.
   */
  virtual std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const = 0;

  /**
   * @brief Encodes a value into the buffer.
   *
   * Writes the value from the provided NamedObject into the buffer slice.
   *
   * @param value The NamedObject containing the value to encode.
   * @param buffer The buffer slice where data should be written.
   * @throws std::invalid_argument if the value type is incompatible with the
   * codec.
   * @throws std::out_of_range if the buffer is too small.
   */
  virtual void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
                      quasar::coretypes::BitBufferSlice &buffer) const = 0;

  /**
   * @brief Gets the fixed size of the encoded data in bits.
   *
   * @return number of bits, or 0 if the size is dynamic.
   */
  virtual size_t getBitSize() const = 0;

  /**
   * @brief Gets the size of the encoded data for a specific value.
   *
   * Necessary for dynamic types like strings.
   *
   * @param value The value to measure.
   * @return number of bits required to encode this value.
   */
  virtual size_t getEncodedBitSize(
      const std::shared_ptr<quasar::named::NamedObject> &value) const = 0;
};

} // namespace datacodec

#endif // DATACODEC_ICODEC_HPP
