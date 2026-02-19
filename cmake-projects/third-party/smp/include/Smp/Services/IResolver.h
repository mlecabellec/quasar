// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Services/IResolver.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_SERVICES_IRESOLVER_H_
#define SMP_SERVICES_IRESOLVER_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/IService.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IObject;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for simulation services
    namespace Services
    {
        /// This interface gives access to the Resolver Service.
        class IResolver :
            public virtual Smp::IService
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~IResolver() noexcept = default;

            /// Resolve reference to an object via absolute path.
            /// An absolute path contains the name of either a Model or
            /// Service, but not the name of the simulator, although the
            /// simulator itself is the top-level object. This allows keeping
            /// names as short as possible, and avoids a dependency on the name
            /// of the simulator itself. The simulator can be retrieved with
            /// the path "/".
            /// @param   absolutePath Absolute path to object in simulation.
            /// @return  Object identified by path, or nullptr if no object
            /// with the given path could be found.
            virtual Smp::IObject* ResolveAbsolute(
                Smp::String8 absolutePath) = 0;

            /// Resolve reference to an object via relative path.
            /// @param   relativePath Relative path to object in simulation.
            /// @param   relativeRoot Object relative to which the path shall
            /// be resolved
            /// @return  Object identified by relative path, or nullptr if no
            /// object with the given path could be found.
            virtual Smp::IObject* ResolveRelative(
                Smp::String8 relativePath,
                Smp::IObject* relativeRoot) = 0;
        };
    }
}

#endif // SMP_SERVICES_IRESOLVER_H_
