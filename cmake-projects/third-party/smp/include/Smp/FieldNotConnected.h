// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/FieldNotConnected.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_FIELDNOTCONNECTED_H_
#define SMP_FIELDNOTCONNECTED_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Exception.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IField;
    class IOutputField;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is raised when trying to disconnect a target field from
    /// a data flow field that has not been connected before.
    class FieldNotConnected : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~FieldNotConnected() noexcept = default;

        /// Get the field for which the Disconnect operation was called.
        /// @return  Field for which the Disconnect operation was called
        virtual const Smp::IOutputField* GetSource() const noexcept = 0;

        /// Get the target field that was passed to the Disconnect operation.
        /// @return  Target field that was passed to the Disconnect operation.
        virtual const Smp::IField* GetTarget() const noexcept = 0;
    };
}

#endif // SMP_FIELDNOTCONNECTED_H_
