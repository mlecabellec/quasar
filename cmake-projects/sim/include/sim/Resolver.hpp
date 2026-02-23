#pragma once

#include <Smp/ISimulator.h>
#include <Smp/Services/IResolver.h>
#include <core/Object.hpp>

namespace sim {

/**
 * @brief Resolver service implementation.
 * @details This service resolves object paths within the simulator.
 * Fulfills [FE-0070.5.1] (IResolver Interface).
 */
class Resolver : public core::Object, public virtual Smp::Services::IResolver {
public:
  Resolver();
  virtual ~Resolver() noexcept = default;

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

  // IResolver methods

  void SetSimulator(Smp::ISimulator *simulator);

  /**
   * @brief Resolves an absolute path to an object.
   * @param absolutePath The absolute path.
   * @return Smp::IObject* The resolved object or nullptr.
   * @details Fulfills [FE-0070.5.2] (IResolver::ResolveAbsolute).
   */
  Smp::IObject *ResolveAbsolute(Smp::String8 absolutePath) override;
  /**
   * @brief Resolves a relative path to an object starting from a root.
   * @param relativePath The relative path.
   * @param startingObject The root object for resolution.
   * @return Smp::IObject* The resolved object or nullptr.
   * @details Fulfills [FE-0070.5.3] (IResolver::ResolveRelative).
   */
  Smp::IObject *ResolveRelative(Smp::String8 relativePath,
                                Smp::IObject *startingObject) override;

private:
  Smp::ISimulator *_simulator = nullptr;
};

} // namespace sim
