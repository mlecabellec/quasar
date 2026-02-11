#include "Smp/Publication/PublishOperation.h"
#include "Smp/Parameter.h"
#include "Smp/Publication/TypeNotRegistered.h"

namespace Smp::Publication {

void PublishOperation::PublishParameter(Smp::String8 name,
                                        Smp::String8 description, Uuid typeUuid,
                                        ParameterDirectionKind direction) {
  IType *type = registry->GetType(typeUuid);
  if (!type)
    throw TypeNotRegistered(typeUuid);

  auto parameter = std::make_unique<Smp::Parameter>(name, description,
                                                    operation, type, direction);
  if (direction == ParameterDirectionKind::PDK_Return) {
    operation->SetReturnParameter(std::move(parameter));
  } else {
    operation->AddParameter(std::move(parameter));
  }
}

} // namespace Smp::Publication
