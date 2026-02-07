#ifndef SMP_COMPONENTSTATEKIND_H
#define SMP_COMPONENTSTATEKIND_H

#include "Smp/PrimitiveTypes.h"

namespace Smp {
enum class ComponentStateKind : Int32 {
  CSK_Created,
  CSK_Publishing,
  CSK_Configured,
  CSK_Connected,
  CSK_Disconnected
};

std::ostream &operator<<(std::ostream &os, const ComponentStateKind &obj);
} // namespace Smp

#endif // SMP_COMPONENTSTATEKIND_H
