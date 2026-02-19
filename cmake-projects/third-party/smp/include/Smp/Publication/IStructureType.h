// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/IStructureType.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_ISTRUCTURETYPE_H_
#define SMP_PUBLICATION_ISTRUCTURETYPE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/DuplicateName.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/InvalidType.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/IType.h"
#include "Smp/Publication/TypeNotRegistered.h"
#include "Smp/Uuid.h"
#include "Smp/ViewKind.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for publication
    namespace Publication
    {
        /// This interface defines a user defined structure type.
        class IStructureType :
            public virtual Smp::Publication::IType
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~IStructureType() noexcept = default;

            /// Add a field to the Structure.
            /// Throws DuplicateName if a field with this name has already been
            /// added before.
            /// Throws InvalidObjectName if the given name is not a valid field
            /// name for a structure field.
            /// Throws TypeNotRegistered if no type with the given type UUID
            /// exists.
            /// Throws InvalidType if publication type is not suitable for
            /// field publication (e.g. String8).
            /// @param   name Name of field.
            /// @param   description Description of field.
            /// @param   uuid Uuid of field Type, which must be a value type,
            /// but not String8.
            /// @param   offset Memory offset of field relative to Structure.
            /// @param   view View kind of field.
            /// @param   state State flag of field. When true, the field shall
            /// be part of store/restore of the simulation state.
            /// @param   input Input flag of field. When true, the field can be
            /// used as target of a dataflow connection.
            /// @param   output Output flag of field. When true, the field can
            /// be used as source of a dataflow connection.
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            /// @throws  Smp::Publication::TypeNotRegistered
            /// @throws  Smp::InvalidType
            virtual void AddField(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Uuid uuid,
                Smp::UInt64 offset,
                Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true,
                Smp::Bool input = false,
                Smp::Bool output = false) = 0;
        };
    }
}

#endif // SMP_PUBLICATION_ISTRUCTURETYPE_H_
