#ifndef SMP_INVALIDTARGET_H
#define SMP_INVALIDTARGET_H

#include "Smp/Exception.h"
#include "Smp/IField.h"

namespace Smp {
class IDataflowField;

class InvalidTarget : public virtual Exception {
public:
  virtual ~InvalidTarget() noexcept = default;

  virtual const IDataflowField *GetSource() const noexcept = 0;
  virtual const IField *GetTarget() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDTARGET_H
