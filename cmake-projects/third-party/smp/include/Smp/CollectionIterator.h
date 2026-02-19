// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/CollectionIterator.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_COLLECTIONITERATOR_H_
#define SMP_COLLECTIONITERATOR_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/PrimitiveTypes.h"
#include <cstddef>
#include <iterator>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// Forward declaration of types
namespace Smp
{
    template <typename T>
    class ICollection;

    /// Iterator implementation for ICollection interfaces.
    /// The C++ language requires iterator to be actual final types rather than
    /// interfaces. To allow collections to be iterated, the Collection
    /// Iterator implements the C++ iterator concept accessing the ICollection
    /// elements sequentially.
    template <typename T>
    class CollectionIterator final
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T*;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        /// Constructor.
        /// @param collection The collection referenced by the iterator.
        /// @param index The index of the element referenced by the iterator.
        CollectionIterator(
            const Smp::ICollection<T>& collection,
            size_t index) noexcept
            : collection(&collection), index(index)
        {
        }

        /// Defaulted copy constructor.
        /// @param other Other instance to copy from.
        CollectionIterator(const CollectionIterator<T>& other) noexcept = default;

        /// Defaulted copy operator.
        /// @param other Other instance to copy from.
        /// @return collection iterator.
        CollectionIterator& operator=(const CollectionIterator<T>& other) noexcept = default;

        /// Defaulted move constructor.
        /// @param other Other instance to move from.
        CollectionIterator(CollectionIterator<T>&& other) noexcept = default;

        /// Defaulted move operator.
        /// @param other Other instance to move from.
        /// @return collection iterator.
        CollectionIterator& operator=(CollectionIterator<T>&& other) noexcept = default;

        /// Dereference operator.
        /// @return the value referenced by this iterator.
        value_type operator*() const
        {
            return collection->at(index);
        }

        /// Array dereference operator.
        /// @param n the displacement to apply to the iterator.
        /// @return the value referenced by this iterator displaced by the provided amount.
        value_type operator[](std::ptrdiff_t n) const
        {
            return collection->at(index + n);
        }

        /// Pre-increment operator.
        /// @return itself (after increment).
        CollectionIterator<T>& operator++() noexcept
        {
            ++(this->index);
            return *this;
        }
        
        /// Post-increment operator.
        /// @return a copy of itself before the increment.
        CollectionIterator<T> operator++(int) noexcept
        {
            auto before = *this;
            ++(this->index);
            return before;
        }

        /// Pre-decrement operator.
        /// @return itself (after decrement).
        CollectionIterator<T>& operator--() noexcept
        {
            --(this->index);
            return *this;
        }

        /// Post-decrement operator.
        /// @return a copy of itself before the decrement.
        CollectionIterator<T> operator--(int) noexcept
        {
            auto before = *this;
            --(this->index);
            return before;
        }

        /// Addition assignment operator.
        /// @param n the amount to add to the iterator.
        /// @return itself (after addition).
        CollectionIterator<T>& operator+=(std::ptrdiff_t n) noexcept
        {
            this->index += n;
            return *this;
        }

        /// Subtraction assignment operator.
        /// @param n the amount to subtract from the iterator.
        /// @return itself (after subtraction).
        CollectionIterator<T>& operator-=(std::ptrdiff_t n) noexcept
        {
            this->index -= n;
            return *this;
        }

        /// Addition operator.
        /// @param n the amount to add to the iterator.
        /// @return iterator obtained from addition.
        CollectionIterator<T> operator+(std::ptrdiff_t n) const noexcept
        {
            return {*this->collection, this->index + n};
        }

        /// Subtraction operator.
        /// @param n the amount to subtract from the iterator.
        /// @return iterator obtained from subtraction.
        CollectionIterator<T> operator-(std::ptrdiff_t n) const noexcept
        {
            return {*this->collection, this->index - n};
        }

        /// Inequality operator
        /// @param other Other instance to compare with.
        /// @return True if instances are different, false otherwise.
        Smp::Bool operator!=(const CollectionIterator<T>& other) const noexcept
        {
            return !((collection == other.collection) && (index == other.index));
        }

        /// Equality operator
        /// @param other Other instance to compare with.
        /// @return True if instances are equal, false otherwise.
        Smp::Bool operator==(const CollectionIterator<T>& other) const noexcept
        {
            return (collection == other.collection) && (index == other.index);
        }

        /// Less than operator
        /// @param other Other instance to compare with.
        /// @return True if instances are iterators of the same collection and
        /// this index is less than other index, false otherwise.
        Smp::Bool operator<(const CollectionIterator<T>& other) const noexcept
        {
            return (collection == other.collection) && (index < other.index);
        }

        /// Less or equal than operator
        /// @param other Other instance to compare with.
        /// @return True if instances are iterators of the same collection and
        /// this index is less or equal than other index, false otherwise.
        Smp::Bool operator<=(const CollectionIterator<T>& other) const noexcept
        {
            return (collection == other.collection) && (index <= other.index);
        }

        /// More than operator
        /// @param other Other instance to compare with.
        /// @return True if instances are iterators of the same collection and
        /// this index is more than other index, false otherwise.
        Smp::Bool operator>(const CollectionIterator<T>& other) const noexcept
        {
            return (collection == other.collection) && (index > other.index);
        }

        /// More or equal than operator
        /// @param other Other instance to compare with.
        /// @return True if instances are iterators of the same collection and
        /// this index is more or equal than other index, false otherwise.
        Smp::Bool operator>=(const CollectionIterator<T>& other) const noexcept
        {
            return (collection == other.collection) && (index >= other.index);
        }

        /// Accessor to the index.
        /// @return The index to the element of the collection this iterator
        /// points to.
        size_t getIndex() const noexcept
        {
            return index;
        }

        /// Accessor to the collection.
        /// @return The collection this iterator points to.
        const Smp::ICollection<T>* getCollection() const noexcept
        {
            return collection;
        }

    private:
        /// Pointer to collection
        const Smp::ICollection<T>* collection;

        /// Pointer to index
        size_t index;
    };
}

namespace std
{
    /// Specialises C++ iterator_traits for the collection iterator
    template <typename T>
    struct iterator_traits<Smp::CollectionIterator<T>>
    {
        using Iter = Smp::CollectionIterator<T>;
        using iterator_category = typename Iter::iterator_category;
        using value_type = typename Iter::value_type;
        using difference_type = typename Iter::difference_type;
        using reference = typename Iter::reference;
        using pointer = typename Iter::pointer;
    };
}

#endif // SMP_COLLECTIONITERATOR_H_
