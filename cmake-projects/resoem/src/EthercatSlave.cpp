#include "resoem/EthercatSlave.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedInteger.hpp"

namespace resoem {

using namespace quasar::named;

std::shared_ptr<EthercatSlave> EthercatSlave::create(const std::string& name, const SlaveInfo& info, std::shared_ptr<NamedObject> parent) {
    struct make_shared_enabler : public EthercatSlave {
        make_shared_enabler(const std::string& n, uint16_t addr) : EthercatSlave(n, addr) {}
    };
    std::shared_ptr<EthercatSlave> slave = std::make_shared<make_shared_enabler>(name, info.configured_address);
    slave->setSelf(slave);
    if (parent) {
        slave->setParent(parent);
    }

    // Populate standard identity fields
    NamedString::create("name", info.name, slave);
    NamedInteger<uint32_t>::create("vendor_id", info.vendor_id, slave);
    NamedInteger<uint32_t>::create("product_code", info.product_code, slave);
    NamedInteger<uint16_t>::create("address", info.configured_address, slave);

    return slave;
}

EthercatSlave::EthercatSlave(const std::string& name, uint16_t address)
    : NamedObject(name), m_address(address)
{
}

} // namespace resoem
