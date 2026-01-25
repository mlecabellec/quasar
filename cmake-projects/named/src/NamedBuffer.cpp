#include "quasar/named/NamedBuffer.h"

namespace quasar::named {

NamedBuffer::NamedBuffer(const std::string& name, size_t size)
    : NamedObject(name), quasar::coretypes::Buffer(size) {}

NamedBuffer::NamedBuffer(const std::string& name, const std::vector<uint8_t>& data)
    : NamedObject(name), quasar::coretypes::Buffer(data) {}

std::shared_ptr<NamedBuffer> NamedBuffer::create(const std::string& name, size_t size, std::shared_ptr<NamedObject> parent) {
    auto obj = std::make_shared<NamedBuffer>(name, size);
    if (parent) {
        obj->setParent(parent);
    }
    return obj;
}

std::shared_ptr<NamedBuffer> NamedBuffer::create(const std::string& name, const std::vector<uint8_t>& data, std::shared_ptr<NamedObject> parent) {
    auto obj = std::make_shared<NamedBuffer>(name, data);
    if (parent) {
        obj->setParent(parent);
    }
    return obj;
}

} // namespace quasar::named
