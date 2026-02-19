// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/PrimitiveTypes.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PRIMITIVETYPES_H_
#define SMP_PRIMITIVETYPES_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include <climits>
#include "Smp/Char8.h"
#include "Smp/Bool.h"
#include "Smp/Int8.h"
#include "Smp/UInt8.h"
#include "Smp/Int16.h"
#include "Smp/UInt16.h"
#include "Smp/Int32.h"
#include "Smp/UInt32.h"
#include "Smp/Int64.h"
#include "Smp/UInt64.h"
#include "Smp/Float32.h"
#include "Smp/Float64.h"
#include "Smp/Duration.h"
#include "Smp/DateTime.h"
#include "Smp/String8.h"
#include "Smp/PrimitiveTypeKind.h"
#include "Smp/Uuid.h"

#if CHAR_BIT != 8
// In the rare case of a platform when char != 8, a lot of assumptions are broken
// (e.g. Int8 and UInt8 presence)
#error "This platform does not respect ISO POSIX standard of 8-bit character type."
#endif

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{

    // ------------------------------------------------------------------------
    // Universal unique identifiers
    // ------------------------------------------------------------------------

    /// Placeholder class with universally unique identifiers of pre-defined SMP types.
    class Uuids
    {
    public:
        /// Unique Identifier of type Uuid.
        static constexpr Uuid Uuid_Uuid = { 0, 0, 0, { ' ',' ',' ',' ','U','u','i','d'} };

        /// Unique Identifier of type Void.
        static constexpr Uuid Uuid_Void     = { 0, 0, 0, { ' ',' ',' ',' ','V','o','i','d'} };

        /// Unique Identifier of type Char8.
        static constexpr Uuid Uuid_Char8    = { 0, 0, 0, { ' ',' ',' ','C','h','a','r','8'} };

        /// Unique Identifier of type Bool.
        static constexpr Uuid Uuid_Bool     = { 0, 0, 0, { ' ',' ',' ',' ','B','o','o','l'} };

        /// Unique Identifier of type Int8.
        static constexpr Uuid Uuid_Int8     = { 0, 0, 0, { ' ',' ',' ',' ','I','n','t','8'} };

        /// Unique Identifier of type UInt8.
        static constexpr Uuid Uuid_UInt8    = { 0, 0, 0, { ' ',' ',' ','U','I','n','t','8'} };

        /// Unique Identifier of type Int16.
        static constexpr Uuid Uuid_Int16    = { 0, 0, 0, { ' ',' ',' ','I','n','t','1','6'} };

        /// Unique Identifier of type UInt16.
        static constexpr Uuid Uuid_UInt16   = { 0, 0, 0, { ' ',' ','U','I','n','t','1','6'} };

        /// Unique Identifier of type Int32.
        static constexpr Uuid Uuid_Int32    = { 0, 0, 0, { ' ',' ',' ','I','n','t','3','2'} };

        /// Unique Identifier of type UInt32.
        static constexpr Uuid Uuid_UInt32   = { 0, 0, 0, { ' ',' ','U','I','n','t','3','2'} };

        /// Unique Identifier of type Int64.
        static constexpr Uuid Uuid_Int64    = { 0, 0, 0, { ' ',' ',' ','I','n','t','6','4'} };

        /// Unique Identifier of type UInt64.
        static constexpr Uuid Uuid_UInt64   = { 0, 0, 0, { ' ',' ','U','I','n','t','6','4'} };

        /// Unique Identifier of type Float32.
        static constexpr Uuid Uuid_Float32  = { 0, 0, 0, { ' ','F','l','o','a','t','3','2'} };

        /// Unique Identifier of type Float64.
        static constexpr Uuid Uuid_Float64  = { 0, 0, 0, { ' ','F','l','o','a','t','6','4'} };

        /// Unique Identifier of type Duration.
        static constexpr Uuid Uuid_Duration = { 0, 0, 0, { 'D','u','r','a','t','i','o','n'} };

        /// Unique Identifier of type DateTime.
        static constexpr Uuid Uuid_DateTime = { 0, 0, 0, { 'D','a','t','e','T','i','m','e'} };

        /// Unique Identifier of type String8.
        static constexpr Uuid Uuid_String8  = { 0, 0, 0, { ' ','S','t','r','i','n','g','8'} };

        /// Unique Identifier of type PrimitiveTypeKind.
        static constexpr Uuid Uuid_PrimitiveTypeKind = {0xd55b0e31, 0xe618, 0x11dc, 0xab64, 0xbf8df6d7b83a};

        /// Unique Identifier of type EventId.
        static constexpr Uuid Uuid_EventId = {0xd54589a4, 0xe618, 0x11dc, 0xab64, 0xbf8df6d7b83a};

        /// Unique Identifier of type LogMessageKind.
        static constexpr Uuid Uuid_LogMessageKind = {0xd543404f, 0xe618, 0x11dc, 0xab64, 0xbf8df6d7b83a};

        /// Unique Identifier of type TimeKind.
        static constexpr Uuid Uuid_TimeKind = {0xd54589a6, 0xe618, 0x11dc, 0xab64, 0xbf8df6d7b83a};

        /// Unique Identifier of type ViewKind.
        static constexpr Uuid Uuid_ViewKind = {0xd579e033, 0xe618, 0x11dc, 0xab64, 0xbf8df6d7b83a};

        /// Unique Identifier of type ParameterDirectionKind.
        static constexpr Uuid Uuid_ParameterDirectionKind = {0x1b3641ad, 0xf0f0, 0x11dc, 0xb3ae, 0x77a8f1ab4ab6};

        /// Unique Identifier of type ComponentStateKind.
        static constexpr Uuid Uuid_ComponentStateKind = {0xd55d57c7, 0xe618, 0x11dc, 0xab64, 0xbf8df6d7b83a};

        /// Unique Identifier of type AccessKind.
        static constexpr Uuid Uuid_AccessKind = {0xe7d5e125, 0xeb8a, 0x11dc, 0x8642, 0xc38618fe0a20};

        /// Unique Identifier of type SimulatorStateKind.
        static constexpr Uuid Uuid_SimulatorStateKind = {0xd56483dc, 0xe618, 0x11dc, 0xab64, 0xbf8df6d7b83a};
    };
}

#endif // SMP_PRIMITIVETYPES_H_
