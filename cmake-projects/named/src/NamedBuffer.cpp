#include "quasar/named/NamedBuffer.hpp"

namespace quasar::named {

NamedBuffer::NamedBuffer(const std::string &name, size_t size)
    : NamedObject(name), quasar::coretypes::Buffer(size) {
    // Fulfills [FE-0030.6] support for a named Buffer.
    // Initializes the NamedObject with the name and the Buffer with the requested size.
}

NamedBuffer::NamedBuffer(const std::string &name,
                         const std::vector<uint8_t> &data)
    : NamedObject(name), quasar::coretypes::Buffer(data) {
    // Fulfills [FE-0030.6] support for a named Buffer.
    // Initializes the NamedObject with the name and the Buffer with the provided initial data.
}

std::shared_ptr<NamedBuffer>
NamedBuffer::create(const std::string &name, size_t size,
                    std::shared_ptr<NamedObject> parent) {
  // Fulfills [FE-0020.6] static method "create".
  // Instantiate a new NamedBuffer.
  std::shared_ptr<NamedBuffer> obj = std::make_shared<NamedBuffer>(name, size);

  // Ensure the object can return a shared_ptr to itself.
  obj->setSelf(obj);

  // If a parent is specified, add this buffer to the hierarchy.
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

std::shared_ptr<NamedBuffer>
NamedBuffer::create(const std::string &name, const std::vector<uint8_t> &data,
                    std::shared_ptr<NamedObject> parent) {
  // Fulfills [FE-0020.6] static method "create".
  // Instantiate a new NamedBuffer with initial data.
  std::shared_ptr<NamedBuffer> obj = std::make_shared<NamedBuffer>(name, data);

  // Ensure the object can return a shared_ptr to itself.
  obj->setSelf(obj);

  // If a parent is specified, add this buffer to the hierarchy.
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

std::string NamedBuffer::getType() const { return "NamedBuffer"; }

} // namespace quasar::named

