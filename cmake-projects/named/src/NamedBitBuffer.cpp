#include "quasar/named/NamedBitBuffer.hpp"

namespace quasar::named {

NamedBitBuffer::NamedBitBuffer(const std::string &name, size_t bitCount)
    : NamedObject(name), quasar::coretypes::BitBuffer(bitCount) {
    // Fulfills [FE-0030.6] support for a named BitBuffer.
    // Constructor initializes the name and allocates the specified number of bits.
}

std::shared_ptr<NamedBitBuffer>
NamedBitBuffer::create(const std::string &name, size_t bitCount,
                       std::shared_ptr<NamedObject> parent) {
  // Fulfills [FE-0020.6] static method "create".
  // Factory method to instantiate the bit buffer safely.
  std::shared_ptr<NamedBitBuffer> obj =
      std::make_shared<NamedBitBuffer>(name, bitCount);
  
  // Initialize self-reference.
  obj->setSelf(obj);
  
  // Link to hierarchy if needed.
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

std::shared_ptr<NamedObject> NamedBitBuffer::clone() const {
  // Fulfills [FE-0020.14] Utilities for copying parts of the tree.
  // Create a new NamedBitBuffer with the same dimensions.
  std::shared_ptr<NamedBitBuffer> newObj = create(getName(), bitSize());
  
  // Copy the underlying bit content from the current instance.
  static_cast<quasar::coretypes::BitBuffer &>(*newObj) =
      static_cast<const quasar::coretypes::BitBuffer &>(*this);
      
  return newObj;
}

std::string NamedBitBuffer::getType() const { return "NamedBitBuffer"; }

} // namespace quasar::named
