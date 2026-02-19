#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace core {

/**
 * A wrapper around a raw byte buffer.
 * Can either own the memory or wrap an existing buffer.
 */
class BitBuffer {
public:
  /**
   * Constructor: Creates a new buffer of given size.
   * Owns the memory.
   */
  explicit BitBuffer(std::size_t size)
      : _size(size), _capacity(size), _ownsMemory(true) {
    _data = new uint8_t[_capacity];
    std::memset(_data, 0, _capacity);
  }

  /**
   * Constructor: Wraps an existing buffer.
   * Does NOT own the memory.
   */
  BitBuffer(uint8_t *buffer, std::size_t size)
      : _data(buffer), _size(size), _capacity(size), _ownsMemory(false) {}

  virtual ~BitBuffer() {
    if (_ownsMemory && _data) {
      delete[] _data;
    }
  }

  // Disable copy for now to avoid ownership issues, implement move if needed
  BitBuffer(const BitBuffer &) = delete;
  BitBuffer &operator=(const BitBuffer &) = delete;

  /**
   * Get pointer to raw data.
   */
  uint8_t *data() { return _data; }
  const uint8_t *data() const { return _data; }

  /**
   * Get size in bytes.
   */
  std::size_t size() const { return _size; }

  /**
   * Copy data from another buffer.
   */
  void copyFrom(const void *src, std::size_t count) {
    std::size_t copySize = std::min(count, _size);
    std::memcpy(_data, src, copySize);
  }

  /**
   * Set all bytes to zero.
   */
  void clear() { std::memset(_data, 0, _size); }

protected:
  uint8_t *_data = nullptr;
  std::size_t _size = 0;
  std::size_t _capacity = 0;
  bool _ownsMemory = false;
};

} // namespace core
