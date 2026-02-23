#include "Smp/AccessKind.h"
#include <iostream>

namespace Smp {
std::ostream &operator<<(std::ostream &os, const AccessKind &obj) {
  switch (obj) {
  case AccessKind::AK_ReadWrite:
    os << "AK_ReadWrite";
    break;
  case AccessKind::AK_ReadOnly:
    os << "AK_ReadOnly";
    break;
  case AccessKind::AK_WriteOnly:
    os << "AK_WriteOnly";
    break;
  default:
    os << "Unknown AccessKind (" << static_cast<int>(obj) << ")";
    break;
  }
  return os;
}
} // namespace Smp
