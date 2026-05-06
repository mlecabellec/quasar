/**
 * @file BitBuffer.hpp
 * @brief Definition of the BitBuffer class for bit-level data manipulation.
 */

#ifndef QUASAR_CORETYPES_BITBUFFER_HPP
#define QUASAR_CORETYPES_BITBUFFER_HPP

#include "quasar/coretypes/Buffer.hpp"

namespace quasar {
namespace coretypes {

class BitBufferSlice;

/**
 * @brief BitBuffer class for granular bit-level manipulation.
 *
 * This class extends the byte-oriented Buffer to provide bit-level granularity. 
 * It is essential for handling bit-packed data formats common in network 
 * protocols, compressed files, and low-level hardware interfaces. Crucial for
 * EtherCAT frame management [FE-0040.2] and PDO configuration [FE-0040.5.3].
 * 
 * **Compliance**:
 * - Fulfills [FE-0010.4] Provide a BitBuffer class.
 * - Fulfills [FE-0010.4.1] Derived from the Buffer class.
 * - Fulfills [FE-0030.8] All methods are thread safe.
 * - Fulfills [FE-0030.9] All methods are const correct.
 * 
 * **Bit Numbering Convention**: 
 * This class follows the Big Endian (Network Order) bit numbering convention:
 * - Bit 0 is the Most Significant Bit (MSB) of the first byte (byte index 0).
 * - Bit 7 is the Least Significant Bit (LSB) of the first byte.
 * - Bit 8 is the MSB of the second byte (byte index 1), and so on.
 * Network order endianness is relevant for EtherCAT [FE-0040.1.2].
 * 
 * **Thread Safety**: 
 * Inherits thread-safety from the Buffer class, protecting the underlying 
 * storage with a recursive mutex.
 */
class BitBuffer : public Buffer {
public:
  using Buffer::Buffer;

  /**
   * @brief Constructs a BitBuffer with a specific bit capacity.
   *
   * The underlying byte buffer is sized to accommodate at least @p bitCount bits.
   * All bits are initialized to 0.
   *
   * @param bitCount The number of bits the buffer should hold.
   */
  explicit BitBuffer(size_t bitCount);

  /**
   * @brief Copy constructor.
   *
   * Creates a new BitBuffer as a deep copy of another. This operation is 
   * thread-safe and ensures that @p bitSize_ is correctly replicated.
   *
   * @param other The BitBuffer to copy from.
   */
  BitBuffer(const BitBuffer &other);

  /**
   * @brief Copy assignment operator.
   *
   * Replaces the contents of this BitBuffer with a copy of @p other.
   * Uses deadlock-avoidance strategies when locking both instances.
   *
   * @param other The BitBuffer to copy from.
   * @return Reference to this BitBuffer instance.
   */
  BitBuffer &operator=(const BitBuffer &other);

  /**
   * @brief Retrieves the value of the bit at the specified index.
   *
   * Uses Big Endian bit numbering (bit 0 is MSB of first byte).
   * 
   * Fulfills [FE-0010.4.2] Operations at bit level.
   *
   * @param bitIndex The zero-based bit index (0 to bitSize() - 1).
   * @return true if the bit is set (1), false otherwise (0).
   * @throws std::out_of_range If the bit index is out of bounds.
   */
  [[nodiscard]] bool getBit(size_t bitIndex) const;

  /**
   * @brief Sets the value of the bit at the specified index.
   *
   * Modifies the underlying byte data to reflect the new bit value.
   * 
   * Fulfills [FE-0010.4.2] Operations at bit level.
   *
   * @param bitIndex The zero-based bit index.
   * @param value The value to set (true for 1, false for 0).
   * @throws std::out_of_range If the bit index is out of bounds.
   */
  void setBit(size_t bitIndex, bool value);

