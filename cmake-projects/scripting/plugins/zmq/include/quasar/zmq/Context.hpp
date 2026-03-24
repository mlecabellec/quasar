/**
 * @file Context.hpp
 * @brief RAII wrapper for ZeroMQ context.
 */

#ifndef QUASAR_ZMQ_CONTEXT_HPP
#define QUASAR_ZMQ_CONTEXT_HPP

#include <zmq.h>
#include <memory>
#include <stdexcept>

namespace quasar::zmq {

/**
 * @brief Functor for terminating a ZeroMQ context.
 */
struct ContextDeleter {
    void operator()(void* context) const {
        if (context) {
            zmq_ctx_term(context);
        }
    }
};

/**
 * @class Context
 * @brief RAII wrapper for ZeroMQ context.
 * 
 * **Compliance**:
 * - Fulfills [CS-0010.10] Use of new or delete keywords is forbidden.
 * - Fulfills [CS-0010.33] Contained in a namespace.
 */
class Context {
public:
    /**
     * @brief Constructs a new ZeroMQ context.
     * @throws std::runtime_error if context creation fails.
     */
    Context() {
        void* ctx = zmq_ctx_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create ZeroMQ context");
        }
        m_ctx = std::unique_ptr<void, ContextDeleter>(ctx);
    }

    /** @brief Destructor. */
    ~Context() = default;

    /** @brief Returns the raw ZeroMQ context pointer. */
    void* get() const { return m_ctx.get(); }

    /** @brief Disallow copy. */
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    /** @brief Allow move. */
    Context(Context&&) = default;
    Context& operator=(Context&&) = default;

private:
    /** @brief Unique pointer to the context. */
    std::unique_ptr<void, ContextDeleter> m_ctx;
};

} // namespace quasar::zmq

#endif // QUASAR_ZMQ_CONTEXT_HPP
