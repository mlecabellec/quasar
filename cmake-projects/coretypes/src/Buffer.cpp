#include "quasar/coretypes/Buffer.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace quasar {
namespace coretypes {

Buffer::Buffer() {
  // Default constructor: empty buffer
}

Buffer::Buffer(size_t size) : data_(size, 0) {
  // Initialize buffer with size and zero it out
}

Buffer::Buffer(const std::vector<uint8_t> &data) : data_(data) {
  // Initialize from existing vector
}

Buffer::Buffer(const Buffer &other) {
  // Thread-safe copy construction
  std::lock_guard<std::mutex> lock(other.mutex_);
  data_ = other.data_;
}

Buffer &Buffer::operator=(const Buffer &other) {
  // Thread-safe assignment
  if (this != &other) {
    std::unique_lock<std::mutex> lock1(mutex_, std::defer_lock);
    std::unique_lock<std::mutex> lock2(other.mutex_, std::defer_lock);
    std::lock(lock1, lock2);
    data_ = other.data_;
  }
  return *this;
}

size_t Buffer::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return data_.size();
}

uint8_t Buffer::get(size_t index) const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (index >= data_.size()) {
    throw std::out_of_range("Buffer index out of range");
  }
  return data_[index];
}

void Buffer::set(size_t index, uint8_t value) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (index >= data_.size()) {
    throw std::out_of_range("Buffer index out of range");
  }
  data_[index] = value;
}

std::string Buffer::toString() const {
  std::lock_guard<std::mutex> lock(mutex_);
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (uint8_t b : data_) {
    ss << std::setw(2) << static_cast<int>(b);
  }
  return ss.str();
}

Buffer Buffer::fromString(const std::string &hex) {
  if (hex.length() % 2 != 0) {
    throw std::invalid_argument("Invalid hex string length");
  }
  std::vector<uint8_t> data;
  data.reserve(hex.length() / 2);
  for (size_t i = 0; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    data.push_back(static_cast<uint8_t>(std::stoi(byteString, nullptr, 16)));
  }
  return Buffer(data);
}

void Buffer::writeInt(int value, size_t index, Endianness endian) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (index + 4 > data_.size()) {
    throw std::out_of_range("Buffer overflow for integer write");
  }

  // Cast to unsigned to avoid signed shift issues
  uint32_t val = static_cast<uint32_t>(value);

  if (endian == Endianness::BigEndian) {
    data_[index] = (val >> 24) & 0xFF;
    data_[index + 1] = (val >> 16) & 0xFF;
    data_[index + 2] = (val >> 8) & 0xFF;
    data_[index + 3] = val & 0xFF;
  } else {
    data_[index] = val & 0xFF;
    data_[index + 1] = (val >> 8) & 0xFF;
    data_[index + 2] = (val >> 16) & 0xFF;
    data_[index + 3] = (val >> 24) & 0xFF;
  }
}

int Buffer::readInt(size_t index, Endianness endian) const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (index + 4 > data_.size()) {
    throw std::out_of_range("Buffer underflow for integer read");
  }

  uint32_t result = 0;
  if (endian == Endianness::BigEndian) {
    result = (static_cast<uint32_t>(data_[index]) << 24) |
             (static_cast<uint32_t>(data_[index + 1]) << 16) |
             (static_cast<uint32_t>(data_[index + 2]) << 8) |
             static_cast<uint32_t>(data_[index + 3]);
  } else {
    result = static_cast<uint32_t>(data_[index]) |
             (static_cast<uint32_t>(data_[index + 1]) << 8) |
             (static_cast<uint32_t>(data_[index + 2]) << 16) |
             (static_cast<uint32_t>(data_[index + 3]) << 24);
  }
  return static_cast<int>(result);
}

Buffer Buffer::slice(size_t start, size_t length) const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (start + length > data_.size()) {
    throw std::out_of_range("Slice out of bounds");
  }
  std::vector<uint8_t> new_data(data_.begin() + start,
                                data_.begin() + start + length);
  return Buffer(new_data);
}

Buffer Buffer::concat(const Buffer &other) const {
  // Need to lock both.
  // To avoid deadlock, use std::lock or ordered locking.
  // However, we are declaring 'const', but we need to access 'other.data_'.
  // 'other' might be 'this'?
  if (this == &other) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<uint8_t> new_data = data_;
    new_data.insert(new_data.end(), data_.begin(), data_.end());
    return Buffer(new_data);
  }

  std::unique_lock<std::mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::mutex> lock2(other.mutex_, std::defer_lock);
  std::lock(lock1, lock2);

  std::vector<uint8_t> new_data = data_;
  new_data.insert(new_data.end(), other.data_.begin(), other.data_.end());
  return Buffer(new_data);
}

bool Buffer::equals(const Buffer &other) const {
  if (this == &other)
    return true;

  std::unique_lock<std::mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::mutex> lock2(other.mutex_, std::defer_lock);
  std::lock(lock1, lock2);

  return data_ == other.data_;
}

void Buffer::reverse() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::reverse(data_.begin(), data_.end());
}

void Buffer::reverse(size_t wordSize) {
  if (wordSize == 0)
    return;
  std::lock_guard<std::mutex> lock(mutex_);
  if (data_.size() % wordSize != 0) {
    throw std::invalid_argument("Buffer size not multiple of word size");
  }
  size_t nChunks = data_.size() / wordSize;
  for (size_t i = 0; i < nChunks / 2; ++i) {
    size_t startA = i * wordSize;
    size_t startB = (nChunks - 1 - i) * wordSize;
    std::swap_ranges(data_.begin() + startA, data_.begin() + startA + wordSize,
                     data_.begin() + startB);
  }
}

Buffer Buffer::clone() const {
  // Copy constructor does exactly this logic.
  return Buffer(*this);
}

} // namespace coretypes
} // namespace quasar
