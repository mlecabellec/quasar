#include "sim/Simulator.hpp"
#include "sched/Scheduler.hpp"
#include "sim/LibraryLoader.hpp"
#include "sim/LinkRegistry.hpp"
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

namespace sim {

Simulator::Simulator()
    : core::Object("Simulator", "SMP Simulator Core", nullptr) {

  // Create services
  _logger = new utils::Logger();
  _timeKeeper = new utils::TimeKeeper();
  _eventManager = new utils::EventManager();
  _scheduler = new sched::Scheduler(_timeKeeper, _logger);
  _resolver = new Resolver();
  _linkRegistry = new LinkRegistry();
  _typeRegistry = new TypeRegistry();

  // Configure services
  _timeKeeper->SetEventManager(_eventManager);
  _resolver->SetSimulator(this);

  // Initialize state
  _simState = Smp::SimulatorStateKind::SSK_Building;
  _compState = Smp::ComponentStateKind::CSK_Created;

  // Create containers
  _modelsContainer = new core::Container(Smp::ISimulator::SMP_SimulatorModels,
                                         "Root Models", this);
  _servicesContainer = new core::Container(
      Smp::ISimulator::SMP_SimulatorServices, "Root Services", this);

  _containers.Add(_modelsContainer);
  _containers.Add(_servicesContainer);
}

Simulator::~Simulator() noexcept {
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

  delete _typeRegistry;
  delete _resolver;
  delete _scheduler;
  delete _eventManager;
  delete _timeKeeper;
  delete _logger;
  delete _linkRegistry;
}

// IObject methods
Smp::String8 Simulator::GetName() const { return core::Object::GetName(); }

Smp::String8 Simulator::GetDescription() const {
  return core::Object::GetDescription();
}

Smp::IObject *Simulator::GetParent() const { return nullptr; }

// IComponent methods
// Internal methods logic

Smp::ComponentStateKind Simulator::GetState() const { return _compState; }

const Smp::Uuid &Simulator::GetUuid() const {
  static Smp::Uuid simulatorUuid = {0, 0, 0, 0, 0}; // Generic UUID
  return simulatorUuid;
}

Smp::IField *Simulator::GetField(Smp::String8 fullName) const {
  return nullptr;
}

const Smp::FieldCollection *Simulator::GetFields() const { return nullptr; }

Smp::AnySimple Simulator::GetSimpleValue(Smp::String8 fullName) const {
  return Smp::AnySimple();
}

void Simulator::SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) {}

void Simulator::GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                    Smp::AnySimple *values,
                                    Smp::UInt64 startIndex) const {}

void Simulator::SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                    Smp::AnySimpleArray values,
                                    Smp::UInt64 startIndex) {}

Smp::Bool Simulator::AddChild(Smp::IObject *child,
                              const Smp::IObject *collection) {
  return false;
}

Smp::Bool Simulator::RemoveChild(Smp::IObject *child,
                                 const Smp::IObject *collection) {
  return false;
}

Smp::IObject *
Simulator::IsChildInCollection(Smp::String8 child,
                               const Smp::IObject *collection) const {
  return nullptr;
}

Smp::IObject *Simulator::GetChild(Smp::String8 name) const {
  if (auto *c = _containers.at(name))
    return c;
  return nullptr;
}

void Simulator::Publish(Smp::IPublication *receiver) {}
void Simulator::Configure(Smp::Services::ILogger *logger,
                          Smp::Services::ILinkRegistry *linkRegistry) {}
void Simulator::Connect(Smp::ISimulator *simulator) {}
void Simulator::Disconnect() {}

// IComposite methods
const Smp::ContainerCollection *Simulator::GetContainers() const {
  return &_containers;
}

Smp::IContainer *Simulator::GetContainer(Smp::String8 name) const {
  return _containers.at(name);
}

// ISimulator methods
void Simulator::Initialise() {
  if (_simState != Smp::SimulatorStateKind::SSK_Standby)
    throw core::InvalidSimulatorState(_simState);

  _simState = Smp::SimulatorStateKind::SSK_Initialising;

  // Call entry points
  // ...

  _simState = Smp::SimulatorStateKind::SSK_Standby;
}

