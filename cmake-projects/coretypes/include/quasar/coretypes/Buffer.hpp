#ifndef QUASAR_CORETYPES_BUFFER_HPP
#define QUASAR_CORETYPES_BUFFER_HPP

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace quasar {
namespace coretypes {

enum class Endianness { BigEndian, LittleEndian };

/**
 * @brief The Buffer class wrapper around a raw byte array.
 *
 * Provides methods for encoding/decoding, slicing, concatenation, and
 * comparison. Thread-safe implementation using internal mutex.
 */
class Buffer {
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
   * @brief Create buffer from hex string.
   */
  static Buffer fromString(const std::string &hex);

  // Numeric conversions
  void writeInt(int value, size_t index,
                Endianness endian = Endianness::BigEndian);
  int readInt(size_t index, Endianness endian = Endianness::BigEndian) const;

  // Slicing
  Buffer slice(size_t start, size_t length) const;

  // Concatenation
  Buffer concat(const Buffer &other) const;

  // Comparison
  bool equals(const Buffer &other) const;

  // Reversing
  void reverse();
  void reverse(size_t wordSize);

  // Cloning (explicit)
  Buffer clone() const;

protected:
  mutable std::mutex mutex_;
  std::vector<uint8_t> data_;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_BUFFER_HPP
