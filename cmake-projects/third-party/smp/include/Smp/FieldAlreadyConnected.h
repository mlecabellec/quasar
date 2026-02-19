// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/FieldAlreadyConnected.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_FIELDALREADYCONNECTED_H_
#define SMP_FIELDALREADYCONNECTED_H_

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
    /// This exception is raised when trying to connect a target field to a
    /// data flow field that is already connected.
    class FieldAlreadyConnected : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~FieldAlreadyConnected() noexcept = default;

        /// Get the field for which the Connect operation was called.
        /// @return  Field for which the Connect operation was called
        virtual const Smp::IOutputField* GetSource() const noexcept = 0;

        /// Get the target field that was passed to the Connect operation.
        /// @return  Target field that was passed to the Connect operation.
        virtual const Smp::IField* GetTarget() const noexcept = 0;
    };
}

#endif // SMP_FIELDALREADYCONNECTED_H_
