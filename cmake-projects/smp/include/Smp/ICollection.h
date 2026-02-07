#ifndef SMP_ICOLLECTION_H
#define SMP_ICOLLECTION_H

#include "Smp/CollectionIterator.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
template <typename T> class ICollection : public virtual IObject {
public:
  using const_iterator = CollectionIterator<T>;
  using iterator = CollectionIterator<T>;

  virtual T *at(String8 name) const = 0;
  virtual T *at(size_t index) const = 0;
  virtual size_t size() const = 0;

  virtual const_iterator begin() const = 0;
  virtual const_iterator end() const = 0;
};

// Inline implementation of CollectionIterator::operator* which depends on
// ICollection definition
template <typename T> T *CollectionIterator<T>::operator*() const {
  return m_collection->at(m_index);
}
} // namespace Smp

#endif // SMP_ICOLLECTION_H
