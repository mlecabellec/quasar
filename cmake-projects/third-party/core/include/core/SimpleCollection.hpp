#pragma once

#include <Smp/ICollection.h>
#include <Smp/PrimitiveTypes.h>
#include <algorithm>
#include <core/Object.hpp>
#include <cstring>
#include <string>
#include <vector>

namespace core {

template <typename T>
class SimpleCollection : public core::Object,
                         public virtual Smp::ICollection<T> {
public:
  SimpleCollection() : core::Object("Collection", "", nullptr) {}
  virtual ~SimpleCollection() noexcept = default;

  T *at(Smp::String8 name) const override {
    if (!name)
      return nullptr;
    for (auto *item : _items) {
      // Assuming T is IObject or has GetName().
      // ICollection is templated on T.
      // In SMP, collections usually contain IObject derived.
      // But T could be anything.
      // However, at(name) implies elements have names.
      // Standard SMP ICollection usually assumes T inherits IObject or we
      // dynamic_cast? "Elements in the collection can be queried by name...".
      // If T is IComponent, it has GetName().
      // Smp::IObject has GetName().
      // We assume T is convertible to IObject or has GetName.
      if (item && std::strcmp(item->GetName(), name) == 0) {
        return item;
      }
    }
    return nullptr;
  }

  T *at(size_t index) const override {
    if (index < _items.size()) {
      return _items[index];
    }
    return nullptr;
  }

  size_t size() const override { return _items.size(); }

  Smp::Bool empty() const { return _items.empty(); }

  typename Smp::ICollection<T>::const_iterator begin() const override {
    return typename Smp::ICollection<T>::const_iterator(*this, 0);
  }

  typename Smp::ICollection<T>::const_iterator end() const override {
    return typename Smp::ICollection<T>::const_iterator(*this, _items.size());
  }

  // Generic methods to add/remove
  void Add(T *item) {
    if (item) {
      _items.push_back(item);
    }
  }

  bool Remove(T *item) {
    auto it = std::find(_items.begin(), _items.end(), item);
    if (it != _items.end()) {
      _items.erase(it);
      return true;
    }
    return false;
  }

  void Clear() { _items.clear(); }

private:
  std::vector<T *> _items;
};

} // namespace core
