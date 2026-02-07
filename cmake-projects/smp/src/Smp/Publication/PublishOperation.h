#ifndef SMP_PUBLICATION_PUBLISHOPERATION_IMPL_H
#define SMP_PUBLICATION_PUBLISHOPERATION_IMPL_H

#include "Smp/Object.h"
#include "Smp/Operation.h"
#include "Smp/Publication/IPublishOperation.h"
#include "Smp/Publication/ITypeRegistry.h"

namespace Smp::Publication {

class PublishOperation : public virtual IPublishOperation, public Smp::Object {
public:
  PublishOperation(Smp::String8 name, Smp::String8 description,
                   Smp::IObject *parent, Smp::ViewKind view,
                   Smp::Operation *operation, ITypeRegistry *registry)
      : Smp::Object(name, description, parent), view(view),
        operation(operation), registry(registry) {}

  virtual ~PublishOperation() noexcept = default;

  void PublishParameter(Smp::String8 name, Smp::String8 description,
                        Uuid typeUuid,
                        ParameterDirectionKind direction) override;

private:
  Smp::ViewKind view;
  Smp::Operation *operation;
  ITypeRegistry *registry;
};

} // namespace Smp::Publication

#endif // SMP_PUBLICATION_PUBLISHOPERATION_IMPL_H
