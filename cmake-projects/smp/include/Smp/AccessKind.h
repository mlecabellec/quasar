#ifndef SMP_ACCESSKIND_H
#define SMP_ACCESSKIND_H

#include "Smp/PrimitiveTypes.h"

namespace Smp {
enum class AccessKind : Int32 { AK_ReadWrite, AK_ReadOnly, AK_WriteOnly };

std::ostream &operator<<(std::ostream &os, const AccessKind &obj);
} // namespace Smp

#endif // SMP_ACCESSKIND_H
