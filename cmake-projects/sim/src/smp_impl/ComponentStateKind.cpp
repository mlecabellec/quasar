#include "Smp/ComponentStateKind.h"
#include <iostream>

namespace Smp {
std::ostream &operator<<(std::ostream &os, const ComponentStateKind &obj) {
  switch (obj) {
  case ComponentStateKind::CSK_Created:
    os << "CSK_Created";
    break;
  case ComponentStateKind::CSK_Publishing:
    os << "CSK_Publishing";
    break;
  case ComponentStateKind::CSK_Configured:
    os << "CSK_Configured";
    break;
  case ComponentStateKind::CSK_Connected:
    os << "CSK_Connected";
    break;
  case ComponentStateKind::CSK_Disconnected:
    os << "CSK_Disconnected";
    break;
  default:
    os << "Unknown ComponentStateKind (" << static_cast<int>(obj) << ")";
    break;
  }
  return os;
}
} // namespace Smp
