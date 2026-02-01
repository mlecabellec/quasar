#include "quasar/coretypes/BitBufferSlice.hpp"
#include <stdexcept>

namespace quasar::coretypes {

BitBufferSlice::BitBufferSlice(std::shared_ptr<BitBuffer> buffer,
                               size_t startBit, size_t bitLength)
    : buffer_(buffer), startBit_(startBit), bitLength_(bitLength) {
  // Validate buffer is not null.
  if (!buffer) {
    throw std::invalid_argument("Buffer cannot be null");
  }
  // Check bounds: BitBuffer::bitSize() ?
  // BitBuffer::bitSize() depends on content.
  size_t parentSize = buffer->bitSize();
  if (parentSize == 0 && buffer->size() > 0)
    parentSize = buffer->size() * 8;

  // Validate slice bounds.
  if (startBit + bitLength > parentSize) {
    throw std::out_of_range("Bit slice out of bounds");
  }
}

size_t BitBufferSlice::size() const { return bitLength_; }

bool BitBufferSlice::getBit(size_t index) const {
  // Check slice bounds.
  if (index >= bitLength_) {
    throw std::out_of_range("Index out of slice bounds");
  }
  // Delegate to parent buffer with offset.
  return buffer_->getBit(startBit_ + index);
}

void BitBufferSlice::setBit(size_t index, bool value) {
  // Check slice bounds.
  if (index >= bitLength_) {
    throw std::out_of_range("Index out of slice bounds");
  }
  // Delegate to parent buffer with offset.
  buffer_->setBit(startBit_ + index, value);
}

BitBufferSlice BitBufferSlice::slice(size_t index, size_t subLength) const {
  // Check sub-slice bounds.
  if (index + subLength > bitLength_) {
    throw std::out_of_range("Sub-slice out of bounds");
  }
  // Return new slice referencing same parent.
  return BitBufferSlice(buffer_, startBit_ + index, subLength);
}

std::shared_ptr<BitBuffer>
BitBufferSlice::concat(const BitBufferSlice &other) const {
  // Implement concat logic
  size_t resSize = bitLength_ + other.bitLength_;
  std::shared_ptr<BitBuffer> res = std::make_shared<BitBuffer>(resSize);

  // Copy bits from this slice.
  for (size_t i = 0; i < bitLength_; ++i) {
    res->setBit(i, getBit(i));
  }
  // Copy bits from other slice.
  for (size_t i = 0; i < other.bitLength_; ++i) {
    res->setBit(bitLength_ + i, other.getBit(i));
  }
  return res;
}

std::vector<uint8_t> BitBufferSlice::toVector() const {
  // Pack bits into bytes
  size_t byteCount = (bitLength_ + 7) / 8;
  std::vector<uint8_t> vec(byteCount, 0);

  // Iterate bits and set in bytes.
  for (size_t i = 0; i < bitLength_; ++i) {
    if (getBit(i)) {
      size_t b = i / 8;
      size_t o = i % 8;
      vec[b] |= (1 << (7 - o));
    }
  }
  return vec;
}

std::shared_ptr<BitBuffer> BitBufferSlice::getParent() const { return buffer_; }

size_t BitBufferSlice::getOffset() const { return startBit_; }

} // namespace quasar::coretypes
