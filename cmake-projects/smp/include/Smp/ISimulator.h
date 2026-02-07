#ifndef SMP_ISIMULATOR_H
#define SMP_ISIMULATOR_H

#include "Smp/DuplicateName.h"
#include "Smp/DuplicateUuid.h"
#include "Smp/IComponent.h"
#include "Smp/IComposite.h"
#include "Smp/IFactory.h"
#include "Smp/InvalidLibrary.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/InvalidSimulatorState.h"
#include "Smp/LibraryNotFound.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/ITypeRegistry.h"
#include "Smp/Services/IEventManager.h"
#include "Smp/Services/ILinkRegistry.h"
#include "Smp/Services/ILogger.h"
#include "Smp/Services/IResolver.h"
#include "Smp/Services/IScheduler.h"
#include "Smp/Services/ITimeKeeper.h"
#include "Smp/SimulatorStateKind.h"

namespace Smp {
class IEntryPoint;
class IModel;
class IService;
class IFactory;

/// This interface gives access to the simulation environment state and
/// state transitions. Further, it provides convenience methods to add
/// models, and to add and retrieve simulation services.
/// This is a mandatory interface that every SMP compliant simulation
/// environment has to implement.
class ISimulator : public virtual IComposite {
public:
  /// Name of the model container.
  static constexpr Char8 SMP_SimulatorModels[] = "Models";
  /// Name of the service container.
  static constexpr Char8 SMP_SimulatorServices[] = "Services";

  /// Virtual destructor to release memory.
  virtual ~ISimulator() noexcept = default;

  /// This method asks the simulation environment to call all
  /// initialisation entry points again.
  /// This method can only be called when in Standby state, and enters
  /// Initialising state. After completion, the simulator automatically
  /// returns to Standby state.
  /// The entry points will be executed in the order they have been added
  /// to the simulator using the AddInitEntryPoint() method.
  virtual void Initialise() = 0;

  /// This method asks the simulation environment to call the Publish()
  /// method of all service and model instances in the component
  /// hierarchy which are still in Created state.
  ///
  /// This method must only be called when in Building state.
  /// @remarks This method is typically called by an external component
  ///          after creating new model instances, typically from
  ///          information in an SMDL Assembly.
  ///          The Publish() operation will traverse recursively through
  ///          both the "Models" container and then the "Services"
  ///          container of the simulator. It will call the Publish()
  ///          operation of each component in CSK_Created state, and then
  ///          immediately on all its child components.
  virtual void Publish() = 0;

  /// This method asks the simulation environment to call the Configure()
  /// method of all service and model instances which are still in
  /// Publishing state.
  ///
  /// This method must only be called when in Building state.
  /// @remarks This method is typically called by an external component
  ///          after setting field values of new model instances,
  ///          typically using the information in an SMDL Assembly or
  ///          SMDL Configuration.
  ///          The Configure() operation will traverse recursively
  ///          through both the "Models" container and then the
  ///          "Services" container of the simulator. For each component,
  ///          it will first call the Publish() operation (if the
  ///          component is still in CSK_Created state), then the
  ///          Configure() operation (if the component is in
  ///          CSK_Publishing state), and then the simulator will
  ///          immediately perform the same operation(s) on all child
  ///          components of the component.
  virtual void Configure() = 0;

  /// This method informs the simulation environment that the hierarchy
  /// of model instances has been configured, and can now be connected to
  /// the simulator. Thus, the simulation environment calls the Connect()
  /// method of all service and model instances.
  /// In this method, the simulation environment first enters Connecting
  /// state and calls the Connect() method of every model in the model
  /// hierarchy, then enters Initialising state and calls the
  /// initialisation entry points, and finally enters Standby state.
  /// This method must only be called when in Building state.
  /// @remarks This method is typically called by an external component
  ///          after configuring all model instances.
  ///          The Connect() operation will traverse recursively through
  ///          both the "Models" container and then the "Services"
  ///          container of the simulator. For each component, it will
  ///          first call the Publish() and Configure() operations (if
  ///          the component is still in CSK_Created state), then the
  ///          Connect() operation (if the component is in CSK_Configured
  ///          state), and then the simulator will immediately perform
  ///          the same operation(s) on all child components of the
  ///          component.
  virtual void Connect() = 0;

  /// This method changes from Standby to Executing state.
  /// This method must only be called when in Standby state, and enters
  /// Executing state.
  virtual void Run() = 0;

  /// This method changes from Executing to Standby state.
  /// This method must only be called when in Executing state, and enters
  /// Standby state.
  /// @param   immediate True if the simulator shall stop immediately
  ///          after completion of the current scheduler event, false if
  ///          the simulator shall still process all events of the
  ///          current simulation time before entering Standby state.
  virtual void Hold(Bool immediate) = 0;

