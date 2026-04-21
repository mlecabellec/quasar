#include "quasar/named/traversal/PredefinedRules.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedBoolean.hpp"

namespace quasar::named::traversal {

TransformationRule PredefinedRules::sliceBuffer(
    const std::string& targetBufferName,
    const std::vector<SliceDefinition>& slices,
    CopyPolicy policy,
    int priority) 
{
    TransformPredicate pred = [targetBufferName](const TransformContext& ctx) {
        std::shared_ptr<NamedObject> node = ctx.getNode();
        return node->getName() == targetBufferName && node->as<NamedBuffer>() != nullptr;
    };

    TransformGenerator gen = [slices, policy](const TransformContext& ctx, Transformer& transformer) -> std::vector<std::shared_ptr<NamedObject>> {
        (void)transformer; // Unused for this specific rule
        std::shared_ptr<NamedBuffer> originalBuffer = ctx.getNode()->as<NamedBuffer>();
        std::vector<std::shared_ptr<NamedObject>> result;

        for (const SliceDefinition& sliceDef : slices) {
            if (policy == CopyPolicy::SHARE) {
                std::shared_ptr<NamedBufferSlice> slice = NamedBufferSlice::create(sliceDef.name, originalBuffer, sliceDef.offset, sliceDef.length);
                result.push_back(slice);
            } else {
                std::vector<uint8_t> duplicateData;
                std::vector<uint8_t> origBytes = originalBuffer->toVector();
                if (sliceDef.offset < origBytes.size()) {
                    size_t end = std::min(sliceDef.offset + sliceDef.length, origBytes.size());
                    duplicateData.assign(origBytes.begin() + sliceDef.offset, origBytes.begin() + end);
                }
                std::shared_ptr<NamedBuffer> newBuffer = NamedBuffer::create(sliceDef.name, duplicateData);
                result.push_back(newBuffer);
            }
        }

        return result;
    };

    return TransformationRule(std::move(pred), std::move(gen), priority);
}

TransformationRule PredefinedRules::castToStructure(
    const std::string& targetBufferName,
    const std::vector<FieldMapping>& mappings,
    int priority)
{
    TransformPredicate pred = [targetBufferName](const TransformContext& ctx) {
        std::shared_ptr<NamedObject> node = ctx.getNode();
        return node->getName() == targetBufferName && 
               (node->as<NamedBuffer>() != nullptr || node->as<NamedBufferSlice>() != nullptr);
    };

    TransformGenerator gen = [mappings](const TransformContext& ctx, Transformer& transformer) -> std::vector<std::shared_ptr<NamedObject>> {
        (void)transformer;
        std::shared_ptr<NamedObject> node = ctx.getNode();
        std::shared_ptr<quasar::coretypes::Buffer> parentBuf;
        size_t baseOffset = 0;

        if (std::shared_ptr<NamedBuffer> buf = node->as<NamedBuffer>()) {
            parentBuf = buf;
        } else if (std::shared_ptr<NamedBufferSlice> slice = node->as<NamedBufferSlice>()) {
            parentBuf = slice->quasar::coretypes::BufferSlice::getParent();
            baseOffset = slice->getOffset();
        }

        if (!parentBuf) return {node}; // Should not happen given predicate

        // Return a copy of the buffer itself, but we will attach children to it.
        std::shared_ptr<NamedObject> container = node->clone(CopyPolicy::SHARE);
        
        for (const FieldMapping& mapping : mappings) {
            if (mapping.type == "int32") {
                std::shared_ptr<NamedInteger<int32_t>> val = NamedInteger<int32_t>::create(mapping.name, 0, container);
                val->setEndianness(mapping.endian);
                val->bind(parentBuf, baseOffset + mapping.offset);
            } else if (mapping.type == "int64") {
                std::shared_ptr<NamedInteger<int64_t>> val = NamedInteger<int64_t>::create(mapping.name, 0, container);
                val->setEndianness(mapping.endian);
                val->bind(parentBuf, baseOffset + mapping.offset);
            } else if (mapping.type == "float64" || mapping.type == "double") {
                std::shared_ptr<NamedFloatingPoint<double>> val = NamedFloatingPoint<double>::create(mapping.name, 0.0, container);
                val->setEndianness(mapping.endian);
                val->bind(parentBuf, baseOffset + mapping.offset);
            } else if (mapping.type == "bool" || mapping.type == "boolean") {
                std::shared_ptr<NamedBoolean> val = NamedBoolean::create(mapping.name, false, container);
                val->setEndianness(mapping.endian);
                val->bind(parentBuf, baseOffset + mapping.offset);
            }
        }

        return {container};
    };

    return TransformationRule(std::move(pred), std::move(gen), priority);
}

} // namespace quasar::named::traversal
