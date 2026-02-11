/**
 * @file Buffer.hpp
 * @brief Definition of the thread-safe Buffer class for byte manipulation.
 */

#ifndef QUASAR_CORETYPES_BUFFER_HPP
#define QUASAR_CORETYPES_BUFFER_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace quasar {
namespace coretypes {

class BufferSlice;

/**
 * @brief Enum representing endianness for multi-byte integer operations.
 */
enum class Endianness { 
  /** @brief Most Significant Byte first. */
  BigEndian, 
  /** @brief Least Significant Byte first. */
  LittleEndian 
};

/**
 * @brief The Buffer class is a thread-safe wrapper around a raw byte array.
 *
 * It provides a rich set of methods for data manipulation, designed for
 * binary protocol processing, file I/O, and data serialization.
 * 
 * Key features include:
 * - **Thread-Safety**: All access is protected by an internal recursive mutex.
 * - **Endianness Support**: Methods for reading/writing multi-byte integers 
 *   with explicit endianness control.
 * - **Slicing**: Support for creating lightweight, shared views of buffer portions.
 * - **Conversion**: Utilities for hexadecimal string and vector conversions.
 * - **Bitwise Ops**: Element-wise AND, OR, XOR, and NOT on buffer contents.
 * 
 * The class uses std::enable_shared_from_this to facilitate safe sharing of 
 * itself with BufferSlice objects.
 */
class Buffer : public std::enable_shared_from_this<Buffer> {
public:
  /**
   * @brief Constructs an empty buffer with zero size.
   */
  Buffer();

  /**
   * @brief Constructs a buffer with the specified size, initialized to zero.
   * @param size The desired size of the buffer in bytes.
   */
  explicit Buffer(size_t size);

  /**
   * @brief Constructs a buffer from an existing vector of bytes.
   * @param data The byte data to initialize the buffer with.
   */
  explicit Buffer(const std::vector<uint8_t> &data);

  /**
   * @brief Copy constructor.
   *
   * Performs a thread-safe deep copy of the source buffer.
   * @param other The buffer to copy from.
   */
  Buffer(const Buffer &other);

  /**
   * @brief Assignment operator.
   *
   * Performs a thread-safe deep copy of the source buffer, using deadlock 
   * avoidance when both buffers are locked.
   * @param other The buffer to copy from.
   * @return Reference to this Buffer instance.
   */
  Buffer &operator=(const Buffer &other);

  /**
   * @brief Virtual destructor.
   */
  virtual ~Buffer() = default;

  /**
   * @brief Returns the current size of the buffer.
   * @return The number of bytes in the buffer.
   */
  size_t size() const;

  /**
   * @brief Retrieves a single byte at the specified index.
   * @param index The zero-based index of the byte to retrieve.
   * @return The byte value at the specified index.
   * @throws std::out_of_range If @p index is greater than or equal to the buffer size.
   */
  uint8_t get(size_t index) const;

  /**
   * @brief Sets a single byte at the specified index.
   * @param index The zero-based index of the byte to set.
   * @param value The byte value to store at the specified index.
   * @throws std::out_of_range If @p index is greater than or equal to the buffer size.
   */
  void set(size_t index, uint8_t value);

  /**
   * @brief Converts the buffer content to a hexadecimal string representation.
   * @return A string containing the hex representation (e.g., "0a1b2c").
   */
  std::string toString() const;

  /**
   * @brief Returns a copy of the internal data as a byte vector.
   * @return A std::vector<uint8_t> containing a copy of the buffer data.
   */
  std::vector<uint8_t> toVector() const;

  /**
   * @brief Creates a new Buffer from a hexadecimal string.
   * @param hex The hex string to parse.
   * @return A Buffer instance containing the parsed data.
   * @throws std::invalid_argument If the hex string is invalid or has an odd length.
   */
  static Buffer fromString(const std::string &hex);

  /**
   * @brief Writes a 32-bit integer to the buffer at the specified index.
   *
   * @param value The integer value to write.
   * @param index The starting index in the buffer.
   * @param endian The endianness to use for encoding (defaults to BigEndian).
   * @throws std::out_of_range If the write would exceed the buffer's boundaries.
   */
  void writeInt(int value, size_t index,
                Endianness endian = Endianness::BigEndian);

