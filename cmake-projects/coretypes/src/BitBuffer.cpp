#include "quasar/coretypes/BitBuffer.hpp"
#include "quasar/coretypes/BitBufferSlice.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <mutex>
#include <stdexcept>

namespace quasar {
namespace coretypes {

BitBuffer::BitBuffer(size_t bitCount)
    : Buffer((bitCount + 7) / 8), bitSize_(bitCount) {
  // Initialize the buffer with the calculated number of bytes.
  // (bitCount + 7) / 8 is a common idiom for ceiling division to get the number
  // of bytes. The bytes are zero-initialized by the Buffer constructor.
}
BitBuffer::BitBuffer(const BitBuffer &other) : Buffer(other) {
  // Lock the source buffer to ensure a consistent snapshot during copy.
  std::lock_guard<std::recursive_timed_mutex> lock(other.mutex_);
  bitSize_ = other.bitSize_;
  if (bitSize_ == 0) {
    // If bitSize_ is 0, it implies the full buffer is used (wrapping an
    // existing Buffer). We explicitly calculate the bit capacity.
    bitSize_ = other.size() * 8;
  }
}

BitBuffer &BitBuffer::operator=(const BitBuffer &other) {
  if (this != &other) {
    // Call the base class assignment operator to copy the buffer data.
    // Buffer::operator= handles the data copy and its own locking.
    Buffer::operator=(other);

    // Lock the other buffer to safely access its bitSize.
    std::lock_guard<std::recursive_timed_mutex> lock(other.mutex_);
    bitSize_ = other.bitSize_;

    // If the other buffer has bitSize_ 0, calculate it from its byte size.
    if (bitSize_ == 0) {
      bitSize_ = other.size() * 8;
    }
  }
  return *this;
}

size_t BitBuffer::bitSize() const {
  // Thread-safe access to bitSize_.
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // If bitSize_ is not explicitly set, we default to the full bit capacity of
  // the underlying byte array.
  if (bitSize_ == 0 && !data_.empty())
    return data_.size() * 8;
  return bitSize_;
}

bool BitBuffer::getBit(size_t bitIndex) const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // Use bitSize_ if set, otherwise fallback to the full byte buffer's bit
  // capacity.
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;
  if (bitIndex >= actualSize) {
    throw std::out_of_range("Bit index out of range");
  }

  // Calculate which byte contains the bit and its position within that byte.
  size_t byteIndex = bitIndex / 8;
  size_t bitOffset = bitIndex % 8;

  // We use Big Endian bit numbering: bit 0 is the Most Significant Bit (0x80).
  // Shift right to move the target bit to the least significant position (bit
  // 0) and mask it. e.g., for bitOffset 0, we shift by 7.
  return (data_[byteIndex] >> (7 - bitOffset)) & 1;
}

void BitBuffer::setBit(size_t bitIndex, bool value) {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  // Validate index against actual bit capacity.
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;
  if (bitIndex >= actualSize) {
    throw std::out_of_range("Bit index out of range");
  }

  // Identify target byte and target bit position (0-7).
  size_t byteIndex = bitIndex / 8;
  size_t bitOffset = bitIndex % 8;

  if (value) {
    // Set the bit: OR with a mask that has a '1' at the target position.
    // 1 << (7 - bitOffset) puts the '1' in the correct Big Endian position.
    data_[byteIndex] |= (1 << (7 - bitOffset));
  } else {
    // Clear the bit: AND with a mask that has a '0' at the target position
    // (using bitwise NOT on the mask).
    data_[byteIndex] &= ~(1 << (7 - bitOffset));
  }
}

BitBuffer BitBuffer::sliceBits(size_t startBit, size_t bitLength) const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;

  // Range validation for the requested slice.
  if (startBit + bitLength > actualSize) {
    throw std::out_of_range("Slice out of range");
  }

  // A bit-level slice requires creating a new buffer and copying bits
  // individually, as the slice might not start on a byte boundary.
  BitBuffer result(bitLength);

  for (size_t i = 0; i < bitLength; ++i) {
    size_t srcIndex = startBit + i;
    size_t byteIndex = srcIndex / 8;
    size_t bitOffset = srcIndex % 8;

    // Extract the bit from the source byte using Big Endian numbering.
    bool bit = (data_[byteIndex] >> (7 - bitOffset)) & 1;

    size_t dstByte = i / 8;
    size_t dstBit = i % 8;

    // Place the bit into the destination byte at the correct offset.
    if (bit) {
      result.data_[dstByte] |= (1 << (7 - dstBit));
    } else {
      result.data_[dstByte] &= ~(1 << (7 - dstBit));
    }
  }
  return result;
}

