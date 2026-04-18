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

namespace resoem {

class EthercatMasterService : public quasar::named::NamedService {
public:
    static std::shared_ptr<EthercatMasterService> create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    ~EthercatMasterService() override;

    void start() override;
    void stop() override;

    std::string getType() const override { return "EthercatMasterService"; }

    // Set the interface name before starting
    void setInterface(const std::string& iface);

    // Refresh all slaves' status (NamedMethod hook target)
    std::shared_ptr<quasar::named::NamedObject> refreshStatus(std::shared_ptr<quasar::named::NamedObject> args);

    // Force INIT state (NamedMethod hook target)
    std::shared_ptr<quasar::named::NamedObject> forceInit(std::shared_ptr<quasar::named::NamedObject> args);

    // Reconfigure a slave (NamedMethod hook target)
    std::shared_ptr<quasar::named::NamedObject> reconfigureSlave(std::shared_ptr<quasar::named::NamedObject> args);

protected:
    EthercatMasterService(const std::string& name);
    void initialize(std::shared_ptr<EthercatMasterService> self);

private:
    std::string m_interfaceName{"eth0"};
    std::unique_ptr<RawSocket> m_socket;
    std::unique_ptr<Enumerator> m_enumerator;
    std::unique_ptr<MailboxHandler> m_mailbox;
    std::unique_ptr<CoEHandler> m_coe;
    std::shared_ptr<quasar::named::NamedObject> m_slavesRoot;
    std::vector<uint16_t> m_slaveStates;

    void discoverSlaves();
    void performDiagnosticSweep();
};

} // namespace resoem
