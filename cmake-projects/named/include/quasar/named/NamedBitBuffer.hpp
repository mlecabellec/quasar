/**
 * @file NamedBitBuffer.hpp
 * @brief Class for named bit-addressable buffers.
 */

#ifndef QUASAR_NAMED_NAMEDBITBUFFER_HPP
#define QUASAR_NAMED_NAMEDBITBUFFER_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/coretypes/BitBuffer.hpp"

namespace quasar::named {

/**
 * @class NamedBitBuffer
 * @brief A named object that manages a bit-addressable buffer.
 * 
 * This class inherits from NamedObject for hierarchy and coretypes::BitBuffer 
 * for bit-level operations.
 * 
 * **Compliance**:
 * - Fulfills [FE-0030.6] support for a named BitBuffer.
 */
class NamedBitBuffer : public NamedObject, public quasar::coretypes::BitBuffer {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~NamedBitBuffer() = default;

    /**
     * @brief Factory method to create a new NamedBitBuffer.
     * 
     * Fulfills [FE-0020.6] static method "create".
     * 
     * @param name The name of the bit buffer object.
     * @param bitCount The capacity of the buffer in bits.
     * @param parent Optional parent in the hierarchy.
     * @return A shared_ptr to the newly created NamedBitBuffer.
     */
    static std::shared_ptr<NamedBitBuffer> create(const std::string& name, size_t bitCount, std::shared_ptr<NamedObject> parent = nullptr);

    /**
     * @brief Creates a standalone copy of this NamedBitBuffer.
     * 
     * Fulfills [FE-0020.14] Utilities for copying parts of the tree.
     * 
     * @return A new NamedBitBuffer with the same name and bit content, but no hierarchy.
     */
    std::shared_ptr<NamedObject> clone() const override;

    /**
     * @brief Constructs a NamedBitBuffer instance.
     * @param name The name of the object.
     * @param bitCount The number of bits.
     */
    NamedBitBuffer(const std::string& name, size_t bitCount);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBITBUFFER_HPP