  /// This method is used to store a state vector to file.
  /// This method must only be called when in Standby state, and enters
  /// Storing state. On completion, it automatically returns to Standby
  /// state.
  /// @param   filename Name including the full path to use for
  ///          simulation state vector file.
  virtual void Store(String8 filename) = 0;

  /// This method is used to restore a state vector from file.
  /// This method must only be called when in Standby state, and enters
  /// Restoring state. On completion, it automatically returns to Standby
  /// state.
  /// @param   filename Name including the full path of simulation state
  ///          vector file.
  virtual void Restore(String8 filename) = 0;

  /// This method asks the simulation environment to reconnect the
  /// component hierarchy starting at the given root component.
  /// This method must only be called when in Standby state.
  /// @remarks This method is typically called after creating additional
  ///          model instances and adding them to the existing model
  ///          hierarchy.
  ///          The simulation environment has to ensure that all models
  ///          under the given root (but not the root itself) are
  ///          published, configured and connected, so that all child
  ///          models are finally in Connected state.
  /// @param   root Root component to start reconnecting from. This can
  ///          be the parent component of a newly added model, or e.g.
  ///          the simulator itself.
  virtual void Reconnect(IComponent *root) = 0;

  /// This method is used for a normal termination of a simulation.
  /// This method must only be called when in Standby state, and enters
  /// Exiting state.
  virtual void Exit() = 0;

  /// This method is used for an abnormal termination of a simulation.
  /// This method can be called from any other state, and enters Aborting
  /// state.
  virtual void Abort() = 0;

  /// Return the current simulator state.
  /// @return  Current simulator state.
  virtual SimulatorStateKind GetState() const = 0;

  /// This method can be used to add entry points that shall be executed
  /// in the Initialising state.
  /// If the simulation is in Building, Connecting or Standby state, it
  /// adds the entry point to the list of entry points to be executed
  /// once the simulation reaches Initialising state.
  /// If the simulation is not in Building, Connecting or Standby state,
  /// then it returns and no action is taken.
  /// This allows components to subscribe to a callback during
  /// initialization phase since there are only explicit methods defined
  /// for Publish, Configure and Connect. This choice is taken since most
  /// models require actions to be taken in Publish, Configure and
  /// Connect, however only a minority require to perform some actions
  /// during initialization.
  /// The entry points will be executed in the order they have been added
  /// to the simulator.
  /// @param   entryPoint Entry point to execute in Initialising state.
  virtual void AddInitEntryPoint(IEntryPoint *entryPoint) = 0;

  /// This method adds a model to the models collection of the simulator,
  /// i.e. to the "Models" container.
  /// This method raises an exception of type DuplicateName if the name
  /// of the new model conflicts with the name of an existing component
  /// (model or service).
  /// The container for the models has no upper limit and thus the
  /// ContainerFull exception will never be thrown.
  /// The method will never throw the InvalidObjectType exception either,
  /// as it gets a component implementing the IModel interface.
  /// @param   model New model to add to collection of root models, i.e.
  ///          to the "Models" container.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidSimulatorState
  virtual void AddModel(IModel *model) = 0;

  /// This method adds a user-defined service to the services collection,
  /// i.e. to the "Services" container.
  /// This method raises an exception of type DuplicateName if the name
  /// of the new service conflicts with the name of an existing component
  /// (model or service).
  /// The container for the services has no upper limit and thus the
  /// ContainerFull exception will never be thrown.
  /// The method will never throw the InvalidObjectType exception either,
  /// as it gets a component implementing the IService interface.
  /// @remarks It is recommended that custom services include a project
  ///          or company acronym as prefix in their name, to avoid
  ///          collision of service names.
  ///
  ///          The container for the services has no upper limit and thus
  ///          the ContainerFull exception will never be thrown.
  ///          The method will never throw the InvalidObjectType
  ///          exception, as it expects a component implementing the
  ///          IService interface.
  /// @param   service Service to add to services collection.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidSimulatorState
  virtual void AddService(IService *service) = 0;

  /// Query for a service component by its name.
  /// The returned component is null if no service with the given name
  /// could be found. Standard names are defined for the standardised
  /// services, while custom services use custom names.
  /// The existence of custom services is not guaranteed, so models
  /// should expect to get a null reference.
  /// @param   name Service name.
  /// @return  Service with the given name, or null if no service with
  ///          the given name could be found.
  virtual IService *GetService(String8 name) const = 0;

