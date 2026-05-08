/**
 * @file Socket.hpp
 * @brief RAII wrapper for ZeroMQ socket.
 */

#ifndef QUASAR_ZMQ_SOCKET_HPP
#define QUASAR_ZMQ_SOCKET_HPP

#include "quasar/zmq/Context.hpp"
#include "quasar/named/Serialization.hpp"
#include <zmq.h>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <optional>

namespace quasar::zmq {

/**
 * @brief Functor for closing a ZeroMQ socket.
 */
struct SocketDeleter {
    void operator()(void* socket) const {
        if (socket != nullptr) {
            zmq_close(socket);
        }
    }
};

/**
 * @class Socket
 * @brief RAII wrapper for ZeroMQ socket.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260311-004.1.2] Support standard ZMQ operations.
 * - Fulfills [CS-0010.10] No raw new/delete.
 * - Fulfills [CS-0010.33] Contained in a namespace.
 */
class Socket {
public:
    /**
     * @brief Constructs a new ZeroMQ socket.
     * @param context The ZeroMQ context.
     * @param type The socket type (e.g., ZMQ_PUB, ZMQ_SUB).
     * @throws std::runtime_error if socket creation fails.
     */
    Socket(const Context& context, int type) {
        void* socket = zmq_socket(context.get(), type);
        if (!socket) {
            throw std::runtime_error(std::string("Failed to create ZeroMQ socket: ") + std::strerror(errno));
        }
        m_socket = std::unique_ptr<void, SocketDeleter>(socket);
    }

    /** @brief Destructor. */
    ~Socket() = default;

    /**
     * @brief Binds the socket to an endpoint.
     * @param endpoint The endpoint string (e.g., "tcp://*:5555").
     * @throws std::runtime_error if bind fails.
     */
    void bind(const std::string& endpoint) {
        if (zmq_bind(m_socket.get(), endpoint.c_str()) != 0) {
            throw std::runtime_error(std::string("Failed to bind ZeroMQ socket: ") + std::strerror(errno));
        }
    }

    /**
     * @brief Connects the socket to an endpoint.
     * @param endpoint The endpoint string.
     * @throws std::runtime_error if connect fails.
     */
    void connect(const std::string& endpoint) {
        if (zmq_connect(m_socket.get(), endpoint.c_str()) != 0) {
            throw std::runtime_error(std::string("Failed to connect ZeroMQ socket: ") + std::strerror(errno));
        }
    }

    /**
     * @brief Subscribes to a topic (for ZMQ_SUB).
     * @param topic The topic string.
     * @throws std::runtime_error if subscription fails.
     */
    void subscribe(const std::string& topic) {
        if (zmq_setsockopt(m_socket.get(), ZMQ_SUBSCRIBE, topic.data(), topic.size()) != 0) {
            throw std::runtime_error(std::string("Failed to subscribe: ") + std::strerror(errno));
        }
    }

    /**
     * @brief Unsubscribes from a topic.
     * @param topic The topic string.
     * @throws std::runtime_error if unsubscription fails.
     */
    void unsubscribe(const std::string& topic) {
        if (zmq_setsockopt(m_socket.get(), ZMQ_UNSUBSCRIBE, topic.data(), topic.size()) != 0) {
            throw std::runtime_error(std::string("Failed to unsubscribe: ") + std::strerror(errno));
        }
    }

    /**
     * @brief Sends a raw string message.
     * @param msg The message string.
     * @param flags ZeroMQ flags (e.g., ZMQ_DONTWAIT, ZMQ_SNDMORE). Default is 0.
     * @throws std::runtime_error if transmission fails.
     */
    void send(const std::string& msg, int flags = 0) {
        if (zmq_send(m_socket.get(), msg.data(), msg.size(), flags) < 0) {
            throw std::runtime_error(std::string("Failed to send: ") + std::strerror(errno));
        }
    }

    /**
     * @brief Receives a raw string message.
     * @param flags ZeroMQ flags (e.g., ZMQ_DONTWAIT). Default is 0.
     * @return The received string, or std::nullopt if ZMQ_DONTWAIT is used and no message is ready.
     * @throws std::runtime_error if receiving fails.
     */
    std::optional<std::string> receive(int flags = 0) {
        zmq_msg_t part;
        zmq_msg_init(&part);
        if (zmq_msg_recv(&part, m_socket.get(), flags) < 0) {
            zmq_msg_close(&part);
            if (errno == EAGAIN) return std::nullopt;
            throw std::runtime_error(std::string("Failed to receive: ") + std::strerror(errno));
        }
        std::string result(static_cast<const char*>(zmq_msg_data(&part)), zmq_msg_size(&part));
        zmq_msg_close(&part);
        return result;
    }

