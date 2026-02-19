/**
 * @file Schema.hpp
 * @brief Schema definitions for datacodec.
 */

#ifndef DATACODEC_SCHEMA_HPP
#define DATACODEC_SCHEMA_HPP

#include "datacodec/ICodec.hpp"
#include "quasar/named/NamedObject.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace datacodec {

/**
 * @class FieldDef
 * @brief Definition of a single field in the schema.
 */
class FieldDef : public quasar::named::NamedObject {
public:
  static std::shared_ptr<FieldDef> create(const std::string &name,
                                          std::shared_ptr<ICodec> codec,
                                          size_t bitOffset = 0) {
    auto obj = std::make_shared<FieldDef>(name, codec, bitOffset);
    // obj->setSelf(obj); // Assuming NamedObject pattern needs this
    return obj;
  }

  FieldDef(const std::string &name, std::shared_ptr<ICodec> codec,
           size_t bitOffset)
      : quasar::named::NamedObject(name), m_codec(codec),
        m_bitOffset(bitOffset) {}

  std::shared_ptr<ICodec> getCodec() const { return m_codec; }
  size_t getBitOffset() const { return m_bitOffset; }

private:
  std::shared_ptr<ICodec> m_codec;
  size_t m_bitOffset;
};

/**
 * @class ContainerDef
 * @brief Definition of a container (composite structure) in the schema.
 */
class ContainerDef : public quasar::named::NamedObject {
public:
  static std::shared_ptr<ContainerDef> create(const std::string &name) {
    auto obj = std::make_shared<ContainerDef>(name);
    return obj;
  }

  ContainerDef(const std::string &name) : quasar::named::NamedObject(name) {}

  void addField(std::shared_ptr<FieldDef> field) {
    m_fields.push_back(field);
    // Also add as child in NamedObject hierarchy for traversal?
    // addChild(field);
  }

  const std::vector<std::shared_ptr<FieldDef>> &getFields() const {
    return m_fields;
  }

private:
  std::vector<std::shared_ptr<FieldDef>> m_fields;
};

} // namespace datacodec

#endif // DATACODEC_SCHEMA_HPP
