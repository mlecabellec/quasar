#include "quasar/named/NamedBitBufferSlice.hpp"

namespace quasar::named {

NamedBitBufferSlice::NamedBitBufferSlice(
    const std::string &name,
    std::shared_ptr<quasar::coretypes::BitBuffer> buffer, size_t startBit,
    size_t bitLength)
    : NamedObject(name),
      quasar::coretypes::BitBufferSlice(buffer, startBit, bitLength) {}

std::shared_ptr<NamedBitBufferSlice> NamedBitBufferSlice::create(
    const std::string &name,
    std::shared_ptr<quasar::coretypes::BitBuffer> buffer, size_t startBit,
    size_t bitLength, std::shared_ptr<NamedObject> parent) {
  std::shared_ptr<NamedBitBufferSlice> obj =
      std::make_shared<NamedBitBufferSlice>(name, buffer, startBit, bitLength);
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

std::shared_ptr<NamedObject> NamedBitBufferSlice::clone() const {
  return NamedBitBufferSlice::create(
      getName(), quasar::coretypes::BitBufferSlice::getParent(), getOffset(),
      size());
}

std::shared_ptr<NamedBitBufferSlice>
NamedBitBufferSlice::sliceView(size_t startBit, size_t bitLength) const {
  return NamedBitBufferSlice::create(
      getName() + "_slice", quasar::coretypes::BitBufferSlice::getParent(),
      getOffset() + startBit, bitLength);
}

} // namespace quasar::named
