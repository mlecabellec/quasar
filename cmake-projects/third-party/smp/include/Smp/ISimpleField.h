// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/ISimpleField.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_ISIMPLEFIELD_H_
#define SMP_ISIMPLEFIELD_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/AnySimple.h"
#include "Smp/IField.h"
#include "Smp/InvalidFieldValue.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface of a field of simple type.
    class ISimpleField :
        public virtual Smp::IField
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~ISimpleField() noexcept = default;

        /// Get primitive type kind that this simple field uses.
        /// @return  Primitive type kind that this field uses.
        virtual Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const = 0;

        /// Get the value of the simple field.
        /// @return  Field value.
        virtual Smp::AnySimple GetValue() const = 0;

        /// Set the value of the simple field.
        /// If the given value simple type kind does match the simple type kind
        /// of the field, than it changes the field value to the given value.
        /// If the given value simple type kind does not match the simple type
        /// kind of the field, then it throws the InvalidFieldValue exception.
        /// @param   value Field value.
        /// @throws  Smp::InvalidFieldValue
        virtual void SetValue(
            Smp::AnySimple value) = 0;
    };
}

#endif // SMP_ISIMPLEFIELD_H_
