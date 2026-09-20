#include "resoem/EthercatMasterService.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "resoem/Diagnostics.hpp"
#include "resoem/EtherCATTypes.hpp"
#include "resoem/EniDocument.hpp"
#include "resoem/EniConfigurator.hpp"
#include "resoem/EniGenerator.hpp"
#include "resoem/EniComparator.hpp"
#include "resoem/HotConnectManager.hpp"
#include "resoem/RedundancyManager.hpp"
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
    : NamedService(name),
      m_lastRedundancyState(redundancy::RedundancyState::DISABLED)
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

    NamedMethod::create("loadEni", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner;
        return this->loadEni(args);
    }, self);

    NamedMethod::create("exportEni", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner;
        return this->exportEni(args);
    }, self);

    NamedMethod::create("compareEni", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner;
        return this->compareEni(args);
    }, self);

    NamedMethod::create("setRedundancy", [this](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner;
        if (args != nullptr) {
            std::string sec_iface = "";
            bool enable = true;
            if (std::shared_ptr<NamedObject> sec_node = args->getChild("secondaryInterface")) {
                if (std::shared_ptr<NamedString> s = std::dynamic_pointer_cast<NamedString>(sec_node)) {
                    sec_iface = s->toString();
                } else {
                    // not string
                }
            } else {
                // no secondaryInterface arg
            }
            if (std::shared_ptr<NamedObject> en_node = args->getChild("enable")) {
                if (std::shared_ptr<NamedBoolean> b = std::dynamic_pointer_cast<NamedBoolean>(en_node)) {
                    enable = b->booleanValue();
                } else {
                    // not boolean
                }
            } else {
                // default enable = true
            }
            this->setRedundancy(enable, sec_iface);
        } else {
            this->setRedundancy(true, "");
        }
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

    // Check redundancy status and dispatch events if state transitioned
    if (m_redundancyMgr != nullptr) {
        redundancy::RedundancyState currentState = m_redundancyMgr->state();
        if (currentState != m_lastRedundancyState) {
            if (currentState == redundancy::RedundancyState::BIFURCATED_BROKEN) {
                std::shared_ptr<NamedObject> faultEvent = NamedObject::create("CableRedundancyFaultEvent");
                NamedString::create("primaryInterface", m_interfaceName, faultEvent);
                NamedString::create("secondaryInterface", m_secondaryInterfaceName, faultEvent);
                NamedString::create("state", "BIFURCATED_BROKEN", faultEvent);
                this->notifyObservers(faultEvent);
            } else if (currentState == redundancy::RedundancyState::CIRCULAR_NORMAL) {
                std::shared_ptr<NamedObject> healedEvent = NamedObject::create("CableRedundancyHealedEvent");
                NamedString::create("primaryInterface", m_interfaceName, healedEvent);
                NamedString::create("secondaryInterface", m_secondaryInterfaceName, healedEvent);
                NamedString::create("state", "CIRCULAR_NORMAL", healedEvent);
                this->notifyObservers(healedEvent);
            } else {
                // Intermediate or unchanged
            }
            m_lastRedundancyState = currentState;
        } else {
            // State unchanged
        }
    } else {
        // Redundancy not active
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

void EthercatMasterService::setRedundancy(bool enable, const std::string& secondary_iface) {
    std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
    if (lock.owns_lock()) {
        m_redundancyEnabled = enable;
        m_secondaryInterfaceName = secondary_iface;
        if (m_socket != nullptr && enable && !secondary_iface.empty()) {
            m_socket = std::make_unique<RawSocket>(m_interfaceName, secondary_iface);
            m_redundancyMgr = std::make_unique<redundancy::RedundancyManager>(*m_socket);
            m_lastRedundancyState = m_redundancyMgr->state();
        } else if (m_socket != nullptr && !enable) {
            m_socket = std::make_unique<RawSocket>(m_interfaceName);
            m_redundancyMgr.reset();
            m_lastRedundancyState = redundancy::RedundancyState::DISABLED;
        } else {
            // Socket not yet open or unchanged
        }
    } else {
        // Lock failed
    }
}

std::shared_ptr<NamedObject> EthercatMasterService::loadEni(std::shared_ptr<NamedObject> args) {
    std::shared_ptr<NamedObject> result = NamedObject::create("result");
    if (args == nullptr) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Missing arguments", result);
        return result;
    } else {
        // Args present
    }

    std::string filepath = "";
    if (std::shared_ptr<NamedObject> fp_node = args->getChild("filepath")) {
        if (std::shared_ptr<NamedString> s = std::dynamic_pointer_cast<NamedString>(fp_node)) {
            filepath = s->toString();
        } else {
            // Not a string
        }
    } else if (std::shared_ptr<NamedObject> p_node = args->getChild("path")) {
        if (std::shared_ptr<NamedString> s = std::dynamic_pointer_cast<NamedString>(p_node)) {
            filepath = s->toString();
        } else {
            // Not a string
        }
    } else {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Missing 'filepath' parameter", result);
        return result;
    }

    std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(500));
    if (!lock.owns_lock()) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Lock timeout", result);
        return result;
    } else {
        // Lock acquired
    }

    m_eniDoc = std::make_unique<eni::EniDocument>();
    Result<void> load_res = m_eniDoc->load_from_file(filepath);
    if (!load_res.has_value()) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Failed to parse ENI XML file: " + filepath, result);
        return result;
    } else {
        // Document parsed
    }

    if (m_socket != nullptr) {
        m_eniConfigurator = std::make_unique<eni::EniConfigurator>(*m_socket, *m_eniDoc);
        m_hotConnectMgr = std::make_unique<hot_connect::HotConnectManager>(*m_eniDoc, *m_socket, *m_eniConfigurator);
        m_hotConnectMgr->auto_discover_groups_from_eni();
        Result<void> cfg_res = m_eniConfigurator->configure_all();
        if (!cfg_res.has_value()) {
            NamedBoolean::create("success", false, result);
            NamedString::create("error", "EniConfigurator failed to transition bus to OP", result);
            return result;
        } else {
            // Network in OP
        }
    } else {
        // Socket not open: only AST loaded
    }

    std::shared_ptr<NamedObject> mount_point = m_slavesRoot != nullptr ? m_slavesRoot : getSelf();
    // [CS-0060.1] Cast nodiscard result to void: tree nodes are attached directly to mount_point hierarchy.
    (void)m_eniDoc->to_named_tree(mount_point);

    NamedBoolean::create("success", true, result);
    NamedInteger<std::uint32_t>::create("slaveCount", static_cast<std::uint32_t>(m_eniDoc->slaves().size()), result);
    return result;
}

