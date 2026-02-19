// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/IClassType.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_ICLASSTYPE_H_
#define SMP_PUBLICATION_ICLASSTYPE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Publication/IStructureType.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for publication
    namespace Publication
    {
        /// This interface defines a user defined class type.
        class IClassType :
            public virtual Smp::Publication::IStructureType
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~IClassType() noexcept = default;
        };
    }
}

#endif // SMP_PUBLICATION_ICLASSTYPE_H_
