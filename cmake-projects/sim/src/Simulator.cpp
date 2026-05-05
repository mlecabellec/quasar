#include "sim/Simulator.hpp"
#include "sched/Scheduler.hpp"
#include "sim/LibraryLoader.hpp"
#include "sim/LinkRegistry.hpp"
#include "sim/Publication.hpp"
#include "sim/Resolver.hpp"
#include "utils/EventManager.hpp"
#include "utils/Logger.hpp"
#include "utils/TimeKeeper.hpp"
#include <Smp/DuplicateName.h>
#include <Smp/IFactory.h>
#include <Smp/IModel.h>
#include <Smp/IService.h>
#include <Smp/InvalidSimulatorState.h>
#include <core/Container.hpp>
#include <core/SimpleCollection.hpp>
#include <core/StandardExceptions.hpp>
#include <iostream>
#include <limits>
#include <memory>

namespace sim {

/**
 * @brief Constructor.
 * @details Initializes services and containers.
 */
Simulator::Simulator()
    : core::Object("Simulator", "SMP Simulator Core", nullptr) {

  // Create services
  _logger = std::make_unique<utils::Logger>();
  _timeKeeper = std::make_unique<utils::TimeKeeper>(); // Used for time-related operations.
  _eventManager = std::make_unique<utils::EventManager>();
  _scheduler = std::make_unique<sched::Scheduler>(_timeKeeper.get(), _logger.get()); // Scheduler relies on TimeKeeper.
  _resolver = std::make_unique<Resolver>();
  _linkRegistry = std::make_unique<LinkRegistry>();
  _typeRegistry = std::make_unique<TypeRegistry>();

  // Configure services
  _timeKeeper->SetEventManager(_eventManager.get());
  _resolver->SetSimulator(this);

  // Initialize state
  // [FE-0050.6.1] Initializes the simulator in the SSK_Building state.
  _simState = Smp::SimulatorStateKind::SSK_Building;
  _compState = Smp::ComponentStateKind::CSK_Created;

  // Create containers
  /// Fulfills [FE-0070.7.2] (Simulator shall have "Models" and "Services"
  /// containers).
  _modelsContainer = std::make_unique<core::Container>(Smp::ISimulator::SMP_SimulatorModels,
                                         "Root Models", this);
  _servicesContainer = std::make_unique<core::Container>(
      Smp::ISimulator::SMP_SimulatorServices, "Root Services", this);

  _containers.Add(_modelsContainer.get());
  _containers.Add(_servicesContainer.get());
}

/**
 * @brief Destructor.
 * @details Cleans up loaded libraries.
 */
Simulator::~Simulator() noexcept {
  // Clear containers first to ensure objects are deleted while library code is still loaded
  _modelsContainer.reset();
  _servicesContainer.reset();

  // Finalise loaded libraries
  for (void *handle : _loadedLibraries) {
    if (handle) {
      typedef bool (*FinaliseFunctionPtr)();
      FinaliseFunctionPtr finalise = reinterpret_cast<FinaliseFunctionPtr>(
          LibraryLoader::GetInstance().GetSymbolAddress(handle, "Finalise"));
      if (finalise) {
        finalise();
      }
      LibraryLoader::GetInstance().UnloadLibrary(handle);
    }
  }
  _loadedLibraries.clear();
}

// IObject methods
Smp::String8 Simulator::GetName() const { return core::Object::GetName(); }

Smp::String8 Simulator::GetDescription() const {
  return core::Object::GetDescription();
}

Smp::IObject *Simulator::GetParent() const { return nullptr; }

Smp::IObject *Simulator::GetChild(Smp::String8 name) const {
  Smp::IContainer* container = GetContainer(name);
  if (container) {
    return container;
  }
  return nullptr;
}

// IComposite methods
const Smp::ContainerCollection *Simulator::GetContainers() const {
  return &_containers;
}

Smp::IContainer *Simulator::GetContainer(Smp::String8 name) const {
  return _containers.at(name);
}

// ISimulator methods
void Simulator::Initialise() {
  // [FE-0050.6.1] Transition to SSK_Initialising and then SSK_Standby.
  if (_simState != Smp::SimulatorStateKind::SSK_Standby)
    throw core::InvalidSimulatorState(_simState);

  _simState = Smp::SimulatorStateKind::SSK_Initialising;

  // Call entry points
  // ...

  _simState = Smp::SimulatorStateKind::SSK_Standby;
}

void Simulator::Publish() {
  // [FE-0050.6.1] Transition from SSK_Building to SSK_Building (self-transition with internal work).
  if (_simState != Smp::SimulatorStateKind::SSK_Building)
    throw core::InvalidSimulatorState(_simState);

  // Recursively publish
  for (Smp::IComponent *component : *_modelsContainer->GetComponents()) {
    RecursivelyPublish(component);
  }

  _simState = Smp::SimulatorStateKind::SSK_Building; // End of publish phase
}

void Simulator::Configure() {
  /// Fulfills [FE-0070.7.5] (ISimulator::Configure).
  // [FE-0050.6.1] Transition from SSK_Building to SSK_Building (internal work).
  if (_simState != Smp::SimulatorStateKind::SSK_Building)
    throw core::InvalidSimulatorState(_simState);

  // Recursively configure
  for (Smp::IComponent *component : *_modelsContainer->GetComponents()) {
    RecursivelyConfigure(component);
  }
}

void Simulator::Connect() {
  /// Fulfills [FE-0070.7.6] (ISimulator::Connect).
  // [FE-0050.6.1] Transition from SSK_Building to SSK_Connecting, then SSK_Initialising, and finally SSK_Standby.
  if (_simState != Smp::SimulatorStateKind::SSK_Building)
    throw core::InvalidSimulatorState(_simState);

  _simState = Smp::SimulatorStateKind::SSK_Connecting;

  // Recursively connect
  for (Smp::IComponent *component : *_modelsContainer->GetComponents()) {
    RecursivelyConnect(component);
  }

  _simState = Smp::SimulatorStateKind::SSK_Initialising;
  // Call init entry points

  _simState = Smp::SimulatorStateKind::SSK_Standby;
}

void Simulator::Run() {
  // [FE-0050.6.1] Transition from SSK_Standby to SSK_Executing.
  if (_simState != Smp::SimulatorStateKind::SSK_Standby)
    throw core::InvalidSimulatorState(_simState);

  _simState = Smp::SimulatorStateKind::SSK_Executing;

  // Safety limit for loop iterations
  Smp::UInt64 iterations = 0;
  constexpr Smp::UInt64 maxIterations = std::numeric_limits<Smp::UInt64>::max();

  while (_simState == Smp::SimulatorStateKind::SSK_Executing) {
    if (iterations >= maxIterations) {
       // Hard limit reached
       break;
    }
    iterations++;

    // [FE-0050.2] The Scheduler uses TimeKeeper to execute events based on simulation time.
    // This implicitly supports Simulation Time (FE-0050.2.1) and potentially others depending on TimeKeeper implementation.
    if (_scheduler->ExecuteNextEvent() < 0) {
      // No more events, exit run loop
      break;
    }
  }
  // When Run exits, it usually implies a hold or completion.
  // If completed normally, it might transition to Standby or another state.
  // If interrupted (e.g. by Hold), it transitions out of Executing.
  // For now, it exits the loop and the caller might handle state change.
}

void Simulator::Run(Smp::Duration time) {
    // [FE-0050.2] Supports time-bounded execution. The Smp::Duration type is used, implying time management capabilities.
    // Basic implementation deferring to Run() for now, 
    // as time-bounded execution logic is not fully specified in snippet.
    // This method itself would manage transitions related to time bounds.
    Run();
}

void Simulator::Hold(Smp::Bool immediate) {
  // [FE-0050.6.1] Transition from SSK_Executing to SSK_Standby.
  if (_simState != Smp::SimulatorStateKind::SSK_Executing)
    throw core::InvalidSimulatorState(_simState);
  _simState = Smp::SimulatorStateKind::SSK_Standby;
}

void Simulator::Store(Smp::String8 filename) {
  // [FE-0050.6.1] Transition from SSK_Standby to SSK_Storing, then back to SSK_Standby.
  if (_simState != Smp::SimulatorStateKind::SSK_Standby) {
    throw core::InvalidSimulatorState(_simState);
  }
  _simState = Smp::SimulatorStateKind::SSK_Storing;
  // TODO: Implement storage logic
  _simState = Smp::SimulatorStateKind::SSK_Standby;
}

void Simulator::Restore(Smp::String8 filename) {
  // [FE-0050.6.1] Transition from SSK_Standby to SSK_Restoring, then back to SSK_Standby.
  if (_simState != Smp::SimulatorStateKind::SSK_Standby) {
    throw core::InvalidSimulatorState(_simState);
  }
  _simState = Smp::SimulatorStateKind::SSK_Restoring;
  // TODO: Implement restore logic
  _simState = Smp::SimulatorStateKind::SSK_Standby;
}

void Simulator::Reconnect(Smp::IComponent *root) {
  // [FE-0050.6.1] Transition from SSK_Standby to SSK_Reconnecting (implied by the recursive call).
  if (_simState != Smp::SimulatorStateKind::SSK_Standby) {
    throw core::InvalidSimulatorState(_simState);
  }
  if (!root) {
    for (Smp::IComponent *component : *_modelsContainer->GetComponents()) {
      RecursivelyConnect(component); // Reconnect uses a similar recursive pattern.
    }
  } else {
    RecursivelyConnect(root);
  }
  // After reconnecting, it likely returns to SSK_Standby.
}

void Simulator::Exit() {
  // [FE-0050.6.1] Transition from SSK_Standby to SSK_Exiting.
  if (_simState != Smp::SimulatorStateKind::SSK_Standby) {
    throw core::InvalidSimulatorState(_simState);
  }
  _simState = Smp::SimulatorStateKind::SSK_Exiting;
}

void Simulator::Abort() { 
  // [FE-0050.6.1] Transition to SSK_Aborting.
  _simState = Smp::SimulatorStateKind::SSK_Aborting; 
}

Smp::SimulatorStateKind Simulator::GetSimulatorState() const {
  return _simState;
}

void Simulator::AddInitEntryPoint(Smp::IEntryPoint *entryPoint) {
  if (!entryPoint)
    return;
  // Initialization entry points are typically added during building or connecting phases.
  if (_simState == Smp::SimulatorStateKind::SSK_Building ||
      _simState == Smp::SimulatorStateKind::SSK_Connecting ||
      _simState == Smp::SimulatorStateKind::SSK_Standby) {
    _initEntryPoints.push_back(entryPoint);
  }
}

void Simulator::RecursivelyPublish(Smp::IComponent *component) {
  if (!component)
    return;
  // Ensures component is in a state to be published.
  if (component->GetState() == Smp::ComponentStateKind::CSK_Created) {
    std::unique_ptr<Publication> publication = std::make_unique<Publication>(GetTypeRegistry());
    Smp::IPublication* pubPtr = publication.get();
    _publications[component] = std::move(publication);
    component->Publish(pubPtr);
  }
  Smp::IComposite *composite = dynamic_cast<Smp::IComposite *>(component);
  if (composite) {
    for (Smp::IContainer *container : *composite->GetContainers()) {
      for (Smp::IComponent *child : *container->GetComponents()) {
        RecursivelyPublish(child);
      }
    }
  }
}

void Simulator::RecursivelyConfigure(Smp::IComponent *component) {
  if (!component)
    return;
  if (component->GetState() == Smp::ComponentStateKind::CSK_Created) {
    RecursivelyPublish(component);
  }
  if (component->GetState() == Smp::ComponentStateKind::CSK_Publishing) {
    component->Configure(GetLogger(), GetLinkRegistry());
  }
  Smp::IComposite *composite = dynamic_cast<Smp::IComposite *>(component);
  if (composite) {
    for (Smp::IContainer *container : *composite->GetContainers()) {
      for (Smp::IComponent *child : *container->GetComponents()) {
        RecursivelyConfigure(child);
      }
    }
  }
}

void Simulator::RecursivelyConnect(Smp::IComponent *component) {
  if (!component)
    return;
  if (component->GetState() == Smp::ComponentStateKind::CSK_Created ||
      component->GetState() == Smp::ComponentStateKind::CSK_Publishing) {
    RecursivelyConfigure(component);
  }
  if (component->GetState() == Smp::ComponentStateKind::CSK_Configured) {
    component->Connect(this);
  }
  Smp::IComposite *composite = dynamic_cast<Smp::IComposite *>(component);
  if (composite) {
    for (Smp::IContainer *container : *composite->GetContainers()) {
      for (Smp::IComponent *child : *container->GetComponents()) {
        RecursivelyConnect(child);
      }
    }
  }
}

void Simulator::RecursivelyDisconnect(Smp::IComponent *component) {
  if (!component)
    return;
  Smp::IComposite *composite = dynamic_cast<Smp::IComposite *>(component);
  if (composite) {
    for (Smp::IContainer *container : *composite->GetContainers()) {
      for (Smp::IComponent *child : *container->GetComponents()) {
        RecursivelyDisconnect(child);
      }
    }
  }
  if (component->GetState() == Smp::ComponentStateKind::CSK_Connected) {
    component->Disconnect();
  }
}

void Simulator::AddModel(Smp::IModel *model) {
  if (!model)
    return;
  _modelsContainer->AddComponent(model);
}

void Simulator::AddService(Smp::IService *service) {
  if (!service)
    return;
  _servicesContainer->AddComponent(service);
}

Smp::IService *Simulator::GetService(Smp::String8 name) const {
  return dynamic_cast<Smp::IService *>(_servicesContainer->GetComponent(name));
}

Smp::Services::ILogger *Simulator::GetLogger() const { return _logger.get(); }
Smp::Services::ITimeKeeper *Simulator::GetTimeKeeper() const {
  return _timeKeeper.get();
}
Smp::Services::IScheduler *Simulator::GetScheduler() const {
  return _scheduler.get();
}
Smp::Services::IEventManager *Simulator::GetEventManager() const {
  return _eventManager.get();
}
Smp::Services::IResolver *Simulator::GetResolver() const { return _resolver.get(); }
Smp::Services::ILinkRegistry *Simulator::GetLinkRegistry() const {
  return _linkRegistry.get();
}

void Simulator::RegisterFactory(Smp::IFactory *componentFactory) {
  // [FE-0050.4.1] Uses Smp::Uuid for identifying factories.
  if (!componentFactory)
    return;
  for (Smp::IFactory *f : this->_factories) {
    if (f->GetUuid() == componentFactory->GetUuid()) {
      throw std::runtime_error("Duplicate UUID for factory " +
                               std::string(componentFactory->GetName()));
    }
  }
  this->_factories.Add(componentFactory);
}

Smp::IComponent *Simulator::CreateInstance(Smp::Uuid uuid, Smp::String8 name,
                                           Smp::String8 description,
                                           Smp::IComposite *parent) {
  // [FE-0050.4.1] Uses Smp::Uuid to look up factories for creating instances.
  const Smp::IFactory *factory = GetFactory(uuid);
  if (factory) {
    return const_cast<Smp::IFactory *>(factory)->CreateInstance(
        name, description, parent);
  }
  return nullptr;
}

Smp::IFactory *Simulator::GetFactory(Smp::Uuid uuid) const {
  // [FE-0050.4.1] Uses Smp::Uuid for retrieving factories.
  for (Smp::IFactory *factory : this->_factories) {
    if (factory->GetUuid() == uuid) {
      return factory;
    }
  }
  return nullptr;
}

const Smp::FactoryCollection *Simulator::GetFactories() const {
  return &this->_factories;
}

Smp::Publication::ITypeRegistry *Simulator::GetTypeRegistry() const {
  return _typeRegistry.get();
}

/**
 * @brief Loads a library into the simulator.
 * @param libraryPath The path to the library.
 * @param flag Loading flags.
 * @details This method allows the simulator to load external shared libraries.
 *          This is crucial for integrating components that are developed
 *          separately, such as an EtherCAT master library (e.g., Resoem,
 *          as described in [FE-0040]). The simulator's responsibility here
 *          is to manage the loading process and ensure the library's
 *          initialization entry point is called. The actual EtherCAT
 *          protocol implementation resides within the loaded library, not
 *          within the simulator framework itself.
 *          Contributes indirectly to [FE-0040] by providing the loading mechanism.
 */
void Simulator::LoadLibrary(Smp::String8 libraryPath,
                            Smp::LibraryLoadingFlag flag) {
  try {
    void *handle = LibraryLoader::GetInstance().LoadLibrary(libraryPath);
    if (!handle) {
      throw std::runtime_error("Failed to load library " +
                               std::string(libraryPath));
    }

    typedef bool (*InitialiseFunctionPtr)(Smp::ISimulator *,
                                          Smp::Publication::ITypeRegistry *);

    InitialiseFunctionPtr initFunc = reinterpret_cast<InitialiseFunctionPtr>(
        LibraryLoader::GetInstance().GetSymbolAddress(handle, "Initialise"));

    if (initFunc) {
      if (!initFunc(this, _typeRegistry.get())) {
        throw std::runtime_error("Library Initialise failed for " +
                                 std::string(libraryPath));
      }
    } else {
      _logger->Log(this,
                   ("Library loaded but 'Initialise' entry point not found: " +
                    std::string(libraryPath))
                       .c_str(),
                   1);
    }

    _loadedLibraries.push_back(handle);
  } catch (const sim::LibraryException &ex) {
    // [FE-0050.5.1] Sim::LibraryException likely inherits from Smp::Exception.
    throw std::runtime_error(ex.what());
  }
}

} // namespace sim
