#include "quasar/coretypes/BitBuffer.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace quasar {
namespace coretypes {

BitBuffer::BitBuffer(size_t bitCount)
    : Buffer((bitCount + 7) / 8), bitSize_(bitCount) {
  // Initialized with sufficient bytes
}

BitBuffer::BitBuffer(const BitBuffer &other) : Buffer(other) {
  std::lock_guard<std::mutex> lock(other.mutex_);
  bitSize_ = other.bitSize_;
  if (bitSize_ == 0)
    bitSize_ = other.size() * 8;
}

BitBuffer &BitBuffer::operator=(const BitBuffer &other) {
  if (this != &other) {
    Buffer::operator=(other);
    std::lock_guard<std::mutex> lock(other.mutex_);
    bitSize_ = other.bitSize_;
    if (bitSize_ == 0)
      bitSize_ = other.size() * 8;
  }
  return *this;
}

size_t BitBuffer::bitSize() const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (bitSize_ == 0 && !data_.empty())
    return data_.size() * 8;
  return bitSize_;
}

bool BitBuffer::getBit(size_t bitIndex) const {
  std::lock_guard<std::mutex> lock(mutex_);
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;
  if (bitIndex >= actualSize) {
    throw std::out_of_range("Bit index out of range");
  }

  size_t byteIndex = bitIndex / 8;
  size_t bitOffset = bitIndex % 8;
  // Big Endian bit numbering: bit 0 is MSB (0x80)
  return (data_[byteIndex] >> (7 - bitOffset)) & 1;
}

void BitBuffer::setBit(size_t bitIndex, bool value) {
  std::lock_guard<std::mutex> lock(mutex_);
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;
  if (bitIndex >= actualSize) {
    throw std::out_of_range("Bit index out of range");
  }

  size_t byteIndex = bitIndex / 8;
  size_t bitOffset = bitIndex % 8;

  if (value) {
    data_[byteIndex] |= (1 << (7 - bitOffset));
  } else {
    data_[byteIndex] &= ~(1 << (7 - bitOffset));
  }
}

BitBuffer BitBuffer::sliceBits(size_t startBit, size_t bitLength) const {
  // Cannot hold lock while calling other methods if they lock?
  // We access data_ directly if in same class.
  std::lock_guard<std::mutex> lock(mutex_);
  size_t actualSize = (bitSize_ == 0) ? data_.size() * 8 : bitSize_;

  if (startBit + bitLength > actualSize) {
    throw std::out_of_range("Slice out of range");
  }

  BitBuffer result(bitLength);
  // Naive bit copy for simplicity and correctness (perf optimization later if
  // needed) Direct data access to result? result is local, no contention.

  // We can't access result.data_ directly because it is protected in Buffer?
  // Buffer has protected data_. BitBuffer inherits it. Yes.

  for (size_t i = 0; i < bitLength; ++i) {
    size_t srcIndex = startBit + i;
    size_t byteIndex = srcIndex / 8;
    size_t bitOffset = srcIndex % 8;
    bool bit = (data_[byteIndex] >> (7 - bitOffset)) & 1;

    // Set in result
    size_t dstByte = i / 8;
    size_t dstBit = i % 8;
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

  // Copy my bits
  // We can iterate via getBit (slow but safe referencing).
  // Or check implementation.
  // Let's use getBit to avoid public interface lock recursion if possible?
  // No, getBit locks.
  // We need internal helpers or just loop.
  // But we are in the class, we can access other.data_ but need to lock it.

  std::unique_lock<std::mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::mutex> lock2(other.mutex_, std::defer_lock);
  std::lock(lock1, lock2);

  // Copy this
  for (size_t i = 0; i < (bitSize_ ? bitSize_ : data_.size() * 8); ++i) {
    size_t byteIndex = i / 8;
    size_t bitOffset = i % 8;
    bool bit = (data_[byteIndex] >> (7 - bitOffset)) & 1;

    if (bit)
      result.data_[i / 8] |= (1 << (7 - (i % 8)));
  }

  // Copy other
  size_t offset = (bitSize_ ? bitSize_ : data_.size() * 8);
  // Check other size
  size_t oSize = (other.bitSize_ ? other.bitSize_ : other.data_.size() * 8);
  for (size_t i = 0; i < oSize; ++i) {
    size_t byteIndex = i / 8;
    size_t bitOffset = i % 8;
    bool bit = (other.data_[byteIndex] >> (7 - bitOffset)) & 1;

    size_t target = offset + i;
    if (bit)
      result.data_[target / 8] |= (1 << (7 - (target % 8)));
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

  // Compare bits
  // Can optimize via byte compare if aligned?
  // Fallback to bit compare
  std::unique_lock<std::mutex> lock1(mutex_, std::defer_lock);
  std::unique_lock<std::mutex> lock2(other.mutex_, std::defer_lock);
  std::lock(lock1, lock2);

  // Recalculate size under lock to be sure
  size_t effectiveSize = (bitSize_ ? bitSize_ : data_.size() * 8);

  for (size_t i = 0; i < effectiveSize; ++i) {
    size_t b = i / 8;
    size_t o = i % 8;
    bool v1 = (data_[b] >> (7 - o)) & 1;
    bool v2 = (other.data_[b] >> (7 - o)) & 1;
    if (v1 != v2)
      return false;
  }
  return true;
}

void BitBuffer::reverseBits() {
  std::lock_guard<std::mutex> lock(mutex_);
  size_t size = (bitSize_ ? bitSize_ : data_.size() * 8);
  for (size_t i = 0; i < size / 2; ++i) {
    size_t j = size - 1 - i;

    size_t b1 = i / 8;
    size_t o1 = i % 8;
    size_t b2 = j / 8;
    size_t o2 = j % 8;

    bool v1 = (data_[b1] >> (7 - o1)) & 1;
    bool v2 = (data_[b2] >> (7 - o2)) & 1;

    if (v1 != v2) {
      // Swap
      if (v2)
        data_[b1] |= (1 << (7 - o1));
      else
        data_[b1] &= ~(1 << (7 - o1));

      if (v1)
        data_[b2] |= (1 << (7 - o2));
      else
        data_[b2] &= ~(1 << (7 - o2));
    }
  }
}

void BitBuffer::reverseBits(size_t groupSize) {
  if (groupSize == 0)
    return;
  std::lock_guard<std::mutex> lock(mutex_);
  size_t size = (bitSize_ ? bitSize_ : data_.size() * 8);

  if (size % groupSize != 0)
    throw std::invalid_argument("Size not multiple of group size");

  size_t groups = size / groupSize;
  // Swap group i with groups-1-i
  // Temporary storage for swap?
  // In-place bit swap is hard.
  // Easier to make a copy or temp buffer for the group.

  // We can just iterate bits inside the group exchange.
  for (size_t i = 0; i < groups / 2; ++i) {
    size_t startA = i * groupSize;
    size_t startB = (groups - 1 - i) * groupSize;

    // Swap bit k of A with bit k of B
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
        if (v2)
          data_[b1] |= (1 << (7 - o1));
        else
          data_[b1] &= ~(1 << (7 - o1));

        if (v1)
          data_[b2] |= (1 << (7 - o2));
        else
          data_[b2] &= ~(1 << (7 - o2));
      }
    }
  }
}

} // namespace coretypes
} // namespace quasar
