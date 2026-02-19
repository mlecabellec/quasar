// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IParameter.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IPARAMETER_H_
#define SMP_IPARAMETER_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/IObject.h"
#include "Smp/Publication/ParameterDirectionKind.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{

    namespace Publication
    {
        class IType;
    }
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This interface describes a parameter of a published operation.
    class IParameter :
        public virtual Smp::IObject
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IParameter() noexcept = default;

        /// Provides the type of the parameter.
        /// @return  Type of the parameter.
        virtual const Smp::Publication::IType* GetType() const = 0;

        /// Provides the parameter direction kind of the parameter.
        /// @return  Parameter direction kind of the parameter.
        virtual Smp::Publication::ParameterDirectionKind GetDirection() const = 0;
    };
}

#endif // SMP_IPARAMETER_H_
