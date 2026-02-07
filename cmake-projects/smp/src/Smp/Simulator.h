#ifndef SMP_SIMULATOR_H
#define SMP_SIMULATOR_H

#include "Smp/Collection.h"
#include "Smp/IComponent.h"
#include "Smp/IComposite.h"
#include "Smp/IContainer.h"
#include "Smp/IEntryPoint.h"
#include "Smp/IFactory.h"
#include "Smp/IModel.h"
#include "Smp/IService.h"
#include "Smp/ISimulator.h"
#include "Smp/Object.h"
#include "Smp/Publication/ITypeRegistry.h"
#include "Smp/SimulatorStateKind.h"
#include <vector>

namespace Smp {

class Simulator : public virtual ISimulator, public Object {
public:
  Simulator(String8 name, String8 description,
            Publication::ITypeRegistry *typeRegistry);
  virtual ~Simulator() noexcept = default;

  // IObject methods
  String8 GetName() const noexcept override { return Object::GetName(); }
  String8 GetDescription() const noexcept override {
    return Object::GetDescription();
  }
  IObject *GetParent() const noexcept override { return Object::GetParent(); }

  // IComposite methods
  const ContainerCollection *GetContainers() const override {
    return &containers;
  }
  IContainer *GetContainer(String8 name) const override {
    return containers.at(name);
  }

  // ISimulator methods
  void Initialise() override;
  void Publish() override;
  void Configure() override;
  void Connect() override;
  void Run() override;
  void Hold(Bool immediate) override;
  void Store(String8 filename) override;
  void Restore(String8 filename) override;
  void Reconnect(IComponent *root) override;
  void Exit() override;
  void Abort() override;

  SimulatorStateKind GetState() const override;
  void AddInitEntryPoint(IEntryPoint *entryPoint) override;
  void AddModel(IModel *model) override;
  void AddService(IService *service) override;

  IService *GetService(String8 name) const override;
  Services::ILogger *GetLogger() const override;
  Services::ITimeKeeper *GetTimeKeeper() const override;
  Services::IScheduler *GetScheduler() const override;
  Services::IEventManager *GetEventManager() const override;
  Services::IResolver *GetResolver() const override;
  Services::ILinkRegistry *GetLinkRegistry() const override;

  void RegisterFactory(IFactory *componentFactory) override;
  IComponent *CreateInstance(Uuid uuid, String8 name, String8 description,
                             IComposite *parent) override;
  IFactory *GetFactory(Uuid uuid) const override;
  const ICollection<IFactory> *GetFactories() const override;

  Publication::ITypeRegistry *GetTypeRegistry() const override;
  void LoadLibrary(String8 libraryPath) override;

private:
  SimulatorStateKind state;
  Publication::ITypeRegistry *typeRegistry;
  Collection<IContainer> containers;
  Collection<IFactory> factories;
  std::vector<IEntryPoint *> initEntryPoints;

  // Containers
  IContainer *models;
  IContainer *services;

  void SetState(SimulatorStateKind newState);
};

} // namespace Smp

#endif // SMP_SIMULATOR_H
