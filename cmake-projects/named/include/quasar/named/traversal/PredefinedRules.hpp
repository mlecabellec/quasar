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
};

class PredefinedRules {
public:
    /**
     * @brief Creates a rule that matches a specific NamedBuffer and splits it into slices.
     * @param targetBufferName The name of the buffer to match.
     * @param slices The definitions of the slices to create.
     * @param policy Specifies whether the new slices should DUPLICATE or SHARE the underlying buffer.
     * @param priority Rule priority.
     * @return A TransformationRule ready to be added to a Transformer.
     */
    static TransformationRule sliceBuffer(
        const std::string& targetBufferName,
        const std::vector<SliceDefinition>& slices,
        CopyPolicy policy = CopyPolicy::SHARE,
        int priority = 10);

    /**
     * @brief Creates a rule that automatically expands a buffer into a tree of pseudo-primitives.
     * 
     * Fulfills [TSK-20260311-001.7] Explicit Cast Transformations.
     * 
     * @param targetBufferName Buffer to match.
     * @param mappings List of field mappings (name, type, offset).
     * @param priority Rule priority.
     * @return A TransformationRule.
     */
    static TransformationRule castToStructure(
        const std::string& targetBufferName,
        const std::vector<FieldMapping>& mappings,
        int priority = 20);

    /**
     * @brief Creates a rule that extracts an integer from a buffer or slice into a new NamedInteger.
     */
    template <typename T>
    static TransformationRule extractIntegerRule(
        const std::string& targetBufferName,
        const std::string& newIntName,
        size_t offset,
        int priority = 10) 
    {
        TransformPredicate pred = [targetBufferName](const TransformContext& ctx) {
            std::shared_ptr<NamedObject> node = ctx.getNode();
            return node->getName() == targetBufferName && 
                   (node->as<NamedBuffer>() != nullptr || node->as<NamedBufferSlice>() != nullptr);
        };

        TransformGenerator gen = [newIntName, offset](const TransformContext& ctx, Transformer& transformer) -> std::vector<std::shared_ptr<NamedObject>> {
            (void)transformer;
            std::shared_ptr<NamedObject> node = ctx.getNode();
            std::shared_ptr<NamedInteger<T>> intNode = NamedInteger<T>::create(newIntName, 0);

            if (std::shared_ptr<NamedBuffer> buf = node->as<NamedBuffer>()) {
                // Bind directly to NamedBuffer.
                intNode->bind(buf, offset);
            } else if (std::shared_ptr<NamedBufferSlice> slice = node->as<NamedBufferSlice>()) {
                // Bind to the underlying buffer of the slice, adjusting offset.
                std::shared_ptr<quasar::coretypes::Buffer> parentBuf = slice->quasar::coretypes::BufferSlice::getParent();
                if (parentBuf) {
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
