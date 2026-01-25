#include "quasar/named/NamedBitBuffer.h"

namespace quasar::named {

NamedBitBuffer::NamedBitBuffer(const std::string& name, size_t bitCount)
    : NamedObject(name), quasar::coretypes::BitBuffer(bitCount) {}

std::shared_ptr<NamedBitBuffer> NamedBitBuffer::create(const std::string& name, size_t bitCount, std::shared_ptr<NamedObject> parent) {
    auto obj = std::make_shared<NamedBitBuffer>(name, bitCount);
    if (parent) {
        obj->setParent(parent);
    }
    return obj;
}

std::shared_ptr<NamedObject> NamedBitBuffer::clone() const {
    auto newObj = create(getName(), bitSize());
    // Copy the BitBuffer part
    static_cast<quasar::coretypes::BitBuffer&>(*newObj) = static_cast<const quasar::coretypes::BitBuffer&>(*this);
    return newObj;
}

} // namespace quasar::named
