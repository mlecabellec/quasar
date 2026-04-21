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
#include <algorithm>
#include <bit>

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
 * binary protocol processing, file I/O, and data serialization. Essential for handling
 * raw Ethernet frames and binary data used in protocols like EtherCAT [FE-0040.1], [FE-0040.2].
 * Furthermore, this class provides fundamental capabilities for implementing
 * protocol definitions as required by [FE-0100], such as parsing headers
 * for protocols like UDP, TCP, IPv4, Ethernet II, CCSDS, CAN, IENA, and ESVF.
 * Its methods for byte-level access, endianness control, and integer read/write
 * are critical for interpreting structured binary data found in network protocols.
 * 
 * **Compliance**:
 * - Fulfills [FE-0010.3] Provide a Buffer class.
 * - Fulfills [FE-0010.3.9] The class is thread safe.
 * - Fulfills [FE-0030.8] Usage of mutexes with timeout preferred (implemented via recursive_timed_mutex).
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
   * 
   * Fulfills [FE-0010.3.1] Encoding and decoding values to and from a basic buffer type.
   * Fulfills [FE-0030.5.10] Methods for conversion from and to std::vector<uint8_t>.
   * 
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
   * 
   * Fulfills [FE-0010.3.2] Encoding and decoding values to and from a string.
   * Fulfills [FE-0030.5.12] Conversion to quasar::coretypes::String.
   * 
   * @return A string containing the hex representation (e.g., "0a1b2c").
   */
  std::string toString() const;

  /**
   * @brief Returns a copy of the internal data as a byte vector.
   * 
   * Fulfills [FE-0030.5.10] Methods for conversion from and to std::vector<uint8_t>.
   * 
   * @return A std::vector<uint8_t> containing a copy of the buffer data.
   */
  std::vector<uint8_t> toVector() const;

  /**
   * @brief Creates a new Buffer from a hexadecimal string.
   * 
   * Fulfills [FE-0010.3.2] Encoding and decoding values to and from a string.
   * 
   * @param hex The hex string to parse.
   * @return A Buffer instance containing the parsed data.
   * @throws std::invalid_argument If the hex string is invalid or has an odd length.
   */
  static Buffer fromString(const std::string &hex);

  /**
   * @brief Writes a 32-bit integer to the buffer at the specified index.
   * 
   * Fulfills [FE-0010.3.3] Methods for conversion from numeric types.
   * Fulfills [FE-0010.3.3.1] Specify endianness.
   * Endianness control is crucial for network protocols like EtherCAT [FE-0040.1.2].
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
   * Fulfills [FE-0030.5.11] Conversion from and to quasar::coretypes::Number.
   *
   * @param index The starting index in the buffer.
   * @param endian The endianness to use for decoding (defaults to BigEndian).
   * @return The 32-bit integer read from the buffer.
   * @throws std::out_of_range If the read would exceed the buffer's boundaries.
   */
  int readInt(size_t index, Endianness endian = Endianness::BigEndian) const;

  /**
   * @brief Writes a value of type T to the buffer with specified endianness.
   * @tparam T The type to write (must be integral or floating point).
   * @param value The value to write.
   * @param index The starting index.
   * @param endian The endianness.
   * @throws std::out_of_range If boundaries are exceeded.
   */
  template <typename T>
  void write(T value, size_t index, Endianness endian = Endianness::BigEndian);

  /**
   * @brief Reads a value of type T from the buffer with specified endianness.
   * @tparam T The type to read.
   * @param index The starting index.
   * @param endian The endianness.
   * @return The value read.
   * @throws std::out_of_range If boundaries are exceeded.
   */
  template <typename T>
  T read(size_t index, Endianness endian = Endianness::BigEndian) const;

  /**
   * @brief Creates a new Buffer containing a deep copy of a slice of this buffer.
   * 
   * Fulfills [FE-0010.3.4] Methods for slicing the buffer.
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
   * Fulfills [FE-0010.3.5] Methods for concatenation of buffers.
   *
   * @param other The buffer to append to this one.
   * @return A new Buffer containing the concatenated data.
   */
  Buffer concat(const Buffer &other) const;

  /**
   * @brief Checks if this buffer is identical to another buffer.
   * 
   * Fulfills [FE-0010.3.6] Methods for comparison.
   * Fulfills [FE-0030.5.1] Comparison with other Buffer objects.
   *
   * Two buffers are equal if they have the same size and identical content.
   * @param other The buffer to compare with.
   * @return true if both buffers are equal.
   */
  bool equals(const Buffer &other) const;

  /**
   * @brief Reverses the order of all bytes in the buffer in-place.
   * 
   * Fulfills [FE-0010.3.7] Methods for reversing the buffer at byte level.
   */
  void reverse();

  /**
   * @brief Reverses the buffer in-place in chunks of a specified word size.
   *
   * Useful for swapping endianness of larger blocks of data (e.g., swapping 
   * 32-bit words).
   * 
   * Fulfills [FE-0010.3.7] Methods for reversing the buffer at word level.
   * Fulfills [FE-0010.3.3.2] Specify word size.
   *
   * @param wordSize The size of chunks to swap (in bytes).
   * @throws std::invalid_argument If the buffer size is not a multiple of @p wordSize, 
   *                               or if @p wordSize is 0.
   */
  void reverse(size_t wordSize);

  /**
   * @brief Creates a deep copy of the buffer.
   * 
   * Fulfills [FE-0010.3.8] Methods for cloning the buffer with associated memory allocation.
   * 
   * @return A new Buffer instance with the same data.
   */
  Buffer clone() const;

  /**
   * @brief Creates a lightweight view slice of the buffer.
   *
   * A slice view provides shared access to a portion of the buffer without copying.
   * Modifications through the view are reflected in the original buffer.
   * Similar to std::span [FE-0040.8.2], this facilitates efficient zero-copy data access.
   * 
   * Fulfills [FE-0030.5.7] Create a BufferSlice class which is a view of the original Buffer.
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
   * Fulfills [FE-0030.5.3] Bitwise operations.
   * Crucial for low-level frame manipulation and bitmasking operations in protocols like EtherCAT [FE-0040.2].
   *
   * @param other The other buffer of the same size.
   * @return A new Buffer containing the result of (this & other).
   * @throws std::invalid_argument If the buffers have different sizes.
   */
  Buffer bitwiseAnd(const Buffer &other) const;

  /**
   * @brief Performs an element-wise bitwise OR with another buffer.
   * 
   * Fulfills [FE-0030.5.3] Bitwise operations.
   *
   * @param other The other buffer of the same size.
   * @return A new Buffer containing the result of (this | other).
   * @throws std::invalid_argument If the buffers have different sizes.
   */
  Buffer bitwiseOr(const Buffer &other) const;

  /**
   * @brief Performs an element-wise bitwise XOR with another buffer.
   * 
   * Fulfills [FE-0030.5.3] Bitwise operations.
   *
   * @param other The other buffer of the same size.
   * @return A new Buffer containing the result of (this ^ other).
   * @throws std::invalid_argument If the buffers have different sizes.
   */
  Buffer bitwiseXor(const Buffer &other) const;

  /**
   * @brief Performs a bitwise NOT operation on the buffer content.
   * 
   * Fulfills [FE-0030.5.3] Bitwise operations.
   *
   * @return A new Buffer containing the bitwise complement of this buffer's data.
   */
  Buffer bitwiseNot() const;

  /**
   * @brief Compares this buffer lexicographically with another.
   * 
   * Fulfills [FE-0010.3.6] Methods for comparison.
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
   * Relies on std::chrono for timeouts, aligning with [FE-0040.8.3].
   */
  mutable std::recursive_timed_mutex mutex_;

  /**
   * @brief The raw byte storage.
   */
  std::vector<uint8_t> data_;
};

