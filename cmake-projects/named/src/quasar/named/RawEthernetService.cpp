#include "quasar/named/RawEthernetService.hpp"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <stdexcept>

namespace quasar::named {

RawEthernetService::RawEthernetService(const std::string& name)
    : NamedService(name) {}

RawEthernetService::~RawEthernetService() {
    // [CS-0010.44] Ensure socket and thread are fully stopped during destruction.
    stop();
}

std::shared_ptr<RawEthernetService> RawEthernetService::create(
    const std::string& name, std::shared_ptr<NamedObject> parent) {
    
    // [CS-0010.10] Use of new/delete is strictly forbidden. Use make_shared with enabler.
    struct make_shared_enabler : public RawEthernetService {
        explicit make_shared_enabler(const std::string& n) : RawEthernetService(n) {}
    };

    std::shared_ptr<RawEthernetService> self = std::make_shared<make_shared_enabler>(name);
    self->setSelf(self);
    if (parent != nullptr) {
        self->setParent(parent);
    }

    // [TSK-20260529-001.2] Initialize sub-tree properties and cached child nodes
    self->m_interfaceNameNode = NamedString::create("interfaceName", "lo", self);
    self->m_etherTypeNode = NamedInteger<uint16_t>::create("etherType", 0x0003, self); // ETH_P_ALL = 0x0003
    self->m_incomingFrameNode = NamedBuffer::create("incomingFrame", DEFAULT_BUFFER_SIZE, self);
    self->m_outgoingFrameNode = NamedBuffer::create("outgoingFrame", DEFAULT_BUFFER_SIZE, self);
    self->m_incomingTreeNode = NamedObject::create("incomingTree", self);

    // [CS-0010.6] Use weak_ptr to avoid circular reference in lambda captures.
    std::weak_ptr<RawEthernetService> weakSelf = self;

    // [TSK-20260529-001.4] Bind the exposed send() NamedMethod.
    NamedMethod::create("send", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
        (void)owner; (void)args;
        if (std::shared_ptr<RawEthernetService> servicePtr = weakSelf.lock()) {
            servicePtr->sendFrame();
        }
        return nullptr;
    }, self);

    // Override the "run" lifecycle hook method. NamedService background loop will call this.
    NamedMethod::create("run", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
        (void)owner; (void)args;
        if (std::shared_ptr<RawEthernetService> servicePtr = weakSelf.lock()) {
            servicePtr->performPoll();
        }
        return nullptr;
    }, self);

    // Set default cycle time for raw packet polling loop
    self->setCycleTime(std::chrono::milliseconds(DEFAULT_CYCLE_TIME_MS));

    return self;
}

void RawEthernetService::start() {
    // [CS-0010.44] Safely acquire state-modifying lock to prevent start/stop race conditions.
    std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring RawEthernetService lock in start()");
    }

    if (m_running) {
        return;
    }

    // Read bound configurations from properties children nodes.
    if (m_interfaceNameNode == nullptr || m_etherTypeNode == nullptr) {
        throw std::runtime_error("Properties nodes not properly initialized");
    }

    std::string iface = m_interfaceNameNode->toString();
    uint16_t ethType = m_etherTypeNode->value();

    // Initialize physical AF_PACKET raw socket
    openRawSocket(iface, ethType);

    // Start background NamedService thread execution loop
    NamedService::start();
}

void RawEthernetService::stop() {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring RawEthernetService lock in stop()");
    }

    if (!m_running) {
        return;
    }

    // Stop background NamedService thread execution
    NamedService::stop();

    // Release raw socket
    closeRawSocket();
}

