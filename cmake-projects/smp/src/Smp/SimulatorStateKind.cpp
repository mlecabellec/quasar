#include "Smp/SimulatorStateKind.h"
#include <iostream>

namespace Smp {
std::ostream &operator<<(std::ostream &os, const SimulatorStateKind &obj) {
  switch (obj) {
  case SimulatorStateKind::SSK_Building:
    os << "SSK_Building";
    break;
  case SimulatorStateKind::SSK_Connecting:
    os << "SSK_Connecting";
    break;
  case SimulatorStateKind::SSK_Initialising:
    os << "SSK_Initialising";
    break;
  case SimulatorStateKind::SSK_Standby:
    os << "SSK_Standby";
    break;
  case SimulatorStateKind::SSK_Executing:
    os << "SSK_Executing";
    break;
  case SimulatorStateKind::SSK_Storing:
    os << "SSK_Storing";
    break;
  case SimulatorStateKind::SSK_Restoring:
    os << "SSK_Restoring";
    break;
  case SimulatorStateKind::SSK_Reconnecting:
    os << "SSK_Reconnecting";
    break;
  case SimulatorStateKind::SSK_Exiting:
    os << "SSK_Exiting";
    break;
  case SimulatorStateKind::SSK_Aborting:
    os << "SSK_Aborting";
    break;
  default:
    os << "Unknown SimulatorStateKind (" << static_cast<int>(obj) << ")";
    break;
  }
  return os;
}
} // namespace Smp
