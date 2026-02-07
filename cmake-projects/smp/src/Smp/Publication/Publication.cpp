#include "Smp/Publication/Publication.h"
#include "Smp/Operation.h"
#include "Smp/Property.h"
#include "Smp/Publication/PublishOperation.h"
#include "Smp/SimpleField.h"

namespace Smp::Publication {

Publication::Publication(Smp::String8 name, Smp::String8 description,
                         Smp::IObject *parent, ITypeRegistry *registry)
    : Smp::Object(name, description, parent), registry(registry),
      fields("Fields", "", this), properties("Properties", "", this),
      operations("Operations", "", this) {}

void Publication::InternalPublishField(Smp::String8 name,
                                       Smp::String8 description, void *address,
                                       Smp::PrimitiveTypeKind kind,
                                       Smp::ViewKind view, Smp::Bool state,
                                       Smp::Bool input, Smp::Bool output) {
  auto type = registry->GetType(kind);
  if (!type)
    throw TypeNotRegistered(kind);
  auto field = new Smp::SimpleField(name, description, this, address, view,
                                    state, input, output, type);
  fields.Add(field);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Char8 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_Char8, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Bool *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_Bool, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Int8 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_Int8, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Int16 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_Int16, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Int32 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_Int32, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Int64 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_Int64, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::UInt8 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_UInt8, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::UInt16 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_UInt16, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::UInt32 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_UInt32, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::UInt64 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_UInt64, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Float32 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_Float32, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Float64 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  InternalPublishField(name, description, address,
                       Smp::PrimitiveTypeKind::PTK_Float64, view, state, input,
                       output);
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               void *address, Uuid typeUuid, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  auto type = registry->GetType(typeUuid);
  if (!type)
    throw TypeNotRegistered(typeUuid);
  auto field = new Smp::SimpleField(name, description, this, address, view,
                                    state, input, output, type);
  fields.Add(field);
}

void Publication::PublishField(Smp::IField *field) {
  if (field)
    fields.Add(field);
}

void Publication::PublishArray(Smp::String8 name, Smp::String8 description,
                               Smp::Int64 count, void *address,
                               Smp::PrimitiveTypeKind type, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  // TODO: Implement ISimpleArrayField and specific logic
}

Smp::IPublication *Publication::PublishArray(Smp::String8 name,
                                             Smp::String8 description,
                                             Smp::ViewKind view,
                                             Smp::Bool state) {
  // TODO
  return nullptr;
}

Smp::IPublication *Publication::PublishStructure(Smp::String8 name,
                                                 Smp::String8 description,
                                                 Smp::ViewKind view,
                                                 Smp::Bool state) {
  // TODO
  return nullptr;
}

IPublishOperation *Publication::PublishOperation(Smp::String8 name,
                                                 Smp::String8 description,
                                                 Smp::ViewKind view) {
  // TODO: Implement PublishOperation
  return nullptr;
}

void Publication::PublishProperty(Smp::String8 name, Smp::String8 description,
                                  Uuid typeUuid, Smp::AccessKind accessKind,
                                  Smp::ViewKind view) {
  auto type = registry->GetType(typeUuid);
  if (!type)
    throw TypeNotRegistered(typeUuid);
  auto prop =
      new Smp::Property(name, description, this, type, accessKind, view);
  properties.Add(prop);
}

Smp::IField *Publication::GetField(Smp::String8 fullName) const {
  return fields.at(fullName);
}

Smp::IRequest *Publication::CreateRequest(Smp::String8 operationName) {
  auto op = operations.at(operationName);
  if (op)
    return op->CreateRequest();
  return nullptr;
}

void Publication::DeleteRequest(Smp::IRequest *request) {
  // TODO
}

void Publication::Unpublish() {
  // Clear collections
  // Note: Concrete implementations should handle memory management of added
  // fields if they own them.
}

} // namespace Smp::Publication
