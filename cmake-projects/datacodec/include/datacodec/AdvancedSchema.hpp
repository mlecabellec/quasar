/**
 * @file AdvancedSchema.hpp
 * @brief Advanced schema definitions including ConditionalField and
 * TransformCodec.
 */

#ifndef DATACODEC_ADVANCED_SCHEMA_HPP
#define DATACODEC_ADVANCED_SCHEMA_HPP

#include "datacodec/Schema.hpp"
#include <functional>

namespace datacodec {

/**
 * @class ConditionalFieldDef
 * @brief A field that is only present if a condition is met.
 *
 * This class can be used to model EtherCAT data structures with optional fields
 * or fields whose presence depends on configuration or other data values. This
 * might be relevant for advanced SDO or PDO configurations where certain fields
 * are conditionally included, contributing to flexible data representation for
 * FE-0040.
 *
 * Contribution to FE-0020: Inherits from FieldDef, which in turn inherits from
 * quasar::named::NamedObject (FE-0020.4). Its `isPresent` method takes a
 * `quasar::named::NamedObject*` as context, demonstrating interaction with the
 * named object hierarchy for conditional logic, aligning with the principle of
 * managing complex named object structures.
 */
class ConditionalFieldDef : public FieldDef {
public:
  using Predicate = std::function<bool(const quasar::named::NamedObject *)>;

  static std::shared_ptr<ConditionalFieldDef>
  create(const std::string &name, std::shared_ptr<ICodec> codec,
         size_t bitOffset, Predicate condition) {
    return std::make_shared<ConditionalFieldDef>(name, codec, bitOffset,
                                                 condition);
  }

  ConditionalFieldDef(const std::string &name, std::shared_ptr<ICodec> codec,
                      size_t bitOffset, Predicate condition)
      : FieldDef(name, codec, bitOffset), m_condition(condition) {}

  bool isPresent(const quasar::named::NamedObject *context) const {
    if (m_condition) {
      return m_condition(context);
    }
    return true;
  }

private:
  Predicate m_condition;
};

/**
 * @class TransformCodec
 * @brief Middleware codec that applies a transformation to the decoded value.
 *
 * This codec can be used to apply pre-processing or post-processing steps
 * during the encoding or decoding of EtherCAT data. For example, it could be used
 * for data validation, type coercion, or applying specific EtherCAT data encoding
 * rules that differ from standard binary representations, potentially aiding in
 * handling complex data requirements for FE-0040.
 *
 * Contribution to FE-0020: Inherits from ICodec and explicitly uses
 * `std::shared_ptr<quasar::named::NamedObject>` in its transformer types and
 * method signatures. This demonstrates that advanced codec operations are built
 * upon the NamedObject abstraction, supporting the manipulation and interpretation
 * of data within the named object framework as envisioned by FE-0020.
 */
class TransformCodec : public ICodec {
public:
  using Transformer = std::function<std::shared_ptr<quasar::named::NamedObject>(
      std::shared_ptr<quasar::named::NamedObject>)>;
  using ReverseTransformer =
      std::function<std::shared_ptr<quasar::named::NamedObject>(
          std::shared_ptr<quasar::named::NamedObject>)>;

  TransformCodec(std::shared_ptr<ICodec> rawCodec, Transformer transform,
                 ReverseTransformer reverseTransform)
      : m_rawCodec(rawCodec), m_transform(transform),
        m_reverseTransform(reverseTransform) {}

  std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const override {
    std::shared_ptr<quasar::named::NamedObject> rawValue =
        m_rawCodec->decode(buffer);
    return m_transform(rawValue);
  }

  void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
              quasar::coretypes::BitBufferSlice &buffer) const override {
    std::shared_ptr<quasar::named::NamedObject> rawValue =
        m_reverseTransform(value);
    m_rawCodec->encode(rawValue, buffer);
  }

  size_t getBitSize() const override { return m_rawCodec->getBitSize(); }

  size_t getEncodedBitSize(
      const std::shared_ptr<quasar::named::NamedObject> &value) const override {
    std::shared_ptr<quasar::named::NamedObject> rawValue =
        m_reverseTransform(value);
    return m_rawCodec->getEncodedBitSize(rawValue);
  }

private:
  std::shared_ptr<ICodec> m_rawCodec;
  Transformer m_transform;
  ReverseTransformer m_reverseTransform;
};

} // namespace datacodec

#endif // DATACODEC_ADVANCED_SCHEMA_HPP
