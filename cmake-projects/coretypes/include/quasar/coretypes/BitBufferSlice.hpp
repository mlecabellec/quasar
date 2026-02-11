/**
 * @file BitBufferSlice.hpp
 * @brief Definition of the BitBufferSlice class for bit-level buffer views.
 */

#ifndef QUASAR_CORETYPES_BITBUFFERSLICE_HPP
#define QUASAR_CORETYPES_BITBUFFERSLICE_HPP

#include "quasar/coretypes/BitBuffer.hpp"
#include <memory>
#include <string>
#include <vector>

namespace quasar::coretypes {

/**
 * @brief Lightweight view into a BitBuffer with bit-level granularity.
 *
 * BitBufferSlice provides a window into an existing BitBuffer. It allows for
 * addressable access to a range of bits within the parent buffer without 
 * copying the underlying data.
 * 
 * This class is particularly useful for:
 * - Extracting bit-fields from packed protocols (e.g., IP headers, custom 
 *   hardware registers).
 * - Iterative parsing of bitstreams where the data structure is discovered 
 *   sequentially.
 * 
 * Modifications made through the slice (via setBit) affect the parent BitBuffer.
 */
class BitBufferSlice {
public:
  /**
   * @brief Constructs a BitBufferSlice viewing a range of bits in a BitBuffer.
   *
   * @param buffer A shared pointer to the parent BitBuffer.
   * @param startBit The absolute starting bit index in the parent buffer.
   * @param bitLength The number of bits to include in the view.
   * @throws std::invalid_argument If @p buffer is null.
   * @throws std::out_of_range If the requested range exceeds the parent's boundaries.
   */
  BitBufferSlice(std::shared_ptr<BitBuffer> buffer, size_t startBit,
                 size_t bitLength);

  /**
   * @brief Virtual destructor.
   */
  virtual ~BitBufferSlice() = default;

  /**
   * @brief Returns the size of the slice in bits.
   * @return The total number of bits covered by this view.
   */
  size_t size() const;

  /**
   * @brief Retrieves the value of the bit at the specified index within the slice.
   *
   * @param index The zero-based bit index relative to the start of this slice.
   * @return true if the bit is set (1), false otherwise (0).
   * @throws std::out_of_range If @p index is out of the slice's bounds.
   */
  bool getBit(size_t index) const;

  /**
   * @brief Sets the value of the bit at the specified index within the slice.
   *
   * Directly modifies the underlying parent BitBuffer.
   *
   * @param index The zero-based bit index relative to the start of this slice.
   * @param value The value to set (true for 1, false for 0).
   * @throws std::out_of_range If @p index is out of the slice's bounds.
   */
  void setBit(size_t index, bool value);

  /**
   * @brief Creates a new sub-slice from the current bit slice.
   *
   * The new slice will point to the same underlying parent BitBuffer with an 
   * adjusted offset.
   *
   * @param index The starting bit index relative to this slice.
   * @param subLength The number of bits to include in the sub-slice.
   * @return A new BitBufferSlice representing the requested sub-range.
   * @throws std::out_of_range If the requested range exceeds current slice bounds.
   */
  BitBufferSlice slice(size_t index, size_t subLength) const;

  /**
   * @brief Concatenates this bit slice with another into a new BitBuffer.
   *
   * Performs a bit-by-bit copy of both slices into a newly allocated BitBuffer.
   *
   * @param other The other bit slice to append.
   * @return A shared pointer to a new BitBuffer containing bits from both slices.
   */
  std::shared_ptr<BitBuffer> concat(const BitBufferSlice &other) const;

  /**
   * @brief Packs the bits of this slice into a new byte vector.
   *
   * The bits are packed into bytes using Big Endian bit numbering (first bit in MSB).
   * If the number of bits is not a multiple of 8, the last byte is zero-padded at 
   * the least significant bits.
   *
   * @return A std::vector<uint8_t> containing the packed bit data.
   */
  std::vector<uint8_t> toVector() const;

  /**
   * @brief Returns a shared pointer to the underlying parent BitBuffer.
   * @return Shared pointer to the BitBuffer this slice is viewing.
   */
  std::shared_ptr<BitBuffer> getParent() const;

  /**
   * @brief Returns the absolute bit offset of this slice within its parent BitBuffer.
   * @return The starting bit position relative to the parent BitBuffer's beginning.
   */
  size_t getOffset() const;

private:
  /**
   * @brief Reference to the parent buffer.
   */
  std::shared_ptr<BitBuffer> buffer_;

  /**
   * @brief Starting bit index in the parent buffer.
   */
  size_t startBit_;

  /**
   * @brief Length of the slice in bits.
   */
  size_t bitLength_;
};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_BITBUFFERSLICE_HPP
