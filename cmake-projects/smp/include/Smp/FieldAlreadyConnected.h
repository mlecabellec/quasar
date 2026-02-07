#ifndef SMP_FIELDALREADYCONNECTED_H
#define SMP_FIELDALREADYCONNECTED_H

#include "Smp/Exception.h"

namespace Smp {
class IDataflowField;
class IField;

class FieldAlreadyConnected : public virtual Exception {
public:
  virtual ~FieldAlreadyConnected() noexcept = default;

  virtual const IDataflowField *GetSource() const noexcept = 0;
  virtual const IField *GetTarget() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_FIELDALREADYCONNECTED_H
