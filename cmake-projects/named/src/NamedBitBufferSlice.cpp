#include "quasar/named/NamedBitBufferSlice.hpp"

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
  return NamedBitBufferSlice::create(
      getName() + "_slice", quasar::coretypes::BitBufferSlice::getParent(),
      getOffset() + startBit, bitLength);
}

} // namespace quasar::named