  /**
   * @brief Reads a 32-bit integer from the buffer at the specified index.
   *
   * @param index The starting index in the buffer.
   * @param endian The endianness to use for decoding (defaults to BigEndian).
   * @return The 32-bit integer read from the buffer.
   * @throws std::out_of_range If the read would exceed the buffer's boundaries.
   */
  int readInt(size_t index, Endianness endian = Endianness::BigEndian) const;

  /**
   * @brief Creates a new Buffer containing a deep copy of a slice of this buffer.
   *
   * @param start The starting byte index.
   * @param length The number of bytes to include in the slice.
   * @return A new Buffer instance containing the sliced data.
   * @throws std::out_of_range If start + length exceeds the buffer size.
   */
  Buffer slice(size_t start, size_t length) const;

  /**
   * @brief Concatenates this buffer with another into a new Buffer instance.
   *
   * @param other The buffer to append to this one.
   * @return A new Buffer containing the concatenated data.
   */
  Buffer concat(const Buffer &other) const;

  /**
   * @brief Checks if this buffer is identical to another buffer.
   *
   * Two buffers are equal if they have the same size and identical content.
   * @param other The buffer to compare with.
   * @return true if both buffers are equal.
   */
  bool equals(const Buffer &other) const;

  /**
   * @brief Reverses the order of all bytes in the buffer in-place.
   */
  void reverse();

  /**
   * @brief Reverses the buffer in-place in chunks of a specified word size.
   *
   * Useful for swapping endianness of larger blocks of data (e.g., swapping 
   * 32-bit words).
   *
   * @param wordSize The size of chunks to swap (in bytes).
   * @throws std::invalid_argument If the buffer size is not a multiple of @p wordSize, 
   *                               or if @p wordSize is 0.
   */
  void reverse(size_t wordSize);

  /**
   * @brief Creates a deep copy of the buffer.
   * @return A new Buffer instance with the same data.
   */
  Buffer clone() const;

  /**
   * @brief Creates a lightweight view slice of the buffer.
   *
   * A slice view provides shared access to a portion of the buffer without copying.
   * Modifications through the view are reflected in the original buffer.
   *
   * @param start The starting index for the view.
   * @param length The number of bytes in the view.
   * @return A shared pointer to a BufferSlice representing the view.
   * @throws std::out_of_range If the requested view is outside the buffer bounds.
   */
  std::shared_ptr<BufferSlice> sliceView(size_t start, size_t length);

  /**
   * @brief Performs an element-wise bitwise AND with another buffer.
   *
   * @param other The other buffer of the same size.
   * @return A new Buffer containing the result of (this & other).
   * @throws std::invalid_argument If the buffers have different sizes.
   */
  Buffer bitwiseAnd(const Buffer &other) const;

  /**
   * @brief Performs an element-wise bitwise OR with another buffer.
   *
   * @param other The other buffer of the same size.
   * @return A new Buffer containing the result of (this | other).
   * @throws std::invalid_argument If the buffers have different sizes.
   */
  Buffer bitwiseOr(const Buffer &other) const;

  /**
   * @brief Performs an element-wise bitwise XOR with another buffer.
   *
   * @param other The other buffer of the same size.
   * @return A new Buffer containing the result of (this ^ other).
   * @throws std::invalid_argument If the buffers have different sizes.
   */
  Buffer bitwiseXor(const Buffer &other) const;

  /**
   * @brief Performs a bitwise NOT operation on the buffer content.
   *
   * @return A new Buffer containing the bitwise complement of this buffer's data.
   */
  Buffer bitwiseNot() const;

  /**
   * @brief Compares this buffer lexicographically with another.
   *
   * @param other The buffer to compare with.
   * @return A negative value if this < other, a positive value if this > other, 
   *         or zero if they are equal.
   */
  int compareTo(const Buffer &other) const;

  /**
   * @brief Checks equality with a raw byte vector.
   *
   * @param other The byte vector to compare with.
   * @return true if the buffer's content matches the vector.
   */
  bool equals(const std::vector<uint8_t> &other) const;

protected:
  /**
   * @brief Recursive mutex ensuring thread-safe access to the buffer data.
   */
  mutable std::recursive_timed_mutex mutex_;

  /**
   * @brief The raw byte storage.
   */
  std::vector<uint8_t> data_;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_BUFFER_HPP
