#ifndef SMP_PUBLICATION_IMPL_H
#define SMP_PUBLICATION_IMPL_H

#include "Smp/Collection.h"
#include "Smp/IPublication.h"
#include "Smp/Object.h"

namespace Smp::Publication {

class Publication : public virtual Smp::IPublication, public Smp::Object {
public:
  Publication(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
              ITypeRegistry *registry);
  virtual ~Publication() noexcept = default;

  ITypeRegistry *GetTypeRegistry() const override { return registry; }

  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Char8 *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Bool *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Int8 *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Int16 *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Int32 *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Int64 *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::UInt8 *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::UInt16 *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::UInt32 *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::UInt64 *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Float32 *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::String8 name, Smp::String8 description,
                    Smp::Float64 *address, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;

  void PublishField(Smp::String8 name, Smp::String8 description, void *address,
                    Uuid typeUuid, Smp::ViewKind view, Smp::Bool state,
                    Smp::Bool input, Smp::Bool output) override;
  void PublishField(Smp::IField *field) override;

  void PublishArray(Smp::String8 name, Smp::String8 description,
                    Smp::Int64 count, void *address,
                    Smp::PrimitiveTypeKind type, Smp::ViewKind view,
                    Smp::Bool state, Smp::Bool input,
                    Smp::Bool output) override;
  Smp::IPublication *PublishArray(Smp::String8 name, Smp::String8 description,
                                  Smp::ViewKind view, Smp::Bool state) override;
  Smp::IPublication *PublishStructure(Smp::String8 name,
                                      Smp::String8 description,
                                      Smp::ViewKind view,
                                      Smp::Bool state) override;

  IPublishOperation *PublishOperation(Smp::String8 name,
                                      Smp::String8 description,
                                      Smp::ViewKind view) override;
  void PublishProperty(Smp::String8 name, Smp::String8 description,
                       Uuid typeUuid, Smp::AccessKind accessKind,
                       Smp::ViewKind view) override;

  Smp::IField *GetField(Smp::String8 fullName) const override;
  const Smp::FieldCollection *GetFields() const override { return &fields; }
  const Smp::PropertyCollection *GetProperties() const override {
    return &properties;
  }
  const Smp::OperationCollection *GetOperations() const override {
    return &operations;
  }

  Smp::IRequest *CreateRequest(Smp::String8 operationName) override;
  void DeleteRequest(Smp::IRequest *request) override;

  void Unpublish() override;

private:
  ITypeRegistry *registry;
  Smp::Collection<Smp::IField> fields;
  Smp::Collection<Smp::IProperty> properties;
  Smp::Collection<Smp::IOperation> operations;

  void InternalPublishField(Smp::String8 name, Smp::String8 description,
                            void *address, Smp::PrimitiveTypeKind kind,
                            Smp::ViewKind view, Smp::Bool state,
                            Smp::Bool input, Smp::Bool output);
};

} // namespace Smp::Publication

#endif // SMP_PUBLICATION_IMPL_H
