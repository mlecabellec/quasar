#include "quasar/named/NamedBufferSlice.hpp"

namespace quasar::named {

NamedBufferSlice::NamedBufferSlice(
    const std::string &name, std::shared_ptr<quasar::coretypes::Buffer> buffer,
    size_t start, size_t length)
    : NamedObject(name), quasar::coretypes::BufferSlice(buffer, start, length) {
}

std::shared_ptr<NamedBufferSlice> NamedBufferSlice::create(
    const std::string &name, std::shared_ptr<quasar::coretypes::Buffer> buffer,
    size_t start, size_t length, std::shared_ptr<NamedObject> parent) {
  std::shared_ptr<NamedBufferSlice> obj =
      std::make_shared<NamedBufferSlice>(name, buffer, start, length);
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

std::shared_ptr<NamedObject> NamedBufferSlice::clone() const {
  // A clone of a slice is a new slice pointing to the same buffer data
  return NamedBufferSlice::create(
      getName(), quasar::coretypes::BufferSlice::getParent(), getOffset(),
      size());
}

std::shared_ptr<NamedBufferSlice>
NamedBufferSlice::sliceView(size_t start, size_t length) const {
  // sliceView returns a NEW NamedBufferSlice that is a sub-slice of this one
  // The new slice will point to the SAME underlying buffer, offset by 'start'.
  // We need to validate bounds here or let BufferSlice handle it?
  // coretypes::BufferSlice constructor or slice method usually handles it.
  // But we need to create a NamedBufferSlice.

  // Adjusted start is relative to the current slice's start.
  return NamedBufferSlice::create(
      getName() + "_slice", quasar::coretypes::BufferSlice::getParent(),
      getOffset() + start, length);
}

} // namespace quasar::named
