#ifndef QUASAR_NAMED_RAWETHERNETSERVICE_HPP
#define QUASAR_NAMED_RAWETHERNETSERVICE_HPP

#include "quasar/named/NamedService.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/traversal/Transformer.hpp"

namespace quasar::named {

/**
 * @class RawEthernetService
 * @brief NamedService that binds on a physical or virtual network interface in raw mode.
 * 
 * Supports capturing raw ethernet frames into a NamedBuffer, materializing them into
 * a named tree using custom transformers, and composing/sending frames using pseudo-primitives.
 * 
 * **Compliance**:
 * - Fulfills [FE-0240.1] Raw Packet Networking.
 * - Fulfills [CS-0010.45] Doxygen comments on all members.
 * - Fulfills [CS-0040] Strategic Constant Management.
 * - Fulfills [CS-0060.1] Semantic [[nodiscard]] on observers.
 * 
 * @feature TSK-20260529-001 Raw Ethernet Socket Service.
 * @exposed
 */
class RawEthernetService : public NamedService {
public:
    /** @brief Strategic timeout constants. */
    static constexpr int DEFAULT_TIMEOUT_MS = 100;
    static constexpr int DEFAULT_CYCLE_TIME_MS = 10;
    static constexpr int DEFAULT_BUFFER_SIZE = 1518;
    static constexpr int MAX_PACKETS_PER_CYCLE = 100;

    /**
     * @brief Factory method to create a new RawEthernetService.
     * @param name Name of the service.
     * @param parent Optional parent.
     * @return Shared pointer to the new RawEthernetService.
     */
    [[nodiscard]] static std::shared_ptr<RawEthernetService> create(
        const std::string& name, std::shared_ptr<NamedObject> parent = nullptr);

    /**
     * @brief Destructor. Closes any open sockets.
     */
    virtual ~RawEthernetService() override;

    /**
     * @brief Starts the service. Opens the raw socket.
     */
    void start() override;

    /**
     * @brief Stops the service. Closes the socket.
     */
    void stop() override;

    /**
     * @brief Composes and transmits the raw byte payload currently in m_outgoingFrame.
     * @feature [TSK-20260529-001.4] Outgoing Composition & Method Dispatch.
     */
    void sendFrame();

    /**
     * @brief Adds a transformation rule for incoming frame parsing.
     * @param rule The transformation rule to register.
     * @feature [TSK-20260529-001.3] Materialization rules.
     */
    void addRule(const traversal::TransformationRule& rule);

    /**
     * @brief Getter for the interfaceName child node.
     * @return Shared pointer to the NamedString node.
     */
    [[nodiscard]] std::shared_ptr<NamedString> getInterfaceNameNode() const;

    /**
     * @brief Getter for the etherType child node.
     * @return Shared pointer to the NamedInteger node.
     */
    [[nodiscard]] std::shared_ptr<NamedInteger<uint16_t>> getEtherTypeNode() const;

    /**
     * @brief Getter for the incomingFrame child node.
     * @return Shared pointer to the NamedBuffer node.
     */
    [[nodiscard]] std::shared_ptr<NamedBuffer> getIncomingFrameNode() const;

    /**
     * @brief Getter for the outgoingFrame child node.
     * @return Shared pointer to the NamedBuffer node.
     */
    [[nodiscard]] std::shared_ptr<NamedBuffer> getOutgoingFrameNode() const;

    /**
     * @brief Getter for the incomingTree child container node.
     * @return Shared pointer to the NamedObject container.
     */
    [[nodiscard]] std::shared_ptr<NamedObject> getIncomingTreeNode() const;

    /**
     * @brief Returns "RawEthernetService".
     * @return String identifier.
     */
    [[nodiscard]] std::string getType() const override { return "RawEthernetService"; }

protected:
    /**
     * @brief Protected constructor.
     * @param name Name of the service.
     */
    RawEthernetService(const std::string& name);

    /**
     * @brief Core loop iteration callback. Polled by the background thread.
     */
    void performPoll();

    /**
     * @brief Open raw socket bound to specified interface and ethertype.
     */
    void openRawSocket(const std::string& iface, uint16_t ethType);

    /**
     * @brief Close raw socket.
     */
    void closeRawSocket();

protected:
    /** @brief The open raw socket file descriptor (-1 if closed). */
    int m_sockFd{-1};

    /** @brief Child node caching. */
    std::shared_ptr<NamedString> m_interfaceNameNode;
    std::shared_ptr<NamedInteger<uint16_t>> m_etherTypeNode;
    std::shared_ptr<NamedBuffer> m_incomingFrameNode;
    std::shared_ptr<NamedBuffer> m_outgoingFrameNode;
    std::shared_ptr<NamedObject> m_incomingTreeNode;

    /** @brief The Transformer engine used for incoming packets parsing. */
    traversal::Transformer m_transformer;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_RAWETHERNETSERVICE_HPP
