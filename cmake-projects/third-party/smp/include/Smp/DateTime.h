// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/DateTime.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_DATETIME_H_
#define SMP_DATETIME_H_

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
    /// Absolute time in nanoseconds.
    /// This type is used for absolute time values. It specifies a time in
    /// nanoseconds, relative to a reference time. The following holds for
    /// DateTime:
    ///      - Time is a value measured in nanoseconds, which is the lowest
    /// level of granularity supported for time in SMP.
    ///      - Time is stored as a signed 64-bit integer value, relative to the
    /// reference time (01.01.2000, 12:00, Modified Julian Date (MJD) 2000+0.5).
    ///      - Positive values correspond to times after the reference time,
    /// and negative values correspond to time values before the reference time.
    /// This allows specifying time values roughly between 1710 and 2290. With
    /// this definition, the DateTime type is compatible with the Duration type.
    typedef int64_t DateTime;
}

#endif // SMP_DATETIME_H_
