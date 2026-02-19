#pragma once

#include <Smp/ISimulator.h>
#include <core/Object.hpp>
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

class Simulator : public core::Object, public Smp::ISimulator {
public:
  Simulator();
  virtual ~Simulator() noexcept;

  // IObject methods (overriding core::Object where necessary or using it)
  Smp::String8 GetName() const override;
  Smp::String8 GetDescription() const override;
  Smp::IObject *GetParent() const override;

  // IComponent methods
  Smp::Publication::IPublication *GetPublication() const override;
  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry) override;
  void Connect(Smp::ISimulator *simulator) override;
  void Disconnect() override;
  Smp::IComponent::ComponentStateKind GetState() const override;
  Smp::String8 GetUuid() const override;

  // IComposite methods
  const Smp::ContainerCollection *GetContainers() const override;
  Smp::IContainer *GetContainer(Smp::String8 name) const override;

  // ISimulator methods
  void Initialise() override;
  void Publish() override;
  void Configure() override;
  void Connect() override;
  void Run() override;
  void Hold(Smp::Bool immediate) override;
  void Store(Smp::String8 filename) override;
  void Restore(Smp::String8 filename) override;
  void Reconnect(Smp::IComponent *root) override;
  void Exit() override;
  void Abort() override;
  virtual Smp::SimulatorStateKind GetSimulatorState() const override;

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
  void RegisterFactory(Smp::IFactory *componentFactory) override;
  Smp::IComponent *CreateInstance(Smp::Uuid uuid, Smp::String8 name,
                                  Smp::String8 description,
                                  Smp::IComposite *parent) override;
  Smp::IFactory *GetFactory(Smp::Uuid uuid) const override;
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
  Smp::IComponent::ComponentStateKind _compState;

  // Collections
  core::SimpleCollection<Smp::IContainer> _containers;
  core::Container *_modelsContainer;
  core::Container *_servicesContainer;
  core::SimpleCollection<Smp::IFactory> _factories;

  // Helper to resolve GetState ambiguity
  // We might need to implement `IComponent::GetState` and
  // `ISimulator::GetState` explicitly if possible? In C++, we cannot have two
  // virtual functions with same name and arguments but different return types.
  // One of them must be renamed in the interface definition if they clash.
  // Let me check IComponent.h content to see if it's different.
};

} // namespace sim
