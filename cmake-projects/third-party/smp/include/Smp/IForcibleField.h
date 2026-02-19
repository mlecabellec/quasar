// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IForcibleField.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IFORCIBLEFIELD_H_
#define SMP_IFORCIBLEFIELD_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/AnySimple.h"
#include "Smp/InvalidFieldValue.h"
#include "Smp/ISimpleField.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface of a forcible field.
    class IForcibleField :
        public virtual Smp::ISimpleField
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IForcibleField() noexcept = default;

        /// Force field to given value.
        /// This method raises an exception of type InvalidFieldValue if called
        /// with an invalid value.
        /// @param   value Value to force field to.
        /// @throws  Smp::InvalidFieldValue
        virtual void Force(
            Smp::AnySimple value) = 0;

        /// Unforce field.
        virtual void Unforce() = 0;

        /// Query for the forced state of the field.
        /// @return  Whether the field is forced or not.
        virtual Smp::Bool IsForced() const = 0;

        /// Force field to its current value.
        virtual void Freeze() = 0;
    };
}

#endif // SMP_IFORCIBLEFIELD_H_
