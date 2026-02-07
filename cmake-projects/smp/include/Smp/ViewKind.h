#ifndef SMP_VIEWKIND_H
#define SMP_VIEWKIND_H

#include "Smp/PrimitiveTypes.h"

namespace Smp {
enum class ViewKind : Int32 { VK_None, VK_Debug, VK_Expert, VK_All };

std::ostream &operator<<(std::ostream &os, const ViewKind &obj);
} // namespace Smp

#endif // SMP_VIEWKIND_H
