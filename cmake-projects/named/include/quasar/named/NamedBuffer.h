#ifndef QUASAR_NAMED_NAMEDBUFFER_H
#define QUASAR_NAMED_NAMEDBUFFER_H

#include "quasar/named/NamedObject.h"
#include "quasar/coretypes/Buffer.hpp"

namespace quasar::named {

class NamedBuffer : public NamedObject, public quasar::coretypes::Buffer {
public:
    virtual ~NamedBuffer() = default;

    static std::shared_ptr<NamedBuffer> create(const std::string& name, size_t size, std::shared_ptr<NamedObject> parent = nullptr);
    static std::shared_ptr<NamedBuffer> create(const std::string& name, const std::vector<uint8_t>& data, std::shared_ptr<NamedObject> parent = nullptr);

    std::shared_ptr<NamedObject> clone() const override {
        // Accessing protected data_ from Buffer
        return create(getName(), data_);
    }

    NamedBuffer(const std::string& name, size_t size);
    NamedBuffer(const std::string& name, const std::vector<uint8_t>& data);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBUFFER_H
