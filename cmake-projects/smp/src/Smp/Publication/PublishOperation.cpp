#include "Smp/Publication/PublishOperation.h"
#include "Smp/Parameter.h"
#include "Smp/Publication/TypeNotRegistered.h"

namespace Smp::Publication {

void PublishOperation::PublishParameter(Smp::String8 name,
                                        Smp::String8 description, Uuid typeUuid,
                                        ParameterDirectionKind direction) {
  auto type = registry->GetType(typeUuid);
  if (!type)
    throw TypeNotRegistered(typeUuid);

  auto parameter =
      new Smp::Parameter(name, description, operation, type, direction);
  if (direction == ParameterDirectionKind::PDK_Return) {
    operation->SetReturnParameter(parameter);
  } else {
    operation->AddParameter(parameter);
  }
}

} // namespace Smp::Publication
