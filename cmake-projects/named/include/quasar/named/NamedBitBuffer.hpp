#ifndef QUASAR_NAMED_NAMEDBITBUFFER_HPP
#define QUASAR_NAMED_NAMEDBITBUFFER_HPP

#include "quasar/named/NamedObject.hpp"
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

#endif // QUASAR_NAMED_NAMEDBITBUFFER_HPP
