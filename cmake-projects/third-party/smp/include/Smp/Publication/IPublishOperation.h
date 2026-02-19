// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/IPublishOperation.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_IPUBLISHOPERATION_H_
#define SMP_PUBLICATION_IPUBLISHOPERATION_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/DuplicateName.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/InvalidType.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/InvalidParameterDirection.h"
#include "Smp/Publication/ParameterDirectionKind.h"
#include "Smp/Publication/TypeNotRegistered.h"
#include "Smp/Uuid.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for publication
    namespace Publication
    {
        /// This interface provides functionality to publish operation
        /// parameters.
        class IPublishOperation
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~IPublishOperation() noexcept = default;

            /// Publish a parameter of an operation with the given name,
            /// description, type and direction.
            /// If the name is not a valid name, an exception of type
            /// InvalidObjectName is thrown.
            /// If a parameter with the same name has already been published,
            /// and exception of type DuplicateName is thrown.
            /// If no type with the given type UUID exists, an exception of
            /// type TypeNotRegistered is thrown.
            /// If the parameter is of type return, but another parameter of
            /// type return has already been published for the same operation,
            /// it throws an exception of type InvalidParameterDirection.
            /// It the primitive type kind of the publication type is None, it
            /// throws an exception of type InvalidType.
            /// @param   name Name of parameter.
            /// @param   description Description of parameter.
            /// @param   typeUuid Uuid of parameter type.
            /// @param   direction Direction kind of parameter.
            /// @throws  Smp::Publication::TypeNotRegistered
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::InvalidObjectName
            /// @throws  Smp::Publication::InvalidParameterDirection
            /// @throws  Smp::InvalidType
            virtual void PublishParameter(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Uuid typeUuid,
                Smp::Publication::ParameterDirectionKind direction = Smp::Publication::ParameterDirectionKind::PDK_In) = 0;
        };
    }
}

#endif // SMP_PUBLICATION_IPUBLISHOPERATION_H_
