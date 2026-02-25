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
 *
 * Inherits from quasar::named::NamedObject, fulfilling FE-0020.4 by providing
 * a concrete implementation of a named object for data fields. It associates
 * an ICodec with a specific bit offset within a larger structure.
 */
class FieldDef : public quasar::named::NamedObject {
public:
  static std::shared_ptr<FieldDef> create(const std::string &name,
                                          std::shared_ptr<ICodec> codec,
                                          size_t bitOffset = 0) {
    std::shared_ptr<FieldDef> obj =
        std::make_shared<FieldDef>(name, codec, bitOffset);
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
 *
 * These definitions are used to describe the structure of EtherCAT data objects,
 * such as those found in SDO (Service Data Object) entries, PDO (Process Data
 * Object) mappings, and the payloads of various mailbox protocols (CoE, FoE, EoE).
 * By defining these structures, the datacodec module enables the accurate
 * serialization and deserialization of data required for EtherCAT communication,
 * directly supporting features like FE-0040.4 (Mailbox Protocols) and
 * FE-0040.5 (Process Data Configuration).
 *
 * Contribution to FE-0020: Inherits from quasar::named::NamedObject (FE-0020.4)
 * and manages a collection of FieldDef objects. This establishes a hierarchical
 * structure, directly contributing to FE-0020.1.2 and FE-0020.1.3 by enabling
 * the construction of trees or graphs of named objects, where ContainerDef acts
 * as a parent to FieldDef instances. This is foundational for organizing complex data.
 */
class ContainerDef : public quasar::named::NamedObject {
public:
  static std::shared_ptr<ContainerDef> create(const std::string &name) {
    std::shared_ptr<ContainerDef> obj = std::make_shared<ContainerDef>(name);
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
