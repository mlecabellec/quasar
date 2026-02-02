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
  // The bytes are zero-initialized by the Buffer constructor.
}
BitBuffer::BitBuffer(const BitBuffer &other) : Buffer(other) {
  std::lock_guard<std::recursive_timed_mutex> lock(other.mutex_);
  bitSize_ = other.bitSize_;
  if (bitSize_ == 0) {
    // If bitSize_ is 0, it implies the full buffer is used.
    bitSize_ = other.size() * 8;
  }
}

BitBuffer &BitBuffer::operator=(const BitBuffer &other) {
  if (this != &other) {
    // Call the base class assignment operator to copy the buffer data.
    Buffer::operator=(other);

    // Lock the other buffer to safely access its bitSize.
    std::lock_guard<std::recursive_timed_mutex> lock(other.mutex_);
    bitSize_ = other.bitSize_;

    // If the other buffer has bitSize_ 0, calculate it from size.
    if (bitSize_ == 0) {
      bitSize_ = other.size() * 8;
    }
  }
  return *this;
}

size_t BitBuffer::bitSize() const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  if (bitSize_ == 0 && !data_.empty())
    return data_.size() * 8;
  return bitSize_;
}

bool BitBuffer::getBit(size_t bitIndex) const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;
  if (bitIndex >= actualSize) {
    throw std::out_of_range("Bit index out of range");
  }

  size_t byteIndex = bitIndex / 8;
  size_t bitOffset = bitIndex % 8;

  // Big Endian bit numbering: bit 0 is MSB (0x80)
  // Shift right to move the target bit to position 0 and mask it.
  return (data_[byteIndex] >> (7 - bitOffset)) & 1;
}

void BitBuffer::setBit(size_t bitIndex, bool value) {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;
  if (bitIndex >= actualSize) {
    throw std::out_of_range("Bit index out of range");
  }

  size_t byteIndex = bitIndex / 8;
  size_t bitOffset = bitIndex % 8;

  if (value) {
    // Set the bit by ORing with a mask that has a 1 at the target position.
    data_[byteIndex] |= (1 << (7 - bitOffset));
  } else {
    // Clear the bit by ANDing with a mask that has a 0 at the target
    // position.
    data_[byteIndex] &= ~(1 << (7 - bitOffset));
  }
}

BitBuffer BitBuffer::sliceBits(size_t startBit, size_t bitLength) const {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;

  if (startBit + bitLength > actualSize) {
    // The requested slice exceeds the valid bits in the buffer.
    throw std::out_of_range("Slice out of range");
  }

  // Create a new BitBuffer to hold the result.
  BitBuffer result(bitLength);

  for (size_t i = 0; i < bitLength; ++i) {
    // Calculate the source bit location.
    size_t srcIndex = startBit + i;
    size_t byteIndex = srcIndex / 8;
    size_t bitOffset = srcIndex % 8;

    // Retrieve the bit from the source data.
    bool bit = (data_[byteIndex] >> (7 - bitOffset)) & 1;

    // Calculate the destination bit location.
    size_t dstByte = i / 8;
    size_t dstBit = i % 8;

    // Set the bit in the result buffer.
    if (bit) {
      result.data_[dstByte] |= (1 << (7 - dstBit));
    } else {
      result.data_[dstByte] &= ~(1 << (7 - dstBit));
    }
  }
  return result;
}

