// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Duration.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_DURATION_H_
#define SMP_DURATION_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include <cstdint>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Duration in nanoseconds.
    /// This type is used for relative time values. It specifies a duration in
    /// nanoseconds. The following holds for Duration:
    ///      - Duration is a value measured in nanoseconds, which is the lowest
    /// level of granularity supported for time in SMP.
    ///      - Duration is stored as a signed 64-bit integer value.
    ///      - Positive values correspond to positive durations, and negative
    /// values correspond to negative durations.
    /// This allows specifying duration values roughly between -290 years and
    /// 290 years. With this definition, the Duration type is compatible with
    /// the DateTime type.
    typedef int64_t Duration;
}

#endif // SMP_DURATION_H_
