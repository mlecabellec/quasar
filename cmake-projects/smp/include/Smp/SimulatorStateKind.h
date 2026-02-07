#ifndef SMP_SIMULATORSTATEKIND_H
#define SMP_SIMULATORSTATEKIND_H

#include "Smp/PrimitiveTypes.h"

namespace Smp {
enum class SimulatorStateKind : Int32 {
  SSK_Building,
  SSK_Connecting,
  SSK_Initialising,
  SSK_Standby,
  SSK_Executing,
  SSK_Storing,
  SSK_Restoring,
  SSK_Reconnecting,
  SSK_Exiting,
  SSK_Aborting
};

std::ostream &operator<<(std::ostream &os, const SimulatorStateKind &obj);
} // namespace Smp

#endif // SMP_SIMULATORSTATEKIND_H
