#ifndef SIM_PUBLICATION_HPP
#define SIM_PUBLICATION_HPP

#include <Smp/IPublication.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <core/SimpleCollection.hpp>
#include <map>
#include <memory>
#include <vector>

namespace sim {

/**
 * @brief Publication implementation.
 * @details Contributes to [FE-0070.9.1] (IPublication interface).
 */
class Publication : public virtual Smp::IPublication {
public:
  Publication(Smp::Publication::ITypeRegistry *typeRegistry);
  virtual ~Publication() noexcept = default;

  /// [FE-0070.9.2] Return Type Registry.
  Smp::Publication::ITypeRegistry *GetTypeRegistry() const override;

  /// [FE-0070.9.17] Publish primitive field.
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::Char8 *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::Bool *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::Int8 *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::Int16 *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::Int32 *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::Int64 *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::UInt8 *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::UInt16 *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::UInt32 *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::UInt64 *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::Float32 *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            Smp::Float64 *address,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;

  /// [FE-0070.9.18] Publish registered type field.
  Smp::IField *PublishField(Smp::String8 name, Smp::String8 description,
                            void *address, Smp::Uuid typeUuid,
                            Smp::ViewKind view = Smp::ViewKind::VK_All,
                            Smp::Bool state = true, Smp::Bool input = false,
                            Smp::Bool output = false) override;
  /// [FE-0070.9.19] Publish field from IField object.
  void PublishField(Smp::IField *field) override;

  /// [FE-0070.9.20] Publish array.
  Smp::IPublication *PublishArray(Smp::String8 name, Smp::String8 description,
                                  Smp::ViewKind view = Smp::ViewKind::VK_All,
                                  Smp::Bool state = true) override;
  Smp::ISimpleArrayField *
  PublishArray(Smp::String8 name, Smp::String8 description, Smp::Int64 count,
               void *address, Smp::PrimitiveTypeKind type,
               Smp::ViewKind view = Smp::ViewKind::VK_All,
               Smp::Bool state = true, Smp::Bool input = false,
               Smp::Bool output = false) override;
  /// [FE-0070.9.22] Publish structure.
  Smp::IPublication *
  PublishStructure(Smp::String8 name, Smp::String8 description,
                   Smp::ViewKind view = Smp::ViewKind::VK_All,
                   Smp::Bool state = true) override;

  /// [FE-0070.9.3] Publish operation.
  Smp::Publication::IPublishOperation *
  PublishOperation(Smp::String8 name, Smp::String8 description,
                   Smp::ViewKind view = Smp::ViewKind::VK_None) override;
  void PublishOperation(Smp::IOperation *operation) override;

  /// [FE-0070.9.4] Publish property.
  Smp::IProperty *
  PublishProperty(Smp::String8 name, Smp::String8 description,
                  Smp::Uuid typeUuid, Smp::AccessKind accessKind,
                  Smp::ViewKind view = Smp::ViewKind::VK_None) override;
  void PublishProperty(Smp::IProperty *property) override;

  Smp::IProperty *GetProperty(Smp::String8 name) const override;
  Smp::IOperation *GetOperation(Smp::String8 name) const override;

  /// [FE-0070.9.23] Return field by name.
  Smp::IField *GetField(Smp::String8 fullName) const override;
  /// [FE-0070.9.24] Return all published fields.
  const Smp::FieldCollection *GetFields() const override;
  /// [FE-0070.9.5] Return published properties.
  const Smp::PropertyCollection *GetProperties() const override;
  /// [FE-0070.9.6] Return published operations.
  const Smp::OperationCollection *GetOperations() const override;

  /// [FE-0070.9.7] Release published data.
  void Unpublish() override;

private:
  Smp::Publication::ITypeRegistry *_typeRegistry;
  std::vector<std::unique_ptr<Smp::IField>> _fields;
  core::SimpleCollection<Smp::IField> _fieldCollection;
  std::vector<std::unique_ptr<Smp::Publication::IPublishOperation>> _operations;
  core::SimpleCollection<Smp::IOperation> _operationCollection;
};

} // namespace sim

#endif // SIM_PUBLICATION_HPP