BitBuffer BitBuffer::concatBits(const BitBuffer &other) const {
  // Need effective size
  size_t mySize = this->bitSize();    // locks
  size_t otherSize = other.bitSize(); // locks

  BitBuffer result(mySize + otherSize);

  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  // Lock both mutexes safely to avoid deadlock.
  std::lock(lock1, lock2);

  // Copy bits from the current buffer to the result.
  for (size_t i = 0; i < (bitSize_ ? bitSize_ : data_.size() * 8); ++i) {
    size_t byteIndex = i / 8;
    size_t bitOffset = i % 8;
    bool bit = (data_[byteIndex] >> (7 - bitOffset)) & 1;

    // Set the bit in the result buffer.
    if (bit) {
      result.data_[i / 8] |= (1 << (7 - (i % 8)));
    }
  }

  // Calculate the offset for the second buffer's bits.
  size_t offset = (bitSize_ ? bitSize_ : data_.size() * 8);
  // Determine the size of the other buffer.
  size_t oSize = (other.bitSize_ ? other.bitSize_ : other.data_.size() * 8);

  // Copy bits from the other buffer to the result.
  for (size_t i = 0; i < oSize; ++i) {
    size_t byteIndex = i / 8;
    size_t bitOffset = i % 8;
    bool bit = (other.data_[byteIndex] >> (7 - bitOffset)) & 1;

    // Calculate the target position in the result buffer.
    size_t target = offset + i;
    if (bit) {
      result.data_[target / 8] |= (1 << (7 - (target % 8)));
    }
  }

  return result;
}

bool BitBuffer::equals(const BitBuffer &other) const {
  if (this == &other)
    return true;
  size_t s1 = this->bitSize(); // locks temporarily
  size_t s2 = other.bitSize();
  if (s1 != s2)
    return false;

  std::unique_lock<std::recursive_timed_mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::recursive_timed_mutex> lock2(other.mutex_,
                                                     std::defer_lock);
  std::lock(lock1, lock2);

  // Recalculate size under lock to be sure of the current state.
  size_t effectiveSize = (bitSize_ ? bitSize_ : data_.size() * 8);

  // Compare bit by bit.
  for (size_t i = 0; i < effectiveSize; ++i) {
    size_t b = i / 8;
    size_t o = i % 8;

    // Extract bits from both buffers.
    bool v1 = (data_[b] >> (7 - o)) & 1;
    bool v2 = (other.data_[b] >> (7 - o)) & 1;

    // Return false if any bit mismatch is found.
    if (v1 != v2) {
      return false;
    }
  }
  return true;
}

void BitBuffer::reverseBits() {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  size_t size = (bitSize_ ? bitSize_ : data_.size() * 8);
  // Iterate through the first half of the bits to swap with the second half.
  for (size_t i = 0; i < size / 2; ++i) {
    size_t j = size - 1 - i;

    // Calculate byte index and bit offset for the first bit.
    size_t b1 = i / 8;
    size_t o1 = i % 8;
    // Calculate byte index and bit offset for the second bit.
    size_t b2 = j / 8;
    size_t o2 = j % 8;

    // Extract the bit values.
    bool v1 = (data_[b1] >> (7 - o1)) & 1;
    bool v2 = (data_[b2] >> (7 - o2)) & 1;

    // Swap the bits if they are different.
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
  if (groupSize == 0)
    return;
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  size_t size = (bitSize_ ? bitSize_ : data_.size() * 8);

  if (size % groupSize != 0)
    throw std::invalid_argument("Size not multiple of group size");

  size_t groups = size / groupSize;
  // Iterate through half of the groups to swap them.
  for (size_t i = 0; i < groups / 2; ++i) {
    size_t startA = i * groupSize;
    size_t startB = (groups - 1 - i) * groupSize;

    // Swap bits within the corresponding groups.
    for (size_t k = 0; k < groupSize; ++k) {
      size_t idxA = startA + k;
      size_t idxB = startB + k;

      size_t b1 = idxA / 8;
      size_t o1 = idxA % 8;
      size_t b2 = idxB / 8;
      size_t o2 = idxB % 8;

      bool v1 = (data_[b1] >> (7 - o1)) & 1;
      bool v2 = (data_[b2] >> (7 - o2)) & 1;

      // Swap the bits if they are different.
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
  return BitBuffer(*this);
}

std::shared_ptr<BitBufferSlice> BitBuffer::sliceBitsView(size_t startBit,
                                                         size_t bitLength) {
  std::lock_guard<std::recursive_timed_mutex> lock(mutex_);
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;
  if (startBit + bitLength > actualSize) {
    throw std::out_of_range("Slice out of range");
  }
  return std::make_shared<BitBufferSlice>(
      std::static_pointer_cast<BitBuffer>(shared_from_this()), startBit,
      bitLength);
}

} // namespace coretypes
} // namespace quasar
