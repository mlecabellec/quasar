#include "resoem/EthercatMasterService.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "resoem/Diagnostics.hpp"
#include "resoem/EtherCATTypes.hpp"
#include <iostream>
#include <stdexcept>
#include <chrono>

namespace resoem {

using namespace quasar::named;

std::shared_ptr<EthercatMasterService> EthercatMasterService::create(const std::string& name, std::shared_ptr<NamedObject> parent) {
    struct make_shared_enabler : public EthercatMasterService {
        explicit make_shared_enabler(const std::string& n) : EthercatMasterService(n) {}
    };
    std::shared_ptr<EthercatMasterService> svc = std::make_shared<make_shared_enabler>(name);
    svc->setSelf(svc);
    if (parent) {
        svc->setParent(parent);
    }
    
    std::weak_ptr<NamedService> weakSelf = svc;
    NamedMethod::create("start", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        if (auto s = weakSelf.lock()) s->start();
        return nullptr;
    }, svc);
    NamedMethod::create("stop", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        if (auto s = weakSelf.lock()) s->stop();
        return nullptr;
    }, svc);

    svc->initialize(svc);
    return svc;
}

EthercatMasterService::EthercatMasterService(const std::string& name)
    : NamedService(name)
{
}

EthercatMasterService::~EthercatMasterService() {
    stop();
}

void EthercatMasterService::initialize(std::shared_ptr<EthercatMasterService> self) {
    // Expose interface name as a string property
    NamedString::create("interface", m_interfaceName, self);

    // Create reflexive methods
    NamedMethod::create("refreshStatus", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        return this->refreshStatus(args);
    }, self);

    NamedMethod::create("forceInit", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        return this->forceInit(args);
    }, self);

    NamedMethod::create("reconfigureSlave", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        return this->reconfigureSlave(args);
    }, self);

    NamedMethod::create("run", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        this->performDiagnosticSweep();
        return nullptr;
    }, self);

    m_slavesRoot = NamedObject::create("slaves", self);
}

void EthercatMasterService::setInterface(const std::string& iface) {
    m_interfaceName = iface;
    auto child = getChild("interface");
    if (child) {
        if (auto ns = std::dynamic_pointer_cast<NamedString>(child)) {
            // Note: NamedString is immutable wrapper of String, so we might need to handle this carefully
            // In quasar, NamedString value can't be easily mutated if it's immutable. We might replace it.
            // For now, let's keep it simple.
        }
    }
}

void EthercatMasterService::start() {
    std::cout << "[EthercatMasterService] Starting on interface " << m_interfaceName << std::endl;
    
    // Initialize socket and enumerator
    try {
        m_socket = std::make_unique<RawSocket>(m_interfaceName);
        m_enumerator = std::make_unique<Enumerator>(*m_socket);
        m_mailbox = std::make_unique<MailboxHandler>(*m_socket);
        m_coe = std::make_unique<CoEHandler>(*m_mailbox);
        discoverSlaves();
    } catch (const std::exception& e) {
        std::cerr << "[EthercatMasterService] Error starting: " << e.what() << std::endl;
        // Depending on requirements, we might want to propagate this
    }

    NamedService::start();
}

void EthercatMasterService::stop() {
    std::cout << "[EthercatMasterService] Stopping..." << std::endl;
    NamedService::stop();

    m_enumerator.reset();
    m_socket.reset();
}

void EthercatMasterService::performDiagnosticSweep() {
    if (!m_enumerator || !m_slavesRoot) return;

    int i = 0;
    const auto& slaves = m_enumerator->slaves();
    if (m_slaveStates.size() != slaves.size()) {
        m_slaveStates.resize(slaves.size(), 0);
    }

    for (const auto& slaveInfo : slaves) {
        auto slaveNode = m_slavesRoot->getChild("slave_" + std::to_string(i));
        if (slaveNode) {
            auto diagNode = slaveNode->getChild("diagnostics");
            if (!diagNode) {
                diagNode = NamedObject::create("diagnostics", slaveNode);
                NamedInteger<uint16_t>::create("al_status", 0, diagNode);
                NamedString::create("al_status_desc", "", diagNode);
                NamedInteger<uint8_t>::create("rx_err_port0", 0, diagNode);
            }

            int wkc = 0;
            // AL Status Code (0x0134)
            uint16_t statusCode = m_enumerator->read_register_fprd<uint16_t>(slaveInfo.configured_address, 0x0134, wkc);
            if (wkc > 0) {
                // Check for state change
                uint16_t currentState = statusCode & 0x0F; // Lower 4 bits are the state
                if (m_slaveStates[i] != currentState) {
                    auto event = NamedObject::create("SlaveStateChanged");
                    NamedInteger<int>::create("slaveIndex", i, event);
                    NamedInteger<uint16_t>::create("oldState", m_slaveStates[i], event);
                    NamedInteger<uint16_t>::create("newState", currentState, event);
                    
                    // Notify master observers
                    this->notifyObservers(event);
                    
                    // Notify slave observers
                    slaveNode->notifyObservers(event);

                    m_slaveStates[i] = currentState;
                }

                if (auto statusField = std::dynamic_pointer_cast<NamedInteger<uint16_t>>(diagNode->getChild("al_status"))) {
                    statusField->setValue(statusCode);
                }
                if (auto descField = std::dynamic_pointer_cast<NamedString>(diagNode->getChild("al_status_desc"))) {
                    auto newDesc = NamedString::create("al_status_desc", std::string(al_status_code_to_string(statusCode)));
                    descField->replaceInTree(newDesc);
                }
            }

            // RX Error Counter Port 0 (0x0300)
            uint8_t rxErr = m_enumerator->read_register_fprd<uint8_t>(slaveInfo.configured_address, 0x0300, wkc);
            if (wkc > 0) {
                if (auto errField = std::dynamic_pointer_cast<NamedInteger<uint8_t>>(diagNode->getChild("rx_err_port0"))) {
                    errField->setValue(rxErr);
                }
            }
        }
        i++;
    }
}

