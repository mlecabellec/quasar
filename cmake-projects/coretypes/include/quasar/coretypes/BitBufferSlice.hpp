#ifndef QUASAR_CORETYPES_BITBUFFERSLICE_HPP
#define QUASAR_CORETYPES_BITBUFFERSLICE_HPP

#include "quasar/coretypes/BitBuffer.hpp"
#include <memory>
#include <string>
#include <vector>

namespace quasar::coretypes {

/**
 * @brief View into a BitBuffer at bit-level granularity.
 */
class BitBufferSlice {
public:
  /**
   * @brief Constructs a BitBufferSlice.
   * @param buffer The parent BitBuffer.
   * @param startBit The start bit index.
   * @param bitLength The length in bits.
   */
  BitBufferSlice(std::shared_ptr<BitBuffer> buffer, size_t startBit,
                 size_t bitLength);
  virtual ~BitBufferSlice() = default;

  /**
   * @brief Returns the size in bits.
   * @return Size in bits.
   */
  size_t size() const; // in bits

  /**
   * @brief Gets the bit at the specified index.
   * @param index The bit index.
   * @return The bit value.
   */
  bool getBit(size_t index) const;

  /**
   * @brief Sets the bit at the specified index.
   * @param index The bit index.
   * @param value The value to set.
   */
  void setBit(size_t index, bool value);

  // Creating a sub-slice
  // Creating a sub-slice
  /**
   * @brief Creates a sub-slice.
   * @param index The start bit relative to this slice.
   * @param subLength The length of the sub-slice.
   * @return A new BitBufferSlice.
   */
  BitBufferSlice slice(size_t index, size_t subLength) const;

  // Concat
  // Concat
  /**
   * @brief Concatenates with another slice.
   * @param other The other slice.
   * @return A new BitBuffer containing the concatenated bits.
   */
  std::shared_ptr<BitBuffer> concat(const BitBufferSlice &other) const;

  // Conversion?
  // Conversion?
  /**
   * @brief Converts to a vector of packed bytes.
   * @return Vector of bytes.
   */
  std::vector<uint8_t> toVector() const; // packed bytes?

  /**
   * @brief Returns the parent BitBuffer.
   * @return Shared pointer to parent.
   */
  std::shared_ptr<BitBuffer> getParent() const;

  /**
   * @brief Returns the absolute start bit in the parent buffer.
   * @return Start bit index.
   */
  size_t getOffset() const;

private:
  std::shared_ptr<BitBuffer> buffer_;
  size_t startBit_;
  size_t bitLength_;
};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_BITBUFFERSLICE_HPP
