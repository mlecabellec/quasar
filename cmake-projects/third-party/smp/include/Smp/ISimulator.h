// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/ISimulator.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_ISIMULATOR_H_
#define SMP_ISIMULATOR_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/DuplicateName.h"
#include "Smp/DuplicateUuid.h"
#include "Smp/FactoryCollection.h"
#include "Smp/FileNotFound.h"
#include "Smp/IComposite.h"
#include "Smp/InvalidFile.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/InvalidSimulatorState.h"
#include "Smp/InvalidSmpVersion.h"
#include "Smp/LibraryLoadingFlag.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/SimulatorStateKind.h"
#include "Smp/Uuid.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp {
class IComponent;
class IEntryPoint;
class IFactory;
class IModel;
class IService;

namespace Publication {
class ITypeRegistry;
}

namespace Services {
class IEventManager;
class ILinkRegistry;
class ILogger;
class IResolver;
class IScheduler;
class ITimeKeeper;
} // namespace Services
} // namespace Smp

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp {
/// This interface gives access to the simulation environment state and
/// state transitions. Further, it provides convenience methods to add
/// models, and to add and retrieve simulation services.
/// This is a mandatory interface that every SMP compliant simulation
/// environment has to implement.
class ISimulator : public virtual Smp::IComposite {
public:
  /// Name of the model container.
  static constexpr Smp::Char8 SMP_SimulatorModels[] = "Models";

  /// Name of the service container.
  static constexpr Smp::Char8 SMP_SimulatorServices[] = "Services";

  /// Virtual destructor to release memory.
  virtual ~ISimulator() noexcept = default;

  /// This method asks the simulation environment to call all
  /// initialisation entry points again.
  /// This method can only be called when in Standby state, and enters
  /// Initialising state. After completion, the simulator automatically
  /// returns to Standby state. The entry points will be executed in the
  /// order they have been added to the simulator using the
  /// AddInitEntryPoint() method.
  /// @throws  Smp::InvalidSimulatorState
  virtual void Initialise() = 0;

  /// This method asks the simulation environment to call the Publish()
  /// method of all service and model instances in the component
  /// hierarchy which are still in Created state.
  /// The Publish() operation will traverse recursively through both the
  /// "Models" container and then the "Services" container of the
  /// simulator. It will call the Publish() operation of each component
  /// in CSK_Created state, and then immediately on all its child
  /// components. This method must only be called when in Building state.
  /// @remarks This method is typically called by an external component
  /// after creating new model instances, typically from information in
  /// an SMDL Assembly.
  /// @throws  Smp::InvalidSimulatorState
  virtual void Publish() = 0;

  /// This method asks the simulation environment to call the Configure()
  /// method of all service and model instances which are still in
  /// Publishing state.
  /// The Configure() operation will traverse recursively through both
  /// the "Models" container and then the "Services" container of the
  /// simulator. For each component, it will first call the Publish()
  /// operation (if the component is still in CSK_Created state), then
  /// the Configure() operation (if the component is in CSK_Publishing
  /// state), and then the simulator will immediately perform the same
  /// operation(s) on all child components of the component. This method
  /// must only be called when in Building state.
  /// @remarks This method is typically called by an external component
  /// after setting field values of new model instances, typically using
  /// the information in an SMDL Assembly or SMDL Configuration.
  /// @throws  Smp::InvalidSimulatorState
  virtual void Configure() = 0;

  /// This method informs the simulation environment that the hierarchy
  /// of model instances has been configured, and can now be connected to
  /// the simulator. Thus, the simulation environment calls the Connect()
  /// method of all service and model instances.
  /// In this method, the simulation environment first enters Connecting
  /// state and calls the Connect() method of every model in the model
  /// hierarchy, then enters Initialising state and calls the
  /// initialisation entry points, and finally enters Standby state. The
  /// Connect() operation will traverse recursively through both the
  /// "Models" container and then the "Services" container of the
  /// simulator. For each component, it will first call the Publish() and
  /// Configure() operations (if the component is still in CSK_Created
  /// state), then the Connect() operation (if the component is in
  /// CSK_Configured state), and then the simulator will immediately
  /// perform the same operation(s) on all child components of the
  /// component. This method must only be called when in Building state.
  /// @remarks This method is typically called by an external component
  /// after configuring all model instances.
  /// @throws  Smp::InvalidSimulatorState
  virtual void Connect() = 0;

  /// This method changes from Standby to Executing state.
  /// This method must only be called when in Standby state, and enters
  /// Executing state.
  /// @throws  Smp::InvalidSimulatorState
  virtual void Run() = 0;

  /// This method changes from Executing to Standby state.
  /// This method must only be called when in Executing state, and enters
  /// Standby state.
  /// @param   immediate True if the simulator shall stop immediately
  /// after completion of the current scheduler event, false if the
  /// simulator shall still process all events of the current simulation
  /// time before entering Standby state.
  /// @throws  Smp::InvalidSimulatorState
  virtual void Hold(Smp::Bool immediate) = 0;

  /// This method is used to store a state vector to file.
  /// This method must only be called when in Standby state, and enters
  /// Storing state. On completion, it automatically returns to Standby
  /// state.
  /// @param   filename Name including the full path to use for
  /// simulation state vector file.
  /// @throws  Smp::InvalidSimulatorState
  virtual void Store(Smp::String8 filename) = 0;

  /// This method is used to restore a state vector from file.
  /// This method must only be called when in Standby state, and enters
  /// Restoring state. On completion, it automatically returns to Standby
  /// state.
  /// @param   filename Name including the full path of simulation state
  /// vector file.
  /// @throws  Smp::InvalidSimulatorState
  virtual void Restore(Smp::String8 filename) = 0;

  /// This method asks the simulation environment to reconnect the
  /// component hierarchy starting at the given root component.
  /// The simulation environment has to ensure that all models under the
  /// given root (but not the root itself) are published, configured and
  /// connected, so that all child models are finally in Connected state.
  /// This method must only be called when in Standby state.
  /// @remarks This method is typically called after creating additional
  /// model instances and adding them to the existing model hierarchy.
  /// @param   root Root component to start reconnecting from. This can
  /// be the parent component of a newly added model. To reconnect all
  /// models under the simulator, use nullptr (as the simulator is no
  /// component).
  /// @throws  Smp::InvalidSimulatorState
  virtual void Reconnect(Smp::IComponent *root) = 0;

  /// This method is used for a normal termination of a simulation.
  /// This method must only be called when in Standby state, and enters
  /// Exiting state.
  /// @throws  Smp::InvalidSimulatorState
  virtual void Exit() = 0;

  /// This method is used for an abnormal termination of a simulation.
  /// This method can be called from any other state, and enters Aborting
  /// state.
  virtual void Abort() = 0;

  /// Return the current simulator state.
  /// @return  Current simulator state.
  virtual Smp::SimulatorStateKind GetSimulatorState() const = 0;

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
  /// The ownership of the entry point remains with the caller.
  /// The entry points will be executed in the order they have been added
  /// to the simulator.
  /// @param   entryPoint Entry point to execute in Initialising state.
  virtual void AddInitEntryPoint(Smp::IEntryPoint *entryPoint) = 0;

  /// This method adds a model to the models collection of the simulator,
  /// i.e. to the "Models" container.
  /// This method raises an exception of type DuplicateName if the name
  /// of the new model conflicts with the name of an existing component
  /// (model or service). The container for the models has no upper limit
  /// and thus the ContainerFull exception will never be thrown. The
  /// method will never throw the InvalidObjectType exception either, as
  /// it gets a component implementing the IModel interface.
  /// @param   model New model to add to collection of root models, i.e.
  /// to the "Models" container. The ownership of the model is handed
  /// over to the object implementing ISimulator.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidSimulatorState
  virtual void AddModel(Smp::IModel *model) = 0;

  /// This method adds a user-defined service to the services collection,
  /// i.e. to the "Services" container.
  /// This method raises an exception of type DuplicateName if the name
  /// of the new service conflicts with the name of an existing component
  /// (model or service). The container for the services has no upper
  /// limit and thus the ContainerFull exception will never be thrown.
  /// The method will never throw the InvalidObjectType exception either,
  /// as it gets a component implementing the IService interface.
  /// @remarks It is recommended that custom services include a project
  /// or company acronym as prefix in their name, to avoid collision of
  /// service names.
  /// @param   service Service to add to collection of services, i.e. to
  /// the "Services" container. The ownership of the service is handed
  /// over to the object implementing ISimulator.
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidSimulatorState
  virtual void AddService(Smp::IService *service) = 0;

  /// Query for a service component by its name.
  /// The returned component is nullptr if no service with the given name
  /// could be found. The existence of custom services is not guaranteed,
  /// so models should expect to get a nullptr reference.
  /// @param   name Service name.
  /// @return  Service with the given name, or nullptr if no service with
  /// the given name could be found.
  virtual Smp::IService *GetService(Smp::String8 name) const = 0;

  /// Return interface to logger service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  /// use the generic GetService() method. For the standardised services,
  /// it is recommended to use the convenience methods, which are
  /// guaranteed to return a valid reference.
  /// @return  Interface to mandatory logger service.
  virtual Smp::Services::ILogger *GetLogger() const = 0;

  /// Return interface to time keeper service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  /// use the generic GetService() method. For the standardised services,
  /// it is recommended to use the convenience methods, which are
  /// guaranteed to return a valid reference.
  /// @return  Interface to mandatory time keeper service.
  virtual Smp::Services::ITimeKeeper *GetTimeKeeper() const = 0;

  /// Return interface to scheduler service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  /// use the generic GetService() method. For the standardised services,
  /// it is recommended to use the convenience methods, which are
  /// guaranteed to return a valid reference.
  /// @return  Interface to mandatory scheduler service.
  virtual Smp::Services::IScheduler *GetScheduler() const = 0;

  /// Return interface to event manager service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  /// use the generic GetService() method. For the standardised services,
  /// it is recommended to use the convenience methods, which are
  /// guaranteed to return a valid reference.
  /// @return  Interface to mandatory event manager service.
  virtual Smp::Services::IEventManager *GetEventManager() const = 0;

  /// Return interface to resolver service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  /// use the generic GetService() method. For the standardised services,
  /// it is recommended to use the convenience methods, which are
  /// guaranteed to return a valid reference.
  /// @return  Interface to mandatory resolver service.
  virtual Smp::Services::IResolver *GetResolver() const = 0;

  /// Return interface to link registry service.
  /// @remarks This is a type-safe convenience method, to avoid having to
  /// use the generic GetService() method. For the standardised services,
  /// it is recommended to use the convenience methods, which are
  /// guaranteed to return a valid reference.
  /// @return  Interface to mandatory link registry service.
  virtual Smp::Services::ILinkRegistry *GetLinkRegistry() const = 0;

  /// This method registers a component factory with the simulator. The
  /// simulator can use this factory to create component instances of the
  /// component implementation in its CreateInstance() method.
  /// The memory management of the factory is passed to the simulator,
  /// who is in charge of deleting the factory at the exiting or aborting
  /// state.
  /// This method raises an exception of type DuplicateUuid if another
  /// factory has been registered using the same uuid already.
  /// @remarks This method is typically called early in the Building
  /// state to register the available component factories before the
  /// hierarchy of model instances is created.
  /// @param   componentFactory Factory to create instance of the
  /// component implementation.
  /// @throws  Smp::DuplicateUuid
  virtual void RegisterFactory(Smp::IFactory *componentFactory) = 0;

  /// This method creates an instance of the component with the given
  /// unique identifier.
  /// @remarks This method is typically called during Creating state when
  /// building the hierarchy of models.
  /// @param   uuid Unique identifier of the component to create.
  /// @param   name Name of the new instance. If the name provided is not
  /// a valid object name, an exception of type InvalidObjectName is
  /// raised.
  /// @param   description Description of the new instance.
  /// @param   parent Parent object of the new instance.
  /// @return  New instance of the component with the given unique
  /// identifier or nullptr in case no factory for the given unique
  /// identifier has been registered.
  /// @throws  Smp::InvalidObjectName
  virtual Smp::IComponent *CreateInstance(Smp::Uuid uuid, Smp::String8 name,
                                          Smp::String8 description,
                                          Smp::IComposite *parent) = 0;

  /// This method returns the factory of the component with the given
  /// unique identifier.
  /// @param   uuid Unique identifier of the component to get the factory
  /// for.
  /// @return  Factory of the component with the given unique identifier
  /// or nullptr in case no factory for the given unique identifier has
  /// been registered.
  virtual Smp::IFactory *GetFactory(Smp::Uuid uuid) const = 0;

  /// This method returns all factories that have been registered with
  /// the simulator.
  /// @return  Collection of factories.
  virtual const Smp::FactoryCollection *GetFactories() const = 0;

  /// Give access to the global type registry.
  /// The type registry is typically a singleton, and must not be null,
  /// to allow use of existing types, and registration of new types.
  /// @return  Reference to global type registry.
  virtual Smp::Publication::ITypeRegistry *GetTypeRegistry() const = 0;

  /// This operation loads a library of a package into memory.
  /// At loading time, the Initialise() function of this library will be
  /// called.
  /// At exiting or aborting time, the Finalise() function of this
  /// library will be called.
  /// @param   libraryPath Path to the library to load.
  /// This needs to be a valid path name given the constraints of the
  /// operating system.
  /// @param   flag Loading flag for the library, to define in which
  /// scope the symbols from the library are made available.
  /// @throws  Smp::FileNotFound
  /// @throws  Smp::InvalidFile
  /// @throws  Smp::InvalidSmpVersion
  virtual void LoadLibrary(
      Smp::String8 libraryPath,
      Smp::LibraryLoadingFlag flag = Smp::LibraryLoadingFlag::LLF_Auto) = 0;
};
} // namespace Smp

#endif // SMP_ISIMULATOR_H_
