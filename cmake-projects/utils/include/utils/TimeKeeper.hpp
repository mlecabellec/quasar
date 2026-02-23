#pragma once

#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ITimeKeeper.h>
#include <core/Object.hpp>
#include <mutex>

namespace utils {

class TimeKeeper : public core::Object,
                   public virtual Smp::Services::ITimeKeeper {
public:
  TimeKeeper();
  virtual ~TimeKeeper() noexcept = default;

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

  Smp::IObject *GetChild(Smp::String8 name) const override;

  // ITimeKeeper methods

  void SetEventManager(Smp::Services::IEventManager *eventManager);

  Smp::Duration GetSimulationTime() const override;
  Smp::DateTime GetEpochTime() const override;
  Smp::DateTime GetMissionStartTime() const override;
  Smp::Duration GetMissionTime() const override;
  Smp::DateTime GetZuluTime() const override;

  void SetSimulationTime(Smp::Duration simulationTime) override;
  void SetEpochTime(Smp::DateTime epochTime) override;
  void SetMissionStartTime(Smp::DateTime missionStart) override;
  void SetMissionTime(Smp::Duration missionTime) override;

private:
  Smp::Services::IEventManager *_eventManager = nullptr;

  Smp::Duration _simulationTime = 0;
  Smp::DateTime _epochOffset = 0;
  Smp::DateTime _missionStart = 0;

  mutable std::mutex _mutex;
};

} // namespace utils
