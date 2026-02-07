#ifndef SMP_ICOMPONENT_H
#define SMP_ICOMPONENT_H

#include "Smp/ComponentStateKind.h"
#include "Smp/ICollection.h"
#include "Smp/IField.h"
#include "Smp/IObject.h"
#include "Smp/InvalidComponentState.h"
#include "Smp/InvalidFieldName.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IPublication;
class ISimulator;

namespace Services {
class ILogger;
class ILinkRegistry;
} // namespace Services

/// This is the base interface for all SMP components.
/// All components need to implement this interface. As components
/// interface to the simulation environment, they have a dependency to it
/// via the two interfaces IPublication and ISimulator.
/// This is the only mandatory interface components have to implement. All
/// other interfaces are optional.
class IComponent : public virtual IObject {
public:
  /// Virtual destructor to release memory.
  virtual ~IComponent() noexcept = default;

  /// Returns the state the component is currently in.
  /// The component state can be changed using the Publish(), Configure()
  /// and Connect() state transition methods.
  /// @return  Current component state.
  virtual ComponentStateKind GetState() const = 0;

  /// Request the component to publish its fields, properties and
  /// operations against the provided publication receiver.
  /// This method can only be called once for each component, and only
  /// when the component is in the Created state. The method raises an
  /// InvalidComponentState exception if the component is not in Created
  /// state.
  /// When this operation is called, the component immediately enters the
  /// Publishing state, before it publishes any of its features.
  /// @remarks The simulation environment typically calls this method in
  ///          the Building state.
  /// @param   receiver Publication receiver.
  /// @throws  Smp::InvalidComponentState
  virtual void Publish(IPublication *receiver) = 0;

  /// Request the component to perform any custom configuration. The
  /// component can create and configure other components using the field
  /// values of its published fields.
  /// This method can only be called once for each component, and only
  /// when the component is in Publishing state. The method raises an
  /// InvalidComponentState exception if the component is not in
  /// Publishing state.
  /// The component can still publish further features in this call, and
  /// can even create other components, but at the end of this call, it
  /// needs to enter the Configured state.
  /// @remarks The simulation environment typically calls this method in
  ///          the Building state.
  /// @param   logger Logger service for logging of error messages during
  ///          configuration.
  /// @param   linkRegistry Reference to the link registry services, so
  ///          that the component can register links that it creates
  ///          during configuration.
  /// @throws  Smp::InvalidComponentState
  virtual void Configure(Services::ILogger *logger,
                         Services::ILinkRegistry *linkRegistry = nullptr) = 0;

  /// Allow the component to connect to the simulator and its simulation
  /// services.
  /// This method can only be called once for each component, and only
  /// when the component is in the Configured state. The method raises an
  /// InvalidComponentState exception if the component is not in
  /// Configured state.
  /// When this operation is called, the component immediately enters the
  /// Connected state, before it uses any of the simulator methods and
  /// services.
  /// In this method, the component may query for and use any of the
  /// available simulation services, as they are all guaranteed to be
  /// fully functional at that time. It may as well connect to
  /// components' functionality (e.g. to event sources), as it is
  /// guaranteed that all models have been created and configured before
  /// the Connect() method of any component is called.
  /// @remarks The simulation environment typically calls this method in
  ///          the Connecting state.
  /// @param   simulator Simulation Environment that hosts the component.
  /// @throws  Smp::InvalidComponentState
  virtual void Connect(ISimulator *simulator) = 0;

  /// Ask the component to disconnect from the simulator and all its
  /// simulation services.
  /// @throws  Smp::InvalidComponentState
  virtual void Disconnect() = 0;

  /// Get the field of given name.
  ///
  /// This method raises an exception of type InvalidFieldName if called
  /// with a field name for which no corresponding field exists.
  /// This method can be used both for fields of simple types (when it
  /// returns an instance of ISimpleField), and for complex fields (when
  /// it returns IArrayField or IStructureField).
  ///
  /// @remarks For getting access to simple fields of structure or array
  ///          types, this method may be called by specifying a full
  ///          field name, e.g. "MyField.Position[2]" in order to access
  ///          an array item of a structure.
  /// @param   fullName Fully qualified field name (relative to the
  ///          component).
  /// @return  Instance of field for given full name.
  /// @throws  Smp::InvalidFieldName
  virtual IField *GetField(String8 fullName) const = 0;

  /// Returns a collection of all fields of the component.
  /// @return  Collection of the fields (immediate children) of the
  ///          component.
  virtual const FieldCollection *GetFields() const = 0;

  /// Get Universally Unique Identifier of Component Type.
  ///
  ///
  /// Operation to get the Universally Unique Identifier of a Component
  /// Type. This allows retrieving metadata from a Catalogue at run-time.
  /// @return  Universally Unique Identifier of Component.
  virtual const Uuid &GetUuid() const = 0;
};

/// A component collection is an ordered collection of components, which
/// allows iterating all members.
using ComponentCollection = ICollection<IComponent>;
} // namespace Smp

#endif // SMP_ICOMPONENT_H
