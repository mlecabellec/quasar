#ifndef SMP_COLLECTIONITERATOR_H
#define SMP_COLLECTIONITERATOR_H

#include "Smp/PrimitiveTypes.h"

namespace Smp {
template <typename T> class ICollection;

template <typename T> class CollectionIterator final {
public:
  CollectionIterator(const ICollection<T> &collection, size_t index)
      : m_collection(&collection), m_index(index) {}

  CollectionIterator(const CollectionIterator &) = default;
  CollectionIterator &operator=(const CollectionIterator &) = default;
  CollectionIterator(CollectionIterator &&) = default;
  CollectionIterator &operator=(CollectionIterator &&) = default;

  CollectionIterator &operator++() {
    ++m_index;
    return *this;
  }

  bool operator!=(const CollectionIterator &other) const {
    return m_collection != other.m_collection || m_index != other.m_index;
  }

  T *operator*() const;

private:
  const ICollection<T> *m_collection;
  size_t m_index;
};
} // namespace Smp

#endif // SMP_COLLECTIONITERATOR_H
