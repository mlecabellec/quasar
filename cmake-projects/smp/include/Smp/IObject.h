#ifndef SMP_IOBJECT_H
#define SMP_IOBJECT_H

#include "Smp/PrimitiveTypes.h"

namespace Smp {
/// This interface is the base interface for almost all other SMP
/// interfaces. While most interfaces derive from IComponent, which itself
/// is derived from IObject, some objects (including IField, IFailure,
/// IEntryPoint, IEventSink, IEventSource, IContainer and IReference) are
/// directly derived from IObject.
/// @remarks The  methods of this interface ensure that all SMP objects can
///          be shown with a name, with an optional description, and under
///          their parent object. Only the simulator itself is expected not
///          to have a parent object, as it is the top-level object of all
///          other objects.
class IObject {
public:
  /// Explicit default constructor.
  IObject() = default;

  /// Virtual destructor to release memory.
  virtual ~IObject() noexcept = default;

  /// Objects cannot be copied
  IObject(const IObject &) = delete;
  /// Objects cannot be copied
  IObject &operator=(const IObject &) = delete;
  /// Objects cannot be moved
  IObject(IObject &&) = delete;
  /// Objects cannot be moved
  IObject &operator=(IObject &&) = delete;

  /// Return the name of the object.
  /// Names must
  ///      - be unique within their context (i.e. for the same parent),
  ///      - not be empty,
  ///      - start with a letter, and
  ///      - only contain letters, digits, the underscore ("_") and
  /// brackets ("[" and "]").
  ///
  /// @remarks Applications may display the name as user readable object
  ///          identification.
  /// @return  Name of object.
  virtual String8 GetName() const = 0;

  /// Return the description of the object ("property getter").
  /// Descriptions are optional and may be empty.
  /// @remarks Applications may display the description as additional
  ///          information about the object.
  /// @return  Description of object.
  virtual String8 GetDescription() const = 0;

  /// Returns the parent object of the object.
  /// Objects link to their parent to allow traversing the tree of
  /// objects upwards to the root object of a composition.
  /// @remarks Typically, only the simulator itself is a root object, so
  ///          all other objects should have a parent object.
  /// @return  Parent object of object or null if object has no parent.
  virtual IObject *GetParent() const = 0;
};
} // namespace Smp

#endif // SMP_IOBJECT_H
