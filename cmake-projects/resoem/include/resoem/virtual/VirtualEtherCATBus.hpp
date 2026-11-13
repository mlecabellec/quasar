/**
 * @file VirtualEtherCATBus.hpp
 * @brief In-memory simulated EtherCAT fieldbus connecting SimulatedSlave instances.
 * @details Contributes to [TSK-20260920-001.11] and [FE-0040.8].
 */

#pragma once

#include "resoem/common.hpp"
#include "resoem/virtual/SimulatedSlave.hpp"
#include "resoem/virtual/VirtualBusConstants.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <vector>

namespace resoem::virtual_bus {

/**
 * @class VirtualEtherCATBus
 * @brief Simulates physical EtherCAT network wiring, frame propagation, and cable faults.
 * @details Connects multiple SimulatedSlave instances in a linear or redundant ring topology,
 * dissecting raw EtherCAT frames and updating datagrams and working counters as they traverse slaves.
 */
class VirtualEtherCATBus : public std::enable_shared_from_this<VirtualEtherCATBus> {
public:
  /**
   * @brief Factory method creating a VirtualEtherCATBus.
   * @return Shared pointer to the new bus instance.
   */
  [[nodiscard]] static std::shared_ptr<VirtualEtherCATBus> create();

  /// @brief Virtual destructor.
  virtual ~VirtualEtherCATBus() noexcept = default;

  // Non-copyable via std::timed_mutex member

  /// @brief Attach a simulated slave to the downstream end of the bus.
  void attach_slave(std::shared_ptr<SimulatedSlave> slave);

  /**
   * @brief Remove all slaves from the bus.
   */
  void clear_slaves();

  /**
   * @brief Get the total number of connected slaves.
   * @return Slave count.
   */
  [[nodiscard]] std::size_t slave_count() const noexcept;

  /**
   * @brief Get a slave by its 0-based topological index.
   * @param index Slave index.
   * @return Shared pointer to slave or nullptr if index out of range.
   */
  [[nodiscard]] std::shared_ptr<SimulatedSlave> get_slave(std::size_t index) const;

  /// @brief Simulate a physical cable break after the specified slave index.
  void break_cable_after(std::size_t slave_index);

  /**
   * @brief Restore physical cable connectivity (heals simulated cable break).
   */
  void restore_cable();

  /**
   * @brief Check whether a cable break is currently simulated.
   * @return True if cable is broken.
   */
  [[nodiscard]] bool is_cable_broken() const noexcept;

  /**
   * @brief Process an incoming raw Ethernet frame through the simulated bus.
   * @param tx_frame Input frame bytes sent by Master.
   * @param from_secondary True if frame entered via the secondary redundant interface.
   * @param rx_frame Output buffer receiving the frame after processing by all reachable slaves.
   */
  void process_frame(std::span<const byte> tx_frame, bool from_secondary,
                     std::vector<byte> &rx_frame);

protected:
  /**
   * @brief Protected constructor for factory pattern.
   */
  VirtualEtherCATBus() = default;

private:
  mutable std::timed_mutex m_mutex;                                 ///< Concurrency guard
  std::vector<std::shared_ptr<SimulatedSlave>> m_slaves;            ///< Attached slaves
  std::optional<std::size_t> m_brokenAfterSlave;                    ///< Cable break location
};

} // namespace resoem::virtual_bus
