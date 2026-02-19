// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/ISimpleArrayField.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_ISIMPLEARRAYFIELD_H_
#define SMP_ISIMPLEARRAYFIELD_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/AnySimple.h"
#include "Smp/AnySimpleArray.h"
#include "Smp/IField.h"
#include "Smp/InvalidArrayIndex.h"
#include "Smp/InvalidArraySize.h"
#include "Smp/InvalidArrayValue.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface to an array where each array item is of a simple type.
    class ISimpleArrayField :
        public virtual Smp::IField
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~ISimpleArrayField() noexcept = default;

        /// Get the size (number of array items) of the field.
        /// @return  Size (number of array items) of the field.
        virtual Smp::UInt64 GetSize() const = 0;

        /// Get a value from a specific index of the array field.
        /// This method raises an exception of type InvalidArrayIndex if called
        /// with an index value beyond the size of the array.
        /// @param   index Index of value to get.
        /// @return  Value from given index.
        /// @throws  Smp::InvalidArrayIndex
        virtual Smp::AnySimple GetValue(
            Smp::UInt64 index) const = 0;

        /// Set a value at given index of the array field.
        /// This method raises an exception of type InvalidArrayValue if called
        /// with an invalid value and an exception of type InvalidArrayIndex if
        /// called with an index value beyond the size of the array.
        /// @param   index Index of value to set.
        /// @param   value Value to set at given index.
        /// @throws  Smp::InvalidArrayIndex
        /// @throws  Smp::InvalidArrayValue
        virtual void SetValue(
            Smp::UInt64 index,
            Smp::AnySimple value) = 0;

        /// Get the values of the simple array field.
        /// The array with the values has to be pre-allocated by the caller,
        /// and has to be released by the caller as well. Therefore, an inout
        /// parameter is used, not a return value of the method.
        /// This method raises an exception of type InvalidArraySize if the get
        /// request spans out of the boundaries of the array field.
        /// @param   length Size of the preallocated values array.
        /// @param   values Preallocated array of values to store result to.
        /// @param   startIndex Start index within the simple array for which
        /// values are returned.
        /// @throws  Smp::InvalidArraySize
        virtual void GetValues(
            Smp::UInt64 length,
            Smp::AnySimple* values,
            Smp::UInt64 startIndex = 0) const = 0;

        /// Set the values of the simple array field.
        /// This method raises an exception of type InvalidArrayValue if called
        /// with values of the wrong primitive type kind.
        /// This method raises an exception of type InvalidArraySize if the set
        /// request spans out of the boundaries of the array field.
        /// @param   length Size of the preallocated values array.
        /// @param   values Array of values to store in array field.
        /// @param   startIndex Start index within the simple array for which
        /// values are to be set.
        /// @throws  Smp::InvalidArraySize
        /// @throws  Smp::InvalidArrayValue
        virtual void SetValues(
            Smp::UInt64 length,
            Smp::AnySimpleArray values,
            Smp::UInt64 startIndex = 0) = 0;
    };
}

#endif // SMP_ISIMPLEARRAYFIELD_H_
