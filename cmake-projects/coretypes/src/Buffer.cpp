#include "quasar/coretypes/Buffer.hpp"
#include "quasar/coretypes/BufferSlice.hpp"
#include <algorithm>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <stdexcept>

namespace quasar::coretypes {

Buffer::Buffer() {}

Buffer::Buffer(size_t size) : data_(size, 0) {
  // Fulfills [FE-0010.3.8] cloning the buffer with associated memory allocation.
  // Constructor initializes the internal vector with the requested size.
  // std::vector's constructor zero-initializes the elements.
}

Buffer::Buffer(const std::vector<uint8_t> &data) : data_(data) {
  // Fulfills [FE-0010.3.1] Encoding and decoding values to and from a basic buffer type.
  // Deep copy of the provided byte vector.
}

Buffer::Buffer(const Buffer &other) : std::enable_shared_from_this<Buffer>() {
  // Fulfills [FE-0010.3.9] The class is thread safe.
  // Lock the source buffer to ensure a consistent snapshot during copy.
  // Using recursive mutex allows the same thread to acquire it multiple times.
  std::lock_guard<std::recursive_timed_mutex> lock(other.mutex_);
  data_ = other.data_;
}

Buffer &Buffer::operator=(const Buffer &other) {
  if (this != &other) {
    // Fulfills [FE-0010.3.9] The class is thread safe.
    // Avoid deadlock when assigning between two buffers by using std::lock
    // which implements a deadlock-avoidance algorithm for multiple mutexes.
    // It acquires both locks in a consistent order across the application.
    std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
    std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                       std::defer_lock);
    std::lock(lock1, lock2);
    // Deep copy the internal data vector.
    data_ = other.data_;
  }
  return *this;
}

size_t Buffer::size() const {
  // Fulfills [FE-0010.3.9] The class is thread safe.
  // Lock required to protect access to the data vector in a thread-safe manner,
  // preventing race conditions with concurrent write operations.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  return data_.size();
}

uint8_t Buffer::get(size_t index) const {
  // Fulfills [FE-0010.3.9] The class is thread safe.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // Bounds checking before access to prevent undefined behavior or crashes.
  if (index >= data_.size()) {
    throw std::out_of_range("Buffer index out of range");
  }
  return data_[index];
}

void Buffer::set(size_t index, uint8_t value) {
  // Fulfills [FE-0010.3.9] The class is thread safe.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // Bounds checking to ensure memory safety during write operations.
  if (index >= data_.size()) {
    throw std::out_of_range("Buffer index out of range");
  }
  data_[index] = value;
}

std::string Buffer::toString() const {
  // Fulfills [FE-0010.3.2] Encoding and decoding values to and from a string.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  std::stringstream ss;
  // Use hex manipulator and setfill/setw to ensure each byte is exactly 2 hex digits.
  // This produces a standardized hex representation string.
  ss << std::hex << std::setfill('0');
  for (uint8_t b : data_) {
    ss << std::setw(2) << static_cast<int>(b);
  }
  return ss.str();
}

std::vector<uint8_t> Buffer::toVector() const {
  // Fulfills [FE-0030.5.10] Methods for conversion from and to std::vector<uint8_t>.
  // Return a copy of the internal data vector to avoid exposing internal state directly.
  // This maintains encapsulation and ensures thread safety for the caller.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  return data_;
}

Buffer Buffer::fromString(const std::string &hex) {
  // Fulfills [FE-0010.3.2] Encoding and decoding values to and from a string.
  // Hex string must represent bytes, so its length must be even (2 characters per byte).
  if (hex.length() % 2 != 0) {
    throw std::invalid_argument("Invalid hex string length: must be even");
  }
  std::vector<uint8_t> data;
  data.reserve(hex.length() / 2);
  // Extract two characters at a time and convert them from hex to a byte value.
  for (size_t i = 0; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    // std::stoi with base 16 parses the hex string into an integer.
    data.push_back(static_cast<uint8_t>(std::stoi(byteString, nullptr, 16)));
  }
  return Buffer(data);
}

