#pragma once

#include "sim/TypeRegistry.hpp"
#include <Smp/ISimulator.h>
#include <core/Container.hpp>
#include <core/Object.hpp>
#include <core/SimpleCollection.hpp>
#include <map>
#include <memory>
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
 * @details This class provides the root of the simulation hierarchy and manages
 * the simulation lifecycle and services.
 * Fulfills [FE-0070.7.1] (ISimulator Object).
 */
class Simulator : public core::Object, public virtual Smp::ISimulator {
public:
  Simulator();
  virtual ~Simulator() noexcept override;

  // IObject methods (overriding core::Object where necessary or using it)
  /**
   * @brief Returns the name of the simulator.
   * @return Smp::String8 The simulator name.
   * @details Fulfills [FE-0070.7.35] (ISimulator::GetName).
   */
  Smp::String8 GetName() const override;
  Smp::String8 GetDescription() const override;
  /**
   * @brief Returns the parent of the simulator (always nullptr).
   * @return Smp::IObject* Always nullptr.
   * @details Fulfills [FE-0070.7.34] (ISimulator::GetParent).
   */
  Smp::IObject *GetParent() const override;
  Smp::IObject *GetChild(Smp::String8 name) const override;

  // IComponent methods removed in ECSS standard 1.1 since ISimulator is not an
  // IComponent

  // IComposite methods
  /**
   * @brief Returns the collection of containers (Models and Services).
   * @return const Smp::ContainerCollection* The containers.
   * @details Fulfills [FE-0070.7.32] (ISimulator::GetContainers).
   */
  const Smp::ContainerCollection *GetContainers() const override;
  /**
   * @brief Returns a container by name.
   * @param name The name of the container.
   * @return Smp::IContainer* The container or nullptr.
   * @details Fulfills [FE-0070.7.33] (ISimulator::GetContainer).
   */
  Smp::IContainer *GetContainer(Smp::String8 name) const override;

  // ISimulator Methods
  /**
   * @brief Initialises the simulator.
   * @details Fulfills [FE-0070.7.7] (ISimulator::Initialise).
   */
  void Initialise() override;
  /**
   * @brief Publishes the simulator and its components.
   * @details Fulfills [FE-0070.7.4] (ISimulator::Publish).
   */
  void Publish() override;
  /**
   * @brief Configures the simulator and its components.
   * @details Fulfills [FE-0070.7.5] (ISimulator::Configure).
   */
  void Configure() override;
  /**
   * @brief Connects the simulator and its components.
   * @details Fulfills [FE-0070.7.6] (ISimulator::Connect).
   */
  void Connect() override;
  /**
   * @brief Runs the simulation.
   * @details Fulfills [FE-0070.7.8] (ISimulator::Run).
   */
  void Run() override;
  void Run(Smp::Duration time);
  /**
   * @brief Holds the simulation.
   * @param immediate Whether to hold immediately.
   * @details Fulfills [FE-0070.7.9] (ISimulator::Hold).
   */
  void Hold(Smp::Bool immediate) override;
  /**
   * @brief Stores the simulation state.
   * @param filename The file to store the state in.
   * @details Fulfills [FE-0070.7.10] (ISimulator::Store).
   */
  void Store(Smp::String8 filename) override;
  /**
   * @brief Restores the simulation state.
   * @param filename The file to restore the state from.
   * @details Fulfills [FE-0070.7.11] (ISimulator::Restore).
   */
  void Restore(Smp::String8 filename) override;
  /**
   * @brief Reconnects the simulation components.
   * @param root The root component to reconnect.
   * @details Fulfills [FE-0070.7.12] (ISimulator::Reconnect).
   */
  void Reconnect(Smp::IComponent *root) override;
  /**
   * @brief Exits the simulation.
   * @details Fulfills [FE-0070.7.13] (ISimulator::Exit).
   */
  void Exit() override;
  /**
   * @brief Aborts the simulation.
   * @details Fulfills [FE-0070.7.14] (ISimulator::Abort).
   */
  void Abort() override;
  /**
   * @brief Returns the current simulator state.
   * @return Smp::SimulatorStateKind The state.
   * @details Fulfills [FE-0070.7.15] (ISimulator::GetState).
   */
  Smp::SimulatorStateKind GetSimulatorState() const override;

  // ISimulator factory methods
  /**
   * @brief Registers a component factory.
   * @param componentFactory The factory to register.
   * @details Fulfills [FE-0070.7.26] (ISimulator::RegisterFactory).
   */
  void RegisterFactory(Smp::IFactory *componentFactory) override;
  /**
   * @brief Returns a factory by its UUID.
   * @param uuid The UUID of the factory.
   * @return Smp::IFactory* The factory or nullptr.
   * @details Fulfills [FE-0070.7.28] (ISimulator::GetFactory).
   */
  Smp::IFactory *GetFactory(Smp::Uuid uuid) const override;

