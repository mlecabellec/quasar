/**
 * @file ProcessImage.hpp
 * @brief Logical process data image management.
 */

#pragma once

#include "resoem/common.hpp"
#include <vector>
#include <span>

namespace resoem {

/**
 * @brief Manages the logical process image buffer.
 * 
 * This class provides bit and byte-level access to the combined process data
 * (inputs and outputs) of all slaves on the network.
 */
class ProcessImage {
public:
  /// @brief Resize the process image buffer.
  void resize(size_t size) { 
    data_.resize(size, 0); 
  }

  /// @brief Get the size of the process image in bytes.
  size_t size() const { return data_.size(); }

  /// @brief Get a span to the mutable image data.
  std::span<byte> data() { return data_; }

  /// @brief Get a span to the constant image data.
  std::span<const byte> data() const { return data_; }

  /// @brief Write a byte to the process image.
  void write_byte(uint32_t offset, byte value) {
    if (offset < data_.size()) {
      data_[offset] = value;
    }
  }

  /// @brief Read a byte from the process image.
  byte read_byte(uint32_t offset) const {
    if (offset < data_.size()) {
      return data_[offset];
    }
    return 0;
  }

  /// @brief Write a single bit in the process image.
  void write_bit(uint32_t byte_offset, uint8_t bit_offset, bool value) {
    if (byte_offset < data_.size()) {
      if (value == true) {
        // Set bit.
        data_[byte_offset] |= (1 << bit_offset);
      } else {
        // Clear bit.
        data_[byte_offset] &= ~(1 << bit_offset);
      }
    }
  }

  /// @brief Read a single bit from the process image.
  bool read_bit(uint32_t byte_offset, uint8_t bit_offset) const {
    if (byte_offset < data_.size()) {
      return (data_[byte_offset] & (1 << bit_offset)) != 0;
    }
    return false;
  }

private:
  std::vector<byte> data_; ///< Internal storage for the process image.
};

} // namespace resoem
