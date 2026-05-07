#pragma once

#include "quasar/named/NamedService.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/Enumerator.hpp"
#include "resoem/EthercatSlave.hpp"
#include "resoem/CoEHandler.hpp"
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <mutex>

namespace resoem {

/**
 * @class EthercatMasterService
 * @brief Service responsible for managing the EtherCAT master stack and reflexive slave discovery.
 */
class EthercatMasterService : public quasar::named::NamedService {
public:
    /**
     * @brief Factory method for creating an EthercatMasterService.
     * @param name The name of the service.
     * @param parent The parent NamedObject.
     * @return A shared pointer to the created service.
     */
    static std::shared_ptr<EthercatMasterService> create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~EthercatMasterService() override;

    /**
     * @brief Start the EtherCAT service.
     */
    void start() override;

    /**
     * @brief Stop the EtherCAT service.
     */
    void stop() override;

    /**
     * @brief Get the type of the service.
     * @return The type name.
     */
    std::string getType() const override { return "EthercatMasterService"; }

    /**
     * @brief Set the network interface name.
     * @param iface The interface name (e.g., "eth0").
     */
    void setInterface(const std::string& iface);

    /**
     * @brief Refresh the status of all slaves.
     * @param args NamedObject containing arguments.
     * @return Result NamedObject.
     */
    std::shared_ptr<quasar::named::NamedObject> refreshStatus(std::shared_ptr<quasar::named::NamedObject> args);

    /**
     * @brief Force all slaves to INIT state.
     * @param args NamedObject containing arguments.
     * @return Result NamedObject.
     */
    std::shared_ptr<quasar::named::NamedObject> forceInit(std::shared_ptr<quasar::named::NamedObject> args);

    /**
     * @brief Reconfigure a specific slave.
     * @param args NamedObject containing "slaveIndex".
     * @return Result NamedObject.
     */
    std::shared_ptr<quasar::named::NamedObject> reconfigureSlave(std::shared_ptr<quasar::named::NamedObject> args);

protected:
    /**
     * @brief Constructor.
     * @param name The name of the service.
     */
    EthercatMasterService(const std::string& name);

    /**
     * @brief Initialize the service components.
     * @param self Shared pointer to self.
     */
    void initialize(std::shared_ptr<EthercatMasterService> self);

private:
    std::string m_interfaceName{"eth0"};
    std::unique_ptr<RawSocket> m_socket;
    std::unique_ptr<Enumerator> m_enumerator;
    std::unique_ptr<MailboxHandler> m_mailbox;
    std::unique_ptr<CoEHandler> m_coe;
    std::shared_ptr<quasar::named::NamedObject> m_slavesRoot;
    std::vector<uint16_t> m_slaveStates;

    /**
     * @brief Mutex for protecting shared fields.
     */
    mutable std::timed_mutex m_mutex;

    /**
     * @brief Discover slaves on the bus.
     */
    void discoverSlaves();

    /**
     * @brief Perform a diagnostic sweep across all slaves.
     */
    void performDiagnosticSweep();
};

} // namespace resoem
