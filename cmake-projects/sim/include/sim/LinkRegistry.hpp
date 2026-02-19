#pragma once

#include <Smp/Services/ILinkRegistry.h>
#include <core/Object.hpp>
#include <core/SimpleCollection.hpp>
#include <map>
#include <mutex>

namespace sim {

class LinkRegistry : public core::Object,
                     public virtual Smp::Services::ILinkRegistry {
public:
  LinkRegistry();
  virtual ~LinkRegistry() noexcept = default;

  // IComponent methods
  Smp::ComponentStateKind GetState() const override;
  void Publish(Smp::IPublication *receiver) override;
  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry) override;
  void Connect(Smp::ISimulator *simulator) override;
  void Disconnect() override;
  const Smp::Uuid &GetUuid() const override;

  Smp::IField *GetField(Smp::String8 fullName) const override;
  const Smp::FieldCollection *GetFields() const override;
  Smp::AnySimple GetSimpleValue(Smp::String8 fullName) const override;
  void SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) override;
  void GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimple *values,
                           Smp::UInt64 startIndex = 0) const override;
  void SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimpleArray values,
                           Smp::UInt64 startIndex = 0) override;
  Smp::Bool AddChild(Smp::IObject *child,
                     const Smp::ICollectionBase *collection) override;
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::ICollectionBase *collection) override;
  Smp::IObject *
  IsChildInCollection(Smp::String8 child,
                      const Smp::ICollectionBase *collection) const override;

  // ILinkRegistry methods

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
