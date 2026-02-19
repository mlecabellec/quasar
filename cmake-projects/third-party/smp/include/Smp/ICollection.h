// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/ICollection.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_ICOLLECTION_H_
#define SMP_ICOLLECTION_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/CollectionIterator.h"
#include "Smp/ICollectionBase.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface for a collection.
    /// A collection allows querying for the contained elements.
    /// Elements in the collection can be queried by name and by position.
    /// The query by position must always return the elements based on order of
    /// insertion.
    template <typename T>
    class ICollection :
        public virtual Smp::ICollectionBase
    {
    public:
        /// Define const_iterator
        typedef Smp::CollectionIterator<T> const_iterator;

        /// Define iterator
        typedef Smp::CollectionIterator<T> iterator;

        /// Virtual destructor to release memory.
        virtual ~ICollection() noexcept = default;

        /// Retrieve element by name
        /// @param name The name of the element to be retrieved.
        /// @return Instance with given name, or nullptr if no such instance exists.
        virtual T* at(
            Smp::String8 name) const = 0;

        /// Retrieve element by position in the sequence (based on order of
        /// insertion).
        /// @param index The position of the elment to be retrieved.
        /// @return Instance at given index, or nullptr if no instance exists.
        virtual T* at(
            size_t index) const = 0;

        /// Get the number of objects in the sequence.
        /// @return Number of objects in the sequence.
        virtual size_t size() const = 0;

        /// Check whether the collection is empty.
        /// @return True if the collection is empty, false otherwise.
        virtual Smp::Bool empty() const = 0;

        /// Get the begin iterator
        /// @return Begin iterator
        virtual const_iterator begin() const = 0;

        /// Get the end iterator
        /// @return End iterator
        virtual const_iterator end() const = 0;
    };
}

#endif // SMP_ICOLLECTION_H_
