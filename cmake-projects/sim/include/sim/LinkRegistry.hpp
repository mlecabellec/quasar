#pragma once

#include <Smp/Services/ILinkRegistry.h>
#include <core/SimpleCollection.hpp>
#include <map>
#include <mutex>

namespace sim {

class LinkRegistry : public Smp::Services::ILinkRegistry {
public:
  LinkRegistry();
  virtual ~LinkRegistry() noexcept = default;

  void AddLink(Smp::IComponent *source, const Smp::IComponent *target) override;
  Smp::UInt32 GetLinkCount(const Smp::IComponent *source,
                           const Smp::IComponent *target) const override;
  Smp::Bool RemoveLink(Smp::IComponent *source,
                       const Smp::IComponent *target) override;
  const Smp::ComponentCollection *
  GetLinkSources(const Smp::IComponent *target) const override;
  Smp::Bool CanRemove(const Smp::IComponent *target) override;
  void RemoveLinks(const Smp::IComponent *target) override;

private:
  // Target -> (Source -> Count)
  std::map<const Smp::IComponent *, std::map<Smp::IComponent *, Smp::UInt32>>
      _counts;

  // Target -> Collection of unique sources
  // We use mutable because GetLinkSources is const but might need to create
  // entry? Actually, if no links, we might return empty collection. Better to
  // maintain it.
  std::map<const Smp::IComponent *, core::SimpleCollection<Smp::IComponent>>
      _collections;

  mutable std::mutex _mutex;
};

} // namespace sim
