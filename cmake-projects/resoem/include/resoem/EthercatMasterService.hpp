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

namespace eni {
class EniDocument;
class EniConfigurator;
}

namespace hot_connect {
class HotConnectManager;
}

namespace redundancy {
class RedundancyManager;
enum class RedundancyState : std::uint8_t;
}

/**
 * @class EthercatMasterService
 * @brief Service responsible for managing the EtherCAT master stack and reflexive slave discovery.
 */
class EthercatMasterService : public quasar::named::NamedService {
public:
    /// @brief Factory method for creating an EthercatMasterService.
    static std::shared_ptr<EthercatMasterService> create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    /// @brief Destructor.
    ~EthercatMasterService() override;

    /// @brief Start the EtherCAT service.
    void start() override;

    /// @brief Stop the EtherCAT service.
    void stop() override;

    /// @brief Get the type of the service.
    std::string getType() const override { return "EthercatMasterService"; }

    /// @brief Set the network interface name (e.g., "eth0").
    void setInterface(const std::string& iface);

    /// @brief Refresh the status of all slaves.
    std::shared_ptr<quasar::named::NamedObject> refreshStatus(std::shared_ptr<quasar::named::NamedObject> args);

    /// @brief Force all slaves to INIT state.
    std::shared_ptr<quasar::named::NamedObject> forceInit(std::shared_ptr<quasar::named::NamedObject> args);

    /// @brief Reconfigure a specific slave by "slaveIndex".
    std::shared_ptr<quasar::named::NamedObject> reconfigureSlave(std::shared_ptr<quasar::named::NamedObject> args);

    /// @brief Configure cable redundancy and secondary interface.
    void setRedundancy(bool enable, const std::string& secondary_iface = "");

    /// @brief Load and apply an ENI configuration file by "filepath".
    std::shared_ptr<quasar::named::NamedObject> loadEni(std::shared_ptr<quasar::named::NamedObject> args);

    /// @brief Export the current bus topology as an ETG.2100 ENI XML file.
    std::shared_ptr<quasar::named::NamedObject> exportEni(std::shared_ptr<quasar::named::NamedObject> args);

    /// @brief Compare a reference ENI file against the live bus.
    std::shared_ptr<quasar::named::NamedObject> compareEni(std::shared_ptr<quasar::named::NamedObject> args);

protected:
    /// @brief Construct an EthercatMasterService.
    EthercatMasterService(const std::string& name);

    /// @brief Initialize the service components and method bindings.
    void initialize(std::shared_ptr<EthercatMasterService> self);

private:
    std::string m_interfaceName{"eth0"};
    std::unique_ptr<RawSocket> m_socket;
    std::unique_ptr<Enumerator> m_enumerator;
    std::unique_ptr<MailboxHandler> m_mailbox;
    std::unique_ptr<CoEHandler> m_coe;
    std::shared_ptr<quasar::named::NamedObject> m_slavesRoot;
    std::vector<uint16_t> m_slaveStates;
    bool m_redundancyEnabled{false};
    std::string m_secondaryInterfaceName{""};
    std::unique_ptr<eni::EniDocument> m_eniDoc;
    std::unique_ptr<eni::EniConfigurator> m_eniConfigurator;
    std::unique_ptr<hot_connect::HotConnectManager> m_hotConnectMgr;
    std::unique_ptr<redundancy::RedundancyManager> m_redundancyMgr;
    redundancy::RedundancyState m_lastRedundancyState;

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