void Simulator::Publish() {
  if (_simState != Smp::SimulatorStateKind::SSK_Building)
    throw core::InvalidSimulatorState(_simState);

  // Recursively publish
  // ...

  _simState = Smp::SimulatorStateKind::SSK_Building; // Stays in building?
  // "The Publish() operation will traverse recursively... This method must only
  // be called when in Building state."
}

void Simulator::Configure() {
  if (_simState != Smp::SimulatorStateKind::SSK_Building)
    throw core::InvalidSimulatorState(_simState);

  // Recursively configure
  // ...
}

void Simulator::Connect() {
  if (_simState != Smp::SimulatorStateKind::SSK_Building)
    throw core::InvalidSimulatorState(_simState);

  _simState = Smp::SimulatorStateKind::SSK_Connecting;

  // Recursively connect
  // ...

  _simState = Smp::SimulatorStateKind::SSK_Initialising;
  // Call init entry points

  _simState = Smp::SimulatorStateKind::SSK_Standby;
}

void Simulator::Run() {
  if (_simState != Smp::SimulatorStateKind::SSK_Standby)
    throw core::InvalidSimulatorState(_simState);

  _simState = Smp::SimulatorStateKind::SSK_Executing;

  while (_simState == Smp::SimulatorStateKind::SSK_Executing) {
    if (_scheduler->ExecuteNextEvent() < 0) {
      // No more events, exit run loop?
      // Or wait?
      // "If no event is scheduled... the scheduler waits..."? No,
      // ExecuteNextEvent returns. If run is called, it usually runs until Hold
      // is called. If no events, we should advance time or break? For now break
      // to avoid infinite loop.
      break;
    }
  }
}

void Simulator::Hold(Smp::Bool immediate) {
  if (_simState != Smp::SimulatorStateKind::SSK_Executing)
    throw core::InvalidSimulatorState(_simState);
  _simState = Smp::SimulatorStateKind::SSK_Standby;
}

void Simulator::Store(Smp::String8 filename) {
  if (_simState != Smp::SimulatorStateKind::SSK_Standby) {
    throw core::InvalidSimulatorState(_simState);
  }
  _simState = Smp::SimulatorStateKind::SSK_Storing;
  // TODO: Implement storage logic
  _simState = Smp::SimulatorStateKind::SSK_Standby;
}

void Simulator::Restore(Smp::String8 filename) {
  if (_simState != Smp::SimulatorStateKind::SSK_Standby) {
    throw core::InvalidSimulatorState(_simState);
  }
  _simState = Smp::SimulatorStateKind::SSK_Restoring;
  // TODO: Implement restore logic
  _simState = Smp::SimulatorStateKind::SSK_Standby;
}

void Simulator::Reconnect(Smp::IComponent *root) {
  if (_simState != Smp::SimulatorStateKind::SSK_Standby) {
    throw core::InvalidSimulatorState(_simState);
  }
  if (!root) {
    for (auto *component : *_modelsContainer->GetComponents()) {
      RecursivelyConnect(component);
    }
  } else {
    RecursivelyConnect(root);
  }
}

void Simulator::Exit() {
  if (_simState != Smp::SimulatorStateKind::SSK_Standby) {
    throw core::InvalidSimulatorState(_simState);
  }
  _simState = Smp::SimulatorStateKind::SSK_Exiting;
}

void Simulator::Abort() { _simState = Smp::SimulatorStateKind::SSK_Aborting; }

Smp::SimulatorStateKind Simulator::GetState() const { return _simState; }

void Simulator::AddInitEntryPoint(Smp::IEntryPoint *entryPoint) {
  if (!entryPoint)
    return;
  if (_simState == Smp::SimulatorStateKind::SSK_Building ||
      _simState == Smp::SimulatorStateKind::SSK_Connecting ||
      _simState == Smp::SimulatorStateKind::SSK_Standby) {
    _initEntryPoints.push_back(entryPoint);
  }
}

