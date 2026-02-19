// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Services/TimeKind.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_SERVICES_TIMEKIND_H_
#define SMP_SERVICES_TIMEKIND_H_

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
    /// Namespace for simulation services
    namespace Services
    {
        /// Enumeration of supported time kinds.
        enum class TimeKind : Smp::Int32
        {
            /// Simulation time.
            /// Simulation time is a relative time. It does only exist within
            /// the time keeper service. The following holds for simulation
            /// time: <ul> <li>Simulation time is a non-negative value measured
            /// in nanoseconds, which is the lowest level of granularity
            /// supported for time in SMP.</li> <li>Simulation time is stored
            /// in a signed 64-bit integer value. This allows specifying time
            /// values of more than 290 years.</li> <li>Simulation time can be
            /// queried using the GetSimulationTime() method of the time keeper
            /// (via the ITimeKeeper interface).</li> <li>Simulation time is
            /// initialised to 0 at the beginning of the initialisation phase.
            /// That is, during initialisation the time keeper service will
            /// return a simulation time of 0.</li> <li>Simulation time is only
            /// progressed when the simulation environment is in Executing
            /// state.</li> <li>When storing a state vector, simulation time is
            /// stored as well.</li> <li>When restoring a state vector,
            /// simulation time is restored as well.</li> </ul> The standard
            /// does not define how quickly simulation time is progressed when
            /// the simulator is in Executing state. Typical examples are: <ul>
            /// <li><b>Real-Time</b>: The simulation time progresses with
            /// real-time, where real-time is typically defined by the computer
            /// clock. Note that two types of real-time simulations exist: hard
            /// real-time and soft real-time simulations. In a hard real-time
            /// simulation, strict requirements on timing have to be met, while
            /// in a soft real-time simulation, the requirements are less
            /// demanding such that latencies in a certain range are allowed,
            /// which is called real-time slip.</li> <li><b>Accelerated</b>:
            /// The simulation time progresses relative to real-time using a
            /// constant acceleration factor. This factor may be larger than
            /// 1.0, which relates to “faster than real-time”, smaller than
            /// 1.0, which means “slower than real-time”, or 1.0, which
            /// coincides with real-time.</li> <li><b>Free Running</b>: The
            /// simulation time progresses as fast as possible, and is not
            /// related to real-time. Typically, the speed is coordinated with
            /// the timed events of the scheduler, which underlines the close
            /// relationship between these two services (Time Keeper and
            /// Scheduler).</li> <li><b>Debugging</b>: The simulation is
            /// executed in a step-by-step manner using break points in order
            /// to inspect data or trace calls within the simulation.</li>
            /// </ul> SMP does not mandate which of these modes a simulation
            /// environment has to support.
            TK_SimulationTime,

            /// Mission time.
            /// Mission time is a relative time, i.e. it measures elapsed time
            /// from a definite point in time (called the mission start).
            /// Mission time is stored as a number relative to the mission
            /// start date. Mission time is maintained using a fixed offset to
            /// epoch time, and hence progresses together with simulation and
            /// epoch time, except for the case when the offset is changed. The
            /// following holds for mission time: <ul> <li>Mission time is a
            /// value measured in nanoseconds, which is the lowest level of
            /// granularity supported for time in SMP.</li> <li>Mission time is
            /// returned as a signed 64-bit integer value, relative to a
            /// mission start date and time (which itself is not stored).</li>
            /// <li>Mission time can be queried using the GetMissionTime()
            /// method of the time keeper (via the ITimeKeeperinterface).</li>
            /// <li>Mission time is initialised to 0 at the beginning of the
            /// initialisation phase, but can be changed already before
            /// entering the execution phase. As epoch time is initialised to
            /// 01.01.2000, 12:00, the default mission start is as well
            /// 01.01.2000, 12:00.</li> <li>Mission time is progressed linearly
            /// with epoch and simulation time (i.e. with a fixed offset to
            /// epoch time). Using either the SetMissionTime() method or the
            /// SetMissionStart() method of the time keeper (via the
            /// ITimeKeeper interface), the offset between simulation time and
            /// mission time can be changed.</li> <li>When storing a state
            /// vector, mission time is stored as well.</li> <li>When restoring
            /// a state vector, mission time is restored as well.</li> </ul>
            TK_MissionTime,

            /// Epoch time.
            /// Epoch time is an absolute time, i.e. it defines a definite
            /// point in time. It is not only used as a way to express date and
            /// time, but as well to determine all time-dependent variables at
            /// that time, such as barycentric positions of all solar system
            /// bodies. Epoch time is stored as a number relative to a
            /// reference date, which has been defined as the 1st of January
            /// 2000 mid-day (01.01.2000, 12:00). Epoch time is maintained
            /// using a fixed offset to simulation time, and hence progresses
            /// together with simulation time, except for the case when the
            /// offset is changed. The following holds for epoch time: <ul>
            /// <li>Epoch time is a value measured in nanoseconds, which is the
            /// lowest level of granularity supported for time in SMP.</li>
            /// <li>Epoch time is returned as a signed 64-bit integer value,
            /// relative to the epoch reference time (01.01.2000, 12:00,
            /// Modified Julian Date 2000+0.5). This allows specifying time
            /// values roughly between 1710 and 2290.</li> <li>Epoch time can
            /// be queried using the GetEpochTime() method of the time keeper
            /// (via the ITimeKeeperinterface).</li> <li>Epoch time is
            /// initialised to 0 (i.e. 01.01.2000, 12:00) at the beginning of
            /// the initialisation phase, but can be changed already before
            /// entering the execution phase.</li> <li>Epoch time is progressed
            /// linearly with simulation time (i.e. with a fixed offset to
            /// simulation time). Using the SetEpochTime() method of the time
            /// keeper (via the ITimeKeeper interface), the offset between
            /// simulation time and epoch time can be changed.</li> <li>When
            /// storing a state vector, epoch time (i.e. its offset to
            /// simulation time) is stored as well.</li> <li>When restoring a
            /// state vector, epoch time (i.e. its offset to simulation time)
            /// is restored as well.</li> </ul>
            TK_EpochTime,

            /// Zulu time.
            /// From the Mobile Aeronautics Education Laboratory (MAEL) of the
            /// NASA, the following definition of Zulu Time is cited
            /// (http://www.grc.nasa.gov/WWW/MAEL/ag/zulu.htm): “The world is
            /// divided into 24 time zones. For easy reference in
            /// communications, a letter of the alphabet has been assigned to
            /// each time zone. The "clock" at Greenwich, England is used as
            /// the standard clock for international reference of time in
            /// communications, military, maritime and other activities that
            /// cross time zones. The letter designator for this clock is Z.
            /// Times are usually written in military time or 24 hour format
            /// such as 1830Z (6:30 pm). To pronounce this, the phonetic
            /// alphabet is used for the letter Z, or Zulu. This time is
            /// sometimes referred to as Zulu Time because of its assigned
            /// letter. Its official name is Coordinated Universal Time or UTC.
            /// Previously it had been known as Greenwich Mean Time or GMT but
            /// this has been replaced with UTC.” In SMP, Zulu time is not
            /// related to simulation time, but typically to the computer clock
            /// (or to some external clock). The following holds for Zulu time:
            /// <ul> <li>Zulu time is measured in nanoseconds.</li> <li>Zulu
            /// time is returned as a signed 64-bit integer value, relative to
            /// the epoch reference time.</li> <li>Zulu time represents the
            /// current time at Greenwich, England, called as well UTC or
            /// GMT.</li> </ul> As Zulu time is not managed by the time keeper
            /// service, but provided based on an external clock (typically the
            /// computer clock), it is not related to simulation time, and
            /// progresses independently of the state of the simulation
            /// environment. When a simulator interfaces to an external system,
            /// for example a ground station or some Hardware-In-The-Loop
            /// (HITL), Zulu time is often used as a time stamp.
            TK_ZuluTime
        };

        /// Stream operator for TimeKind to be able to print an enum value.
        /// @param os Output stream to output to.
        /// @param obj Object to output to stream.
        std::ostream& operator << (std::ostream& os, const TimeKind& obj);
    }
}

#endif // SMP_SERVICES_TIMEKIND_H_
