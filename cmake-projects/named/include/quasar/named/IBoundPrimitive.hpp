#ifndef QUASAR_NAMED_IBOUNDPRIMITIVE_HPP
#define QUASAR_NAMED_IBOUNDPRIMITIVE_HPP

#include <cstddef>

namespace quasar {
namespace named {

/**
 * @brief Interface for pseudo-primitives that are bound to a parent buffer's memory.
 *
 * An object implementing IBoundPrimitive does not own its data payload (e.g., m_value).
 * Instead, it maps directly to an offset within its structural parent's memory buffer.
 */
class IBoundPrimitive {
public:
    virtual ~IBoundPrimitive() = default;

    /**
     * @brief Returns whether this object is currently bound to a parent buffer.
     * @return true if bound, false if it owns its own memory.
     */
    virtual bool isBound() const = 0;

    /**
     * @brief Returns the offset within the parent buffer where this primitive's data begins.
     * @return The offset in bytes.
     */
    virtual std::size_t getBoundOffset() const = 0;

    /**
     * @brief Returns the length of this primitive's data.
     * @return The length in bytes.
     */
    virtual std::size_t getBoundLength() const = 0;
};

} // namespace named
} // namespace quasar

#endif // QUASAR_NAMED_IBOUNDPRIMITIVE_HPP
