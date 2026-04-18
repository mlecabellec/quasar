#pragma once

#include "quasar/named/NamedObject.hpp"
#include "resoem/Slave.hpp"
#include <memory>
#include <string>

namespace resoem {

/**
 * @class EthercatSlave
 * @brief A NamedObject representing an EtherCAT slave in the tree.
 * 
 * It supports observers to notify about state changes or errors.
 */
class EthercatSlave : public quasar::named::NamedObject {
public:
    /**
     * @brief Factory method to create an EthercatSlave.
     * @param name The name of the slave node.
     * @param info Slave identity and configuration.
     * @param parent Optional parent node.
     * @return Shared pointer to the new slave node.
     */
    static std::shared_ptr<EthercatSlave> create(const std::string& name, const SlaveInfo& info, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    /**
     * @brief Gets the type of the object.
     * @return "EthercatSlave".
     */
    std::string getType() const override { return "EthercatSlave"; }

    /**
     * @brief Gets the configured station address.
     */
    uint16_t getAddress() const { return m_address; }

protected:
    /**
     * @brief Protected constructor.
     */
    EthercatSlave(const std::string& name, uint16_t address);

private:
    uint16_t m_address;
};

} // namespace resoem
