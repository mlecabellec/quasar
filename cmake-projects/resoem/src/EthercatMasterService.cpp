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

/**
 * @brief Create an instance of EthercatMasterService.
 */
std::shared_ptr<EthercatMasterService> EthercatMasterService::create(const std::string& name, std::shared_ptr<NamedObject> parent) {
    struct make_shared_enabler : public EthercatMasterService {
        explicit make_shared_enabler(const std::string& n) : EthercatMasterService(n) {}
    };
    std::shared_ptr<EthercatMasterService> svc = std::make_shared<make_shared_enabler>(name);
    svc->setSelf(svc);
    if (parent != nullptr) {
        svc->setParent(parent);
    }
    
    std::weak_ptr<NamedService> weakSelf = svc;
    NamedMethod::create("start", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        if (std::shared_ptr<NamedService> s = weakSelf.lock()) s->start();
        return nullptr;
    }, svc);
    NamedMethod::create("stop", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        if (std::shared_ptr<NamedService> s = weakSelf.lock()) s->stop();
        return nullptr;
    }, svc);

    svc->initialize(svc);
    return svc;
}

/**
 * @brief Constructor.
 */
EthercatMasterService::EthercatMasterService(const std::string& name)
    : NamedService(name)
{
}

/**
 * @brief Destructor.
 */
EthercatMasterService::~EthercatMasterService() {
    stop();
}

/**
 * @brief Initialize reflexive methods and properties.
 */
void EthercatMasterService::initialize(std::shared_ptr<EthercatMasterService> self) {
    // Expose interface name as a string property
    NamedString::create("interface", m_interfaceName, self);

    // Create reflexive methods
    NamedMethod::create("refreshStatus", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner;
        return this->refreshStatus(args);
    }, self);

    NamedMethod::create("forceInit", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner;
        return this->forceInit(args);
    }, self);

    NamedMethod::create("reconfigureSlave", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner;
        return this->reconfigureSlave(args);
    }, self);

    NamedMethod::create("run", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        this->performDiagnosticSweep();
        return nullptr;
    }, self);

    m_slavesRoot = NamedObject::create("slaves", self);
}

/**
 * @brief Set interface name.
 */
void EthercatMasterService::setInterface(const std::string& iface) {
    // [CS-0010.46] Timed mutex mandatory.
    if (!m_mutex.try_lock_for(std::chrono::milliseconds(100))) {
        throw std::runtime_error("Failed to acquire mutex in setInterface");
    }
    std::lock_guard<std::timed_mutex> lock(m_mutex, std::adopt_lock);
    m_interfaceName = iface;
    std::shared_ptr<NamedObject> child = getChild("interface");
    if (child != nullptr) {
        if (std::shared_ptr<NamedString> ns = std::dynamic_pointer_cast<NamedString>(child)) {
            // Note: NamedString is immutable wrapper of String in this version
        }
    }
}

/**
 * @brief Start the master service.
 */
void EthercatMasterService::start() {
    // [CS-0010.46] Timed mutex mandatory.
    if (!m_mutex.try_lock_for(std::chrono::milliseconds(1000))) {
        throw std::runtime_error("Failed to acquire mutex in start");
    }
    std::lock_guard<std::timed_mutex> lock(m_mutex, std::adopt_lock);

    std::cout << "[EthercatMasterService] Starting on interface " << m_interfaceName << std::endl;
    
    // Initialize socket and enumerator
    try {
        m_socket = std::make_unique<RawSocket>(m_interfaceName);
        m_enumerator = std::make_unique<Enumerator>(*m_socket);
        m_mailbox = std::make_unique<MailboxHandler>(*m_socket);
        m_coe = std::make_unique<CoEHandler>(*m_mailbox);
        
        // Manual unlock to call discoverSlaves which also locks
        m_mutex.unlock();
        discoverSlaves();
        m_mutex.lock();
    } catch (const std::exception& e) {
        std::cerr << "[EthercatMasterService] Error starting: " << e.what() << std::endl;
    }

    NamedService::start();
}

/**
 * @brief Stop the master service.
 */
