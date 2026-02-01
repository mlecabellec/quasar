#include "quasar/coretypes/Buffer.hpp"
#include "quasar/coretypes/BufferSlice.hpp"
#include <algorithm>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <stdexcept>

namespace quasar::coretypes {

Buffer::Buffer() {}

Buffer::Buffer(size_t size) : data_(size, 0) {}

Buffer::Buffer(const std::vector<uint8_t> &data) : data_(data) {}

Buffer::Buffer(const Buffer &other) : std::enable_shared_from_this<Buffer>() {
  std::lock_guard<std::recursive_timed_mutex> lock(other.mutex_);
  data_ = other.data_;
}

Buffer &Buffer::operator=(const Buffer &other) {
  if (this != &other) {
    // Avoid deadlock by locking both mutexes with std::lock.
    // Use std::defer_lock to create the lock wrappers first.
    std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
    std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                       std::defer_lock);
    std::lock(lock1, lock2);
    // Copy the data vector.
    data_ = other.data_;
  }
  return *this;
}

size_t Buffer::size() const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  return data_.size();
}

uint8_t Buffer::get(size_t index) const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  if (index >= data_.size()) {
    throw std::out_of_range("Buffer index out of range");
  }
  return data_[index];
}

void Buffer::set(size_t index, uint8_t value) {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  if (index >= data_.size()) {
    throw std::out_of_range("Buffer index out of range");
  }
  data_[index] = value;
}

std::string Buffer::toString() const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  // Iterate over each byte and format it as a 2-digit hex string.
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
  // Iterate over the string in pairs of characters.
  for (size_t i = 0; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    // Parse the 2-digit hex string into a uint8_t.
    data.push_back(static_cast<uint8_t>(std::stoi(byteString, nullptr, 16)));
  }
  return Buffer(data);
}

void Buffer::writeInt(int value, size_t index, Endianness endian) {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  if (index + 4 > data_.size()) {
    throw std::out_of_range("Buffer overflow for integer write");
  }

  uint32_t val = static_cast<uint32_t>(value);

  if (endian == Endianness::BigEndian) {
    // Write bytes in Big Endian order (MSB first).
    data_[index] = (val >> 24) & 0xFF;
    data_[index + 1] = (val >> 16) & 0xFF;
    data_[index + 2] = (val >> 8) & 0xFF;
    data_[index + 3] = val & 0xFF;
  } else {
    // Write bytes in Little Endian order (LSB first).
    data_[index] = val & 0xFF;
    data_[index + 1] = (val >> 8) & 0xFF;
    data_[index + 2] = (val >> 16) & 0xFF;
    data_[index + 3] = (val >> 24) & 0xFF;
  }
}

int Buffer::readInt(size_t index, Endianness endian) const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  if (index + 4 > data_.size()) {
    throw std::out_of_range("Buffer underflow for integer read");
  }

  uint32_t result = 0;
  if (endian == Endianness::BigEndian) {
    // Read bytes in Big Endian order (MSB first) and reconstruct.
    result = (static_cast<uint32_t>(data_[index]) << 24) |
             (static_cast<uint32_t>(data_[index + 1]) << 16) |
             (static_cast<uint32_t>(data_[index + 2]) << 8) |
             static_cast<uint32_t>(data_[index + 3]);
  } else {
    // Read bytes in Little Endian order (LSB first) and reconstruct.
    result = static_cast<uint32_t>(data_[index]) |
             (static_cast<uint32_t>(data_[index + 1]) << 8) |
             (static_cast<uint32_t>(data_[index + 2]) << 16) |
             (static_cast<uint32_t>(data_[index + 3]) << 24);
  }
  return static_cast<int>(result);
}

Buffer Buffer::slice(size_t start, size_t length) const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  if (start + length > data_.size()) {
    throw std::out_of_range("Slice out of bounds");
  }
  std::vector<uint8_t> new_data(data_.begin() + start,
                                data_.begin() + start + length);
  return Buffer(new_data);
}

