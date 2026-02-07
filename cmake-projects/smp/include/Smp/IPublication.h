#ifndef SMP_IPUBLICATION_H
#define SMP_IPUBLICATION_H

#include "Smp/AccessKind.h"
#include "Smp/DuplicateName.h"
#include "Smp/IField.h"
#include "Smp/IOperation.h"
#include "Smp/IProperty.h"
#include "Smp/IRequest.h"
#include "Smp/InvalidFieldName.h"
#include "Smp/InvalidFieldType.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/IPublishOperation.h"
#include "Smp/Publication/ITypeRegistry.h"
#include "Smp/Publication/TypeNotRegistered.h"
#include "Smp/Uuid.h"
#include "Smp/ViewKind.h"

namespace Smp {
/// Interface that provides functionality to allow publishing members,
/// including fields, properties and operations.
class IPublication {
public:
  /// Virtual destructor to release memory.
  virtual ~IPublication() noexcept = default;

  /// Give access to the global type registry.
  /// The type registry is typically a singleton, and must not be null,
  /// to allow use of existing types, and registration of new types.
  /// @return  Reference to global type registry.
  virtual Publication::ITypeRegistry *GetTypeRegistry() const = 0;

  /// Publish a Char8 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, Char8 *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a Bool field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, Bool *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a Int8 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, Int8 *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a Int16 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, Int16 *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a Int32 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, Int32 *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a Int64 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, Int64 *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a UInt8 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, UInt8 *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a UInt16 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, UInt16 *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a UInt32 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, UInt32 *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a UInt64 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, UInt64 *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a Float32 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, Float32 *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a Float64 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual void PublishField(String8 name, String8 description, Float64 *address,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish a field of any type with the given name, description,
  /// address, type, view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// If no type with the given type UUID exists, an exception of type
  /// TypeNotRegistered is thrown.
  /// If the type selected with the type UUID is not a valid value type
  /// for publication of fields, an exception of type InvalidFieldType is
  /// thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   typeUuid Uuid of field type (determines the size).
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidFieldType
  /// @throws  Smp::InvalidObjectName
  /// @throws  Smp::Publication::TypeNotRegistered
  virtual void PublishField(String8 name, String8 description, void *address,
                            Uuid typeUuid, ViewKind view = ViewKind::VK_All,
                            Bool state = true, Bool input = false,
                            Bool output = false) = 0;

  /// Publish a field defined internally that implements the IField
  /// interface.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// @param   field Field to publish.
  /// @throws  Smp::DuplicateName
  virtual void PublishField(IField *field) = 0;

  /// Publish array of simple type.
  /// This method can only be used for arrays of simple type, as each
  /// simple type can be mapped to a primitive type. The memory layout of
  /// the array has to be without any padding, i.e. the array element
  /// with index i (0-based) is assumed to be stored at address +
  /// i*sizeof(primitiveType).
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   count Size of array.
  /// @param   address Field memory address.
  /// @param   type Array item type.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidFieldType
  /// @throws  Smp::InvalidObjectName
  virtual void PublishArray(String8 name, String8 description, Int64 count,
                            void *address, PrimitiveTypeKind type,
                            ViewKind view = ViewKind::VK_All, Bool state = true,
                            Bool input = false, Bool output = false) = 0;

  /// Publish top-level node of an array without using the type registry.
  /// This operation can be used, together with subsequent calls to
  /// PublishField, for direct publication of an array.
  /// This method can be used for arrays of any type. Individual array
  /// elements have to be added manually to the returned IPublication
  /// interface, where each array element can (and has to) be published
  /// with its own memory address.
  /// @param   name Array name.
  /// @param   description Array description.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @return  Interface to publish item type against.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual IPublication *PublishArray(String8 name, String8 description,
                                     ViewKind view = ViewKind::VK_All,
                                     Bool state = true) = 0;

