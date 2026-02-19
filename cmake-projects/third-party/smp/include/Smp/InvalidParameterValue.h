// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidParameterValue.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDPARAMETERVALUE_H_
#define SMP_INVALIDPARAMETERVALUE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/InvalidAnyType.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is raised when trying to assign an invalid value to a
    /// parameter of an operation, either in setting up the Request or handling
    /// it in the Invoke.
    /// For the return parameter, the parameter name is the empty string.
    class InvalidParameterValue : public virtual Smp::InvalidAnyType
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidParameterValue() noexcept = default;

        /// Get the name of operation being handled.
        /// @return  Name of operation being handled.
        virtual Smp::String8 GetOperationName() const noexcept = 0;

        /// Get the name of parameter being assigned.
        /// @return  Name of parameter value was assigned to.
        virtual Smp::String8 GetParameterName() const noexcept = 0;
    };
}

#endif // SMP_INVALIDPARAMETERVALUE_H_
