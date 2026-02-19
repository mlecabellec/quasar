// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/ITypeRegistry.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_ITYPEREGISTRY_H_
#define SMP_PUBLICATION_ITYPEREGISTRY_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/InvalidObjectName.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/InvalidArrayItemType.h"
#include "Smp/Publication/InvalidPrimitiveType.h"
#include "Smp/Publication/TypeAlreadyRegistered.h"
#include "Smp/Publication/TypeNotRegistered.h"
#include "Smp/Uuid.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{

    namespace Publication
    {
        class IArrayType;
        class IClassType;
        class IEnumerationType;
        class IStringType;
        class IStructureType;
        class IType;
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
        /// This interface defines a registration mechanism for user defined
        /// types.
        /// Via the IPublication interface (that has to be provided to each
        /// component during the Publish state transition), it is mandatory
        /// that a simulator provides a type registry. The simulator has to
        /// ensure that this type registry contains at least all pre-defined
        /// value types of the SMP standard, using their pre-defined
        /// universally unique identifiers. However, it is not mandatory for
        /// models to make use of this type registry.
        class ITypeRegistry
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~ITypeRegistry() noexcept = default;

            /// Returns a type by its primitive type kind.
            /// @remarks This method can be used to map primitive types to the
            /// IType interface, to treat all types identically.
            /// @param   type Primitive type the type is requested for.
            /// @return  Interface to the requested type.
            virtual Smp::Publication::IType* GetType(
                Smp::PrimitiveTypeKind type) const = 0;

            /// Returns a type by universally unique identifier.
            /// @remarks This method can be used to find out whether a specific
            /// type has been registered before.
            /// @param   typeUuid Universally unique identifier for the
            /// requested type.
            /// @return  Interface of the requested type, or nullptr if no type
            /// with the registered Uuid could be found.
            virtual Smp::Publication::IType* GetType(
                Smp::Uuid typeUuid) const = 0;

            /// Add a float type to the registry.
            /// IComponent and IDynamicInvocation support fields, parameters
            /// and operations of Float types via the PTK_Float32 and
            /// PTK_Float64 primitive type, as a Float is mapped either to
            /// Float32 or Float64.
            /// @param   name Name of the type.
            /// @param   description Description of the type.
            /// @param   typeUuid Universally unique identifier of the type.
            /// @param   minimum Minimum value for float.
            /// @param   maximum Maximum value for float.
            /// @param   minInclusive Flag whether the minimum value for float
            /// is valid or not.
            /// @param   maxInclusive Flag whether the maximum value for float
            /// is valid or not.
            /// @param   unit Unit of the type.
            /// @param   type Primitive type to use for Float type. This has to
            /// be either PTK_Float32 or PTK_Float64. For all other types, an
            /// exception of type InvalidPrimitiveType is raised.
            /// @return  Interface to new type.
            /// @throws  Smp::Publication::TypeAlreadyRegistered
            /// @throws  Smp::Publication::InvalidPrimitiveType
            /// @throws  Smp::InvalidObjectName
            virtual Smp::Publication::IType* AddFloatType(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Uuid typeUuid,
                Smp::Float64 minimum,
                Smp::Float64 maximum,
                Smp::Bool minInclusive,
                Smp::Bool maxInclusive,
                Smp::String8 unit,
                Smp::PrimitiveTypeKind type = Smp::PrimitiveTypeKind::PTK_Float64) = 0;

            /// Add an integer type to the registry.
            /// IComponent and IDynamicInvocation support fields, parameters
            /// and operations of Integer types via the PTK_Int primitive
            /// types, as an Integer is mapped to one of Int8 / Int16 / Int32 /
            /// Int64 / UInt8 / UInt16 / UInt32 / UInt64.
            /// @param   name Name of the type.
            /// @param   description Description of the type.
            /// @param   typeUuid Universally unique identifier of the type.
            /// @param   minimum Minimum value for integer.
            /// @param   maximum Maximum value for integer.
            /// @param   unit Unit of the type.
            /// @param   type Primitive type to use for Integer type. This has
            /// to be one of the available signed or unsigned integer types.
            /// For all other types, an exception of type InvalidPrimitiveType
            /// is raised.
            /// @return  Interface to new type.
            /// @throws  Smp::Publication::TypeAlreadyRegistered
            /// @throws  Smp::Publication::InvalidPrimitiveType
            /// @throws  Smp::InvalidObjectName
            virtual Smp::Publication::IType* AddIntegerType(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Uuid typeUuid,
                Smp::Int64 minimum,
                Smp::Int64 maximum,
                Smp::String8 unit,
                Smp::PrimitiveTypeKind type = Smp::PrimitiveTypeKind::PTK_Int32) = 0;

            /// Add an enumeration type to the registry.
            /// @param   name Name of the type.
            /// @param   description Description of the type.
            /// @param   typeUuid Universally unique identifier of the type.
            /// @return  Interface to new type.
            /// @throws  Smp::Publication::TypeAlreadyRegistered
            /// @throws  Smp::InvalidObjectName
            virtual Smp::Publication::IEnumerationType* AddEnumerationType(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Uuid typeUuid) = 0;

            /// Add an array type to the registry.
            /// @param   name Name of the type.
            /// @param   description Description of the type.
            /// @param   typeUuid Universally unique identifier of the type.
            /// @param   itemTypeUuid Universally unique identifier of the Type
            /// of the array items.
            /// @param   itemSize Size of an array item in bytes. This needs to
            /// take possible padding into account, as it may be used by the
            /// simulation environment to calculate the memory offset between
            /// array items.
            /// @param   arrayCount Number of elements in the array.
            /// @param   simpleArray True if array shall be implemented using
            /// ISimpleArrayField, i.e. without representing array items as
            /// fields.
            /// @return  Interface to new type.
            /// @throws  Smp::Publication::TypeAlreadyRegistered
            /// @throws  Smp::Publication::TypeNotRegistered
            /// @throws  Smp::Publication::InvalidArrayItemType
            /// @throws  Smp::InvalidObjectName
            virtual Smp::Publication::IArrayType* AddArrayType(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Uuid typeUuid,
                Smp::Uuid itemTypeUuid,
                Smp::UInt64 itemSize,
                Smp::UInt64 arrayCount,
                Smp::Bool simpleArray = false) = 0;

            /// Add a string type to the registry.
            /// @param   name Name of the type.
            /// @param   description Description of the type.
            /// @param   typeUuid Universally unique identifier of the type.
            /// @param   length Maximum length of the string.
            /// @return  Interface to new type.
            /// @throws  Smp::Publication::TypeAlreadyRegistered
            /// @throws  Smp::InvalidObjectName
            virtual Smp::Publication::IStringType* AddStringType(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Uuid typeUuid,
                Smp::UInt64 length) = 0;

            /// Add a structure type to the registry.
            /// @param   name Name of the type.
            /// @param   description Description of the type.
            /// @param   typeUuid Universally unique identifier of the type.
            /// @return  Interface to new type that allows adding fields.
            /// @throws  Smp::Publication::TypeAlreadyRegistered
            /// @throws  Smp::InvalidObjectName
            virtual Smp::Publication::IStructureType* AddStructureType(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Uuid typeUuid) = 0;

            /// Add a class type to the registry.
            /// @param   name Name of the type.
            /// @param   description Description of the type.
            /// @param   typeUuid Universally unique identifier of the type.
            /// @param   baseClassUuid Universally unique identifier of the
            /// base class.
            /// Use Uuid_Void when the type has no base type.
            /// @return  Interface to new type that allows adding fields.
            /// @throws  Smp::Publication::TypeAlreadyRegistered
            /// @throws  Smp::InvalidObjectName
            virtual Smp::Publication::IClassType* AddClassType(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Uuid typeUuid,
                Smp::Uuid baseClassUuid) = 0;
        };
    }
}

#endif // SMP_PUBLICATION_ITYPEREGISTRY_H_