void EthercatMasterService::stop() {
    // [CS-0010.46] Timed mutex mandatory.
    if (!m_mutex.try_lock_for(std::chrono::milliseconds(1000))) {
        return; // Already stopping or locked
    }
    std::lock_guard<std::timed_mutex> lock(m_mutex, std::adopt_lock);

    std::cout << "[EthercatMasterService] Stopping..." << std::endl;
    NamedService::stop();

    m_enumerator.reset();
    m_mailbox.reset();
    m_coe.reset();
    m_socket.reset();
}

/**
 * @brief Sweep AL status and error counters.
 */
void EthercatMasterService::performDiagnosticSweep() {
    // [CS-0010.46] Timed mutex mandatory.
    if (!m_mutex.try_lock_for(std::chrono::milliseconds(50))) {
        return; // Skip sweep if busy
    }
    std::lock_guard<std::timed_mutex> lock(m_mutex, std::adopt_lock);

    if (m_enumerator == nullptr || m_slavesRoot == nullptr) return;

    int i = 0;
    const std::vector<SlaveInfo>& slaves = m_enumerator->slaves();
    if (m_slaveStates.size() != slaves.size()) {
        m_slaveStates.resize(slaves.size(), 0);
    }

    // [CS-0010.37] Loop hard limit.
    size_t slave_count = 0;
    for (const SlaveInfo& slaveInfo : slaves) {
        if (++slave_count > 65535) break;

        std::shared_ptr<NamedObject> slaveNode = m_slavesRoot->getChild("slave_" + std::to_string(i));
        if (slaveNode) {
            std::shared_ptr<NamedObject> diagNode = slaveNode->getChild("diagnostics");
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
                    std::shared_ptr<NamedObject> event = NamedObject::create("SlaveStateChanged");
                    NamedInteger<int>::create("slaveIndex", i, event);
                    NamedInteger<uint16_t>::create("oldState", m_slaveStates[i], event);
                    NamedInteger<uint16_t>::create("newState", currentState, event);
                    
                    // Notify observers
                    this->notifyObservers(event);
                    slaveNode->notifyObservers(event);

                    m_slaveStates[i] = currentState;
                }

                if (std::shared_ptr<NamedInteger<uint16_t>> statusField = std::dynamic_pointer_cast<NamedInteger<uint16_t>>(diagNode->getChild("al_status"))) {
                    statusField->setValue(statusCode);
                }
                if (std::shared_ptr<NamedString> descField = std::dynamic_pointer_cast<NamedString>(diagNode->getChild("al_status_desc"))) {
                    std::shared_ptr<NamedString> newDesc = NamedString::create("al_status_desc", std::string(al_status_code_to_string(statusCode)));
                    descField->replaceInTree(newDesc);
                }
            }

            // RX Error Counter Port 0 (0x0300)
            uint8_t rxErr = m_enumerator->read_register_fprd<uint8_t>(slaveInfo.configured_address, 0x0300, wkc);
            if (wkc > 0) {
                if (std::shared_ptr<NamedInteger<uint8_t>> errField = std::dynamic_pointer_cast<NamedInteger<uint8_t>>(diagNode->getChild("rx_err_port0"))) {
                    errField->setValue(rxErr);
                }
            }
        }
        i++;
    }
}

/**
 * @brief Reconfigure slave mailbox/PDO.
 */
