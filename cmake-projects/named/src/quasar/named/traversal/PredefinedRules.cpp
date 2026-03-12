#include "quasar/named/traversal/PredefinedRules.hpp"

namespace quasar::named::traversal {

TransformationRule PredefinedRules::sliceBuffer(
    const std::string& targetBufferName,
    const std::vector<SliceDefinition>& slices,
    CopyPolicy policy,
    int priority) 
{
    TransformPredicate pred = [targetBufferName](const TransformContext& ctx) {
        auto node = ctx.getNode();
        return node->getName() == targetBufferName && node->as<NamedBuffer>() != nullptr;
    };

    TransformGenerator gen = [slices, policy](const TransformContext& ctx) -> std::vector<std::shared_ptr<NamedObject>> {
        auto originalBuffer = ctx.getNode()->as<NamedBuffer>();
        std::vector<std::shared_ptr<NamedObject>> result;

        for (const auto& sliceDef : slices) {
            if (policy == CopyPolicy::SHARE) {
                auto slice = NamedBufferSlice::create(sliceDef.name, originalBuffer, sliceDef.offset, sliceDef.length);
                result.push_back(slice);
            } else {
                std::vector<uint8_t> duplicateData;
                auto origBytes = originalBuffer->toVector();
                if (sliceDef.offset < origBytes.size()) {
                    auto end = std::min(sliceDef.offset + sliceDef.length, origBytes.size());
                    duplicateData.assign(origBytes.begin() + sliceDef.offset, origBytes.begin() + end);
                }
                auto newBuffer = NamedBuffer::create(sliceDef.name, duplicateData);
                result.push_back(newBuffer);
            }
        }

        return result;
    };

    return TransformationRule(std::move(pred), std::move(gen), priority);
}

} // namespace quasar::named::traversal
