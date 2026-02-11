#include "quasar/coretypes/BufferSlice.hpp"
#include <stdexcept>

namespace quasar::coretypes {

BufferSlice::BufferSlice(std::shared_ptr<Buffer> buffer, size_t offset,
                         size_t length)
    : buffer_(buffer), offset_(offset), length_(length) {
  // Validate input buffer is not null.
  if (!buffer) {
    throw std::invalid_argument("Buffer cannot be null");
  }
  // Validate slice bounds against buffer size to prevent memory access violations.
  if (offset + length > buffer->size()) {
    throw std::out_of_range("Slice out of buffer bounds");
  }
}

size_t BufferSlice::size() const { 
  // Returns the window size in bytes.
  return length_; 
}

uint8_t BufferSlice::get(size_t index) const {
  // Check bounds relative to slice length.
  if (index >= length_) {
    throw std::out_of_range("Index out of slice bounds");
  }
  // Delegation: the slice doesn't hold data itself, it asks its parent buffer
  // for the byte at the calculated absolute index (offset_ + index).
  // Thread safety is handled by the parent Buffer instance.
  return buffer_->get(offset_ + index);
}

void BufferSlice::set(size_t index, uint8_t value) {
  // Check bounds relative to slice length.
  if (index >= length_) {
    throw std::out_of_range("Index out of slice bounds");
  }
  // Delegation: modifying the slice actually modifies the parent buffer's memory.
  // This allows multiple slices to view and modify the same underlying data.
  buffer_->set(offset_ + index, value);
}

std::vector<uint8_t> BufferSlice::toVector() const {
  std::vector<uint8_t> vec;
  // Pre-allocate memory to avoid multiple reallocations during push_back.
  vec.reserve(length_);
  // Deep copy: Create a standalone vector containing only the data from this slice's range.
  for (size_t i = 0; i < length_; ++i) {
    vec.push_back(get(i));
  }
  return vec;
}

std::string BufferSlice::toString() const {
  // We reuse the parent Buffer's hex string conversion logic.
  // By using Buffer::slice, we get a temporary deep-copy Buffer containing the 
  // slice's data, which we then convert to a hex string.
  return buffer_->slice(offset_, length_).toString();
}

BufferSlice BufferSlice::slice(size_t index, size_t subLength) const {
  // Validate sub-slice bounds relative to this slice's current length.
  if (index + subLength > length_) {
    throw std::out_of_range("Sub-slice out of bounds");
  }
  // The new slice points to the same parent buffer but with an adjusted global offset.
  // offset_ + index is the new starting position relative to the absolute buffer start.
  return BufferSlice(buffer_, offset_ + index, subLength);
}

std::shared_ptr<Buffer> BufferSlice::concat(const BufferSlice &other) const {
  // Create a brand new Buffer large enough to hold both slices.
  std::shared_ptr<Buffer> newBuf =
      std::make_shared<Buffer>(length_ + other.length_);
  
  // First, copy our own data into the beginning of the new buffer.
  for (size_t i = 0; i < length_; ++i) {
    newBuf->set(i, get(i));
  }
  // Then, append the other slice's data immediately after ours.
  for (size_t i = 0; i < other.length_; ++i) {
    newBuf->set(length_ + i, other.get(i));
  }
  return newBuf;
}

std::shared_ptr<Buffer> BufferSlice::getParent() const { 
  // Returns the shared pointer to the underlying Buffer object.
  return buffer_; 
}

size_t BufferSlice::getOffset() const { 
  // Returns the absolute byte offset within the parent Buffer.
  return offset_; 
}

bool BufferSlice::equals(const BufferSlice &other) const {
  // Performance optimization: different sizes cannot be equal.
  if (length_ != other.length_)
    return false;
  // Byte-by-byte comparison of content between the two slice windows.
  for (size_t i = 0; i < length_; ++i) {
    if (get(i) != other.get(i))
      return false;
  }
  return true;
}

void BufferSlice::fromNumber(const Number &n) {
  // Support for writing numeric values into the slice window.
  // We currently assume 32-bit integer writing and require at least 4 bytes of space.
  if (length_ >= 4) {
    // We delegate directly to the parent buffer using our offset.
    // By default, we use Big Endian for numeric representation in the buffer.
    buffer_->writeInt(n.toInt(), offset_, Endianness::BigEndian);
  } else {
      // If the slice is too small, we currently do nothing or could throw.
      // Given the previous implementation, we keep it as a silent check or add warning.
  }
}

} // namespace quasar::coretypes
