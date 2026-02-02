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
 * @brief Enum representing endianness for integer operations.
 */
enum class Endianness { BigEndian, LittleEndian };

/**
 * @brief The Buffer class wrapper around a raw byte array.
 *
 * Provides methods for encoding/decoding, slicing, concatenation, and
 * comparison. Thread-safe implementation using internal mutex.
 */
class Buffer : public std::enable_shared_from_this<Buffer> {
public:
  /**
   * @brief Constructs an empty buffer.
   */
  Buffer();

  /**
   * @brief Constructs a buffer with specified size, initialized to zero.
   * @param size size in bytes.
   */
  explicit Buffer(size_t size);

  /**
   * @brief Constructs a buffer from a vector of bytes.
   * @param data The data.
   */
  explicit Buffer(const std::vector<uint8_t> &data);

  /**
   * @brief Copy constructor.
   * Thread-safe copy.
   */
  Buffer(const Buffer &other);

  /**
   * @brief Assignment operator.
   * Thread-safe assignment.
   */
  Buffer &operator=(const Buffer &other);

  virtual ~Buffer() = default;

  /**
   * @brief Get the size of the buffer.
   */
  size_t size() const;

  /**
   * @brief Get a byte at processing index.
   * Throws out_of_range.
   */
  uint8_t get(size_t index) const;

  /**
   * @brief Set a byte at index.
   */
  void set(size_t index, uint8_t value);

  /**
   * @brief Returns a string hex representation or raw?
   * Requirement: "encoding and decoding values to and from a string".
   * Assuming hex or base64? Ill implement Hex for now.
   */
  std::string toString() const;

  /**
   * @brief Returns a copy of the internal data as a vector.
   */
  std::vector<uint8_t> toVector() const;

  /**
   * @brief Create buffer from hex string.
   */
  static Buffer fromString(const std::string &hex);

  // Numeric conversions
  void writeInt(int value, size_t index,
                Endianness endian = Endianness::BigEndian);
  int readInt(size_t index, Endianness endian = Endianness::BigEndian) const;

  // Slicing
  /**
   * @brief Slices the buffer.
   * @param start The start index.
   * @param length The length of the slice.
   * @return A new Buffer containing the slice.
   * @throws std::out_of_range if start + length > size().
   */
  Buffer slice(size_t start, size_t length) const;

  // Concatenation
  /**
   * @brief Concatenates this buffer with another.
   * @param other The buffer to concatenate.
   * @return A new Buffer containing the concatenated data.
   */
  Buffer concat(const Buffer &other) const;

  // Comparison
  /**
   * @brief Checks if two buffers are equal.
   * @param other The buffer to compare with.
   * @return true if equal, false otherwise.
   */
  bool equals(const Buffer &other) const;

  // Reversing
  /**
   * @brief Reverses the buffer content.
   */
  void reverse();

  /**
   * @brief Reverses the buffer in chunks of wordSize.
   * @param wordSize The size of chunks to swap.
   * @throws std::invalid_argument if size() is not a multiple of wordSize.
   */
  void reverse(size_t wordSize);

  // Cloning (explicit)
  /**
   * @brief Clones the buffer.
   * @return A deep copy of the buffer.
   */
  Buffer clone() const;

  // New features for FE-0030
  /**
   * @brief Creates a slice view of the buffer.
   * @param start The start index.
   * @param length The length of the slice.
   * @return A shared pointer to a BufferSlice.
   */
  std::shared_ptr<BufferSlice> sliceView(size_t start, size_t length);

  // Bitwise operations
  /**
   * @brief Performs bitwise AND.
   * @param other The other buffer.
   * @return Result buffer.
   */
  Buffer bitwiseAnd(const Buffer &other) const;

  /**
   * @brief Performs bitwise OR.
   * @param other The other buffer.
   * @return Result buffer.
   */
  Buffer bitwiseOr(const Buffer &other) const;

  /**
   * @brief Performs bitwise XOR.
   * @param other The other buffer.
   * @return Result buffer.
   */
  Buffer bitwiseXor(const Buffer &other) const;

  /**
   * @brief Performs bitwise NOT.
   * @return Result buffer.
   */
  Buffer bitwiseNot() const;

  // Comparison enhancements
  /**
   * @brief Compares two buffers lexicographically.
   * @param other The buffer to compare with.
   * @return -1 if this < other, 1 if this > other, 0 if equal.
   */
  int compareTo(const Buffer &other) const;

  /**
   * @brief Checks equality with a raw vector.
   * @param other The vector to compare with.
   * @return true if equal.
   */
  bool equals(const std::vector<uint8_t> &other) const;

protected:
  /**
   * @brief Mutex for thread safety.
   */
  mutable std::recursive_timed_mutex mutex_;

  /**
   * @brief The raw byte data.
   */
  std::vector<uint8_t> data_;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_BUFFER_HPP
