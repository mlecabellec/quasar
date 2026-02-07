#include "Smp/ViewKind.h"
#include <iostream>

namespace Smp {
std::ostream &operator<<(std::ostream &os, const ViewKind &obj) {
  switch (obj) {
  case ViewKind::VK_None:
    os << "VK_None";
    break;
  case ViewKind::VK_Debug:
    os << "VK_Debug";
    break;
  case ViewKind::VK_Expert:
    os << "VK_Expert";
    break;
  case ViewKind::VK_All:
    os << "VK_All";
    break;
  default:
    os << "Unknown ViewKind (" << static_cast<int>(obj) << ")";
    break;
  }
  return os;
}
} // namespace Smp
