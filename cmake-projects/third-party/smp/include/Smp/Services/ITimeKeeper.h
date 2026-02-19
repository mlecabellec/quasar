// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Services/ITimeKeeper.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_SERVICES_ITIMEKEEPER_H_
#define SMP_SERVICES_ITIMEKEEPER_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/IService.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Services/InvalidSimulationTime.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for simulation services
    namespace Services
    {
        /// This interface gives access to the Time Keeper Service.
        /// Components can query for the time (using the four available time
        /// kinds), and can change the epoch or mission time.
        class ITimeKeeper :
            public virtual Smp::IService
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~ITimeKeeper() noexcept = default;

            /// Return Simulation time.
            /// Simulation time is a relative time that starts at 0.
            /// @return  Current simulation time.
            virtual Smp::Duration GetSimulationTime() const = 0;

            /// Return Epoch time.
            /// Epoch time is an absolute time with a fixed offset to
            /// simulation time. Epoch time typically progresses with
            /// simulation time, but can be changed with SetEpochTime.
            /// @return  Current epoch time.
            virtual Smp::DateTime GetEpochTime() const = 0;

            /// Get mission start time.
            /// @return  Mission start date and time.
            virtual Smp::DateTime GetMissionStartTime() const = 0;

            /// Return Mission time.
            /// Mission time is a relative time with a fixed offset to
            /// simulation time. Mission time typically progresses with
            /// simulation time, but can be changed with the two methods
            /// SetMissionTime and SetMissionStart. Further, mission time is
            /// updated when changing epoch time with SetEpochTime.
            /// @return  Current mission time.
            virtual Smp::Duration GetMissionTime() const = 0;

            /// Return Zulu time.
            /// Zulu time is a system dependent time and not related to
            /// simulation time. Zulu time is typically related to the system
            /// clock of the computer.
            /// @return  Current Zulu time.
            virtual Smp::DateTime GetZuluTime() const = 0;

            /// Manually advance Simulation time.
            /// This method can only be called during a PreSimTimeChange event.
            /// When the Time Keeper updates simulation time in response to the
            /// Scheduler executing a new event, the update is performed in
            /// three steps. 1. First the PreSimTimeChange event is emitted. 2.
            /// Then the Simulation time is changed to the time of the event
            /// that is about to be executed. 3. Finally the PostSimTimeChange
            /// event is emitted. This method can only set SimTime between
            /// current time and the time that is about to the set by the
            /// procedure described above. Also, invoking this method will not
            /// trigger again the PreSimTimeChange or PostSimTimeChange events.
            /// @param   simulationTime New value of simulation time to set in
            /// the Time Keeper. This has to be in the future.
            /// @throws  Smp::Services::InvalidSimulationTime
            virtual void SetSimulationTime(
                Smp::Duration simulationTime) = 0;

            /// Set Epoch time.
            /// Changes the offset between simulation time and epoch time.
            /// Calling this method shall raise a global EpochTimeChanged event
            /// in the Event Manager.
            /// @param   epochTime New epoch time.
            virtual void SetEpochTime(
                Smp::DateTime epochTime) = 0;

            /// Set Mission time by defining the mission start time.
            /// In future calls to GetMissionTime, the mission time is
            /// calculated using the formula MissionTime = EpochTime –
            /// missionStart.
            /// Calling this method shall raise a global MissionTimeChanged
            /// event in the Event Manager.
            /// Changes the offset between simulation time and mission time.
            /// The mission time itself will be calculated as the offset
            /// between the current epoch time and the given mission start:
            /// MissionTime = EpochTime - MissionStart
            /// @param   missionStart New mission start date and time.
            virtual void SetMissionStartTime(
                Smp::DateTime missionStart) = 0;

            /// Set Mission time by providing the current mission time.
            /// This effectively sets the MissionStartTime using the formula
            /// MissionStartTime = EpochTime - missionTime.
            /// Calling this method shall raise a global MissionTimeChanged
            /// event in the Event Manager.
            /// Changes the offset between simulation time and mission time.
            /// @param   missionTime New mission time.
            virtual void SetMissionTime(
                Smp::Duration missionTime) = 0;
        };
    }
}

#endif // SMP_SERVICES_ITIMEKEEPER_H_
