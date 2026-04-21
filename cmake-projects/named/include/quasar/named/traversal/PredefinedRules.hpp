#ifndef QUASAR_NAMED_TRAVERSAL_PREDEFINEDRULES_HPP
#define QUASAR_NAMED_TRAVERSAL_PREDEFINEDRULES_HPP

#include "quasar/named/traversal/TransformationRule.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedBufferSlice.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/CopyPolicy.hpp"
#include <string>
#include <vector>

namespace quasar::named::traversal {

struct SliceDefinition {
    std::string name;
    size_t offset;
    size_t length;
};

struct FieldMapping {
    std::string name;
    std::string type;
    size_t offset;
    quasar::coretypes::Endianness endian = quasar::coretypes::Endianness::BigEndian;
};

/**
 * @class PredefinedRules
 * @brief Utility class providing factory methods for common transformation rules.
 * 
 * Includes rules for slicing buffers, casting memory regions to structured 
 * hierarchies, and extracting specific primitives.
 * 
 * @feature [TSK-20260311-001.7] Explicit Cast Transformations.
 */
class PredefinedRules {
public:
    /**
     * @brief Creates a rule that matches a specific NamedBuffer and splits it into slices.
     * 
     * This rule is essential for decomposing linear byte streams into
     * logical components.
     * 
     * @param targetBufferName The name of the buffer to match.
     * @param slices The definitions of the slices to create.
     * @param policy Specifies whether the new slices should DUPLICATE or SHARE the underlying buffer.
     * @param priority Rule priority.
     * @return A TransformationRule ready to be added to a Transformer.
     * @feature [TSK-20260311-001.5] Multi-Slicing Hierarchies.
     */
    static TransformationRule sliceBuffer(
        const std::string& targetBufferName,
        const std::vector<SliceDefinition>& slices,
        CopyPolicy policy = CopyPolicy::SHARE,
        int priority = 10);

    /**
     * @brief Creates a rule that automatically expands a buffer into a tree of pseudo-primitives.
     * 
     * Reinterprets contiguous memory ranges within a NamedBuffer or 
     * NamedBufferSlice directly into specialized NamedObject primitives 
     * (e.g., NamedInteger, NamedFloatingPoint).
     * 
     * Fulfills [TSK-20260311-001.7] Explicit Cast Transformations.
     * 
     * @param targetBufferName Buffer to match.
     * @param mappings List of field mappings (name, type, offset, endian).
     * @param priority Rule priority.
     * @return A TransformationRule.
     * @feature [TSK-20260311-001.6] Buffer-to-Primitive Binding.
     */
    static TransformationRule castToStructure(
        const std::string& targetBufferName,
        const std::vector<FieldMapping>& mappings,
        int priority = 20);

    /**
     * @brief Creates a rule that extracts an integer from a buffer or slice into a new NamedInteger.
     * 
     * @tparam T The integer type to extract.
     * @param targetBufferName Name of the source buffer.
     * @param newIntName Name of the output integer node.
     * @param offset Byte offset in the buffer.
     * @param endian Endianness of the data in the buffer.
     * @param priority Rule priority.
     * @return A configured TransformationRule.
     */
    template <typename T>
    static TransformationRule extractIntegerRule(
        const std::string& targetBufferName,
        const std::string& newIntName,
        size_t offset,
        quasar::coretypes::Endianness endian = quasar::coretypes::Endianness::BigEndian,
        int priority = 10) 
    {
        TransformPredicate pred = [targetBufferName](const TransformContext& ctx) {
            std::shared_ptr<NamedObject> node = ctx.getNode();
            return node->getName() == targetBufferName && 
                   (node->as<NamedBuffer>() != nullptr || node->as<NamedBufferSlice>() != nullptr);
        };

        TransformGenerator gen = [newIntName, offset, endian](const TransformContext& ctx, Transformer& transformer) -> std::vector<std::shared_ptr<NamedObject>> {
            (void)transformer;
            std::shared_ptr<NamedObject> node = ctx.getNode();
            std::shared_ptr<NamedInteger<T>> intNode = NamedInteger<T>::create(newIntName, 0);

            if (std::shared_ptr<NamedBuffer> buf = node->as<NamedBuffer>()) {
                // Bind directly to NamedBuffer.
                intNode->setEndianness(endian);
                intNode->bind(buf, offset);
            } else if (std::shared_ptr<NamedBufferSlice> slice = node->as<NamedBufferSlice>()) {
                // Bind to the underlying buffer of the slice, adjusting offset.
                std::shared_ptr<quasar::coretypes::Buffer> parentBuf = slice->quasar::coretypes::BufferSlice::getParent();
                if (parentBuf) {
                    intNode->setEndianness(endian);
                    intNode->bind(parentBuf, offset + slice->getOffset());
                }
            }
            return {intNode};
        };

        return TransformationRule(std::move(pred), std::move(gen), priority);
    }
};

} // namespace quasar::named::traversal

#endif // QUASAR_NAMED_TRAVERSAL_PREDEFINEDRULES_HPP
