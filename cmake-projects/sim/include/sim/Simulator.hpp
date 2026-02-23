#pragma once

#include "sim/TypeRegistry.hpp"
#include <Smp/ISimulator.h>
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

/**
 * @brief Simulator Core implementation.
 * @details Contributes to [FE-0070.7.1] (ISimulator Object).
 */
class Simulator : public core::Object, public virtual Smp::ISimulator {
public:
  Simulator();
  virtual ~Simulator() noexcept;

  // IObject methods (overriding core::Object where necessary or using it)
  /// [FE-0070.7.35] Return a valid name.
  Smp::String8 GetName() const override;
  Smp::String8 GetDescription() const override;
  /// [FE-0070.7.34] Return nullptr as simulator is root.
  Smp::IObject *GetParent() const override;
  Smp::IObject *GetChild(Smp::String8 name) const override;

  // IComponent methods removed in ECSS standard 1.1 since ISimulator is not an
  // IComponent

  // IComposite methods
  /// [FE-0070.7.32] Return Models and Services containers.
  const Smp::ContainerCollection *GetContainers() const override;
  /// [FE-0070.7.33] Return a container by name.
  Smp::IContainer *GetContainer(Smp::String8 name) const override;

  // ISimulator Methods
  /// [FE-0070.7.7] Call initialisation entry points.
  void Initialise() override;
  /// [FE-0070.7.4] Call Publish on components.
  void Publish() override;
  /// [FE-0070.7.5] Call Configure on components.
  void Configure() override;
  /// [FE-0070.7.6] Call Connect on components.
  void Connect() override;
  /// [FE-0070.7.8] Change state to Executing and run event loop.
  void Run() override;
  void Run(Smp::Duration time);
  /// [FE-0070.7.9] Change state to Standby.
  void Hold(Smp::Bool immediate) override;
  /// [FE-0070.7.10] Store simulation state.
  void Store(Smp::String8 filename) override;
  /// [FE-0070.7.11] Restore simulation state.
  void Restore(Smp::String8 filename) override;
  /// [FE-0070.7.12] Reconnect components.
  void Reconnect(Smp::IComponent *root) override;
  /// [FE-0070.7.13] Terminate simulation.
  void Exit() override;
  /// [FE-0070.7.14] Abort simulation.
  void Abort() override;
  /// [FE-0070.7.15] Return simulator state.
  Smp::SimulatorStateKind GetSimulatorState() const override;

  // ISimulator factory methods
  /// [FE-0070.7.26] Register a factory.
  void RegisterFactory(Smp::IFactory *componentFactory) override;
  /// [FE-0070.7.28] Return a factory by UUID.
  Smp::IFactory *GetFactory(Smp::Uuid uuid) const override;

  // ISimulator methods (remaining from original ISimulator)
  /// [FE-0070.7.16] Add init entry point.
  void AddInitEntryPoint(Smp::IEntryPoint *entryPoint) override;
  /// [FE-0070.7.17] Add a model to "Models" container.
  void AddModel(Smp::IModel *model) override;
  /// [FE-0070.7.18] Add a service to "Services" container.
  void AddService(Smp::IService *service) override;
  /// [FE-0070.7.19] Return a service by name.
  Smp::IService *GetService(Smp::String8 name) const override;

  /// [FE-0070.7.20] Return the Logger service.
  Smp::Services::ILogger *GetLogger() const override;
  /// [FE-0070.7.21] Return the TimeKeeper service.
  Smp::Services::ITimeKeeper *GetTimeKeeper() const override;
  /// [FE-0070.7.22] Return the Scheduler service.
  Smp::Services::IScheduler *GetScheduler() const override;
  /// [FE-0070.7.23] Return the EventManager service.
  Smp::Services::IEventManager *GetEventManager() const override;
  /// [FE-0070.7.24] Return the Resolver service.
  Smp::Services::IResolver *GetResolver() const override;
  /// [FE-0070.7.25] Return the LinkRegistry service.
  Smp::Services::ILinkRegistry *GetLinkRegistry() const override;

  /// [FE-0070.7.27] Create a component instance using registered factories.
  Smp::IComponent *CreateInstance(Smp::Uuid uuid, Smp::String8 name,
                                  Smp::String8 description,
                                  Smp::IComposite *parent) override;
  /// [FE-0070.7.29] Return all registered factories.
  const Smp::FactoryCollection *GetFactories() const override;
  /// [FE-0070.7.30] Return Type Registry.
  Smp::Publication::ITypeRegistry *GetTypeRegistry() const override;
  /// [FE-0070.7.31] Load a library.
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
