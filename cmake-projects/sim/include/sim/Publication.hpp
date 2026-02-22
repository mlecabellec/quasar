#ifndef SIM_PUBLICATION_HPP
#define SIM_PUBLICATION_HPP

#include <Smp/IPublication.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <core/SimpleCollection.hpp>
#include <map>
#include <memory>
#include <vector>

namespace sim {

class Publication : public virtual Smp::IPublication {
public:
  Publication(Smp::Publication::ITypeRegistry *typeRegistry);
  virtual ~Publication() noexcept = default;

  Smp::Publication::ITypeRegistry *GetTypeRegistry() const override;

  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Char8 *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Bool *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Int8 *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Int16 *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Int32 *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Int64 *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::UInt8 *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::UInt16 *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::UInt32 *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::UInt64 *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Float32 *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Float64 *address,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;

  void PublishField(Smp::String8 name, Smp::String8 description, void *address,
                    Smp::Uuid typeUuid,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  void PublishField(Smp::IField *field) override;

  Smp::IPublication *PublishArray(Smp::String8 name, Smp::String8 description,
                                  Smp::ViewKind view = Smp::ViewKind::VK_All,
                                  Smp::Bool state = true) override;
  void PublishArray(Smp::String8 name, Smp::String8 description,
                    Smp::Int64 count, void *address,
                    Smp::PrimitiveTypeKind type,
                    Smp::ViewKind view = Smp::ViewKind::VK_All,
                    Smp::Bool state = true, Smp::Bool input = false,
                    Smp::Bool output = false) override;
  Smp::IPublication *
  PublishStructure(Smp::String8 name, Smp::String8 description,
                   Smp::ViewKind view = Smp::ViewKind::VK_All,
                   Smp::Bool state = true) override;

  Smp::Publication::IPublishOperation *
  PublishOperation(Smp::String8 name, Smp::String8 description,
                   Smp::ViewKind view = Smp::ViewKind::VK_None) override;
  void PublishProperty(Smp::String8 name, Smp::String8 description,
                       Smp::Uuid typeUuid, Smp::AccessKind accessKind,
                       Smp::ViewKind view = Smp::ViewKind::VK_None) override;

  Smp::IField *GetField(Smp::String8 fullName) const override;
  const Smp::FieldCollection *GetFields() const override;
  const Smp::PropertyCollection *GetProperties() const override;
  const Smp::OperationCollection *GetOperations() const override;

  Smp::IRequest *CreateRequest(Smp::String8 operationName) override;
  void DeleteRequest(Smp::IRequest *request) override;

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