std::shared_ptr<NamedObject> EthercatMasterService::reconfigureSlave(std::shared_ptr<NamedObject> args) {
    // [CS-0010.46] Timed mutex mandatory.
    if (!m_mutex.try_lock_for(std::chrono::milliseconds(500))) {
         std::shared_ptr<NamedObject> result = NamedObject::create("result");
         NamedBoolean::create("success", false, result);
         return result;
    }
    std::lock_guard<std::timed_mutex> lock(m_mutex, std::adopt_lock);

    std::cout << "[EthercatMasterService] Reconfiguring slave..." << std::endl;
    
    int slaveIdx = 0;
    if (args) {
        if (std::shared_ptr<NamedInteger<int>> idxObj = std::dynamic_pointer_cast<NamedInteger<int>>(args->getChild("slaveIndex"))) {
            slaveIdx = idxObj->value();
        }
    }

    if (!m_enumerator || !m_coe) {
         std::shared_ptr<NamedObject> result = NamedObject::create("result");
         NamedBoolean::create("success", false, result);
         return result;
    }

    const std::vector<SlaveInfo>& slaves = m_enumerator->slaves();
    if (slaveIdx < 0 || slaveIdx >= static_cast<int>(slaves.size())) {
         std::shared_ptr<NamedObject> result = NamedObject::create("result");
         NamedBoolean::create("success", false, result);
         return result;
    }

    SlaveInfo& slaveInfo = const_cast<SlaveInfo&>(slaves[slaveIdx]);

    try {
        // 1. Transition to PRE-OP
        Result<uint16_t> resPreOp = m_enumerator->request_state(slaveIdx, states::PRE_OP);
        if (!resPreOp) {
            throw std::runtime_error("Failed to transition to PRE-OP");
        }

        // 2. Update mappings
        uint8_t zero = 0;
        Result<> resWrite = m_coe->sdo_write(slaveInfo, 0x1C12, 0x00, std::span<const byte>(&zero, 1));
        if (!resWrite) {
            throw std::runtime_error("Failed to update mapping SDO (0x1C12:00)");
        }

        // 3. Return to OP
        Result<uint16_t> resOp = m_enumerator->request_state(slaveIdx, states::OP);
        if (!resOp) {
            throw std::runtime_error("Failed to return to OP");
        }

        // 4. Update tree
        m_mutex.unlock();
        discoverSlaves();
        m_mutex.lock();
    } catch (const std::exception& e) {
        std::cerr << "[EthercatMasterService] Reconfiguration failed: " << e.what() << std::endl;
        std::shared_ptr<NamedObject> result = NamedObject::create("result");
        NamedBoolean::create("success", false, result);
        return result;
    }

    std::shared_ptr<NamedObject> result = NamedObject::create("result");
    NamedBoolean::create("success", true, result);
    return result;
}

/**
 * @brief Refresh slave list.
 */
void EthercatMasterService::discoverSlaves() {
    // [CS-0010.46] Timed mutex mandatory.
    if (!m_mutex.try_lock_for(std::chrono::milliseconds(1000))) {
        return;
    }
    std::lock_guard<std::timed_mutex> lock(m_mutex, std::adopt_lock);

    if (!m_enumerator) return;
    
    Result<size_t> result = m_enumerator->enumerate();
    if (!result) {
        std::cerr << "[EthercatMasterService] Failed to discover slaves." << std::endl;
        return;
    }
    int count = result.value();
    std::cout << "[EthercatMasterService] Discovered " << count << " slaves." << std::endl;

    std::shared_ptr<NamedObject> newSlavesRoot = NamedObject::create("slaves");
    
    int i = 0;
    // [CS-0010.37] Loop hard limit.
    size_t slave_count = 0;
    for (const SlaveInfo& slaveInfo : m_enumerator->slaves()) {
        if (++slave_count > 65535) break;
        EthercatSlave::create("slave_" + std::to_string(i), slaveInfo, newSlavesRoot);
        i++;
    }

    // Atomic replacement in tree
    if (m_slavesRoot) {
        m_slavesRoot->replaceInTree(newSlavesRoot);
        m_slavesRoot = newSlavesRoot;
    }
}

/**
 * @brief Refresh status hook.
 */
std::shared_ptr<NamedObject> EthercatMasterService::refreshStatus(std::shared_ptr<NamedObject> args) {
    (void)args;
    std::cout << "[EthercatMasterService] refreshing status..." << std::endl;
    discoverSlaves(); 
    std::shared_ptr<NamedObject> result = NamedObject::create("result");
    NamedBoolean::create("success", true, result);
    return result;
}

/**
 * @brief Force INIT hook.
 */
std::shared_ptr<NamedObject> EthercatMasterService::forceInit(std::shared_ptr<NamedObject> args) {
    (void)args;
    // [CS-0010.46] Timed mutex mandatory.
    if (!m_mutex.try_lock_for(std::chrono::milliseconds(500))) {
         std::shared_ptr<NamedObject> result = NamedObject::create("result");
         NamedBoolean::create("success", false, result);
         return result;
    }
    std::lock_guard<std::timed_mutex> lock(m_mutex, std::adopt_lock);

    std::cout << "[EthercatMasterService] Forcing slaves to INIT state..." << std::endl;
    if (m_enumerator) {
        m_enumerator->reset_to_init();
    }
    std::shared_ptr<NamedObject> result = NamedObject::create("result");
    NamedBoolean::create("success", true, result);
    return result;
}

} // namespace resoem
