#include "resoem/Slave.hpp"
#include <array>
#include <cassert>
#include <iostream>
#include <vector>

// We replicate the logic from Enumerator::measure_propagation_delays
// to verify the algorithm correctness independent of hardware.
using namespace resoem;

// Helper from Enumerator.cpp
static uint8_t get_prev_port(uint8_t current_port,
                             const std::array<SlaveInfo::PortInfo, 4> &ports) {
  if (current_port == 0) {
    if (ports[2].active)
      return 2;
    if (ports[1].active)
      return 1;
    if (ports[3].active)
      return 3;
    return 0;
  } else if (current_port == 1) {
    if (ports[3].active)
      return 3;
    if (ports[0].active)
      return 0;
    if (ports[2].active)
      return 2;
    return 1;
  } else if (current_port == 2) {
    if (ports[1].active)
      return 1;
    if (ports[3].active)
      return 3;
    if (ports[0].active)
      return 0;
    return 2;
  } else if (current_port == 3) {
    if (ports[0].active)
      return 0;
    if (ports[2].active)
      return 2;
    if (ports[1].active)
      return 1;
    return 3;
  }
  return 0;
}

void calculate_delays(std::vector<SlaveInfo> &slaves,
                      const std::vector<std::array<uint32_t, 4>> &recv_times) {
  if (slaves.empty())
    return;

  slaves[0].propagation_delay = 0;
  slaves[0].has_dc = true;

  for (size_t i = 1; i < slaves.size(); ++i) {
    SlaveInfo &slave = slaves[i];
    int parent_idx = slave.parent_index;
    if (parent_idx < 0)
      continue;

    SlaveInfo &parent = slaves[parent_idx];

    uint8_t pport = slave.parent_port;
    uint8_t p_entry = parent.entry_port;

    uint32_t t_parent_exit = recv_times[parent_idx][pport];
    uint32_t t_parent_entry = recv_times[parent_idx][p_entry];

    int32_t internal_delay =
        static_cast<int32_t>(t_parent_exit - t_parent_entry);
    int32_t wire_delay = 0;

    slave.propagation_delay =
        parent.propagation_delay + internal_delay + wire_delay;
    slave.has_dc = true;
  }
}

void test_dc_linear() {
  // 0 -> 1 -> 2
  std::vector<SlaveInfo> slaves(3);
  std::vector<std::array<uint32_t, 4>> times(3);

  // Topology setup
  // Slave 0: Port 0 (Entry), Port 3 (Exit to 1)
  slaves[0].entry_port = 0;
  slaves[0].ports[0].active = true;
  slaves[0].ports[3].active = true;

  // Slave 1: Parent 0, Port 3. Port 0 (Entry), Port 3 (Exit to 2)
  slaves[1].parent_index = 0;
  slaves[1].parent_port = 3;
  slaves[1].entry_port = 0;
  slaves[1].ports[0].active = true;
  slaves[1].ports[3].active = true;

  // Slave 2: Parent 1, Port 3. Port 0 (Entry)
  slaves[2].parent_index = 1;
  slaves[2].parent_port = 3;
  slaves[2].entry_port = 0;
  slaves[2].ports[0].active = true;

  // Times
  // Slave 0: Enters at 1000, Exits Port 3 at 1100 (Internal = 100)
  times[0][0] = 1000;
  times[0][3] = 1100;

  // Slave 1: Enters at 1200 (Wire delay 100?), Exits Port 3 at 1300 (Internal =
  // 100)
  times[1][0] = 1200;
  times[1][3] = 1300;

  // Slave 2: Enters at 1400
  times[2][0] = 1400;

  calculate_delays(slaves, times);

  // Expected:
  // Slave 0: Delay 0
  assert(slaves[0].propagation_delay == 0);

  // Slave 1: ParentDelay(0) + Internal(0) + Wire(0)
  // Internal(0) = T_exit(0, P3) - T_entry(0, P0) = 1100 - 1000 = 100
  // Delay = 0 + 100 + 0 = 100
  std::cout << "Slave 1 Delay: " << slaves[1].propagation_delay << std::endl;
  assert(slaves[1].propagation_delay == 100);

  // Slave 2: ParentDelay(1) + Internal(1) + Wire(0)
  // Internal(1) = T_exit(1, P3) - T_entry(1, P0) = 1300 - 1200 = 100
  // Delay = 100 + 100 + 0 = 200
  std::cout << "Slave 2 Delay: " << slaves[2].propagation_delay << std::endl;
  assert(slaves[2].propagation_delay == 200);

  std::cout << "Linear DC Test Passed" << std::endl;
}

void test_dc_branch() {
  // 0 -> 1 (Port 3)
  //   -> 2 (Port 1)
  std::vector<SlaveInfo> slaves(3);
  std::vector<std::array<uint32_t, 4>> times(3);

  // Slave 0
  slaves[0].entry_port = 0;
  slaves[0].ports[0].active = true;
  slaves[0].ports[3].active = true; // to 1
  slaves[0].ports[1].active = true; // to 2

  // Slave 1
  slaves[1].parent_index = 0;
  slaves[1].parent_port = 3;
  slaves[1].entry_port = 0;

  // Slave 2
  slaves[2].parent_index = 0;
  slaves[2].parent_port = 1;
  slaves[2].entry_port = 0;

  // Times
  // Slave 0: Enters 1000.
  // Exits Port 3 at 1100 (Internal = 100)
  // Exits Port 1 at 1200 (Internal = 200 - assuming processing order 0->3->1)
  times[0][0] = 1000;
  times[0][3] = 1100;
  times[0][1] = 1200;

  calculate_delays(slaves, times);

  // Slave 1
  // Delay = 0 + (1100 - 1000) = 100
  assert(slaves[1].propagation_delay == 100);

  // Slave 2
  // Delay = 0 + (1200 - 1000) = 200
  assert(slaves[2].propagation_delay == 200);

  std::cout << "Branch DC Test Passed" << std::endl;
}

int main() {
  test_dc_linear();
  test_dc_branch();
  return 0;
}
