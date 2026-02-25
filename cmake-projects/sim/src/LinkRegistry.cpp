/**
 * @file LinkRegistry.cpp
 * @brief Implementation of sim::LinkRegistry service.
 *
 * This service manages links between simulation components, ensuring that
 * relationships are tracked and managed correctly.
 *
 * Contribution to FE-0030:
 * - [FE-0030.8] Thread safety: The implementation extensively uses
 *   `std::timed_mutex` to protect shared data structures (`_counts`,
 *   `_collections`). This ensures that operations like adding, removing, and
 *   querying links are thread-safe, fulfilling the requirement for thread
 *   safety with preferred timeout mechanisms.
 *
 * Missing parts related to FE-0030:
 * - Does not implement functionalities for `quasar::coretypes::Number`,
 *   `quasar::coretypes::String`, `Buffer`, or `BitBuffer` types or their
 *   associated operations (arithmetic, bitwise, comparison, reflection).
 *
 * Const Correctness ([FE-0030.9]):
 * - Methods like `GetState()`, `GetUuid()`, and `GetLinkSources()` are correctly
 *   marked `const`, contributing to the requirement.
 */
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
  /// Fulfills [FE-0070.6.2] (ILinkRegistry::AddLink).
  // Acquire lock with timeout to ensure thread safety [FE-0030.8]
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring LinkRegistry lock");
  }

  auto &targetCounts = _counts[target];
  if (targetCounts[source] == 0) {
    _collections[target].Add(source);
  }
  targetCounts[source]++;
}

Smp::UInt32 LinkRegistry::GetLinkCount(const Smp::IComponent *source,
                                       const Smp::IComponent *target) const {
  // Acquire lock with timeout to ensure thread safety [FE-0030.8]
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring LinkRegistry lock");
  }

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
  // Acquire lock with timeout to ensure thread safety [FE-0030.8]
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring LinkRegistry lock");
  }

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
  // Acquire lock with timeout to ensure thread safety [FE-0030.8]
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring LinkRegistry lock");
  }

  auto it = _collections.find(target);
  if (it != _collections.end()) {
    return &it->second;
  }

  // Return empty collection if no links found.
  // Note: `_collections` is not mutable, so operator[] cannot be used in a const method.
  // Returning a static empty collection is a common pattern for const getters.
  static core::SimpleCollection<Smp::IComponent> emptyCollection;
  return &emptyCollection;
}

Smp::Bool LinkRegistry::CanRemove(const Smp::IComponent *target) {
  // Acquire lock with timeout to ensure thread safety [FE-0030.8]
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring LinkRegistry lock");
  }

  auto itCollection = _collections.find(target);
  if (itCollection == _collections.end()) {
    return true; // No links means it can be removed.
  }

  for (auto *source : itCollection->second) {
    if (!dynamic_cast<Smp::ILinkingComponent *>(source)) {
      return false; // Cannot remove if any source is not a ILinkingComponent.
    }
  }
  return true;
}

void LinkRegistry::RemoveLinks(const Smp::IComponent *target) {
  // We need to copy sources because calling RemoveLinks on them might call
  // RemoveLink on us, modifying the collection.
  std::vector<Smp::ILinkingComponent *> sourcesToRemove;

  {
    // Acquire lock with timeout to ensure thread safety [FE-0030.8]
    std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
    if (!lock.owns_lock()) {
      throw std::runtime_error("Timeout acquiring LinkRegistry lock");
    }
    auto itCollection = _collections.find(target);
    if (itCollection != _collections.end()) {
      for (auto *source : itCollection->second) {
        if (auto *linking = dynamic_cast<Smp::ILinkingComponent *>(source)) {
          sourcesToRemove.push_back(linking);
        }
      }
    }
  }

  // Perform removal outside lock to avoid deadlocks and allow re-entrancy.
  for (auto *linking : sourcesToRemove) {
    linking->RemoveLinks(target);
  }
}

} // namespace sim
