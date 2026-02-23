/**
 * @file BufferSlice.hpp
 * @brief Definition of the BufferSlice class for non-owning buffer views.
 */

#ifndef QUASAR_CORETYPES_BUFFERSLICE_HPP
#define QUASAR_CORETYPES_BUFFERSLICE_HPP

#include "quasar/coretypes/Buffer.hpp"
#include "quasar/coretypes/Number.hpp"
#include "quasar/coretypes/String.hpp"
#include <memory>
#include <string>
#include <vector>

namespace quasar::coretypes {

/**
 * @brief A lightweight, non-owning view into a Buffer object.
 *
 * BufferSlice provides a window (offset and length) into an existing Buffer. 
 * It holds a shared_ptr to the underlying Buffer to ensure the data remains
 * valid for the lifetime of the slice.
 * 
 * **Compliance**:
 * - Fulfills [FE-0030.5.7] Create a BufferSlice class which is a view of the original Buffer.
 * - Fulfills [FE-0030.9] All methods are const correct.
 * 
 * Slices are useful for:
 * - Parsing sub-fields of a larger binary message.
 * - Passing parts of a buffer to functions without copying.
 * - Abstracting away the absolute positioning within a large data stream.
 * 
 * Note that modifications made through the slice are directly reflected in 
 * the parent buffer.
 */
class BufferSlice {
public:
  /**
   * @brief Constructs a new BufferSlice.
   * 
   * Fulfills [FE-0030.7.1] A slice shall be defined by a starting offset and a length.
   * 
   * @param buffer Shared pointer to the parent Buffer.
   * @param offset The starting byte offset within the parent Buffer.
   * @param length The number of bytes in the slice.
   * @throws std::out_of_range If the offset and length exceed parent buffer bounds.
   */
  BufferSlice(std::shared_ptr<Buffer> buffer, size_t offset, size_t length);

  /**
   * @brief Virtual destructor.
   */
  virtual ~BufferSlice() = default;

  /**
   * @brief Returns the size of the slice in bytes.
   * @return Size in bytes.
   */
  size_t size() const;

  /**
   * @brief Gets a byte at the specified index relative to the start of the slice.
   * 
   * @param index The zero-based index within the slice.
   * @return The byte value at the specified index.
   * @throws std::out_of_range If @p index is greater than or equal to the slice size.
   */
  uint8_t get(size_t index) const;

  /**
   * @brief Sets a byte at the specified index relative to the start of the slice.
   * 
   * Directly modifies the underlying parent Buffer.
   * 
   * @param index The zero-based index within the slice.
   * @param value The byte value to set.
   * @throws std::out_of_range If @p index is greater than or equal to the slice size.
   */
  void set(size_t index, uint8_t value);

  /**
   * @brief Converts the slice data into a new std::vector.
   * 
   * Fulfills [FE-0030.5.10] Methods for conversion to std::vector<uint8_t>.
   * 
   * @return A std::vector<uint8_t> containing a copy of the slice data.
   */
  std::vector<uint8_t> toVector() const;

  /**
   * @brief Converts the slice data to a hexadecimal string representation.
   * 
   * Fulfills [FE-0030.5.12] Conversion to quasar::coretypes::String.
   * 
   * @return A string containing the hex representation of the slice data.
   */
  std::string toString() const;

  /**
   * @brief Creates a new sub-slice from the current slice.
   * 
   * The new slice will point to the same underlying parent Buffer with an 
   * adjusted offset.
   * 
   * Fulfills [FE-0030.7.6] Slices can be created from a slice.
   * 
   * @param index The start index relative to the beginning of this slice.
   * @param length The length of the new sub-slice.
   * @return A new BufferSlice representing the requested sub-range.
   * @throws std::out_of_range If the sub-slice range exceeds current slice bounds.
   */
  BufferSlice slice(size_t index, size_t length) const;

  /**
   * @brief Concatenates this slice with another into a new Buffer.
   * 
   * Performs a copy of data from both slices into a newly allocated Buffer instance.
   * 
   * Fulfills [FE-0030.7.3] A slice shall be able to be concatenated with other slices.
   * 
   * @param other The other slice to append to this one.
   * @return A shared pointer to a new Buffer containing data from both slices.
   */
  std::shared_ptr<Buffer> concat(const BufferSlice &other) const;

  /**
   * @brief Returns a shared pointer to the underlying parent Buffer.
   * 
   * @return Shared pointer to the Buffer this slice is viewing.
   */
  std::shared_ptr<Buffer> getParent() const;

  /**
   * @brief Returns the absolute offset of this slice within its parent Buffer.
   * 
   * @return The starting position in bytes relative to the parent Buffer's beginning.
   */
  size_t getOffset() const;

  /**
   * @brief Checks if this slice's content is identical to another slice's content.
   * 
   * Fulfills [FE-0030.5.1] Comparison with other Buffer objects.
   * 
   * @param other The other slice to compare with.
   * @return true if both slices have the same size and identical data.
   */
  bool equals(const BufferSlice &other) const;

  /**
   * @brief Writes a numeric value into the buffer at the slice's position.
   * 
   * Currently supports writing the integer value of a Number object if the 
   * slice provides sufficient space (typically 4 bytes for 32-bit int).
   * 
   * Fulfills [FE-0030.5.11] Methods for conversion from quasar::coretypes::Number.
   * 
   * @param n The Number object containing the value to write.
   * @throws std::out_of_range If the slice is too small to contain the value.
   */
  void fromNumber(const Number &n);

private:
  /**
   * @brief Reference to the parent Buffer.
   */
  std::shared_ptr<Buffer> buffer_;

  /**
   * @brief Byte offset within the parent Buffer.
   */
  size_t offset_;

  /**
   * @brief Length of the slice in bytes.
   */
  size_t length_;
};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_BUFFERSLICE_HPP