    /**
     * @brief Publishes a NamedObject tree to a topic.
     * 
     * Fulfills [TSK-20260311-004.3.1] publishTree method.
     * Fulfills [TSK-20260311-004.4.2] Multi-part messages (Topic + Tree).
     * 
     * @param topic The topic string.
     * @param root The root of the tree to publish.
     * @throws std::runtime_error if transmission fails.
     */
    void publishTree(const std::string& topic, const std::shared_ptr<quasar::named::NamedObject>& root) {
        // Step 1: Serialize topic envelope
        if (zmq_send(m_socket.get(), topic.data(), topic.size(), ZMQ_SNDMORE) < 0) {
            throw std::runtime_error(std::string("Failed to send topic: ") + std::strerror(errno));
        }

        // Step 2: Serialize tree to binary (BSON) inside unique_ptr to own the underlying buffer
        std::unique_ptr<std::vector<uint8_t>> buffer = std::make_unique<std::vector<uint8_t>>(
            quasar::named::serialization::toBinary(root)
        );

        // Step 3: Zero-copy send by transferring ownership to ZMQ
        zmq_msg_t msg;
        zmq_msg_init_data(&msg, buffer->data(), buffer->size(),
            [](void*, void* hint) {
                // Recover the unique_ptr and let it go out of scope to properly deallocate without 'delete' constraint
                std::unique_ptr<std::vector<uint8_t>> p(static_cast<std::vector<uint8_t>*>(hint));
            },
            buffer.get()
        );
        buffer.release(); // ZMQ now owns it via the callback

        if (zmq_msg_send(&msg, m_socket.get(), 0) < 0) {
            zmq_msg_close(&msg);
            throw std::runtime_error(std::string("Failed to send tree data: ") + std::strerror(errno));
        }
    }

    /**
     * @brief Receives a NamedObject tree.
     * 
     * Fulfills [TSK-20260311-004.3.2] receiveTree method.
     * 
     * @param flags ZeroMQ flags (e.g., ZMQ_DONTWAIT). Default is 0.
     * @return Shared pointer to the reconstructed tree, or nullptr if DONTWAIT is used and no message is ready.
     * @throws std::runtime_error if receiving or parsing fails.
     */
    std::shared_ptr<quasar::named::NamedObject> receiveTree(int flags = 0) {
        zmq_msg_t part;
        
        // Step 1: Receive topic
        zmq_msg_init(&part);
        if (zmq_msg_recv(&part, m_socket.get(), flags) < 0) {
            zmq_msg_close(&part);
            if (errno == EAGAIN) return nullptr;
            throw std::runtime_error(std::string("Failed to receive topic part: ") + std::strerror(errno));
        }
        
        if (!zmq_msg_more(&part)) {
            zmq_msg_close(&part);
            throw std::runtime_error("Received single-part message, expected multi-part (topic + tree)");
        }
        zmq_msg_close(&part);

        // Step 2: Receive binary tree payload
        zmq_msg_init(&part);
        if (zmq_msg_recv(&part, m_socket.get(), flags) < 0) {
            zmq_msg_close(&part);
            if (errno == EAGAIN) return nullptr;
            throw std::runtime_error(std::string("Failed to receive tree part: ") + std::strerror(errno));
        }

        // Step 3: Deserialize
        const uint8_t* data = static_cast<const uint8_t*>(zmq_msg_data(&part));
        size_t size = zmq_msg_size(&part);
        std::vector<uint8_t> binary(data, data + size);
        zmq_msg_close(&part);

        return quasar::named::serialization::fromBinary(binary);
    }

    /** @brief Returns the raw ZeroMQ socket pointer. */
    void* get() const { return m_socket.get(); }

    /** @brief Disallow copy. */
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    /** @brief Allow move. */
    Socket(Socket&&) = default;
    Socket& operator=(Socket&&) = default;

private:
    /** @brief Unique pointer to the socket. */
    std::unique_ptr<void, SocketDeleter> m_socket;
};

} // namespace quasar::zmq

#endif // QUASAR_ZMQ_SOCKET_HPP
