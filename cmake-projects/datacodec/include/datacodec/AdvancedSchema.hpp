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
    auto rawValue = m_rawCodec->decode(buffer);
    return m_transform(rawValue);
  }

  void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
              quasar::coretypes::BitBufferSlice &buffer) const override {
    auto rawValue = m_reverseTransform(value);
    m_rawCodec->encode(rawValue, buffer);
  }

  size_t getBitSize() const override { return m_rawCodec->getBitSize(); }

  size_t getEncodedBitSize(
      const std::shared_ptr<quasar::named::NamedObject> &value) const override {
    auto rawValue = m_reverseTransform(value);
    return m_rawCodec->getEncodedBitSize(rawValue);
  }

private:
  std::shared_ptr<ICodec> m_rawCodec;
  Transformer m_transform;
  ReverseTransformer m_reverseTransform;
};

} // namespace datacodec

#endif // DATACODEC_ADVANCED_SCHEMA_HPP