void Buffer::writeInt(int value, size_t index, Endianness endian) {
  // Fulfills [FE-0010.3.3] The class shall provide methods for conversion from numeric types.
  // Fulfills [FE-0010.3.3.1] The class shall provide conversion methods allowing to specify endianness at byte or word level.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // Ensure there's enough space for a 4-byte (32-bit) integer write.
  if (index + 4 > data_.size()) {
    throw std::out_of_range("Buffer overflow for integer write: need 4 bytes");
  }

  // Use uint32_t for bitwise manipulation to avoid issues with sign extension.
  // Converting signed to unsigned preserves the bit pattern in two's complement.
  uint32_t val = static_cast<uint32_t>(value);

  if (endian == Endianness::BigEndian) {
    // Big Endian: Most Significant Byte (MSB) at the lowest index.
    // We extract bytes by shifting and masking with 0xFF.
    data_[index] = (val >> 24) & 0xFF;     // Most significant byte
    data_[index + 1] = (val >> 16) & 0xFF; // Second most significant
    data_[index + 2] = (val >> 8) & 0xFF;  // Third byte
    data_[index + 3] = val & 0xFF;         // Least significant byte
  } else {
    // Little Endian: Least Significant Byte (LSB) at the lowest index.
    data_[index] = val & 0xFF;             // Least significant byte
    data_[index + 1] = (val >> 8) & 0xFF;  // Second byte
    data_[index + 2] = (val >> 16) & 0xFF; // Third byte
    data_[index + 3] = (val >> 24) & 0xFF; // Most significant byte
  }
}

int Buffer::readInt(size_t index, Endianness endian) const {
  // Fulfills [FE-0010.3.3] The class shall provide methods for conversion from numeric types.
  // Fulfills [FE-0010.3.3.1] The class shall provide conversion methods allowing to specify endianness at byte or word level.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // Ensure there's enough data to read a 4-byte integer.
  if (index + 4 > data_.size()) {
    throw std::out_of_range("Buffer underflow for integer read: need 4 bytes");
  }

  uint32_t result = 0;
  if (endian == Endianness::BigEndian) {
    // Reconstruct the 32-bit integer from Big Endian byte order (MSB first).
    // We shift each byte to its correct position and combine them using bitwise OR.
    result = (static_cast<uint32_t>(data_[index]) << 24) |
             (static_cast<uint32_t>(data_[index + 1]) << 16) |
             (static_cast<uint32_t>(data_[index + 2]) << 8) |
             static_cast<uint32_t>(data_[index + 3]);
  } else {
    // Reconstruct the 32-bit integer from Little Endian byte order (LSB first).
    result = static_cast<uint32_t>(data_[index]) |
             (static_cast<uint32_t>(data_[index + 1]) << 8) |
             (static_cast<uint32_t>(data_[index + 2]) << 16) |
             (static_cast<uint32_t>(data_[index + 3]) << 24);
  }
  // Cast the unsigned bit pattern back to signed int before returning.
  // This is safe and produces the expected signed value on two's complement systems.
  return static_cast<int>(result);
}

Buffer Buffer::slice(size_t start, size_t length) const {
  // Fulfills [FE-0010.3.4] Methods for slicing the buffer.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // Verify slice boundaries against current buffer size.
  if (start + length > data_.size()) {
    throw std::out_of_range("Slice out of bounds");
  }
  // Create a new vector by copying the specified range of the current data.
  std::vector<uint8_t> new_data(data_.begin() + start,
                                data_.begin() + start + length);
  return Buffer(new_data);
}

Buffer Buffer::concat(const Buffer &other) const {
  // Fulfills [FE-0010.3.5] Methods for concatenation of buffers.
  if (this == &other) {
    // Special case for concatenating a buffer with itself to avoid deadlock 
    // or iterator invalidation while reading/writing to the same vector.
    std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
    std::vector<uint8_t> new_data = data_;
    new_data.insert(new_data.end(), data_.begin(), data_.end());
    return Buffer(new_data);
  }

  // Lock both buffers simultaneously to prevent deadlocks in concurrent environments.
  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  // Combine data from both buffers into a new instance by appending the second to the first.
  std::vector<uint8_t> new_data = data_;
  new_data.insert(new_data.end(), other.data_.begin(), other.data_.end());
  return Buffer(new_data);
}

bool Buffer::equals(const Buffer &other) const {
  // Fulfills [FE-0010.3.6] Methods for comparison.
  if (this == &other)
    return true;

  // Use deadlock-safe locking when comparing two different buffer instances.
  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  // std::vector's operator== efficiently compares size and each element.
  return data_ == other.data_;
}

void Buffer::reverse() {
  // Fulfills [FE-0010.3.7] Methods for reversing the buffer at byte level.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // Use STL algorithm to reverse the entire data vector in-place.
  std::reverse(data_.begin(), data_.end());
}

