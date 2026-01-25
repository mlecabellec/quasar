#ifndef QUASAR_NAMED_NAMEDBITBUFFER_H
#define QUASAR_NAMED_NAMEDBITBUFFER_H

#include "quasar/named/NamedObject.h"
#include "quasar/coretypes/BitBuffer.hpp"

namespace quasar::named {

class NamedBitBuffer : public NamedObject, public quasar::coretypes::BitBuffer {
public:
    virtual ~NamedBitBuffer() = default;

    static std::shared_ptr<NamedBitBuffer> create(const std::string& name, size_t bitCount, std::shared_ptr<NamedObject> parent = nullptr);

    std::shared_ptr<NamedObject> clone() const override;

    NamedBitBuffer(const std::string& name, size_t bitCount);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBITBUFFER_H
