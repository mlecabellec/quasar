#include "quasar/named/NamedBufferSlice.hpp"
#include "quasar/named/NamedBuffer.hpp"

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

  // Connect to the object tree if a parent exists.
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

std::shared_ptr<NamedBufferSlice> NamedBufferSlice::create(
    const std::string &name, std::shared_ptr<quasar::named::NamedBuffer> buffer,
    size_t start, size_t length, std::shared_ptr<NamedObject> parent) {
  // Fulfills [TSK-20260301-001.6]
  return create(name,
                std::static_pointer_cast<quasar::coretypes::Buffer>(buffer),
                start, length, parent);
}

std::shared_ptr<NamedBufferSlice>
NamedBufferSlice::create(const std::string &name,
                         std::shared_ptr<quasar::named::NamedBufferSlice> slice,
                         size_t start, size_t length,
                         std::shared_ptr<NamedObject> parent) {
  // Fulfills [TSK-20260301-001.7]
  if (start + length > slice->size()) {
    throw std::out_of_range("Slice bounds exceed parent slice capacity");
  }
  return create(name, slice->quasar::coretypes::BufferSlice::getParent(),
                slice->getOffset() + start, length, parent);
}

std::shared_ptr<NamedObject> NamedBufferSlice::clone() const {
  // Fulfills [FE-0030.7.2] A slice shall be able to be copied.
  // A clone of a slice creates a new slice object that references the same
  // segment of the original buffer, but exists independently in terms of
  // hierarchy.
  return NamedBufferSlice::create(getName(),
                                  quasar::coretypes::BufferSlice::getParent(),
                                  getOffset(), size());
}

std::shared_ptr<NamedBufferSlice>
NamedBufferSlice::sliceView(size_t start, size_t length) const {
  // Fulfills [FE-0030.7.6] Slices can be created from a slice.
  // Creates a nested slice (sub-slice). The new slice's offset is relative to
  // the current slice's offset. The underlying buffer remains the same.
  if (start + length > size()) {
    throw std::out_of_range("Slice bounds exceed parent slice capacity");
  }
  return NamedBufferSlice::create(getName() + "_slice",
                                  quasar::coretypes::BufferSlice::getParent(),
                                  getOffset() + start, length);
}

std::shared_ptr<NamedObject>
NamedBufferSlice::deepCopy(std::shared_ptr<NamedObject> originalParent,
                           std::shared_ptr<NamedObject> newParent) const {

  std::shared_ptr<NamedBufferSlice> clonedSlice = nullptr;

  if (originalParent && newParent) {
    auto underlyingBuffer = quasar::coretypes::BufferSlice::getParent();

    if (auto nb = std::dynamic_pointer_cast<NamedBuffer>(originalParent)) {
      if (underlyingBuffer == nb) {
        if (auto newNb = std::dynamic_pointer_cast<NamedBuffer>(newParent)) {
          clonedSlice =
              NamedBufferSlice::create(getName(), newNb, getOffset(), size());
        }
      }
    } else if (auto nbs = std::dynamic_pointer_cast<NamedBufferSlice>(
                   originalParent)) {
      if (underlyingBuffer ==
          nbs->quasar::coretypes::BufferSlice::getParent()) {
        if (auto newNbs =
                std::dynamic_pointer_cast<NamedBufferSlice>(newParent)) {
          if (getOffset() >= newNbs->getOffset()) {
            size_t relativeStart = getOffset() - newNbs->getOffset();
            clonedSlice = NamedBufferSlice::create(getName(), newNbs,
                                                   relativeStart, size());
          }
        }
      }
    }
  }

  if (!clonedSlice) {
    clonedSlice = std::dynamic_pointer_cast<NamedBufferSlice>(clone());
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