void RawEthernetService::openRawSocket(const std::string& iface, uint16_t ethType) {
    // [TSK-20260529-001.1] Create raw AF_PACKET OS socket
    m_sockFd = ::socket(AF_PACKET, SOCK_RAW, htons(ethType));
    if (m_sockFd < 0) {
        throw std::runtime_error("Failed to create raw socket: " + std::string(std::strerror(errno)));
    }

    // Fetch network interface index via ioctl
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    const std::size_t copyLen = std::min(iface.size(), static_cast<std::size_t>(IFNAMSIZ - 1));
    std::memcpy(ifr.ifr_name, iface.c_str(), copyLen);
    ifr.ifr_name[copyLen] = '\0';

    if (::ioctl(m_sockFd, SIOCGIFINDEX, &ifr) < 0) {
        ::close(m_sockFd);
        m_sockFd = -1;
        throw std::runtime_error("Failed to get interface index for " + iface + ": " + std::string(std::strerror(errno)));
    }
    int ifIndex = ifr.ifr_ifindex;

    // Bind raw socket to specified interface
    struct sockaddr_ll sll;
    std::memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifIndex;
    sll.sll_protocol = htons(ethType);

    if (::bind(m_sockFd, reinterpret_cast<struct sockaddr*>(&sll), sizeof(sll)) < 0) {
        ::close(m_sockFd);
        m_sockFd = -1;
        throw std::runtime_error("Failed to bind raw socket to " + iface + ": " + std::string(std::strerror(errno)));
    }

    // [CS-0040.2] Set non-blocking mode on raw socket
    int flags = ::fcntl(m_sockFd, F_GETFL, 0);
    if (flags < 0 || ::fcntl(m_sockFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        ::close(m_sockFd);
        m_sockFd = -1;
        throw std::runtime_error("Failed to set non-blocking flags on raw socket: " + std::string(std::strerror(errno)));
    }
}

void RawEthernetService::closeRawSocket() {
    if (m_sockFd >= 0) {
        ::close(m_sockFd);
        m_sockFd = -1;
    }
}

void RawEthernetService::performPoll() {
    if (m_sockFd < 0) {
        return;
    }

    std::vector<uint8_t> buffer(65536);
    int packetCount = 0;

    // [CS-0010.37] Infinite loop protection with hard limit
    while (m_running && packetCount < MAX_PACKETS_PER_CYCLE) {
        ssize_t received = ::recv(m_sockFd, buffer.data(), buffer.size(), MSG_DONTWAIT);
        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // No more incoming packets in queue
            }
            if (errno == EINTR) {
                continue; // Retry if interrupted by signal
            }
            // Real socket error
            break;
        }

        if (received > 0) {
            packetCount++;

            // Truncate payload to bytes received
            std::vector<uint8_t> packetData(buffer.begin(), buffer.begin() + received);

            // [TSK-20260529-001.2] Update incomingFrame buffer payload
            if (m_incomingFrameNode != nullptr) {
                m_incomingFrameNode->setBufferData(packetData);

                // [TSK-20260529-001.3] Trigger tree transformer parsing and materialization
                std::vector<std::shared_ptr<NamedObject>> materialized = m_transformer.transform(m_incomingFrameNode);

                std::unique_lock<std::recursive_timed_mutex> treeLock(m_mutex, std::chrono::seconds(1));
                if (treeLock.owns_lock() && m_incomingTreeNode != nullptr) {
                    // Detach all old children nodes
                    std::list<std::shared_ptr<NamedObject>> oldChildren = m_incomingTreeNode->getChildren();
                    for (const std::shared_ptr<NamedObject>& child : oldChildren) {
                        if (child != nullptr) {
                            child->setParent(nullptr);
                        }
                    }

                    // Attach newly materialized hierarchical child nodes under incomingTree
                    for (const std::shared_ptr<NamedObject>& node : materialized) {
                        if (node != nullptr) {
                            node->setParent(m_incomingTreeNode);
                        }
                    }

                    // Signal structural update version
                    incrementTreeVersion();
                }
            }
        }
    }

    // Breach warning check (loop overflow prevention check)
    if (packetCount >= MAX_PACKETS_PER_CYCLE) {
        std::cerr << "Warning: RawEthernetService [" << getName() 
                  << "] reached hard packet poll iteration limit: yielding cycle." << std::endl;
    }
}

void RawEthernetService::sendFrame() {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring RawEthernetService lock in sendFrame()");
    }

    if (m_sockFd < 0) {
        throw std::runtime_error("Raw socket not open or bound");
    }

    if (m_outgoingFrameNode == nullptr) {
        throw std::runtime_error("Outgoing frame buffer not initialized");
    }

    // Retrieve composed raw frame payload
    std::vector<uint8_t> data = m_outgoingFrameNode->toVector();
    if (data.empty()) {
        return;
    }

    // [TSK-20260529-001.4] Transmit raw payload via bound AF_PACKET descriptor
    ssize_t sent = ::send(m_sockFd, data.data(), data.size(), 0);
    if (sent < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            throw std::runtime_error("Failed to send raw frame: " + std::string(std::strerror(errno)));
        }
    }
}

void RawEthernetService::addRule(const traversal::TransformationRule& rule) {
    m_transformer.addRule(rule);
}

std::shared_ptr<NamedString> RawEthernetService::getInterfaceNameNode() const {
    return m_interfaceNameNode;
}

std::shared_ptr<NamedInteger<uint16_t>> RawEthernetService::getEtherTypeNode() const {
    return m_etherTypeNode;
}

std::shared_ptr<NamedBuffer> RawEthernetService::getIncomingFrameNode() const {
    return m_incomingFrameNode;
}

std::shared_ptr<NamedBuffer> RawEthernetService::getOutgoingFrameNode() const {
    return m_outgoingFrameNode;
}

std::shared_ptr<NamedObject> RawEthernetService::getIncomingTreeNode() const {
    return m_incomingTreeNode;
}

} // namespace quasar::named
