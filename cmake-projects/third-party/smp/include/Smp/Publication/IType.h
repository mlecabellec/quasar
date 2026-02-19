// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/IType.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_ITYPE_H_
#define SMP_PUBLICATION_ITYPE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/DuplicateName.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/InvalidType.h"
#include "Smp/IObject.h"
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
        /// This base interface defines a type in the type registry.
        class IType :
            public virtual Smp::IObject
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~IType() noexcept = default;

            /// Get primitive type kind that this type maps to, or PTK_None
            /// when the type cannot be mapped to a primitive type.
            /// @return  Primitive type kind that this type can be mapped to,
            /// or PTK_None for none.
            virtual Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const = 0;

            /// Get Universally Unique Identifier of type.
            /// @return  Universally Unique Identifier of type.
            virtual Smp::Uuid GetUuid() const = 0;

            /// Publish an instance of the type against a receiver.
            /// Exceptions that may be raised by the publication receiver are
            /// propagated to the caller.
            /// @param   receiver Receiver to publish against.
            /// @param   name Name of instance.
            /// @param   description Description of instance.
            /// @param   address Address of instance.
            /// @param   view View kind of instance.
            /// @param   state State flag of field. When true, the field shall
            /// be part of store/restore of the simulation state.
            /// @param   input Input flag of field. When true, the field can be
            /// used as target of a dataflow connection.
            /// @param   output Output flag of field. When true, the field can
            /// be used as source of a dataflow connection.
            /// @return  Reference to the field that has been created by the
            /// publication call.
            /// @throws  Smp::Publication::TypeNotRegistered
            /// @throws  Smp::InvalidType
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            virtual Smp::IField* Publish(
                Smp::Publication::IPublishField* receiver,
                Smp::String8 name,
                Smp::String8 description,
                Smp::Void* address,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;
        };
    }
}

#endif // SMP_PUBLICATION_ITYPE_H_
