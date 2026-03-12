#ifndef QUASAR_NAMED_COPYPOLICY_HPP
#define QUASAR_NAMED_COPYPOLICY_HPP

namespace quasar {
namespace named {

/**
 * @brief Defines how memory and ownership are handled during object duplication.
 */
enum class CopyPolicy {
    /**
     * @brief A full, independent deep copy. Leaf payloads are fully duplicated.
     */
    DUPLICATE,

    /**
     * @brief A shared-memory copy. Structural nodes are duplicated, but leaf
     * payloads (like buffers) are shared via aliasing or slicing.
     */
    SHARE
};

} // namespace named
} // namespace quasar

#endif // QUASAR_NAMED_COPYPOLICY_HPP