  /**
   * @brief Creates a new BitBuffer containing a range of bits from this buffer.
   *
   * The slice is a deep copy of the requested range. Essential for extracting
   * bit-fields from EtherCAT frames [FE-0040.2.3] and configuring PDOs [FE-0040.5.3].
   * 
   * Fulfills [FE-0010.4.3] Slicing the buffer with bit granularity.
   * Fulfills [FE-0030.5.6] Slicing at bit level.
   *
   * @param startBit The starting bit index (inclusive).
   * @param bitLength The number of bits to include in the slice.
   * @return A new BitBuffer instance containing the copied bit data.
   * @throws std::out_of_range If the requested slice range exceeds the buffer's bit size.
   */
  [[nodiscard]] BitBuffer sliceBits(size_t startBit, size_t bitLength) const;

  /**
   * @brief Creates a view slice at the bit level.
   *
   * Unlike sliceBits, this creates a lightweight view (BitBufferSlice) that 
   * points to the original data. Modifications through the view affect this buffer.
   * Analogous to `std::span` [FE-0040.8.2], this provides efficient zero-copy access
   * to bit sub-ranges, useful for parsing EtherCAT data.
   * 
   * Fulfills [FE-0030.5.7] Creation of BitBufferSlice views.
   *
   * @param startBit The starting bit index for the view.
   * @param bitLength The number of bits in the view.
   * @return A shared pointer to a BitBufferSlice representing the view.
   * @throws std::out_of_range If the requested view range exceeds the buffer's bit size.
   */
  std::shared_ptr<BitBufferSlice> sliceBitsView(size_t startBit,
                                                size_t bitLength);

  /**
   * @brief Concatenates this BitBuffer with another.
   *
   * Creates a new BitBuffer that contains all bits of this buffer followed 
   * by all bits of @p other. Useful for assembling multi-part EtherCAT frames [FE-0040.2].
   * 
   * Fulfills [FE-0010.4.4] Concatenation of buffers.
   * Fulfills [FE-0030.5.5] Concatenation.
   *
   * @param other The BitBuffer to append.
   * @return A new BitBuffer containing the combined bits.
   */
  [[nodiscard]] BitBuffer concatBits(const BitBuffer &other) const;

  /**
   * @brief Checks if two BitBuffers are equal at the bit level.
   *
   * Comparison is done bit-by-bit up to the bitSize() of both buffers.
   * 
   * Fulfills [FE-0010.4.5] Comparison.
   * Fulfills [FE-0030.5.1] Comparison with other BitBuffer objects.
   *
   * @param other The BitBuffer to compare with.
   * @return true if both have the same bit size and identical bit values.
   */
  [[nodiscard]] bool equals(const BitBuffer &other) const;

  /**
   * @brief Reverses the order of all bits in the buffer.
   *
   * The first bit becomes the last, the second becomes the second-to-last, etc.
   * Useful for byte or bit-level reversal operations in low-level protocols like EtherCAT [FE-0040.2].
   * 
   * Fulfills [FE-0010.4.7] Reversing at bit level.
   */
  void reverseBits();

  /**
   * @brief Reverses the order of groups of bits.
   *
   * Useful for swapping bit-endianness in sub-byte fields. For example, 
   * reversing with groupSize=4 in a byte buffer swaps the nibbles.
   * 
   * Fulfills [FE-0010.4.7] Reversing at bit level.
   *
   * @param groupSize The number of bits in each group to swap.
   * @throws std::invalid_argument If the buffer's bit size is not a multiple of @p groupSize.
   */
  void reverseBits(size_t groupSize);

  /**
   * @brief Creates a deep copy of the BitBuffer.
   *
   * Equivalent to using the copy constructor.
   * 
   * Fulfills [FE-0010.4.8] Cloning the buffer with memory allocation.
   *
   * @return A new BitBuffer instance with the same data and bit size.
   */
  [[nodiscard]] BitBuffer clone() const;

  /**
   * @brief Returns the number of valid bits in the buffer.
   *
   * If the buffer was created from a byte array without specifying bits, 
   * this returns size() * 8.
   *
   * @return The total bit count.
   */
  [[nodiscard]] size_t bitSize() const;

private:
  /**
   * @brief The number of valid bits in the buffer.
   *
   * If 0, it indicates that the bit size should be inferred from the underlying 
   * Buffer's byte size (size() * 8).
   */
  size_t bitSize_ = 0;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_BITBUFFER_HPP