std::shared_ptr<NamedObject> EthercatMasterService::reconfigureSlave(std::shared_ptr<NamedObject> args) {
    std::cout << "[EthercatMasterService] Reconfiguring slave..." << std::endl;
    
    int slaveIdx = 0;
    if (args) {
        if (auto idxObj = std::dynamic_pointer_cast<NamedInteger<int>>(args->getChild("slaveIndex"))) {
            slaveIdx = idxObj->value();
        }
    }

    if (!m_enumerator || !m_coe) {
         auto result = NamedObject::create("result");
         NamedBoolean::create("success", false, result);
         return result;
    }

    const auto& slaves = m_enumerator->slaves();
    if (slaveIdx < 0 || slaveIdx >= static_cast<int>(slaves.size())) {
         auto result = NamedObject::create("result");
         NamedBoolean::create("success", false, result);
         return result;
    }

    auto& slaveInfo = const_cast<SlaveInfo&>(slaves[slaveIdx]);

    try {
        // 1. Transition to PRE-OP
        m_enumerator->request_state(slaveIdx, states::PRE_OP);

        // 2. Update mappings (Example: dummy write to 0x1C12:00 to clear mappings)
        uint8_t zero = 0;
        m_coe->sdo_write(slaveInfo, 0x1C12, 0x00, std::span<const byte>(&zero, 1));

        // 3. Return to OP
        m_enumerator->request_state(slaveIdx, states::OP);

        // 4. Update tree
        discoverSlaves();
    } catch (const std::exception& e) {
        std::cerr << "[EthercatMasterService] Reconfiguration failed: " << e.what() << std::endl;
        auto result = NamedObject::create("result");
        NamedBoolean::create("success", false, result);
        return result;
    }

    auto result = NamedObject::create("result");
    NamedBoolean::create("success", true, result);
    return result;
}

void EthercatMasterService::discoverSlaves() {
    if (!m_enumerator) return;
    
    auto result = m_enumerator->enumerate();
    if (!result) {
        std::cerr << "[EthercatMasterService] Failed to discover slaves." << std::endl;
        return;
    }
    int count = result.value();
    std::cout << "[EthercatMasterService] Discovered " << count << " slaves." << std::endl;

    // Clear existing slaves from tree
    // NamedObject API: remove children? We can just create a new root and replace it.
    auto newSlavesRoot = NamedObject::create("slaves");
    
    int i = 0;
    for (const auto& slaveInfo : m_enumerator->slaves()) {
        EthercatSlave::create("slave_" + std::to_string(i), slaveInfo, newSlavesRoot);
        i++;
    }

    // Atomic replacement in tree
    if (m_slavesRoot) {
        m_slavesRoot->replaceInTree(newSlavesRoot);
        m_slavesRoot = newSlavesRoot;
    }
}

std::shared_ptr<NamedObject> EthercatMasterService::refreshStatus(std::shared_ptr<NamedObject> args) {
    std::cout << "[EthercatMasterService] refreshing status..." << std::endl;
    discoverSlaves(); // For now, discovering again serves as a full refresh
    auto result = NamedObject::create("result");
    NamedBoolean::create("success", true, result);
    return result;
}

std::shared_ptr<NamedObject> EthercatMasterService::forceInit(std::shared_ptr<NamedObject> args) {
    std::cout << "[EthercatMasterService] Forcing slaves to INIT state..." << std::endl;
    if (m_enumerator) {
        m_enumerator->reset_to_init();
    }
    auto result = NamedObject::create("result");
    NamedBoolean::create("success", true, result);
    return result;
}

} // namespace resoem
