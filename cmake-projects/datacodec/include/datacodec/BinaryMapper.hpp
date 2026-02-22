/**
 * @file BinaryMapper.hpp
 * @brief Mapper for converting between Binary Buffers and NamedObjects using a
 * Schema.
 */

#ifndef DATACODEC_BINARYMAPPER_HPP
#define DATACODEC_BINARYMAPPER_HPP

#include "datacodec/AdvancedSchema.hpp"
#include "datacodec/Schema.hpp"
#include "quasar/coretypes/BitBufferSlice.hpp"
#include "quasar/named/NamedObject.hpp"

namespace datacodec {

class BinaryMapper {
public:
  /**
   * @brief Decodes a buffer into a NamedObject hierarchy based on the container
   * definition.
   */
  static std::shared_ptr<quasar::named::NamedObject>
  decode(std::shared_ptr<ContainerDef> schema,
         const quasar::coretypes::BitBufferSlice &buffer) {

    // Create a root container to hold results
    std::shared_ptr<quasar::named::NamedObject> resultContainer =
        quasar::named::NamedObject::create(schema->getName());

    for (const std::shared_ptr<FieldDef> &field : schema->getFields()) {

      // Check for conditional presence
      std::shared_ptr<ConditionalFieldDef> condField =
          std::dynamic_pointer_cast<ConditionalFieldDef>(field);
      if (condField && !condField->isPresent(resultContainer.get())) {
        continue;
      }

      // Create a sub-slice for the field
      // Note: In a real implementation we track current bit offset.
      // Here we use the absolute offset from the field definition relative to
      // the buffer start. Dynamic sizing would require accumulation of offsets.
      size_t offset = field->getBitOffset();
      size_t size = field->getCodec()->getBitSize();

      // Handle dynamic size codecs by looking ahead or having context?
      // For now assume fixed size or handle exception

      try {
        quasar::coretypes::BitBufferSlice slice = buffer.slice(offset, size);
        std::shared_ptr<quasar::named::NamedObject> decodedValue =
            field->getCodec()->decode(slice);

        // TODO: Properly attach decodedValue to resultContainer
        // resultContainer->addChild(decodedValue);
      } catch (const std::exception &e) {
        // Handle decoding error
      }
    }

    return resultContainer;
  }

  /**
   * @brief Encodes a NamedObject hierarchy into a buffer based on the container
   * definition.
   */
  static void encode(std::shared_ptr<ContainerDef> schema,
                     const std::shared_ptr<quasar::named::NamedObject> &data,
                     quasar::coretypes::BitBufferSlice &buffer) {

    // Iterate schema and write fields
    for (const std::shared_ptr<FieldDef> &field : schema->getFields()) {
      // ... encoding logic mirroring decode ...
    }
  }
};

} // namespace datacodec

#endif // DATACODEC_BINARYMAPPER_HPP