Buffer Buffer::concat(const Buffer &other) const {
  if (this == &other) {
    std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
    std::vector<uint8_t> new_data = data_;
    new_data.insert(new_data.end(), data_.begin(), data_.end());
    return Buffer(new_data);
  }

  // Lock both buffers safely.
  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  // Initialize with current data and append other data.
  std::vector<uint8_t> new_data = data_;
  new_data.insert(new_data.end(), other.data_.begin(), other.data_.end());
  return Buffer(new_data);
}

bool Buffer::equals(const Buffer &other) const {
  if (this == &other)
    return true;

  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  return data_ == other.data_;
}

void Buffer::reverse() {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  std::reverse(data_.begin(), data_.end());
}

void Buffer::reverse(size_t wordSize) {
  if (wordSize == 0)
    return;
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  if (data_.size() % wordSize != 0) {
    throw std::invalid_argument("Buffer size not multiple of word size");
  }
  size_t nChunks = data_.size() / wordSize;
  // Swap chunks from start and end moving inwards.
  for (size_t i = 0; i < nChunks / 2; ++i) {
    size_t startA = i * wordSize;
    size_t startB = (nChunks - 1 - i) * wordSize;
    // Swap the ranges.
    std::swap_ranges(data_.begin() + startA, data_.begin() + startA + wordSize,
                     data_.begin() + startB);
  }
}

Buffer Buffer::clone() const { return Buffer(*this); }

// New implementations

std::shared_ptr<BufferSlice> Buffer::sliceView(size_t start, size_t length) {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  if (start + length > data_.size()) {
    throw std::out_of_range("Slice out of bounds");
  }
  return std::make_shared<BufferSlice>(shared_from_this(), start, length);
}

Buffer Buffer::bitwiseAnd(const Buffer &other) const {
  if (this == &other)
    return clone();

  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  if (data_.size() != other.data_.size()) {
    throw std::invalid_argument(
        "Buffer sizes must match for bitwise operations");
  }
  // Create result buffer.
  std::vector<uint8_t> resData(data_.size());
  // Perform bitwise AND byte by byte.
  for (size_t i = 0; i < data_.size(); ++i) {
    resData[i] = data_[i] & other.data_[i];
  }
  return Buffer(resData);
}

Buffer Buffer::bitwiseOr(const Buffer &other) const {
  if (this == &other)
    return clone();

  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  if (data_.size() != other.data_.size()) {
    throw std::invalid_argument(
        "Buffer sizes must match for bitwise operations");
  }
  // Create result buffer.
  std::vector<uint8_t> resData(data_.size());
  // Perform bitwise OR byte by byte.
  for (size_t i = 0; i < data_.size(); ++i) {
    resData[i] = data_[i] | other.data_[i];
  }
  return Buffer(resData);
}

Buffer Buffer::bitwiseXor(const Buffer &other) const {
  if (this == &other) {
    // Xor with self is 0
    std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
    return Buffer(data_.size()); // Initialized to 0
  }

  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  if (data_.size() != other.data_.size()) {
    throw std::invalid_argument(
        "Buffer sizes must match for bitwise operations");
  }
  // Create result buffer.
  std::vector<uint8_t> resData(data_.size());
  // Perform bitwise XOR byte by byte.
  for (size_t i = 0; i < data_.size(); ++i) {
    resData[i] = data_[i] ^ other.data_[i];
  }
  return Buffer(resData);
}

Buffer Buffer::bitwiseNot() const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  std::vector<uint8_t> resData(data_.size());
  for (size_t i = 0; i < data_.size(); ++i) {
    resData[i] = ~data_[i];
  }
  return Buffer(resData);
}

int Buffer::compareTo(const Buffer &other) const {
  if (this == &other)
    return 0;
  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  // Lexicographical comparison
  if (data_ < other.data_)
    return -1;
  if (data_ > other.data_)
    return 1;
  return 0;
}

bool Buffer::equals(const std::vector<uint8_t> &other) const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  return data_ == other;
}

} // namespace quasar::coretypes
