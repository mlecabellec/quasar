// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/IPublishField.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_IPUBLISHFIELD_H_
#define SMP_PUBLICATION_IPUBLISHFIELD_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/DuplicateName.h"
#include "Smp/FieldCollection.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/InvalidType.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/TypeNotRegistered.h"
#include "Smp/Uuid.h"
#include "Smp/ViewKind.h"
#include "Smp/Void.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IField;
    class ISimpleArrayField;

    namespace Publication
    {
        class IPublishField;
    }
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for publication
    namespace Publication
    {
        /// This interface provides functionality to publish fields in
        /// Components and Fields.
        class IPublishField
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~IPublishField() noexcept = default;

            /// Publish a Bool field with the given name, description, address,
            /// view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Bool* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a Char8 field with the given name, description,
            /// address, view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Char8* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a Int8 field with the given name, description, address,
            /// view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Int8* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a Int16 field with the given name, description,
            /// address, view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Int16* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a Int32 field with the given name, description,
            /// address, view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Int32* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a Int64 field with the given name, description,
            /// address, view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Int64* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a UInt8 field with the given name, description,
            /// address, view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::UInt8* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a UInt16 field with the given name, description,
            /// address, view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::UInt16* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a UInt32 field with the given name, description,
            /// address, view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::UInt32* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a UInt64 field with the given name, description,
            /// address, view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::UInt64* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a Float32 field with the given name, description,
            /// address, view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Float32* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a Float64 field with the given name, description,
            /// address, view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Float64* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a field of any type with the given name, description,
            /// address, type, view kind and state, input and output flags.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// If no type with the given type UUID exists, an exception of
            /// type TypeNotRegistered is thrown.
            /// If publication type is not suitable for field publication (e.g.
            /// String8), it throws an exception of type InvalidType.
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   address Field memory address.
            /// @param   typeUuid Uuid of field type (determines the size).
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::Publication::TypeNotRegistered
            /// @throws  Smp::InvalidType
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* PublishField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Void* address,
                Smp::Uuid typeUuid,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish a field defined internally that implements the IField
            /// interface.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// @param   field Field to publish.
            /// @throws  Smp::DuplicateName
            virtual void PublishField(
                Smp::IField* field) = 0;

            /// Publish top-level object of an array without using the type
            /// registry.
            /// This operation can be used, together with subsequent calls to
            /// PublishField, for direct publication of an array.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// This method can be used for arrays of any type. Individual
            /// array elements have to be added manually to the returned
            /// IPublishField interface, where each array element can (and has
            /// to) be published with its own memory address.
            /// @param   name Array name.
            /// @param   description Array description.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @return  Interface to publish item type against.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::Publication::IPublishField* PublishArray(
                Smp::String8 name,
                Smp::String8 description,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true) = 0;

            /// Publish array of simple type.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// If primitive type is not suitable for simple array field
            /// publication (i.e. None, String8), it throws an exception of
            /// type InvalidType.
            /// This method can only be used for arrays of simple type, as each
            /// simple type can be mapped to a primitive type. The memory
            /// layout of the array has to be without any padding, i.e. the
            /// array element with index i (0-based) is assumed to be stored at
            /// address + i*sizeof(primitiveType).
            /// @param   name Field name.
            /// @param   description Field description.
            /// @param   count Size of array.
            /// @param   address Field memory address.
            /// @param   type Array item type.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @param   input True if field is an input field, false otherwise.
            /// @param   output True if field is an output field, false
            /// otherwise.
            /// @return  Reference to the simple array field that has been
            /// created by the publication call.
            /// @throws  Smp::InvalidType
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::ISimpleArrayField* PublishArray(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Int64 count,
                Smp::Void* address,
                Smp::PrimitiveTypeKind type,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;

            /// Publish top-level object of a structure without using the type
            /// registry.
            /// This operation can be used, together with subsequent calls to
            /// PublishField, for direct publication of a structure.
            /// If an object with the same name has been published before, an
            /// exception of type DuplicateName is thrown.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// @param   name Structure name.
            /// @param   description Structure description.
            /// @param   view Show field in model tree.
            /// @param   state Include field in store/restore of simulation
            /// state.
            /// @return  Reference to publish structure fields against.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::Publication::IPublishField* PublishStructure(
                Smp::String8 name,
                Smp::String8 description,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true) = 0;

            /// Get the field of given name.
            /// This method returns nullptr if called with a field name for
            /// which no corresponding field exists.
            /// This method can be used both for fields of simple types (when
            /// it returns an instance of ISimpleField), and for complex fields
            /// (when it returns IArrayField or IStructureField).
            /// @remarks For getting access to simple fields of structure or
            /// array types, this method may be called by specifying a full
            /// field name, e.g. "MyField.Position[2]" in order to access an
            /// array item of a structure.
            /// @param   fullName Fully qualified field name (relative to the
            /// component).
            /// @return  Instance of field for given full name, or nullptr if
            /// no field exists with the given name.
            virtual Smp::IField* GetField(
                Smp::String8 fullName) const = 0;

            /// Returns a collection of all fields that have been published.
            /// @remarks The collection shall exactly contain the fields that
            /// have been published via the PublishField methods.
            /// @return  Collection of the fields (immediate children) of the
            /// component.
            virtual const Smp::FieldCollection* GetFields() const = 0;
        };
    }
}

#endif // SMP_PUBLICATION_IPUBLISHFIELD_H_
