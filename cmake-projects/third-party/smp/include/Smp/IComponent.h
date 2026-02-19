// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IComponent.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_ICOMPONENT_H_
#define SMP_ICOMPONENT_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/AnySimple.h"
#include "Smp/AnySimpleArray.h"
#include "Smp/ComponentStateKind.h"
#include "Smp/FieldCollection.h"
#include "Smp/InvalidArraySize.h"
#include "Smp/InvalidArrayValue.h"
#include "Smp/InvalidComponentState.h"
#include "Smp/InvalidFieldName.h"
#include "Smp/InvalidFieldValue.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Uuid.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class ICollectionBase;
    class IField;
    class IPublication;
    class ISimulator;

    namespace Services
    {
        class ILinkRegistry;
        class ILogger;
    }
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This is the base interface for all SMP components.
    /// All components need to implement this interface. As components
    /// interface to the simulation environment, they have a dependency to it
    /// via the two interfaces IPublication and ISimulator. This is the only
    /// mandatory interface components have to implement. All other interfaces
    /// are optional.
    class IComponent :
        public virtual Smp::IObject
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IComponent() noexcept = default;

        /// Returns the state the component is currently in.
        /// The component state can be changed using the Publish(), Configure()
        /// and Connect() state transition methods.
        /// @return  Current component state.
        virtual Smp::ComponentStateKind GetState() const = 0;

        /// Request the component to publish its fields, properties and
        /// operations against the provided publication receiver.
        /// This method can only be called once for each component, and only
        /// when the component is in the Created state. The method raises an
        /// InvalidComponentState exception if the component is not in Created
        /// state. When this operation is called, the component immediately
        /// enters the Publishing state, before it publishes any of its
        /// features.
        /// @remarks The simulation environment typically calls this method in
        /// the Building state.
        /// @param   receiver Publication receiver.
        /// @throws  Smp::InvalidComponentState
        virtual void Publish(
            Smp::IPublication* receiver) = 0;

        /// Request the component to perform any custom configuration. The
        /// component can create and configure other components using the field
        /// values of its published fields.
        /// This method can only be called once for each component, and only
        /// when the component is in Publishing state. The method raises an
        /// InvalidComponentState exception if the component is not in
        /// Publishing state. The component can still publish further features
        /// in this call, and can even create other components, but at the end
        /// of this call, it needs to enter the Configured state.
        /// @remarks The simulation environment typically calls this method in
        /// the Building state.
        /// @param   logger Logger service for logging of error messages during
        /// configuration.
        /// @param   linkRegistry Reference to the link registry services, so
        /// that the component can register links that it creates during
        /// configuration.
        /// @throws  Smp::InvalidComponentState
        virtual void Configure(
            Smp::Services::ILogger* logger,
            Smp::Services::ILinkRegistry* linkRegistry = nullptr) = 0;

        /// Allow the component to connect to the simulator and its simulation
        /// services.
        /// This method can only be called once for each component, and only
        /// when the component is in the Configured state. The method raises an
        /// InvalidComponentState exception if the component is not in
        /// Configured state. When this operation is called, the component
        /// immediately enters the Connected state, before it uses any of the
        /// simulator methods and services. In this method, the component may
        /// query for and use any of the available simulation services, as they
        /// are all guaranteed to be fully functional at that time. It may as
        /// well connect to components' functionality (e.g. to event sources),
        /// as it is guaranteed that all models have been created and
        /// configured before the Connect() method of any component is called.
        /// @remarks The simulation environment typically calls this method in
        /// the Connecting state.
        /// @param   simulator Simulation Environment that hosts the component.
        /// @throws  Smp::InvalidComponentState
        virtual void Connect(
            Smp::ISimulator* simulator) = 0;

        /// Ask the component to disconnect from the simulator and all its
        /// simulation services.
        /// @throws  Smp::InvalidComponentState
        virtual void Disconnect() = 0;

        /// Get the field of given name.
        /// This method can be used both for fields of simple types (when it
        /// returns an instance of ISimpleField), and for complex fields (when
        /// it returns IArrayField or IStructureField).
        /// @remarks For getting access to simple fields of structure or array
        /// types, this method may be called by specifying a full field name,
        /// e.g. "MyField.Position[2]" in order to access an array item of a
        /// structure.
        /// @param   fullName Fully qualified field name (relative to the
        /// component).
        /// @return  Instance of field for given full name, or nullptr if no
        /// field exists with the given name.
        virtual Smp::IField* GetField(
            Smp::String8 fullName) const = 0;

        /// Returns a collection of all fields of the component.
        /// @return  Collection of the fields (immediate children) of the
        /// component.
        virtual const Smp::FieldCollection* GetFields() const = 0;

        /// Get Universally Unique Identifier of Component Type.
        /// Operation to get the Universally Unique Identifier of a Component
        /// Type. This allows retrieving metadata from a Catalogue at run-time.
        /// @return  Universally Unique Identifier of Component.
        virtual const Smp::Uuid& GetUuid() const = 0;

        /// Get the value of the field or simple array field element with the
        /// given path relative to the component.
        /// This method can only be used for fields of simple types (when it
        /// can return an AnySimple), not for complex fields (as their value
        /// cannot be stored into AnySimple).
        /// This method raises an exception of type InvalidFieldName if called
        /// with a field name for which no corresponding field exists, or which
        /// does not represent a simple field.
        /// @remarks For getting access to simple fields of structure or array
        /// types, this method may be called by specifying a full field name,
        /// e.g. "MyField.Position[2]" in order to access an array item of a
        /// structure.
        /// @param   fullName Path relative to the component of the field or
        /// array field element for which the value is retrieved.
        /// @return  Value of field with given full name.
        /// @throws  Smp::InvalidFieldName
        virtual Smp::AnySimple GetSimpleValue(
            Smp::String8 fullName) const = 0;

        /// Path relative to the component of the field or array field element
        /// for which the value is set.
        /// This method can only be used for fields of simple types (when it
        /// can return an AnySimple), not for complex fields (as their value
        /// cannot be stored into AnySimple).
        /// This method raises an exception of type InvalidFieldName if called
        /// with a field name for which no corresponding field exists, or which
        /// does not represent a simple field.
        /// @remarks For getting access to simple fields of structure or array
        /// types, this method may be called by specifying a full field name,
        /// e.g. "MyField.Position[2]" in order to access an array item of a
        /// structure.
        /// @param   fullName Fully qualified field name (relative to the
        /// component).
        /// @param   value New value for field with given full name.
        /// @throws  Smp::InvalidFieldName
        /// @throws  Smp::InvalidFieldValue
        virtual void SetSimpleValue(
            Smp::String8 fullName,
            Smp::AnySimple value) = 0;

        /// Get the values of an array field or a simple array field.
        /// The array with the values has to be preallocated by the caller, and
        /// has to be released by the caller as well. Therefore, an in-out
        /// parameter is used, not a return value of the method.
        /// This method raises an exception of type InvalidArraySize if the get
        /// request spans out of the boundaries of the array field.
        /// This method raises an exception of type InvalidFieldName if called
        /// with a field name for which no corresponding field exists, or which
        /// does not represent an array field of simple array items.
        /// @param   fullName Fully qualified field name (relative to the
        /// component).
        /// @param   length Size of the preallocated values array.
        /// @param   values Preallocated array of values to store result to.
        /// @param   startIndex Start index within the simple array for which
        /// values are returned.
        /// @throws  Smp::InvalidArraySize
        /// @throws  Smp::InvalidFieldName
        virtual void GetSimpleArrayValue(
            Smp::String8 fullName,
            Smp::UInt64 length,
            Smp::AnySimple* values,
            Smp::UInt64 startIndex = 0) const = 0;

        /// Set the values of an array field or a simple array field.
        /// This method raises an exception of type InvalidArrayValue if called
        /// with values of the wrong primitive type kind.
        /// This method raises an exception of type InvalidArraySize if the set
        /// request spans out of the boundaries of the array field.
        /// This method raises an exception of type InvalidFieldName if called
        /// with a field name for which no corresponding field exists, or which
        /// does not represent an array field of simple array items.
        /// @param   fullName Fully qualified field name (relative to the
        /// component).
        /// @param   length Size of the preallocated values array.
        /// @param   values Array of values to store in array field.
        /// @param   startIndex Start index within the simple array for which
        /// values are to be set.
        /// @throws  Smp::InvalidArraySize
        /// @throws  Smp::InvalidArrayValue
        /// @throws  Smp::InvalidFieldName
        virtual void SetSimpleArrayValue(
            Smp::String8 fullName,
            Smp::UInt64 length,
            Smp::AnySimpleArray values,
            Smp::UInt64 startIndex = 0) = 0;

        /// This operation attempts to register a new object under the
        /// component.
        /// If another child with the same name is already present, the
        /// operation returns false.
        /// This operation is only to be called when an object is being added
        /// to one of the collections of this Component that participate in
        /// name resolution. The insertion shall fail if this method returns
        /// false.
        /// @param   child The child being added to the collection.
        /// @param   collection The collection in which this object is being
        /// inserted.
        /// @return  True if the child was added. False is the child name is
        /// duplicated.
        virtual Smp::Bool AddChild(
            Smp::IObject* child,
            const Smp::ICollectionBase* collection) = 0;

        /// This operation attempts to remove a child object from the component.
        /// If the child is present and the collection is the same that was
        /// used in AddChild, the operation will succeed and the return value
        /// is true.
        /// If the child is not present, or the collection is not the same that
        /// was used in AddChild, the operation will fail and the return value
        /// is false.
        /// This operation is only to be called when an object is being removed
        /// from one of the collections. Is not required to call this operation
        /// when the component (and the collections) are being destroyed.
        /// @param   child The child being removed from the collection.
        /// @param   collection The collection from which this object is being
        /// removed.
        /// @return  True if the child was removed. False otherwise.
        virtual Smp::Bool RemoveChild(
            Smp::IObject* child,
            const Smp::ICollectionBase* collection) = 0;

        /// This operation check if a name has been registered under the
        /// component, and if so returns the registered object.
        /// This operation can be used by Collections to verify whether a child
        /// is registered as part of the collection itself.
        /// @param   child The child being requested.
        /// @param   collection The collection in which this object was
        /// inserted.
        /// @return  The child object or nullptr.
        virtual Smp::IObject* IsChildInCollection(
            Smp::String8 child,
            const Smp::ICollectionBase* collection) const = 0;
    };
}

#endif // SMP_ICOMPONENT_H_
