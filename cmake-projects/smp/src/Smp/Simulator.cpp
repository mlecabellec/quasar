#include "Smp/Simulator.h"
#include "Smp/InvalidSimulatorState.h"
#include <algorithm>

namespace Smp {

// Simple Container implementation for internal use
class SimulatorContainer : public virtual IContainer, public Object {
public:
  SimulatorContainer(String8 name, String8 description, IObject *parent)
      : Object(name, description, parent), components(name, description, this) {
  }

  const ComponentCollection *GetComponents() const override {
    return &components;
  }
  IComponent *GetComponent(String8 name) const override {
    return components.at(name);
  }
  void AddComponent(IComponent *component) override {
    components.Add(component);
  }
  void RemoveComponent(IComponent *component) {
    // Basic implementation - not in IContainer? Wait, IContainer has
    // DeleteComponent
  }
  void DeleteComponent(IComponent *component) override {
    // components.Remove(component); // Collection doesn't have Remove yet
  }
  Smp::Int64 GetCount() const override {
    return static_cast<Smp::Int64>(components.size());
  }
  Smp::Int64 GetUpper() const override { return -1; } // Unbounded
  Smp::Int64 GetLower() const override { return 0; }

  // IObject overrides
  String8 GetName() const noexcept override { return Object::GetName(); }
  String8 GetDescription() const noexcept override {
    return Object::GetDescription();
  }
  IObject *GetParent() const noexcept override { return Object::GetParent(); }

private:
  Collection<IComponent> components;
};

Simulator::Simulator(String8 name, String8 description,
                     Publication::ITypeRegistry *typeRegistry)
    : Object(name, description, nullptr),
      state(SimulatorStateKind::SSK_Building), typeRegistry(typeRegistry),
      containers("Containers", "Container Collection", this),
      factories("Factories", "Factory Collection", this) {

  models =
      new SimulatorContainer(SMP_SimulatorModels, "Models container", this);
  services =
      new SimulatorContainer(SMP_SimulatorServices, "Services container", this);

  containers.Add(models);
  containers.Add(services);
}

void Simulator::Initialise() {
  if (state != SimulatorStateKind::SSK_Standby)
    throw InvalidSimulatorState(state);

  SetState(SimulatorStateKind::SSK_Initialising);
  for (auto *ep : initEntryPoints) {
    if (ep)
      ep->Execute();
  }
  SetState(SimulatorStateKind::SSK_Standby);
}

void Simulator::Publish() {
  if (state != SimulatorStateKind::SSK_Building)
    throw InvalidSimulatorState(state);

  // In a full implementation, this would recursively call Publish() on all
  // components.
}

void Simulator::Configure() {
  if (state != SimulatorStateKind::SSK_Building)
    throw InvalidSimulatorState(state);
}

void Simulator::Connect() {
  if (state != SimulatorStateKind::SSK_Building)
    throw InvalidSimulatorState(state);

  SetState(SimulatorStateKind::SSK_Connecting);
  SetState(SimulatorStateKind::SSK_Standby);
}

void Simulator::Run() {
  if (state != SimulatorStateKind::SSK_Standby)
    throw InvalidSimulatorState(state);

  SetState(SimulatorStateKind::SSK_Executing);
}

void Simulator::Hold(Bool immediate) {
  if (state != SimulatorStateKind::SSK_Executing)
    throw InvalidSimulatorState(state);

  SetState(SimulatorStateKind::SSK_Standby);
}

void Simulator::Store(String8 filename) {
  if (state != SimulatorStateKind::SSK_Standby &&
      state != SimulatorStateKind::SSK_Executing)
    throw InvalidSimulatorState(state);

  SetState(SimulatorStateKind::SSK_Storing);
  // Store logic here
  SetState(state == SimulatorStateKind::SSK_Storing
               ? SimulatorStateKind::SSK_Standby
               : state); // Basic hack to return to previous state
}

void Simulator::Restore(String8 filename) {
  if (state != SimulatorStateKind::SSK_Standby)
    throw InvalidSimulatorState(state);

  SetState(SimulatorStateKind::SSK_Restoring);
  // Restore logic here
  SetState(SimulatorStateKind::SSK_Standby);
}

void Simulator::Reconnect(IComponent *root) {}

void Simulator::Exit() {
  SetState(SimulatorStateKind::SSK_Exiting);
  // Cleanup logic here
}

void Simulator::Abort() { SetState(SimulatorStateKind::SSK_Aborting); }

SimulatorStateKind Simulator::GetState() const { return state; }

void Simulator::SetState(SimulatorStateKind newState) { state = newState; }

void Simulator::AddInitEntryPoint(IEntryPoint *entryPoint) {
  initEntryPoints.push_back(entryPoint);
}

void Simulator::AddModel(IModel *model) {
  if (models)
    models->AddComponent(model);
}

void Simulator::AddService(IService *service) {
  if (services)
    services->AddComponent(service);
}

IService *Simulator::GetService(String8 name) const {
  if (services) {
    auto *comp = services->GetComponents()->at(name);
    return dynamic_cast<IService *>(comp);
  }
  return nullptr;
}

Services::ILogger *Simulator::GetLogger() const {
  return dynamic_cast<Services::ILogger *>(GetService("Logger"));
}

Services::ITimeKeeper *Simulator::GetTimeKeeper() const {
  return dynamic_cast<Services::ITimeKeeper *>(GetService("TimeKeeper"));
}

Services::IScheduler *Simulator::GetScheduler() const {
  return dynamic_cast<Services::IScheduler *>(GetService("Scheduler"));
}

Services::IEventManager *Simulator::GetEventManager() const {
  return dynamic_cast<Services::IEventManager *>(GetService("EventManager"));
}

Services::IResolver *Simulator::GetResolver() const {
  return dynamic_cast<Services::IResolver *>(GetService("Resolver"));
}

Services::ILinkRegistry *Simulator::GetLinkRegistry() const {
  return dynamic_cast<Services::ILinkRegistry *>(GetService("LinkRegistry"));
}

void Simulator::RegisterFactory(IFactory *componentFactory) {
  factories.Add(componentFactory);
}

IComponent *Simulator::CreateInstance(Uuid uuid, String8 name,
                                      String8 description, IComposite *parent) {
  auto *factory = GetFactory(uuid);
  if (factory) {
    return factory->CreateInstance(name, description, parent);
  }
  return nullptr;
}

IFactory *Simulator::GetFactory(Uuid uuid) const {
  for (size_t i = 0; i < factories.size(); ++i) {
    auto *f = factories.at(i);
    if (f && f->GetUuid() == uuid)
      return f;
  }
  return nullptr;
}

const ICollection<IFactory> *Simulator::GetFactories() const {
  return &factories;
}

Publication::ITypeRegistry *Simulator::GetTypeRegistry() const {
  return typeRegistry;
}

void Simulator::LoadLibrary(String8 libraryPath) {}

} // namespace Smp
