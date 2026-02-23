#include "quasar/named/NamedBufferSlice.hpp"

namespace quasar::named {

NamedBufferSlice::NamedBufferSlice(
    const std::string &name, std::shared_ptr<quasar::coretypes::Buffer> buffer,
    size_t start, size_t length)
    : NamedObject(name), quasar::coretypes::BufferSlice(buffer, start, length) {
    // Fulfills [FE-0030.7] NamedBufferSlice view of a Buffer.
    // Constructor initializes both the named identity and the slice parameters.
}

std::shared_ptr<NamedBufferSlice> NamedBufferSlice::create(
    const std::string &name, std::shared_ptr<quasar::coretypes::Buffer> buffer,
    size_t start, size_t length, std::shared_ptr<NamedObject> parent) {
  // Fulfills [FE-0030.7.5] Slices can be created from a Buffer.
  // Create the slice instance.
  std::shared_ptr<NamedBufferSlice> obj =
      std::make_shared<NamedBufferSlice>(name, buffer, start, length);
  
  // Register the object for shared-from-this functionality via setSelf.
  obj->setSelf(obj);
  
  // Attach to the object tree if a parent is provided.
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

std::shared_ptr<NamedObject> NamedBufferSlice::clone() const {
  // Fulfills [FE-0030.7.2] A slice shall be able to be copied.
  // A clone of a slice creates a new slice object that references the same segment
  // of the original buffer, but exists independently in terms of hierarchy.
  return NamedBufferSlice::create(
      getName(), quasar::coretypes::BufferSlice::getParent(), getOffset(),
      size());
}

std::shared_ptr<NamedBufferSlice>
NamedBufferSlice::sliceView(size_t start, size_t length) const {
  // Fulfills [FE-0030.7.6] Slices can be created from a slice.
  // Creates a nested slice (sub-slice). The new slice's offset is relative to
  // the current slice's offset. The underlying buffer remains the same.
  return NamedBufferSlice::create(
      getName() + "_slice", quasar::coretypes::BufferSlice::getParent(),
      getOffset() + start, length);
}

} // namespace quasar::named
