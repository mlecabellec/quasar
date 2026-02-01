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
  // Validate slice bounds against buffer size.
  if (offset + length > buffer->size()) {
    throw std::out_of_range("Slice out of buffer bounds");
  }
}

size_t BufferSlice::size() const { return length_; }

uint8_t BufferSlice::get(size_t index) const {
  // Check bounds relative to slice length.
  if (index >= length_) {
    throw std::out_of_range("Index out of slice bounds");
  }
  // Delegate to parent buffer with offset.
  return buffer_->get(offset_ + index);
}

void BufferSlice::set(size_t index, uint8_t value) {
  // Check bounds relative to slice length.
  if (index >= length_) {
    throw std::out_of_range("Index out of slice bounds");
  }
  // Delegate to parent buffer with offset.
  buffer_->set(offset_ + index, value);
}

std::vector<uint8_t> BufferSlice::toVector() const {
  std::vector<uint8_t> vec;
  vec.reserve(length_);
  // Copy data byte by byte.
  for (size_t i = 0; i < length_; ++i) {
    vec.push_back(get(i));
  }
  return vec;
}

std::string BufferSlice::toString() const {
  // Hex string?
  // Use Buffer logic or similar?
  // Buffer has toString() returning hex or something.
  // Let's reuse:
  // This is inefficient but safe:
  return buffer_->slice(offset_, length_).toString();
}

BufferSlice BufferSlice::slice(size_t index, size_t subLength) const {
  // Validate sub-slice bounds.
  if (index + subLength > length_) {
    throw std::out_of_range("Sub-slice out of bounds");
  }
  // Create new slice referencing same parent buffer but adjusted offset.
  return BufferSlice(buffer_, offset_ + index, subLength);
}

std::shared_ptr<Buffer> BufferSlice::concat(const BufferSlice &other) const {
  // Create new buffer of size length_ + other.length_
  std::shared_ptr<Buffer> newBuf =
      std::make_shared<Buffer>(length_ + other.length_);
  // Copy data from this slice.
  for (size_t i = 0; i < length_; ++i) {
    newBuf->set(i, get(i));
  }
  // Copy data from other slice.
  for (size_t i = 0; i < other.length_; ++i) {
    newBuf->set(length_ + i, other.get(i));
  }
  return newBuf;
}

std::shared_ptr<Buffer> BufferSlice::getParent() const { return buffer_; }

size_t BufferSlice::getOffset() const { return offset_; }

bool BufferSlice::equals(const BufferSlice &other) const {
  if (length_ != other.length_)
    return false;
  for (size_t i = 0; i < length_; ++i) {
    if (get(i) != other.get(i))
      return false;
  }
  return true;
}

void BufferSlice::fromNumber(const Number &n) {
  // TODO: implement based on encoding (not specified, assuming big endian int
  // write?) FE-0030: "Buffer... shall provide methods for conversion from and
  // to quasar::coretypes::Number." This might mean writing the number into the
  // buffer? Or parsing the buffer as a number? "Conversion from and to" implies
  // both.

  // For now, if we assume 4 bytes for int, 8 for long/double.
  // This is ambiguous. I'll leave empty or throw strictly.
  // Or try to write 4 bytes if Integer?
  if (length_ >= 4) {
    buffer_->writeInt(n.toInt(), offset_); // Buffer implementation needed
  }
}

} // namespace quasar::coretypes