  /// Return interface to logger service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  ///          use the generic GetService() method. For the standardised
  ///          services, it is recommended to use the convenience
  ///          methods, which are guaranteed to return a valid reference.
  /// @return  Interface to mandatory logger service.
  virtual Services::ILogger *GetLogger() const = 0;

  /// Return interface to time keeper service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  ///          use the generic GetService() method. For the standardised
  ///          services, it is recommended to use the convenience
  ///          methods, which are guaranteed to return a valid reference.
  /// @return  Interface to mandatory time keeper service.
  virtual Services::ITimeKeeper *GetTimeKeeper() const = 0;

  /// Return interface to scheduler service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  ///          use the generic GetService() method. For the standardised
  ///          services, it is recommended to use the convenience
  ///          methods, which are guaranteed to return a valid reference.
  /// @return  Interface to mandatory scheduler service.
  virtual Services::IScheduler *GetScheduler() const = 0;

  /// Return interface to event manager service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  ///          use the generic GetService() method. For the standardised
  ///          services, it is recommended to use the convenience
  ///          methods, which are guaranteed to return a valid reference.
  /// @return  Interface to mandatory event manager service.
  virtual Services::IEventManager *GetEventManager() const = 0;

  /// Return interface to resolver service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  ///          use the generic GetService() method. For the standardised
  ///          services, it is recommended to use the convenience
  ///          methods, which are guaranteed to return a valid reference.
  /// @return  Interface to mandatory resolver service.
  virtual Services::IResolver *GetResolver() const = 0;

  /// Return interface to link registry service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  ///          use the generic GetService() method. For the standardised
  ///          services, it is recommended to use the convenience
  ///          methods, which are guaranteed to return a valid reference.
  /// @return  Interface to mandatory link registry service.
  virtual Services::ILinkRegistry *GetLinkRegistry() const = 0;

  /// This method registers a component factory with the simulator. The
  /// simulator can use this factory to create component instances of the
  /// component implementation in its CreateInstance() method.
  /// The memory management of the factory is passed to the simulator,
  /// who is in charge of deleting the factory at the exiting or aborting
  /// state.
  /// This method raises an exception of type DuplicateUuid if another
  /// factory has been registered using the same uuid already.
  /// @remarks This method is typically called early in the Building
  ///          state to register the available component factories before
  ///          the hierarchy of model instances is created.
  /// @param   componentFactory Factory to create instance of the
  ///          component implementation.
  /// @throws  Smp::DuplicateUuid
  virtual void RegisterFactory(IFactory *componentFactory) = 0;

  /// This method creates an instance of the component with the given
  /// unique identifier.
  /// @remarks This method is typically called during Creating state when
  ///          building the hierarchy of models.
  /// @param   uuid Unique identifier of the component to create.
  /// @param   name Name of the new instance.
  /// f the name provided is not a valid object name, an exception of
  /// type InvalidObjectName is raised.
  /// @param   description Description of the new instance.
  /// @param   parent Parent object of the new instance.
  /// @return  New instance of the component with the given
  ///          implementation identifier or null in case no factory for
  ///          the given implementation identifier has been registered.
  /// @throws  Smp::InvalidObjectName
  virtual IComponent *CreateInstance(Uuid uuid, String8 name,
                                     String8 description,
                                     IComposite *parent) = 0;

  /// This method returns the factory of the component with the given
  /// implementation identifier.
  /// @param   uuid Unique identifier of the component to get the factory
  ///          for.
  /// @return  Factory of the component with the given implementation
  ///          identifier or null in case no factory for the given
  ///          implementation identifier has been registered.
  virtual IFactory *GetFactory(Uuid uuid) const = 0;

  /// This method returns all factories that have been registered with
  /// the simulator.
  /// @return  Collection of factories.
  virtual const FactoryCollection *GetFactories() const = 0;

  /// Give access to the global type registry.
  /// The type registry is typically a singleton, and must not be null,
  /// to allow use of existing types, and registration of new types.
  /// @return  Reference to global type registry.
  virtual Publication::ITypeRegistry *GetTypeRegistry() const = 0;

  /// This operation loads a library of a package into memory.
  /// At loading time, the Initialise() function of this library will be
  /// called.
  /// At exiting or aborting time, the Finalise() function of this
  /// library will be called.
  /// @param   libraryPath Path to the library to load.
  /// This needs to be a valid path name given the constraints of the
  /// operating system.
  /// @throws  Smp::InvalidLibrary
  /// @throws  Smp::LibraryNotFound
  virtual void LoadLibrary(String8 libraryPath) = 0;
};
} // namespace Smp

#endif // SMP_ISIMULATOR_H
