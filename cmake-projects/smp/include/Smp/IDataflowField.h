#ifndef SMP_IDATAFLOWFIELD_H
#define SMP_IDATAFLOWFIELD_H

#include "Smp/FieldAlreadyConnected.h"
#include "Smp/IField.h"
#include "Smp/InvalidTarget.h"

namespace Smp {
class IDataflowField : public virtual IField {
public:
  virtual ~IDataflowField() noexcept = default;

  virtual void Connect(IField *target) = 0;
  virtual void Push() = 0;
};
} // namespace Smp

#endif // SMP_IDATAFLOWFIELD_H