BitBuffer BitBuffer::concatBits(const BitBuffer &other) const {
  // Determine final sizes before locking to minimize critical section.
  size_t mySize = this->bitSize();
  size_t otherSize = other.bitSize();

  // Create a new BitBuffer large enough to hold the sum of bits.
  BitBuffer result(mySize + otherSize);

  // Lock both buffers simultaneously to prevent deadlocks.
  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  // Copy bits from 'this' buffer into the beginning of the result.
  size_t effectiveMySize = (bitSize_ ? bitSize_ : data_.size() * 8);
  for (size_t i = 0; i < effectiveMySize; ++i) {
    size_t byteIndex = i / 8;
    size_t bitOffset = i % 8;
    bool bit = (data_[byteIndex] >> (7 - bitOffset)) & 1;

    if (bit) {
      result.data_[i / 8] |= (1 << (7 - (i % 8)));
    }
  }

  // Copy bits from the 'other' buffer, starting at the offset after 'this'
  // bits.
  size_t offset = effectiveMySize;
  size_t oSize = (other.bitSize_ ? other.bitSize_ : other.data_.size() * 8);

  for (size_t i = 0; i < oSize; ++i) {
    size_t byteIndex = i / 8;
    size_t bitOffset = i % 8;
    bool bit = (other.data_[byteIndex] >> (7 - bitOffset)) & 1;

    if (bit) {
      size_t target = offset + i;
      result.data_[target / 8] |= (1 << (7 - (target % 8)));
    }
  }

  return result;
}

bool BitBuffer::equals(const BitBuffer &other) const {
  // Reference equality check.
  if (this == &other)
    return true;

  // Initial size check (potentially locking via bitSize() call).
  size_t s1 = this->bitSize();
  size_t s2 = other.bitSize();
  if (s1 != s2)
    return false;

  // Lock both buffers for content comparison.
  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  size_t effectiveSize = (bitSize_ ? bitSize_ : data_.size() * 8);

  // Bit-by-bit comparison ensures that even if bitSize_ is not a multiple of 8,
  // we only compare valid bits and ignore any padding in the last byte.
  for (size_t i = 0; i < effectiveSize; ++i) {
    size_t b = i / 8;
    size_t o = i % 8;

    bool v1 = (data_[b] >> (7 - o)) & 1;
    bool v2 = (other.data_[b] >> (7 - o)) & 1;

    if (v1 != v2) {
      return false;
    }
  }
  return true;
}

void BitBuffer::reverseBits() {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  size_t size = (bitSize_ ? bitSize_ : data_.size() * 8);
  // Swap bits from outer edges moving towards the center.
  // Efficiency: we only need to iterate half-way.
  for (size_t i = 0; i < size / 2; ++i) {
    size_t j = size - 1 - i;

    size_t b1 = i / 8;
    size_t o1 = i % 8;
    size_t b2 = j / 8;
    size_t o2 = j % 8;

    bool v1 = (data_[b1] >> (7 - o1)) & 1;
    bool v2 = (data_[b2] >> (7 - o2)) & 1;

    // If bits are different, they need to be flipped in their new positions.
    // If they are the same, swapping doesn't change anything.
    if (v1 != v2) {
      if (v2) {
        data_[b1] |= (1 << (7 - o1));
      } else {
        data_[b1] &= ~(1 << (7 - o1));
      }

      if (v1) {
        data_[b2] |= (1 << (7 - o2));
      } else {
        data_[b2] &= ~(1 << (7 - o2));
      }
    }
  }
}

void BitBuffer::reverseBits(size_t groupSize) {
  // If groupSize is 0, no operation is defined.
  if (groupSize == 0)
    return;
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  size_t size = (bitSize_ ? bitSize_ : data_.size() * 8);

  // Reversing groups requires the total size to be divisible by groupSize.
  if (size % groupSize != 0)
    throw std::invalid_argument("Size not multiple of group size");

  size_t groups = size / groupSize;
  // Iterate through pairs of groups and swap their entire bit contents.
  for (size_t i = 0; i < groups / 2; ++i) {
    size_t startA = i * groupSize;
    size_t startB = (groups - 1 - i) * groupSize;

    // Inner loop swaps individual bits within the two chosen groups.
    for (size_t k = 0; k < groupSize; ++k) {
      size_t idxA = startA + k;
      size_t idxB = startB + k;

      size_t b1 = idxA / 8;
      size_t o1 = idxA % 8;
      size_t b2 = idxB / 8;
      size_t o2 = idxB % 8;

      bool v1 = (data_[b1] >> (7 - o1)) & 1;
      bool v2 = (data_[b2] >> (7 - o2)) & 1;

      if (v1 != v2) {
        if (v2) {
          data_[b1] |= (1 << (7 - o1));
        } else {
          data_[b1] &= ~(1 << (7 - o1));
        }

        if (v1) {
          data_[b2] |= (1 << (7 - o2));
        } else {
          data_[b2] &= ~(1 << (7 - o2));
        }
      }
    }
  }
}

BitBuffer BitBuffer::clone() const {
  // Delegation to copy constructor which handles deep copy and thread safety.
  return BitBuffer(*this);
}

std::shared_ptr<BitBufferSlice> BitBuffer::sliceBitsView(size_t startBit,
                                                         size_t bitLength) {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;
  // Bounds check for the view.
  if (startBit + bitLength > actualSize) {
    throw std::out_of_range("Slice out of range");
  }
  // Create the slice object. shared_from_this() ensures the parent remains
  // alive.
  return std::make_shared<BitBufferSlice>(
      std::static_pointer_cast<BitBuffer>(shared_from_this()), startBit,
      bitLength);
}

} // namespace coretypes
} // namespace quasar