void Buffer::reverse(size_t wordSize) {
  // Fulfills [FE-0010.3.7] Methods for reversing the buffer at word level.
  // Fulfills [FE-0010.3.3.2] Specify word size.
  // wordSize 0 is invalid for division.
  if (wordSize == 0) {
    throw std::invalid_argument("Word size must be greater than zero");
  }
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // This operation only makes sense if the data can be cleanly divided into words.
  if (data_.size() % wordSize != 0) {
    throw std::invalid_argument("Buffer size not multiple of word size");
  }
  size_t nChunks = data_.size() / wordSize;
  // Iterate through half of the chunks and swap with their corresponding mirror chunk.
  for (size_t i = 0; i < nChunks / 2; ++i) {
    size_t startA = i * wordSize;
    size_t startB = (nChunks - 1 - i) * wordSize;
    // std::swap_ranges efficiently swaps two contiguous blocks of data of the same length.
    std::swap_ranges(data_.begin() + startA, data_.begin() + startA + wordSize,
                     data_.begin() + startB);
  }
}

Buffer Buffer::clone() const {
  // Fulfills [FE-0010.3.8] Methods for cloning the buffer with associated memory allocation.
  // Explicit clone uses the copy constructor which is already thread-safe.
  return Buffer(*this);
}

// New implementations

std::shared_ptr<BufferSlice> Buffer::sliceView(size_t start, size_t length) {
  // Fulfills [FE-0030.5.7] Lightweight views creation.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // Bounds check for the view window.
  if (start + length > data_.size()) {
    throw std::out_of_range("Slice out of bounds");
  }
  // Create a view that shares ownership of this buffer instance.
  // shared_from_this() requires the buffer to already be managed by a shared_ptr.
  return std::make_shared<BufferSlice>(shared_from_this(), start, length);
}

Buffer Buffer::bitwiseAnd(const Buffer &other) const {
  // Fulfills [FE-0030.5.3] Bitwise operations.
  // Optimized case for self-AND.
  if (this == &other)
    return clone();

  // Multi-lock for thread safety.
  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  // Bitwise operations require buffers of identical size.
  if (data_.size() != other.data_.size()) {
    throw std::invalid_argument(
        "Buffer sizes must match for bitwise operations");
  }
  
  std::vector<uint8_t> resData(data_.size());
  // Perform bitwise AND element-wise across the entire range.
  for (size_t i = 0; i < data_.size(); ++i) {
    resData[i] = data_[i] & other.data_[i];
  }
  return Buffer(resData);
}

Buffer Buffer::bitwiseOr(const Buffer &other) const {
  // Fulfills [FE-0030.5.3] Bitwise operations.
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

  std::vector<uint8_t> resData(data_.size());
  // Perform bitwise OR element-wise.
  for (size_t i = 0; i < data_.size(); ++i) {
    resData[i] = data_[i] | other.data_[i];
  }
  return Buffer(resData);
}

Buffer Buffer::bitwiseXor(const Buffer &other) const {
  // Fulfills [FE-0030.5.3] Bitwise operations.
  if (this == &other) {
    // XORing any value with itself results in 0.
    std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
    return Buffer(data_.size()); // Buffer constructor initializes to zeros.
  }

  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  if (data_.size() != other.data_.size()) {
    throw std::invalid_argument(
        "Buffer sizes must match for bitwise operations");
  }

  std::vector<uint8_t> resData(data_.size());
  // Perform bitwise XOR element-wise.
  for (size_t i = 0; i < data_.size(); ++i) {
    resData[i] = data_[i] ^ other.data_[i];
  }
  return Buffer(resData);
}

Buffer Buffer::bitwiseNot() const {
  // Fulfills [FE-0030.5.3] Bitwise operations.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  std::vector<uint8_t> resData(data_.size());
  // Flip all bits in each byte using the bitwise NOT operator.
  for (size_t i = 0; i < data_.size(); ++i) {
    resData[i] = ~data_[i];
  }
  return Buffer(resData);
}

int Buffer::compareTo(const Buffer &other) const {
  // Fulfills [FE-0010.3.6] Methods for comparison.
  // Identity check.
  if (this == &other)
    return 0;
  
  // Safe locking for cross-instance comparison.
  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  // Lexicographical comparison using vector's built-in operators.
  if (data_ < other.data_)
    return -1;
  if (data_ > other.data_)
    return 1;
  return 0;
}

bool Buffer::equals(const std::vector<uint8_t> &other) const {
  // Fulfills [FE-0010.3.6] Methods for comparison.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // Compare internal data vector with a raw vector of bytes.
  return data_ == other;
}

} // namespace quasar::coretypes
