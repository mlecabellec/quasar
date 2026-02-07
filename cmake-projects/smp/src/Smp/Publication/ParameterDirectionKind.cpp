#include "Smp/Publication/ParameterDirectionKind.h"
#include <iostream>

namespace Smp {
namespace Publication {

std::ostream &operator<<(std::ostream &os, const ParameterDirectionKind &obj) {
  switch (obj) {
  case ParameterDirectionKind::PDK_In:
    os << "In";
    break;
  case ParameterDirectionKind::PDK_Out:
    os << "Out";
    break;
  case ParameterDirectionKind::PDK_InOut:
    os << "InOut";
    break;
  case ParameterDirectionKind::PDK_Return:
    os << "Return";
    break;
  default:
    os << "UnknownParameterDirectionKind(" << static_cast<int>(obj) << ")";
    break;
  }
  return os;
}

} // namespace Publication
} // namespace Smp
