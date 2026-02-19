// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Exception.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_EXCEPTION_H_
#define SMP_EXCEPTION_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/PrimitiveTypes.h"
#include <exception>

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
    /// This is the base class for all SMP exceptions.
    /// This exception is the base class for all other SMP exceptions. It
    /// provides Name, Description and Message.
    class Exception : public virtual std::exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~Exception() noexcept = default;

        /// Implements the C++ standard interface for exceptions.
        /// @return  Name of the exception class.
        virtual const Smp::Char8* what() const noexcept override = 0;

        /// Returns the name of the exception class. This name can be used e.g.
        /// for debugging purposes.
        /// @return  Name of the exception class.
        virtual Smp::String8 GetName() const noexcept = 0;

        /// Returns a textual description of the exception class. This
        /// description can be used e.g. for debugging purposes.
        /// @return  Textual description of the exception class.
        virtual Smp::String8 GetDescription() const noexcept = 0;

        /// Returns the description of the problem encountered. This message
        /// can be used e.g. for debugging purposes.
        /// @return  Textual description of the problem encountered.
        virtual Smp::String8 GetMessage() const noexcept = 0;

        /// Returns the sender of the exception instance. This object (and its
        /// name and path) can be used e.g. for debugging purposes.
        /// @return  Object that emitted the exception.
        virtual const Smp::IObject* GetSender() const noexcept = 0;
    };
}

#endif // SMP_EXCEPTION_H_