  /// Publish top-level node of a structure without using the type
  /// registry.
  /// This operation can be used, together with subsequent calls to
  /// PublishField, for direct publication of a structure.
  /// @param   name Structure name.
  /// @param   description Structure description.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @return  Reference to publish structure fields against.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectName
  virtual IPublication *PublishStructure(String8 name, String8 description,
                                         ViewKind view = ViewKind::VK_All,
                                         Bool state = true) = 0;

  /// Publish an operation with the given name, description and view
  /// kind.
  /// If an Operations with the same name has already been published,
  /// this will update the "description" and "view"  of the previous
  /// publication and it returns the same IPublishOperation of the
  /// previously published Operation.
  /// If an Operation with the same name has not been published, this
  /// creates a new IPublishOperation instance and returns it.
  /// The operation parameters (including an optional return parameter)
  /// may be published against the returned IPublishOperation interface
  /// after calling PublishOperation().
  /// @param   name Name of operation.
  /// @param   description Description of operation.
  /// @param   view View kind of operation node in simulation tree.
  /// @return  Reference to publish parameters against.
  /// @throws  Smp::InvalidObjectName
  virtual Publication::IPublishOperation *
  PublishOperation(String8 name, String8 description,
                   ViewKind view = ViewKind::VK_None) = 0;

  /// Publish a property
  /// @param   name Property name.
  /// @param   description Property description.
  /// @param   typeUuid Uuid of type of property.
  /// @param   accessKind Access kind of Property.
  /// @param   view Show field in model tree.
  /// @throws  Smp::InvalidObjectName
  /// @throws  Smp::Publication::TypeNotRegistered
  virtual void PublishProperty(String8 name, String8 description, Uuid typeUuid,
                               AccessKind accessKind,
                               ViewKind view = ViewKind::VK_None) = 0;

  /// Get the field of given name.
  ///
  /// This method raises an exception of type InvalidFieldName if called
  /// with a field name for which no corresponding field exists.
  /// This method can be used both for fields of simple types (when it
  /// returns an instance of ISimpleField), and for complex fields (when
  /// it returns IArrayField or IStructureField).
  ///
  /// @remarks For getting access to simple fields of structured or array
  ///          types, this method may be called by specifying a full
  ///          field name, e.g. "MyField.Position[2]" in order to access
  ///          an array item of a structure.
  /// @param   fullName Fully qualified field name (relative to the
  ///          component).
  /// @return  Instance of field for given full name.
  /// @throws  Smp::InvalidFieldName
  virtual IField *GetField(String8 fullName) const = 0;

  /// Returns a collection of all fields that have been published.
  /// @return  Collection of the fields (immediate children) of the
  ///          component.
  virtual const FieldCollection *GetFields() const = 0;

  /// Provides the collection of properties that have been published.
  /// @return  Collection of properties that have been published, which
  ///          may be empty.
  virtual const PropertyCollection *GetProperties() const = 0;

  /// Provides the collection of operations that have been published.
  /// @return  Collection of operations that have been published, which
  ///          may be empty.
  virtual const OperationCollection *GetOperations() const = 0;

  /// Create request object.
  /// Returns a request object for the given operation that describes the
  /// parameters and the return value.
  /// Request object may be undefined if no operation with given name
  /// could be found.
  /// @param   operationName Name of operation.
  /// @return  Request object for operation.
  virtual IRequest *CreateRequest(String8 operationName) = 0;

  /// Delete request object.
  /// Destroys a request object that has been created with the
  /// CreateRequest() method before.
  /// The request object must not be used anymore after DeleteRequest has
  /// been called for it.
  /// @param   request Request object to delete.
  virtual void DeleteRequest(IRequest *request) = 0;

  /// Call this operation to release all data created during earlier
  /// Publish calls to this instance.
  /// This invalidated all pointers retrieved earlier via GetField(),
  /// GetFields() or CreateRequest().
  virtual void Unpublish() = 0;
};
} // namespace Smp

#endif // SMP_IPUBLICATION_H
