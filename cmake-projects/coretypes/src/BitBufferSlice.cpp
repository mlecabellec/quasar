#include "quasar/coretypes/BitBufferSlice.hpp"
#include <stdexcept>

namespace quasar::coretypes {

BitBufferSlice::BitBufferSlice(std::shared_ptr<BitBuffer> buffer,
                               size_t startBit, size_t bitLength)
    : buffer_(buffer), startBit_(startBit), bitLength_(bitLength) {
  // Fulfills [FE-0030.7.1] A slice shall be defined by a starting offset and a length.
  // Validate buffer is not null.
  if (!buffer) {
    throw std::invalid_argument("Buffer cannot be null");
  }

  // Obtain parent size to validate slice boundaries.
  size_t parentSize = buffer->bitSize();
  // If bitSize() returns 0, it might mean the parent is wrapping a raw Buffer.
  if (parentSize == 0 && buffer->size() > 0)
    parentSize = buffer->size() * 8;

  // Ensure the entire slice fits within the parent buffer.
  if (startBit + bitLength > parentSize) {
    throw std::out_of_range("Bit slice out of bounds");
  }
}

size_t BitBufferSlice::size() const { 
  // Returns the length in bits.
  return bitLength_; 
}

bool BitBufferSlice::getBit(size_t index) const {
  // Ensure the requested bit is within this slice's boundaries.
  if (index >= bitLength_) {
    throw std::out_of_range("Index out of slice bounds");
  }
  // Delegation: the slice calculates the absolute bit position (startBit_ + index)
  // and retrieves it from the parent BitBuffer. The parent handles thread safety.
  return buffer_->getBit(startBit_ + index);
}

void BitBufferSlice::setBit(size_t index, bool value) {
  // Ensure the target bit is within this slice's boundaries.
  if (index >= bitLength_) {
    throw std::out_of_range("Index out of slice bounds");
  }
  // Delegation: modifying a bit in the slice directly updates the parent BitBuffer.
  buffer_->setBit(startBit_ + index, value);
}

BitBufferSlice BitBufferSlice::slice(size_t index, size_t subLength) const {
  // Fulfills [FE-0030.7.6] Slices can be created from a slice.
  // Validate sub-slice boundaries relative to this slice's current bit length.
  if (index + subLength > bitLength_) {
    throw std::out_of_range("Sub-slice out of bounds");
  }
  // The new sub-slice shares the same parent BitBuffer but with a further shifted offset.
  return BitBufferSlice(buffer_, startBit_ + index, subLength);
}

std::shared_ptr<BitBuffer>
BitBufferSlice::concat(const BitBufferSlice &other) const {
  // Fulfills [FE-0030.7.3] A slice shall be able to be concatenated with other slices.
  // Determine the total size of the resulting concatenated buffer.
  size_t resSize = bitLength_ + other.bitLength_;
  // Create a new BitBuffer instance to hold the result.
  std::shared_ptr<BitBuffer> res = std::make_shared<BitBuffer>(resSize);

  // Copy bits from the current slice. We use bit-by-bit copying because 
  // slices might not be byte-aligned.
  for (size_t i = 0; i < bitLength_; ++i) {
    res->setBit(i, getBit(i));
  }
  // Copy bits from the other slice, appending them after the first set.
  for (size_t i = 0; i < other.bitLength_; ++i) {
    res->setBit(bitLength_ + i, other.getBit(i));
  }
  return res;
}

std::vector<uint8_t> BitBufferSlice::toVector() const {
  // Fulfills [FE-0030.5.10] Methods for conversion from and to std::vector<uint8_t>.
  // Determine how many bytes are needed to store the bits of this slice.
  // Ceiling division (bits + 7) / 8.
  size_t byteCount = (bitLength_ + 7) / 8;
  std::vector<uint8_t> vec(byteCount, 0);

  // Pack each bit into the byte vector using Big Endian bit numbering.
  for (size_t i = 0; i < bitLength_; ++i) {
    if (getBit(i)) {
      size_t b = i / 8;
      size_t o = i % 8;
      // Set the bit in the destination byte at the appropriate offset.
      // (1 << (7 - o)) puts the '1' in the correct position for Big Endian.
      vec[b] |= (1 << (7 - o));
    }
  }
  return vec;
}

std::shared_ptr<BitBuffer> BitBufferSlice::getParent() const { 
  // Return the shared pointer to the underlying parent buffer.
  return buffer_; 
}

size_t BitBufferSlice::getOffset() const { 
  // Return the starting bit offset relative to the parent.
  return startBit_; 
}

} // namespace quasar::coretypes
