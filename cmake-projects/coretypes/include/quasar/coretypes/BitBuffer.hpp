#ifndef QUASAR_CORETYPES_BITBUFFER_HPP
#define QUASAR_CORETYPES_BITBUFFER_HPP

#include "quasar/coretypes/Buffer.hpp"

namespace quasar {
namespace coretypes {

class BitBufferSlice;

/**
 * @brief BitBuffer class for bit-level manipulation.
 * Derives from Buffer.
 */
class BitBuffer : public Buffer {
public:
  using Buffer::Buffer;

  /**
   * @brief Constructs with specific bit capacity.
   */
  explicit BitBuffer(size_t bitCount);

  /**
   * @brief Copy constructor needed to copy bitSize_.
   */
  BitBuffer(const BitBuffer &other);

  /**
   * @brief Copy assignment operator.
   * @param other The BitBuffer to copy from.
   * @return Reference to this BitBuffer.
   */
  BitBuffer &operator=(const BitBuffer &other);

  /**
   * @brief Get bit at specific bit index.
   * @param bitIndex The bit index (0 to bitSize - 1).
   * @return true if set, false otherwise.
   */
  bool getBit(size_t bitIndex) const;

  /**
   * @brief Set bit at specific bit index.
   * @param bitIndex The bit index.
   * @param value The value to set.
   */
  void setBit(size_t bitIndex, bool value);

  /**
   * @brief Slice buffers at bit level.
   */
  BitBuffer sliceBits(size_t startBit, size_t bitLength) const;

  /**
   * @brief Create a view slice at bit level.
   */
  std::shared_ptr<BitBufferSlice> sliceBitsView(size_t startBit,
                                                size_t bitLength);

  /**
   * @brief Concatenate BitBuffers.
   */
  BitBuffer concatBits(const BitBuffer &other) const;

  /**
   * @brief Comparison.
   */
  bool equals(const BitBuffer &other) const;

  /**
   * @brief Reverse at bit level (entire buffer).
   */
  void reverseBits();

  /**
   * @brief Reverse groups of bits.
   * @param groupSize Number of bits in each group to swap.
   */
  void reverseBits(size_t groupSize);

  /**
   * @brief Clones the BitBuffer.
   * @return A deep copy of the BitBuffer.
   */
  BitBuffer clone() const;

  /**
   * @brief Get size in bits.
   */
  size_t bitSize() const;

private:
  /**
   * @brief The number of valid bits in the buffer.
   * If 0, it means all bits in the underlying Buffer are valid (size() * 8).
   */
  size_t bitSize_ = 0; // Tracking valid bits
                       // Note: Buffer::data_ holds the bytes.
  // If bitSize_ is 0, we assume it matches size()*8 (wrapping existing buffer).
  // Or we should enforce constructor?
  // Requirement says "derivated from Buffer". Buffer constructors init data_.
  // If constructed from Buffer, bitSize = size * 8.
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_BITBUFFER_HPP