  // ISimulator methods (remaining from original ISimulator)
  /**
   * @brief Adds an initialisation entry point.
   * @param entryPoint The entry point to add.
   * @details Fulfills [FE-0070.7.16] (ISimulator::AddInitEntryPoint).
   */
  void AddInitEntryPoint(Smp::IEntryPoint *entryPoint) override;
  /**
   * @brief Adds a model to the simulator.
   * @param model The model to add.
   * @details Fulfills [FE-0070.7.17] (ISimulator::AddModel).
   */
  void AddModel(Smp::IModel *model) override;
  /**
   * @brief Adds a service to the simulator.
   * @param service The service to add.
   * @details Fulfills [FE-0070.7.18] (ISimulator::AddService).
   */
  void AddService(Smp::IService *service) override;
  /**
   * @brief Returns a service by name.
   * @param name The name of the service.
   * @return Smp::IService* The service or nullptr.
   * @details Fulfills [FE-0070.7.19] (ISimulator::GetService).
   */
  Smp::IService *GetService(Smp::String8 name) const override;

  /**
   * @brief Returns the logger service.
   * @return Smp::Services::ILogger* The logger.
   * @details Fulfills [FE-0070.7.20] (ISimulator::GetLogger).
   */
  Smp::Services::ILogger *GetLogger() const override;
  /**
   * @brief Returns the time keeper service.
   * @return Smp::Services::ITimeKeeper* The time keeper.
   * @details Fulfills [FE-0070.7.21] (ISimulator::GetTimeKeeper).
   */
  Smp::Services::ITimeKeeper *GetTimeKeeper() const override;
  /**
   * @brief Returns the scheduler service.
   * @return Smp::Services::IScheduler* The scheduler.
   * @details Fulfills [FE-0070.7.22] (ISimulator::GetScheduler).
   */
  Smp::Services::IScheduler *GetScheduler() const override;
  /**
   * @brief Returns the event manager service.
   * @return Smp::Services::IEventManager* The event manager.
   * @details Fulfills [FE-0070.7.23] (ISimulator::GetEventManager).
   */
  Smp::Services::IEventManager *GetEventManager() const override;
  /**
   * @brief Returns the resolver service.
   * @return Smp::Services::IResolver* The resolver.
   * @details Fulfills [FE-0070.7.24] (ISimulator::GetResolver).
   */
  Smp::Services::IResolver *GetResolver() const override;
  /**
   * @brief Returns the link registry service.
   * @return Smp::Services::ILinkRegistry* The link registry.
   * @details Fulfills [FE-0070.7.25] (ISimulator::GetLinkRegistry).
   */
  Smp::Services::ILinkRegistry *GetLinkRegistry() const override;

  /**
   * @brief Creates an instance of a component.
   * @param uuid The UUID of the component type.
   * @param name The name of the instance.
   * @param description The description of the instance.
   * @param parent The parent of the instance.
   * @return Smp::IComponent* The created component or nullptr.
   * @details Fulfills [FE-0070.7.27] (ISimulator::CreateInstance).
   */
  Smp::IComponent *CreateInstance(Smp::Uuid uuid, Smp::String8 name,
                                  Smp::String8 description,
                                  Smp::IComposite *parent) override;
  /**
   * @brief Returns all registered factories.
   * @return const Smp::FactoryCollection* The factories.
   * @details Fulfills [FE-0070.7.29] (ISimulator::GetFactories).
   */
  const Smp::FactoryCollection *GetFactories() const override;
  /**
   * @brief Returns the type registry.
   * @return Smp::Publication::ITypeRegistry* The type registry.
   * @details Fulfills [FE-0070.7.30] (ISimulator::GetTypeRegistry).
   */
  Smp::Publication::ITypeRegistry *GetTypeRegistry() const override;
  /**
   * @brief Loads a library.
   * @param libraryPath The path to the library.
   * @param flag Loading flags.
   * @details Fulfills [FE-0070.7.31] (ISimulator::LoadLibrary).
   */
  void LoadLibrary(Smp::String8 libraryPath,
                   Smp::LibraryLoadingFlag flag =
                       Smp::LibraryLoadingFlag::LLF_Auto) override;

private:
  // Services
  std::unique_ptr<utils::Logger> _logger;
  std::unique_ptr<utils::TimeKeeper> _timeKeeper;
  std::unique_ptr<utils::EventManager> _eventManager;
  std::unique_ptr<sched::Scheduler> _scheduler;
  std::unique_ptr<Resolver> _resolver;
  std::unique_ptr<LinkRegistry> _linkRegistry;

  // State
  Smp::SimulatorStateKind _simState;
  Smp::ComponentStateKind _compState;

  // Collections
  core::SimpleCollection<Smp::IContainer> _containers;
  std::unique_ptr<core::Container> _modelsContainer;
  std::unique_ptr<core::Container> _servicesContainer;
  core::SimpleCollection<Smp::IFactory> _factories;
  std::unique_ptr<TypeRegistry> _typeRegistry;
  std::vector<Smp::IEntryPoint *> _initEntryPoints;
  std::vector<void *> _loadedLibraries;

  // Recursive helpers
  void RecursivelyPublish(Smp::IComponent *component);
  void RecursivelyConfigure(Smp::IComponent *component);
  void RecursivelyConnect(Smp::IComponent *component);
  void RecursivelyDisconnect(Smp::IComponent *component);

  std::map<Smp::IComponent *, std::unique_ptr<Smp::IPublication>> _publications;

  // Helper to resolve GetState ambiguity
  // We might need to implement `IComponent::GetState` and
  // `ISimulator::GetState` explicitly if possible? In C++, we cannot have two
  // virtual functions with same name and arguments but different return types.
  // One of them must be renamed in the interface definition if they clash.
  // Let me check IComponent.h content to see if it's different.
};

} // namespace sim
