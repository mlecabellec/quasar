#ifndef SMP_INVALIDFIELDNAME_H
#define SMP_INVALIDFIELDNAME_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidFieldName : public virtual Exception {
public:
  InvalidFieldName(String8 name)
      : Exception("InvalidFieldName", "Invalid field name",
                  "The field name is invalid"),
        invalidName(name ? name : "") {}
  virtual ~InvalidFieldName() noexcept = default;

  virtual String8 GetFieldName() const noexcept { return invalidName.c_str(); }

protected:
  std::string invalidName;
};
} // namespace Smp

#endif // SMP_INVALIDFIELDNAME_H
