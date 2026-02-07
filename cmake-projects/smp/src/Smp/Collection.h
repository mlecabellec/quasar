#ifndef SMP_COLLECTION_IMPL_H
#define SMP_COLLECTION_IMPL_H

#include "Smp/ICollection.h"
#include "Smp/Object.h"
#include <map>
#include <string>
#include <vector>

namespace Smp {

template <typename T>
class Collection : public virtual ICollection<T>, public Object {
public:
  Collection(String8 name, String8 description, IObject *parent)
      : Object(name, description, parent) {}

  virtual ~Collection() noexcept = default;

  T *at(String8 name) const override {
    auto it = named_members.find(name ? name : "");
    if (it != named_members.end())
      return it->second;
    return nullptr;
  }

  T *at(size_t index) const override {
    if (index < members.size())
      return members[index];
    return nullptr;
  }

  size_t size() const override { return members.size(); }

  typename ICollection<T>::const_iterator begin() const override {
    return typename ICollection<T>::const_iterator(*this, 0);
  }

  typename ICollection<T>::const_iterator end() const override {
    return typename ICollection<T>::const_iterator(*this, size());
  }

  void Add(T *member) {
    if (!member)
      return;
    members.push_back(member);
    named_members[member->GetName()] = member;
  }

private:
  std::vector<T *> members;
  std::map<std::string, T *> named_members;
};

} // namespace Smp

#endif // SMP_COLLECTION_IMPL_H
