#ifndef SMP_PUBLICATION_IPUBLISHOPERATION_H
#define SMP_PUBLICATION_IPUBLISHOPERATION_H

#include "Smp/DuplicateName.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/ParameterDirectionKind.h"
#include "Smp/Publication/TypeNotRegistered.h"
#include "Smp/Uuid.h"

namespace Smp {
namespace Publication {
class IPublishOperation {
public:
  virtual ~IPublishOperation() noexcept = default;

  virtual void PublishParameter(
      String8 name, String8 description, Uuid typeUuid,
      ParameterDirectionKind direction = ParameterDirectionKind::PDK_In) = 0;
};
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_IPUBLISHOPERATION_H