void Simulator::RecursivelyPublish(Smp::IComponent *component) {
  if (!component)
    return;
  if (component->GetState() == Smp::ComponentStateKind::CSK_Created) {
    // component->Publish(nullptr); // Needs real Publication
  }
  if (auto *composite = dynamic_cast<Smp::IComposite *>(component)) {
    for (auto *container : *composite->GetContainers()) {
      for (auto *child : *container->GetComponents()) {
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
  if (auto *composite = dynamic_cast<Smp::IComposite *>(component)) {
    for (auto *container : *composite->GetContainers()) {
      for (auto *child : *container->GetComponents()) {
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
  if (auto *composite = dynamic_cast<Smp::IComposite *>(component)) {
    for (auto *container : *composite->GetContainers()) {
      for (auto *child : *container->GetComponents()) {
        RecursivelyConnect(child);
      }
    }
  }
}

void Simulator::RecursivelyDisconnect(Smp::IComponent *component) {
  if (!component)
    return;
  // Disconnect children first?
  if (auto *composite = dynamic_cast<Smp::IComposite *>(component)) {
    for (auto *container : *composite->GetContainers()) {
      for (auto *child : *container->GetComponents()) {
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

Smp::Services::ILogger *Simulator::GetLogger() const { return _logger; }
Smp::Services::ITimeKeeper *Simulator::GetTimeKeeper() const {
  return _timeKeeper;
}
Smp::Services::IScheduler *Simulator::GetScheduler() const {
  return _scheduler;
}
Smp::Services::IEventManager *Simulator::GetEventManager() const {
  return _eventManager;
}
Smp::Services::IResolver *Simulator::GetResolver() const { return _resolver; }
Smp::Services::ILinkRegistry *Simulator::GetLinkRegistry() const {
  return _linkRegistry;
}

void Simulator::RegisterFactory(Smp::IFactory *componentFactory) {
  if (!componentFactory)
    return;
  // Check for duplicate UUID
  for (auto *f : this->_factories) {
    if (f->GetUuid() == componentFactory->GetUuid()) {
      throw core::DuplicateUuid(f->GetName(), componentFactory->GetName());
    }
  }
  this->_factories.Add(componentFactory);
}

Smp::IComponent *Simulator::CreateInstance(Smp::Uuid uuid, Smp::String8 name,
                                           Smp::String8 description,
                                           Smp::IComposite *parent) {
  const Smp::IFactory *factory = GetFactory(uuid);
  if (factory) {
    return const_cast<Smp::IFactory *>(factory)->CreateInstance(
        name, description, parent);
  }
  return nullptr;
}
Smp::IComponent *Simulator::CreateInstance(const Smp::Uuid implUuid) {
  for (auto *factory : this->_factories) {
    if (factory->GetUuid() == implUuid) {
      return factory->CreateInstance("", "", nullptr);
    }
  }
  return nullptr;
}

const Smp::IFactory *Simulator::GetFactory(const Smp::Uuid implUuid) const {
  for (auto *factory : this->_factories) {
    if (factory->GetUuid() == implUuid) {
      return factory;
    }
  }
  return nullptr;
}

const Smp::FactoryCollection *Simulator::GetFactories() const {
  return &this->_factories;
}

const Smp::FactoryCollection *
Simulator::GetFactories(const Smp::Uuid specUuid) const {
  // Return just generic factories, or filter if we had spec UUIDs
  return &this->_factories;
}

Smp::Publication::ITypeRegistry *Simulator::GetTypeRegistry() const {
  return _typeRegistry;
}

void Simulator::LoadLibrary(Smp::String8 libraryPath,
                            Smp::LibraryLoadingFlag flag) {
  try {
    void *handle = LibraryLoader::GetInstance().LoadLibrary(libraryPath);
    if (!handle) {
      throw core::Exception(
          "SimControl",
          ("Failed to load library " + std::string(libraryPath)).c_str());
    }

    // Attempt to locate and call Initialise
    typedef bool (*InitialiseFunctionPtr)(Smp::IDynamicSimulator *,
                                          Smp::Publication::ITypeRegistry *);

    InitialiseFunctionPtr initFunc = reinterpret_cast<InitialiseFunctionPtr>(
        LibraryLoader::GetInstance().GetSymbolAddress(handle, "Initialise"));

    if (initFunc) {
      if (!initFunc(this, _typeRegistry)) {
        throw core::Exception("SimControl", ("Library Initialise failed for " +
                                             std::string(libraryPath))
                                                .c_str());
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
    throw core::Exception("SimControl", ex.what());
  }
}

} // namespace sim
