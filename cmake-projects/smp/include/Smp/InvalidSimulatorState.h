#ifndef SMP_INVALIDSIMULATORSTATE_H
#define SMP_INVALIDSIMULATORSTATE_H

#include "Smp/Exception.h"
#include "Smp/SimulatorStateKind.h"

namespace Smp {
class InvalidSimulatorState : public virtual Exception {
public:
  InvalidSimulatorState(SimulatorStateKind state)
      : Exception("InvalidSimulatorState",
                  "Invalid simulator state for operation",
                  "Simulator is in an invalid state for this operation"),
        invalidState(state) {}
  virtual ~InvalidSimulatorState() noexcept = default;

  virtual SimulatorStateKind GetInvalidState() const noexcept {
    return invalidState;
  }

  const char *what() const noexcept override { return message.c_str(); }

protected:
  SimulatorStateKind invalidState;
};
} // namespace Smp

#endif // SMP_INVALIDSIMULATORSTATE_H
