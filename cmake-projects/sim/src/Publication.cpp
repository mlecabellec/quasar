#include <sim/Field.hpp>
#include <sim/Publication.hpp>
#include <sim/PublishOperation.hpp>

namespace sim {

Publication::Publication(Smp::Publication::ITypeRegistry *typeRegistry)
    : _typeRegistry(typeRegistry) {}

Smp::Publication::ITypeRegistry *Publication::GetTypeRegistry() const {
  return _typeRegistry;
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Char8 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_Char8);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Bool *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_Bool);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Int8 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_Int8);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Int16 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_Int16);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Int32 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_Int32);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Int64 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_Int64);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::UInt8 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_UInt8);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::UInt16 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_UInt16);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::UInt32 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_UInt32);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::UInt64 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_UInt64);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Float32 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_Float32);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               Smp::Float64 *address, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  const Smp::Publication::IType *type =
      _typeRegistry->GetType(Smp::PrimitiveTypeKind::PTK_Float64);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::String8 name, Smp::String8 description,
                               void *address, Smp::Uuid typeUuid,
                               Smp::ViewKind view, Smp::Bool state,
                               Smp::Bool input, Smp::Bool output) {
  const Smp::Publication::IType *type = _typeRegistry->GetType(typeUuid);
  auto field = std::make_unique<SimpleField>(
      name, description, nullptr, type, address, view, state, input, output);
  _fieldCollection.Add(field.get());
  _fields.push_back(std::move(field));
}

void Publication::PublishField(Smp::IField *field) {
  // Need to handle external field ownership or just wrap it
}

Smp::IPublication *Publication::PublishArray(Smp::String8 name,
                                             Smp::String8 description,
                                             Smp::ViewKind view,
                                             Smp::Bool state) {
  return nullptr; // Stub
}

void Publication::PublishArray(Smp::String8 name, Smp::String8 description,
                               Smp::Int64 count, void *address,
                               Smp::PrimitiveTypeKind type, Smp::ViewKind view,
                               Smp::Bool state, Smp::Bool input,
                               Smp::Bool output) {
  // Stub
}

Smp::IPublication *Publication::PublishStructure(Smp::String8 name,
                                                 Smp::String8 description,
                                                 Smp::ViewKind view,
                                                 Smp::Bool state) {
  return nullptr; // Stub
}

Smp::Publication::IPublishOperation *
Publication::PublishOperation(Smp::String8 name, Smp::String8 description,
                              Smp::ViewKind view) {
  auto op =
      std::make_unique<sim::PublishOperation>(name, description, _typeRegistry);
  Smp::Publication::IPublishOperation *opPtr = op.get();
  _operations.push_back(std::move(op));
  // Note: operationCollection expects Smp::IOperation*
  // If PublishOperation doesn't implement IOperation, we might need a wrapper
  // or just skip adding to collection for now if not used.
  return opPtr;
}

void Publication::PublishProperty(Smp::String8 name, Smp::String8 description,
                                  Smp::Uuid typeUuid,
                                  Smp::AccessKind accessKind,
                                  Smp::ViewKind view) {
  // Stub
}

Smp::IField *Publication::GetField(Smp::String8 fullName) const {
  return _fieldCollection.at(fullName);
}

const Smp::FieldCollection *Publication::GetFields() const {
  return &_fieldCollection;
}

const Smp::PropertyCollection *Publication::GetProperties() const {
  return nullptr; // TODO
}

const Smp::OperationCollection *Publication::GetOperations() const {
  return nullptr; // TODO
}

Smp::IRequest *Publication::CreateRequest(Smp::String8 operationName) {
  return nullptr; // Stub
}

void Publication::DeleteRequest(Smp::IRequest *request) {
  // Stub
}

void Publication::Unpublish() {
  _fieldCollection.Clear();
  _fields.clear();
}

} // namespace sim
