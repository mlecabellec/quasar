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
            auto node = ctx.getNode();
            return node->getName() == targetBufferName && 
                   (node->as<NamedBuffer>() != nullptr || node->as<NamedBufferSlice>() != nullptr);
        };

        TransformGenerator gen = [newIntName, offset](const TransformContext& ctx) -> std::vector<std::shared_ptr<NamedObject>> {
            auto node = ctx.getNode();
            T value = 0;
            if (auto buf = node->as<NamedBuffer>()) {
                if (offset + sizeof(T) <= buf->size()) {
                    value = static_cast<T>(buf->readInt(offset));
                }
            } else if (auto slice = node->as<NamedBufferSlice>()) {
                if (offset + sizeof(T) <= slice->size()) {
                    auto parentBuf = slice->quasar::coretypes::BufferSlice::getParent();
                    if (parentBuf) {
                        value = static_cast<T>(parentBuf->readInt(offset + slice->getOffset()));
                    }
                }
            }
            auto intNode = NamedInteger<T>::create(newIntName, value);
            return {intNode};
        };

        return TransformationRule(std::move(pred), std::move(gen), priority);
    }
};

} // namespace quasar::named::traversal

#endif // QUASAR_NAMED_TRAVERSAL_PREDEFINEDRULES_HPP
