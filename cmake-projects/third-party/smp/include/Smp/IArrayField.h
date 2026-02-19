// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IArrayField.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IARRAYFIELD_H_
#define SMP_IARRAYFIELD_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/IField.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface of a field which is of array type.
    class IArrayField :
        public virtual Smp::IField
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IArrayField() noexcept = default;

        /// Get the size (number of array items) of the field.
        /// @return  Size (number of array items) of the field.
        virtual Smp::UInt64 GetSize() const = 0;

        /// Get an array item by index.
        /// @param   index Index of item to get.
        /// @return  Array item (Field) at given index, or nullptr if index is
        /// outside of array size.
        virtual Smp::IField* GetItem(
            Smp::UInt64 index) const = 0;
    };
}

#endif // SMP_IARRAYFIELD_H_
