// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/PrimitiveTypeKind.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PRIMITIVETYPEKIND_H_
#define SMP_PRIMITIVETYPEKIND_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include <iosfwd>
#include "Smp/Int32.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This is an enumeration of the available primitive types.
    enum class PrimitiveTypeKind : Smp::Int32
    {
        /// No type, e.g. for void.
        PTK_None,

        /// 8 bit character type.
        PTK_Char8,

        /// Boolean with true and false.
        PTK_Bool,

        /// 8 bit signed integer type.
        PTK_Int8,

        /// 8 bit unsigned integer type.
        PTK_UInt8,

        /// 16 bit signed integer type.
        PTK_Int16,

        /// 16 bit unsigned integer type.
        PTK_UInt16,

        /// 32 bit signed integer type.
        PTK_Int32,

        /// 32 bit unsigned integer type.
        PTK_UInt32,

        /// 64 bit signed integer type.
        PTK_Int64,

        /// 64 bit unsigned integer type.
        PTK_UInt64,

        /// 32 bit single-precision floating-point type.
        PTK_Float32,

        /// 64 bit double-precision floating-point type.
        PTK_Float64,

        /// Duration in nanoseconds.
        PTK_Duration,

        /// Absolute time in nanoseconds.
        PTK_DateTime,

        /// 8 bit character string.
        PTK_String8
    };

    /// Stream operator for PrimitiveTypeKind to be able to print an enum value.
    /// @param os Output stream to output to.
    /// @param obj Object to output to stream.
    std::ostream& operator << (std::ostream& os, const PrimitiveTypeKind& obj);
}

#endif // SMP_PRIMITIVETYPEKIND_H_
