// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IObject.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IOBJECT_H_
#define SMP_IOBJECT_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

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
    /// This interface is the base interface for almost all other SMP
    /// interfaces. While most interfaces derive from IComponent, which itself
    /// is derived from IObject, some objects (including IField, IFailure,
    /// IEntryPoint, IEventSink, IEventSource, IContainer and IReference) are
    /// directly derived from IObject.
    /// @remarks The methods of this interface ensure that all SMP objects can
    /// be shown with a name, with an optional description, and under their
    /// parent object. Only the simulator itself is expected not to have a
    /// parent object, as it is the top-level object of all other objects.
    class IObject
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IObject() noexcept = default;

        /// Return the name of the object.
        /// Names must <ul> <li>be unique within their context (i.e. for the
        /// same parent),</li> <li>not be empty,</li> <li>start with a letter,
        /// and</li> <li>only contain letters, digits, the underscore ("_") and
        /// brackets ("[" and "]").</li> </ul>
        /// @remarks Applications may display the name as user readable object
        /// identification.
        /// @return  Name of object.
        virtual Smp::String8 GetName() const = 0;

        /// Return the description of the object ("property getter").
        /// Descriptions are optional and may be empty.
        /// @remarks Applications may display the description as additional
        /// information about the object.
        /// @return  Description of object.
        virtual Smp::String8 GetDescription() const = 0;

        /// Returns the parent object of the object.
        /// Objects link to their parent to allow traversing the tree of
        /// objects upwards to the root object of a composition.
        /// @remarks Typically, only the simulator itself is a root object, so
        /// all other objects should have a parent object.
        /// @return  Parent object of object or nullptr if object has no parent.
        virtual Smp::IObject* GetParent() const = 0;

        /// Returns the child object with the given name, or nullptr if no such
        /// object is present among the children of this object.
        /// GetChild can be used while adding elements under this object to
        /// check if a DuplicateName exception has to be raised.
        /// GetChild has to provide all and only the objects that the Resolver
        /// shall resolve. The Resolver can be implemented in terms of a chain
        /// of GetChild and GetParent.
        /// @param   name The name of the requested child object.
        /// @return  The children object with the given name, or nullptr.
        virtual Smp::IObject* GetChild(
            Smp::String8 name) const = 0;
    };
}

#endif // SMP_IOBJECT_H_
