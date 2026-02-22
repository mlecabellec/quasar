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
  const Uuid &GetUuid() const override;
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

  // IComponent methods
  ComponentStateKind GetState() const override;
  void Publish(IPublication *receiver) override;
  void Configure(Services::ILogger *logger, Services::ILinkRegistry *linkRegistry = nullptr) override;
  void Connect(ISimulator *simulator) override;
  void Disconnect() override;
  IField *GetField(String8 fullName) const override;
  const FieldCollection *GetFields() const override;
  AnySimple GetSimpleValue(String8 fullName) const override;
  void SetSimpleValue(String8 fullName, AnySimple value) override;
  void GetSimpleArrayValue(String8 fullName, UInt64 length, AnySimple *values, UInt64 startIndex = 0) const override;
  void SetSimpleArrayValue(String8 fullName, UInt64 length, AnySimpleArray values, UInt64 startIndex = 0) override;
  Bool AddChild(IObject *child, const IObject *collection) override;
  Bool RemoveChild(IObject *child, const IObject *collection) override;
  IObject *IsChildInCollection(String8 child, const IObject *collection) const override;

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

  SimulatorStateKind GetSimulatorState() const override;
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
  void
  LoadLibrary(String8 libraryPath,
              LibraryLoadingFlag flag = LibraryLoadingFlag::LLF_Auto) override;

private:
  SimulatorStateKind state;
  Publication::ITypeRegistry *typeRegistry;
  Collection<IContainer> containers;
  Collection<IFactory> factories;
  std::vector<IEntryPoint *> initEntryPoints;
  Uuid uuid;
  Collection<IField> fields;

  // Containers
  IContainer *models;
  IContainer *services;

  void SetState(SimulatorStateKind newState);
};

} // namespace Smp

#endif // SMP_SIMULATOR_H
