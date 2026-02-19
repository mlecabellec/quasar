// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/InvalidParameterDirection.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_INVALIDPARAMETERDIRECTION_H_
#define SMP_PUBLICATION_INVALIDPARAMETERDIRECTION_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for publication
    namespace Publication
    {
        /// This exception is raised when trying to add a second return
        /// parameter to an Operation.
        class InvalidParameterDirection : public virtual Smp::Exception
        {
        public:

            /// Virtual destructor to release memory.
            virtual ~InvalidParameterDirection() noexcept = default;

            /// Get the name of the invalid parameter that cannot be added.
            /// @return  Name of the new parameter that cannot be added.
            virtual Smp::String8 GetParameterName() const noexcept = 0;
        };
    }
}

#endif // SMP_PUBLICATION_INVALIDPARAMETERDIRECTION_H_
