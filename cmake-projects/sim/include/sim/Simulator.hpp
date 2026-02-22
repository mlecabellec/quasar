#pragma once

#include "sim/TypeRegistry.hpp"
#include <Smp/IDynamicSimulator.h>
#include <core/Container.hpp>
#include <core/Object.hpp>
#include <core/SimpleCollection.hpp>
#include <map>
#include <string>
#include <vector>

namespace utils {
class Logger;
class TimeKeeper;
class EventManager;
} // namespace utils

namespace sched {
class Scheduler;
}

namespace sim {

class Resolver;
class LinkRegistry;

class Simulator : public core::Object, public virtual Smp::IDynamicSimulator {
public:
  Simulator();
  virtual ~Simulator() noexcept;

  // IObject methods (overriding core::Object where necessary or using it)
  Smp::String8 GetName() const override;
  Smp::String8 GetDescription() const override;
  Smp::IObject *GetParent() const override;
  Smp::IObject *GetChild(Smp::String8 name) const override;

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
                     const Smp::IObject *collection) override;
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::IObject *collection) override;
  Smp::IObject *
  IsChildInCollection(Smp::String8 child,
                      const Smp::IObject *collection) const override;

  // IComposite methods
  const Smp::ContainerCollection *GetContainers() const override;
  Smp::IContainer *GetContainer(Smp::String8 name) const override;

  // ISimulator Methods
  void Initialise() override;
  void Publish() override;
  void Run() override;
  void Run(Smp::Duration time) override;
  void Store(Smp::String8 filename) override;
  void Restore(Smp::String8 filename) override;
  void Exit() override;
  void Abort() override;
  Smp::SimulatorStateKind GetSimulatorState() const override;

  // IDynamicSimulator Methods
  void RegisterFactory(Smp::IFactory *componentFactory) override;
  Smp::IComponent *CreateInstance(const Smp::Uuid implUuid) override;
  const Smp::IFactory *GetFactory(const Smp::Uuid implUuid) const override;
  const Smp::FactoryCollection *
  GetFactories(const Smp::Uuid specUuid) const override;

  // ISimulator methods (remaining from original ISimulator)
  void AddInitEntryPoint(Smp::IEntryPoint *entryPoint) override;
  void AddModel(Smp::IModel *model) override;
  void AddService(Smp::IService *service) override;
  Smp::IService *GetService(Smp::String8 name) const override;
  Smp::Services::ILogger *GetLogger() const override;
  Smp::Services::ITimeKeeper *GetTimeKeeper() const override;
  Smp::Services::IScheduler *GetScheduler() const override;
  Smp::Services::IEventManager *GetEventManager() const override;
  Smp::Services::IResolver *GetResolver() const override;
  Smp::Services::ILinkRegistry *GetLinkRegistry() const override;
  Smp::IComponent *CreateInstance(Smp::Uuid uuid, Smp::String8 name,
                                  Smp::String8 description,
                                  Smp::IComposite *parent) override;
  const Smp::FactoryCollection *GetFactories() const override;
  Smp::Publication::ITypeRegistry *GetTypeRegistry() const override;
  void LoadLibrary(Smp::String8 libraryPath,
                   Smp::LibraryLoadingFlag flag =
                       Smp::LibraryLoadingFlag::LLF_Auto) override;

private:
  // Services
  utils::Logger *_logger;
  utils::TimeKeeper *_timeKeeper;
  utils::EventManager *_eventManager;
  sched::Scheduler *_scheduler;
  Resolver *_resolver;
  LinkRegistry *_linkRegistry;

  // State
  Smp::SimulatorStateKind _simState;
  Smp::ComponentStateKind _compState;

  // Collections
  core::SimpleCollection<Smp::IContainer> _containers;
  core::Container *_modelsContainer;
  core::Container *_servicesContainer;
  core::SimpleCollection<Smp::IFactory> _factories;
  TypeRegistry *_typeRegistry;
  std::vector<Smp::IEntryPoint *> _initEntryPoints;
  std::vector<void *> _loadedLibraries;

  // Recursive helpers
  void RecursivelyPublish(Smp::IComponent *component);
  void RecursivelyConfigure(Smp::IComponent *component);
  void RecursivelyConnect(Smp::IComponent *component);
  void RecursivelyDisconnect(Smp::IComponent *component);

  // Helper to resolve GetState ambiguity
  // We might need to implement `IComponent::GetState` and
  // `ISimulator::GetState` explicitly if possible? In C++, we cannot have two
  // virtual functions with same name and arguments but different return types.
  // One of them must be renamed in the interface definition if they clash.
  // Let me check IComponent.h content to see if it's different.
};

} // namespace sim
