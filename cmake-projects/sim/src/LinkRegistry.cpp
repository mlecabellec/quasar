#include "sim/LinkRegistry.hpp"
#include <Smp/ILinkingComponent.h>

namespace sim {

LinkRegistry::LinkRegistry()
    : core::Object("LinkRegistry", "SMP Link Registry Service", nullptr) {}

Smp::ComponentStateKind LinkRegistry::GetState() const {
  return Smp::ComponentStateKind::CSK_Connected;
}

void LinkRegistry::Publish(Smp::IPublication *receiver) {}

void LinkRegistry::Configure(Smp::Services::ILogger *logger,
                             Smp::Services::ILinkRegistry *linkRegistry) {}

void LinkRegistry::Connect(Smp::ISimulator *simulator) {}

void LinkRegistry::Disconnect() {}

const Smp::Uuid &LinkRegistry::GetUuid() const {
  static Smp::Uuid uuid = {0, 0, 0, 0, 1}; // Generic Service UUID
  return uuid;
}

Smp::IField *LinkRegistry::GetField(Smp::String8 fullName) const {
  return nullptr;
}

const Smp::FieldCollection *LinkRegistry::GetFields() const { return nullptr; }

Smp::AnySimple LinkRegistry::GetSimpleValue(Smp::String8 fullName) const {
  return Smp::AnySimple();
}

void LinkRegistry::SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) {
}

void LinkRegistry::GetSimpleArrayValue(Smp::String8 fullName,
                                       Smp::UInt64 length,
                                       Smp::AnySimple *values,
                                       Smp::UInt64 startIndex) const {}

void LinkRegistry::SetSimpleArrayValue(Smp::String8 fullName,
                                       Smp::UInt64 length,
                                       Smp::AnySimpleArray values,
                                       Smp::UInt64 startIndex) {}

Smp::Bool LinkRegistry::AddChild(Smp::IObject *child,
                                 const Smp::ICollectionBase *collection) {
  return false;
}

Smp::Bool LinkRegistry::RemoveChild(Smp::IObject *child,
                                    const Smp::ICollectionBase *collection) {
  return false;
}

Smp::IObject *LinkRegistry::IsChildInCollection(
    Smp::String8 child, const Smp::ICollectionBase *collection) const {
  return nullptr;
}

void LinkRegistry::AddLink(Smp::IComponent *source,
                           const Smp::IComponent *target) {
  std::lock_guard<std::mutex> lock(_mutex);

  auto &targetCounts = _counts[target];
  if (targetCounts[source] == 0) {
    _collections[target].Add(source);
  }
  targetCounts[source]++;
}

Smp::UInt32 LinkRegistry::GetLinkCount(const Smp::IComponent *source,
                                       const Smp::IComponent *target) const {
  std::lock_guard<std::mutex> lock(_mutex);

  auto itTarget = _counts.find(target);
  if (itTarget != _counts.end()) {
    auto itSource =
        itTarget->second.find(const_cast<Smp::IComponent *>(source));
    if (itSource != itTarget->second.end()) {
      return itSource->second;
    }
  }
  return 0;
}

Smp::Bool LinkRegistry::RemoveLink(Smp::IComponent *source,
                                   const Smp::IComponent *target) {
  std::lock_guard<std::mutex> lock(_mutex);

  auto itTarget = _counts.find(target);
  if (itTarget != _counts.end()) {
    auto &targetCounts = itTarget->second;
    auto itSource = targetCounts.find(source);
    if (itSource != targetCounts.end() && itSource->second > 0) {
      itSource->second--;
      if (itSource->second == 0) {
        targetCounts.erase(itSource);
        _collections[target].Remove(source);
        if (targetCounts.empty()) {
          _counts.erase(itTarget);
          _collections.erase(target);
        }
      }
      return true;
    }
  }
  return false;
}

const Smp::ComponentCollection *
LinkRegistry::GetLinkSources(const Smp::IComponent *target) const {
  std::lock_guard<std::mutex> lock(_mutex);

  auto it = _collections.find(target);
  if (it != _collections.end()) {
    return &it->second;
  }

  // Return empty collection?
  // We can interpret this as: no links -> null? No, interface says "Collection
  // of source components". Return empty collection. We need a persistent empty
  // collection or just create an entry. Since map operator[] creates entry, we
  // can't use it in const method without mutable. But `_collections` is not
  // mutable in previous header version. I should make `_collections` mutable or
  // use a static empty collection. Or simpler: change header to `mutable`. (I
  // did remove `mutable` in Description but check code). In code:
  // `std::map<...> _collections;` (not mutable). So I can't modify it. I'll
  // return a pointer to a static empty collection if not found.
  static core::SimpleCollection<Smp::IComponent> emptyCollection;
  return &emptyCollection;
}

Smp::Bool LinkRegistry::CanRemove(const Smp::IComponent *target) {
  std::lock_guard<std::mutex> lock(_mutex);

  auto itCollection = _collections.find(target);
  if (itCollection == _collections.end()) {
    return true; // No links
  }

  for (auto *source : itCollection->second) {
    if (!dynamic_cast<Smp::ILinkingComponent *>(source)) {
      return false;
    }
  }
  return true;
}

void LinkRegistry::RemoveLinks(const Smp::IComponent *target) {
  // We need to copy sources because calling RemoveLinks on them might call
  // RemoveLink on us, modifying the collection.
  std::vector<Smp::ILinkingComponent *> sourcesToRemove;

  {
    std::lock_guard<std::mutex> lock(_mutex);
    auto itCollection = _collections.find(target);
    if (itCollection != _collections.end()) {
      for (auto *source : itCollection->second) {
        if (auto *linking = dynamic_cast<Smp::ILinkingComponent *>(source)) {
          sourcesToRemove.push_back(linking);
        }
      }
    }
  }

  // Perform removal outside lock
  for (auto *linking : sourcesToRemove) {
    linking->RemoveLinks(target);
  }
}

} // namespace sim
