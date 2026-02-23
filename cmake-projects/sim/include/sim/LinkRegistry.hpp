#pragma once

#include <Smp/Services/ILinkRegistry.h>
#include <core/Object.hpp>
#include <core/SimpleCollection.hpp>
#include <map>
#include <mutex>

namespace sim {

/**
 * @brief Link Registry service implementation.
 * @details This service manages links between components.
 * Fulfills [FE-0070.6.1] (ILinkRegistry Interface).
 */
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

  /**
   * @brief Adds a link between two components.
   * @param source The source component.
   * @param target The target component.
   * @details Fulfills [FE-0070.6.2] (ILinkRegistry::AddLink).
   */
  void AddLink(Smp::IComponent *source, const Smp::IComponent *target) override;
  /**
   * @brief Returns the number of links between source and target.
   * @param source The source component.
   * @param target The target component.
   * @return Smp::UInt32 The number of links.
   * @details Fulfills [FE-0070.6.3] (ILinkRegistry::GetLinkCount).
   */
  Smp::UInt32 GetLinkCount(const Smp::IComponent *source,
                           const Smp::IComponent *target) const override;
  /**
   * @brief Removes a link between two components.
   * @param source The source component.
   * @param target The target component.
   * @return Smp::Bool True if the link was removed, false otherwise.
   * @details Fulfills [FE-0070.6.4] (ILinkRegistry::RemoveLink).
   */
  Smp::Bool RemoveLink(Smp::IComponent *source,
                       const Smp::IComponent *target) override;
  /**
   * @brief Returns the collection of source components linked to a target.
   * @param target The target component.
   * @return const Smp::ComponentCollection* The source components.
   * @details Fulfills [FE-0070.6.5] (ILinkRegistry::GetLinkSources).
   */
  const Smp::ComponentCollection *
  GetLinkSources(const Smp::IComponent *target) const override;
  /**
   * @brief Checks if a component can be removed.
   * @param component The component to check.
   * @return Smp::Bool True if it can be removed, false otherwise.
   * @details Fulfills [FE-0070.6.6] (ILinkRegistry::CanRemove).
   */
  Smp::Bool CanRemove(const Smp::IComponent *component) override;
  /**
   * @brief Removes all links associated with a component.
   * @param component The component.
   * @details Fulfills [FE-0070.6.7] (ILinkRegistry::RemoveLinks).
   */
  void RemoveLinks(const Smp::IComponent *component) override;

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

  mutable std::timed_mutex _mutex;
};

} // namespace sim