std::shared_ptr<NamedObject> EthercatMasterService::exportEni(std::shared_ptr<NamedObject> args) {
    std::shared_ptr<NamedObject> result = NamedObject::create("result");
    if (args == nullptr) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Missing arguments", result);
        return result;
    } else {
        // Args present
    }

    std::string filepath = "";
    if (std::shared_ptr<NamedObject> fp_node = args->getChild("filepath")) {
        if (std::shared_ptr<NamedString> s = std::dynamic_pointer_cast<NamedString>(fp_node)) {
            filepath = s->toString();
        } else {
            // Not a string
        }
    } else {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Missing 'filepath' parameter", result);
        return result;
    }

    std::string master_name = "QuasarMaster";
    if (std::shared_ptr<NamedObject> mn_node = args->getChild("masterName")) {
        if (std::shared_ptr<NamedString> s = std::dynamic_pointer_cast<NamedString>(mn_node)) {
            master_name = s->toString();
        } else {
            // Not a string
        }
    } else {
        // Default master name
    }

    std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(500));
    if (!lock.owns_lock()) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Lock timeout", result);
        return result;
    } else {
        // Lock acquired
    }

    if (m_socket == nullptr) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Socket not initialized", result);
        return result;
    } else {
        // Valid socket
    }

    eni::EniGenerator generator(*m_socket);
    Result<eni::EniDocument> gen_res = generator.generate(master_name);
    if (!gen_res.has_value()) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "EniGenerator failed to discover network", result);
        return result;
    } else {
        // Generation succeeded
    }

    Result<void> save_res = generator.save_to_xml_file(filepath, gen_res.value());
    if (!save_res.has_value()) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Failed to write XML file to: " + filepath, result);
        return result;
    } else {
        NamedBoolean::create("success", true, result);
        NamedString::create("filepath", filepath, result);
        return result;
    }
}

std::shared_ptr<NamedObject> EthercatMasterService::compareEni(std::shared_ptr<NamedObject> args) {
    std::shared_ptr<NamedObject> result = NamedObject::create("result");
    if (args == nullptr) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Missing arguments", result);
        return result;
    } else {
        // Args present
    }

    std::string filepath = "";
    if (std::shared_ptr<NamedObject> fp_node = args->getChild("filepath")) {
        if (std::shared_ptr<NamedString> s = std::dynamic_pointer_cast<NamedString>(fp_node)) {
            filepath = s->toString();
        } else {
            // Not a string
        }
    } else {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Missing 'filepath' parameter", result);
        return result;
    }

    eni::ComparisonLevel comp_level = eni::ComparisonLevel::ALL;
    if (std::shared_ptr<NamedObject> lvl_node = args->getChild("level")) {
        if (std::shared_ptr<NamedString> s = std::dynamic_pointer_cast<NamedString>(lvl_node)) {
            std::string l_str = s->toString();
            if (l_str == "topology") {
                comp_level = eni::ComparisonLevel::TOPOLOGY;
            } else if (l_str == "type") {
                comp_level = eni::ComparisonLevel::TYPE;
            } else if (l_str == "mapping") {
                comp_level = eni::ComparisonLevel::MAPPING;
            } else if (l_str == "hotconnect") {
                comp_level = eni::ComparisonLevel::HOT_CONNECT;
            } else {
                comp_level = eni::ComparisonLevel::ALL;
            }
        } else {
            // Not string
        }
    } else {
        // Default ALL
    }

    std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(500));
    if (!lock.owns_lock()) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Lock timeout", result);
        return result;
    } else {
        // Lock acquired
    }

    eni::EniDocument reference_doc;
    Result<void> load_res = reference_doc.load_from_file(filepath);
    if (!load_res.has_value()) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Failed to load reference ENI file: " + filepath, result);
        return result;
    } else {
        // Loaded
    }

    if (m_socket == nullptr) {
        NamedBoolean::create("success", false, result);
        NamedString::create("error", "Socket not initialized", result);
        return result;
    } else {
        // Valid socket
    }

    eni::ComparisonReport report = eni::EniComparator::compare_live_bus(reference_doc, *m_socket, comp_level);
    NamedBoolean::create("success", true, result);
    NamedBoolean::create("matched", report.matched, result);
    NamedInteger<std::uint32_t>::create("mismatchesCount", static_cast<std::uint32_t>(report.mismatches.size()), result);
    NamedString::create("summary", report.format_summary(), result);
    return result;
}

} // namespace resoem
