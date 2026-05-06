#include "quasar/coretypes/BitBuffer.hpp"
#include "quasar/coretypes/BitBufferSlice.hpp"
#include "utils/Constants.hpp"
#include <algorithm>

#include <chrono>
#include <cmath>
#include <cstring>
#include <mutex>
#include <stdexcept>

namespace quasar {
namespace coretypes {

BitBuffer::BitBuffer(size_t bitCount)
    : Buffer((bitCount + 7) / 8), bitSize_(bitCount) {
  // Fulfills [FE-0010.4.8] associated memory allocation.
  // Initialize the buffer with the calculated number of bytes.
  // (bitCount + 7) / 8 is a common idiom for ceiling division to get the number
  // of bytes. The bytes are zero-initialized by the Buffer constructor.
}
BitBuffer::BitBuffer(const BitBuffer &other) : Buffer() {
  // Fulfills [FE-0030.8] usage of recursive_timed_mutex for thread safety.
  // Lock the source buffer to ensure a consistent snapshot of both data and size.
  std::unique_lock<std::recursive_timed_mutex> lock(other.mutex_,
                                                     std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring lock on source bit buffer");
  }
  data_ = other.data_;
  bitSize_ = other.bitSize_;
  if (bitSize_ == 0) {
    // If bitSize_ is 0, it implies the full buffer is used.
    bitSize_ = data_.size() * 8;
  }
}

BitBuffer &BitBuffer::operator=(const BitBuffer &other) {
  if (this != &other) {
    // Fulfills [FE-0030.8] thread safety during assignment.
    // Avoid deadlock by using address-based ordering.
    std::recursive_timed_mutex *first = &mutex_ < &other.mutex_ ? &mutex_ : &other.mutex_;
    std::recursive_timed_mutex *second = &mutex_ < &other.mutex_ ? &other.mutex_ : &mutex_;

    std::unique_lock<std::recursive_timed_mutex> lock1(*first, std::chrono::seconds(1));
    if (!lock1.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock (1)");
    std::unique_lock<std::recursive_timed_mutex> lock2(*second, std::chrono::seconds(1));
    if (!lock2.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock (2)");

    // Copy both data and bitSize atomically.
    data_ = other.data_;
    bitSize_ = other.bitSize_;

    // If the other buffer has bitSize_ 0, calculate it from its byte size.
    if (bitSize_ == 0) {
      bitSize_ = data_.size() * 8;
    }
  }
  return *this;
}

size_t BitBuffer::bitSize() const {
  // Fulfills [FE-0030.8] thread safe access.
  std::unique_lock<std::recursive_timed_mutex> lock(mutex_, std::chrono::seconds(1));
  if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock");
  // If bitSize_ is not explicitly set, we default to the full bit capacity of
  // the underlying byte array.
  if (bitSize_ == 0 && !data_.empty())
    return data_.size() * 8;
  return bitSize_;
}

bool BitBuffer::getBit(size_t bitIndex) const {
  // Fulfills [FE-0010.4.2] operations at bit level.
  std::unique_lock<std::recursive_timed_mutex> lock(mutex_, std::chrono::seconds(1));
  if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock");
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
  // Fulfills [FE-0010.4.2] operations at bit level.
  std::unique_lock<std::recursive_timed_mutex> lock(mutex_, std::chrono::seconds(1));
  if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock");
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
  // Fulfills [FE-0010.4.3] slicing with bit granularity.
  // Fulfills [FE-0030.5.8] offset and lengths as combination of bytes and bits.
  std::unique_lock<std::recursive_timed_mutex> lock(mutex_, std::chrono::seconds(1));
  if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock");
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;

  // Range validation for the requested slice.
  if (startBit + bitLength > actualSize) {
    throw std::out_of_range("Slice out of range");
  }

  // A bit-level slice requires creating a new buffer and copying bits
  // individually, as the slice might not start on a byte boundary.
  BitBuffer result(bitLength);

  const size_t limit = 1000000;
  size_t count = 0;
  for (size_t i = 0; i < bitLength; ++i) {
    if (++count > limit) throw std::runtime_error("Loop limit exceeded in BitBuffer::sliceBits");
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
  // Fulfills [FE-0010.4.4] concatenation of buffers.
  // Determine final sizes before locking.
  size_t mySize = this->bitSize();
  size_t otherSize = other.bitSize();

  // Create a new BitBuffer large enough to hold the sum of bits.
  BitBuffer result(mySize + otherSize);

  const size_t limit = 1000000;
  size_t count = 0;

  if (this == &other) {
    std::unique_lock<std::recursive_timed_mutex> lock(mutex_, std::chrono::seconds(1));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock");
    
    size_t effectiveSize = (bitSize_ ? bitSize_ : data_.size() * 8);
    for (size_t i = 0; i < effectiveSize; ++i) {
      if (++count > limit) throw std::runtime_error("Loop limit exceeded in BitBuffer::concatBits (self)");
      bool bit = (data_[i / 8] >> (7 - (i % 8))) & 1;
      if (bit) {
        result.setBit(i, true);
        result.setBit(effectiveSize + i, true);
      }
    }
    return result;
  }

  // Lock both buffers simultaneously to prevent deadlocks.
  std::recursive_timed_mutex *first = &mutex_ < &other.mutex_ ? &mutex_ : &other.mutex_;
  std::recursive_timed_mutex *second = &mutex_ < &other.mutex_ ? &other.mutex_ : &mutex_;

  std::unique_lock<std::recursive_timed_mutex> lock1(*first, std::chrono::seconds(1));
  if (!lock1.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock (1)");
  std::unique_lock<std::recursive_timed_mutex> lock2(*second, std::chrono::seconds(1));
  if (!lock2.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock (2)");

  // Copy bits from 'this' buffer.
  size_t effectiveMySize = (bitSize_ ? bitSize_ : data_.size() * 8);
  for (size_t i = 0; i < effectiveMySize; ++i) {
    if (++count > limit) throw std::runtime_error("Loop limit exceeded in BitBuffer::concatBits (this)");
    if ((data_[i / 8] >> (7 - (i % 8))) & 1) {
      result.data_[i / 8] |= (1 << (7 - (i % 8)));
    }
  }

  // Copy bits from the 'other' buffer.
  size_t oSize = (other.bitSize_ ? other.bitSize_ : other.data_.size() * 8);
  for (size_t i = 0; i < oSize; ++i) {
    if (++count > limit) throw std::runtime_error("Loop limit exceeded in BitBuffer::concatBits (other)");
    if ((other.data_[i / 8] >> (7 - (i % 8))) & 1) {
      size_t target = effectiveMySize + i;
      result.data_[target / 8] |= (1 << (7 - (target % 8)));
    }
  }

  return result;
}

bool BitBuffer::equals(const BitBuffer &other) const {
  // Fulfills [FE-0010.4.5] comparison.
  // Reference equality check.
  if (this == &other)
    return true;

  // Initial size check.
  size_t s1 = this->bitSize();
  size_t s2 = other.bitSize();
  if (s1 != s2)
    return false;

  // Lock both buffers for content comparison.
  std::recursive_timed_mutex *first = &mutex_ < &other.mutex_ ? &mutex_ : &other.mutex_;
  std::recursive_timed_mutex *second = &mutex_ < &other.mutex_ ? &other.mutex_ : &mutex_;

  std::unique_lock<std::recursive_timed_mutex> lock1(*first, std::chrono::seconds(1));
  if (!lock1.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock (1)");
  std::unique_lock<std::recursive_timed_mutex> lock2(*second, std::chrono::seconds(1));
  if (!lock2.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock (2)");

  size_t effectiveSize = (bitSize_ ? bitSize_ : data_.size() * 8);
  const size_t limit = 1000000;
  size_t count = 0;

  for (size_t i = 0; i < effectiveSize; ++i) {
    if (++count > limit) throw std::runtime_error("Loop limit exceeded in BitBuffer::equals");
    bool v1 = (data_[i / 8] >> (7 - (i % 8))) & 1;
    bool v2 = (other.data_[i / 8] >> (7 - (i % 8))) & 1;
    if (v1 != v2) return false;
  }
  return true;
}

void BitBuffer::reverseBits() {
  // Fulfills [FE-0010.4.7] reversing at bit level.
  std::unique_lock<std::recursive_timed_mutex> lock(mutex_, std::chrono::seconds(1));
  if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock");
  size_t size = (bitSize_ ? bitSize_ : data_.size() * 8);
  // Swap bits from outer edges moving towards the center.
  // Efficiency: we only need to iterate half-way.
  const size_t limit = 1000000;
  size_t count = 0;
  for (size_t i = 0; i < size / 2; ++i) {
    if (++count > limit) throw std::runtime_error("Loop limit exceeded in BitBuffer::reverseBits");
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
  // Fulfills [FE-0010.4.7] reversing at bit level.
  // If groupSize is 0, no operation is defined.
  if (groupSize == 0)
    return;
  std::unique_lock<std::recursive_timed_mutex> lock(mutex_, std::chrono::seconds(1));
  if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock");
  size_t size = (bitSize_ ? bitSize_ : data_.size() * 8);

  // Reversing groups requires the total size to be divisible by groupSize.
  if (size % groupSize != 0)
    throw std::invalid_argument("Size not multiple of group size");

  size_t groups = size / groupSize;
  const size_t limitOuter = utils::BIT_BUFFER_MAX_SAFE_SIZE;
  size_t countOuter = 0;
  // Iterate through pairs of groups and swap their entire bit contents.
  for (size_t i = 0; i < groups / 2; ++i) {
    if (++countOuter > limitOuter) throw std::runtime_error("Outer loop limit exceeded in BitBuffer::reverseBits(groupSize)");
    size_t startA = i * groupSize;
    size_t startB = (groups - 1 - i) * groupSize;

    // Inner loop swaps individual bits within the two chosen groups.
    const size_t limitInner = utils::BIT_BUFFER_MAX_SAFE_SIZE;

    size_t countInner = 0;
    for (size_t k = 0; k < groupSize; ++k) {
      if (++countInner > limitInner) throw std::runtime_error("Inner loop limit exceeded in BitBuffer::reverseBits(groupSize)");
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
  // Fulfills [FE-0010.4.8] cloning with memory allocation.
  // Delegation to copy constructor which handles deep copy and thread safety.
  return BitBuffer(*this);
}

std::shared_ptr<BitBufferSlice> BitBuffer::sliceBitsView(size_t startBit,
                                                         size_t bitLength) {
  // Fulfills [FE-0030.5.7] bit-level view creation.
  std::unique_lock<std::recursive_timed_mutex> lock(mutex_, std::chrono::seconds(1));
  if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring bit buffer lock");
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