template <typename T>
void Buffer::write(T value, size_t index, Endianness endian) {
    std::unique_lock<std::recursive_timed_mutex> lock(mutex_, std::chrono::seconds(1));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring buffer lock");
    if (index + sizeof(T) > data_.size()) {
        throw std::out_of_range("Buffer overflow for write");
    }

    uint64_t val = 0;
    if constexpr (std::is_floating_point_v<T>) {
        if constexpr (sizeof(T) == 4) {
            float f = static_cast<float>(value);
            val = std::bit_cast<uint32_t>(f);
        } else {
            double d = static_cast<double>(value);
            val = std::bit_cast<uint64_t>(d);
        }
    } else {
        val = static_cast<uint64_t>(value);
    }

    if (endian == Endianness::BigEndian) {
        for (size_t i = 0; i < sizeof(T); ++i) {
            data_[index + sizeof(T) - 1 - i] = static_cast<uint8_t>(val >> (i * 8));
        }
    } else {
        for (size_t i = 0; i < sizeof(T); ++i) {
            data_[index + i] = static_cast<uint8_t>(val >> (i * 8));
        }
    }
}

template <typename T>
T Buffer::read(size_t index, Endianness endian) const {
    std::unique_lock<std::recursive_timed_mutex> lock(mutex_, std::chrono::seconds(1));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring buffer lock");
    if (index + sizeof(T) > data_.size()) {
        throw std::out_of_range("Buffer underflow for read");
    }

    uint64_t val = 0;
    if (endian == Endianness::BigEndian) {
        for (size_t i = 0; i < sizeof(T); ++i) {
            val |= static_cast<uint64_t>(data_[index + sizeof(T) - 1 - i]) << (i * 8);
        }
    } else {
        for (size_t i = 0; i < sizeof(T); ++i) {
            val |= static_cast<uint64_t>(data_[index + i]) << (i * 8);
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        if constexpr (sizeof(T) == 4) {
            uint32_t v32 = static_cast<uint32_t>(val);
            return static_cast<T>(std::bit_cast<float>(v32));
        } else {
            return static_cast<T>(std::bit_cast<double>(val));
        }
    } else {
        return static_cast<T>(val);
    }
}

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_BUFFER_HPP
