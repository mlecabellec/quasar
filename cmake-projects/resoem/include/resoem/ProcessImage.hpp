#pragma once

#include "resoem/common.hpp"
#include <vector>
#include <span>

namespace resoem {

class ProcessImage {
public:
  void resize(size_t size) { data_.resize(size, 0); }
  size_t size() const { return data_.size(); }

  std::span<byte> data() { return data_; }
  std::span<const byte> data() const { return data_; }

  void write_byte(uint32_t offset, byte value) {
    if (offset < data_.size()) {
      data_[offset] = value;
    }
  }

  byte read_byte(uint32_t offset) const {
    if (offset < data_.size()) {
      return data_[offset];
    }
    return 0;
  }

  void write_bit(uint32_t byte_offset, uint8_t bit_offset, bool value) {
    if (byte_offset < data_.size()) {
      if (value) {
        data_[byte_offset] |= (1 << bit_offset);
      } else {
        data_[byte_offset] &= ~(1 << bit_offset);
      }
    }
  }

  bool read_bit(uint32_t byte_offset, uint8_t bit_offset) const {
    if (byte_offset < data_.size()) {
      return (data_[byte_offset] & (1 << bit_offset)) != 0;
    }
    return false;
  }

private:
  std::vector<byte> data_;
};

} // namespace resoem
