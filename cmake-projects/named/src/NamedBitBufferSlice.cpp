#include "quasar/named/NamedBitBufferSlice.hpp"
#include "quasar/named/NamedBitBuffer.hpp"

namespace quasar::named {

NamedBitBufferSlice::NamedBitBufferSlice(
    const std::string &name,
    std::shared_ptr<quasar::coretypes::BitBuffer> buffer, size_t startBit,
    size_t bitLength)
    : NamedObject(name),
      quasar::coretypes::BitBufferSlice(buffer, startBit, bitLength) {
  // Fulfills [FE-0030.7] NamedBitBufferSlice view of a BitBuffer.
  // Initializes the named slice with bit-level offsets and length.
}

std::shared_ptr<NamedBitBufferSlice> NamedBitBufferSlice::create(
    const std::string &name,
    std::shared_ptr<quasar::coretypes::BitBuffer> buffer, size_t startBit,
    size_t bitLength, std::shared_ptr<NamedObject> parent) {
  // Fulfills [FE-0030.7.5] Slices can be created from a BitBuffer.
  // Factory creation for the bit slice.
  std::shared_ptr<NamedBitBufferSlice> obj =
      std::make_shared<NamedBitBufferSlice>(name, buffer, startBit, bitLength);

  // Set internal weak-to-self pointer.
  obj->setSelf(obj);

  // Connect to the object tree if a parent exists.
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

std::shared_ptr<NamedBitBufferSlice> NamedBitBufferSlice::create(
    const std::string &name,
    std::shared_ptr<quasar::named::NamedBitBuffer> buffer, size_t startBit,
    size_t bitLength, std::shared_ptr<NamedObject> parent) {
  // Fulfills [TSK-20260301-001.9]
  return create(name,
                std::static_pointer_cast<quasar::coretypes::BitBuffer>(buffer),
                startBit, bitLength, parent);
}

std::shared_ptr<NamedBitBufferSlice> NamedBitBufferSlice::create(
    const std::string &name,
    std::shared_ptr<quasar::named::NamedBitBufferSlice> slice, size_t startBit,
    size_t bitLength, std::shared_ptr<NamedObject> parent) {
  // Fulfills [TSK-20260301-001.9]
  if (startBit + bitLength > slice->size()) {
    throw std::out_of_range("Slice bounds exceed parent bit slice capacity");
  }
  return create(name, slice->quasar::coretypes::BitBufferSlice::getParent(),
                slice->getOffset() + startBit, bitLength, parent);
}

std::shared_ptr<NamedObject> NamedBitBufferSlice::clone() const {
  // Fulfills [FE-0030.7.2] A slice shall be able to be copied.
  // Returns a new slice instance that looks at the same bit range.
  return NamedBitBufferSlice::create(
      getName(), quasar::coretypes::BitBufferSlice::getParent(), getOffset(),
      size());
}

std::shared_ptr<NamedBitBufferSlice>
NamedBitBufferSlice::sliceView(size_t startBit, size_t bitLength) const {
  // Fulfills [FE-0030.7.6] Slices can be created from a slice.
  // Returns a sub-slice of bits relative to the current slice's starting bit.
  if (startBit + bitLength > size()) {
    throw std::out_of_range("Slice bounds exceed parent bit slice capacity");
  }
  return NamedBitBufferSlice::create(
      getName() + "_slice", quasar::coretypes::BitBufferSlice::getParent(),
      getOffset() + startBit, bitLength);
}

std::shared_ptr<NamedObject>
NamedBitBufferSlice::deepCopy(std::shared_ptr<NamedObject> originalParent,
                              std::shared_ptr<NamedObject> newParent) const {

  std::shared_ptr<NamedBitBufferSlice> clonedSlice = nullptr;

  if (originalParent && newParent) {
    auto underlyingBuffer = quasar::coretypes::BitBufferSlice::getParent();

    if (auto nb = std::dynamic_pointer_cast<NamedBitBuffer>(originalParent)) {
      if (underlyingBuffer == nb) {
        if (auto newNb = std::dynamic_pointer_cast<NamedBitBuffer>(newParent)) {
          clonedSlice = NamedBitBufferSlice::create(getName(), newNb,
                                                    getOffset(), size());
        }
      }
    } else if (auto nbs = std::dynamic_pointer_cast<NamedBitBufferSlice>(
                   originalParent)) {
      if (underlyingBuffer ==
          nbs->quasar::coretypes::BitBufferSlice::getParent()) {
        if (auto newNbs =
                std::dynamic_pointer_cast<NamedBitBufferSlice>(newParent)) {
          if (getOffset() >= newNbs->getOffset()) {
            size_t relativeStart = getOffset() - newNbs->getOffset();
            clonedSlice = NamedBitBufferSlice::create(getName(), newNbs,
                                                      relativeStart, size());
          }
        }
      }
    }
  }

  if (!clonedSlice) {
    clonedSlice = std::dynamic_pointer_cast<NamedBitBufferSlice>(clone());
  }

  if (newParent) {
    clonedSlice->setParent(newParent);
  }

  auto childList = getChildren();
  for (const auto &child : childList) {
    child->deepCopy(getSelf(), clonedSlice);
  }

  return clonedSlice;
}

} // namespace quasar::named
