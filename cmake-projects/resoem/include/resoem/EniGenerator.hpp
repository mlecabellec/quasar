/**
 * @file EniGenerator.hpp
 * @brief Automatic generation and export of ETG.2100 ENI configuration from detected EtherCAT topology.
 * @details Contributes to [TSK-20260920-001.3] and [FE-0040.8].
 */

#pragma once

#include "resoem/EniDocument.hpp"
#include "resoem/IRawSocket.hpp"
#include "resoem/common.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace resoem::eni {

/**
 * @class EniGenerator
 * @brief Discovers physical or virtual EtherCAT network layout and generates a standard ENI document.
 * @details Performs hybrid online interrogation: reads SII EEPROM strings, hardware link states,
 * SyncManager and FMMU allocations, and CoE SDO dictionary entries (0x1C12/0x1C13, 0x1600/0x1A00)
 * to build an ETG.2100 compliant AST model and serialize it to XML.
 */
class EniGenerator {
public:
  /// @brief Construct an EniGenerator bound to transport socket.
  explicit EniGenerator(IRawSocket &socket);

  /// @brief Virtual destructor.
  virtual ~EniGenerator() noexcept = default;

  // Non-copyable via std::timed_mutex member

  /**
   * @brief Interrogate the connected network and generate a complete EniDocument AST.
   * @param master_name Name assigned to the master node in the generated XML.
   * @return Result containing the generated EniDocument or an ECError.
   */
  [[nodiscard]] Result<EniDocument> generate(const std::string &master_name = "Master");

  /**
   * @brief Serialize an EniDocument AST to an ETG.2100 compliant XML string.
   * @param doc The EniDocument AST model.
   * @return Formatted XML text string.
   */
  [[nodiscard]] std::string serialize_to_xml_string(const EniDocument &doc) const;

  /**
   * @brief Save an EniDocument directly to an XML file.
   * @param filepath Target file path.
   * @param doc The EniDocument AST model.
   * @return Result<> indicating success or error.
   */
  [[nodiscard]] Result<void> save_to_xml_file(const std::string &filepath, const EniDocument &doc) const;

private:
  mutable std::timed_mutex m_mutex;
  IRawSocket &m_socket;

  /// @brief Probe slave SyncManager registers.
  void probe_slave_sync_managers(std::uint16_t phys_addr, EniSlave &out_slave);
  /// @brief Probe slave FMMU registers.
  void probe_slave_fmmus(std::uint16_t phys_addr, EniSlave &out_slave);
  /// @brief Probe slave port topology descriptors.
  void probe_slave_topology(std::size_t slave_idx, std::uint16_t phys_addr, EniSlave &out_slave);
  /// @brief Probe slave PDO mappings and CoE dictionary metadata.
  void probe_slave_pdos_and_coe(std::uint16_t phys_addr, EniSlave &out_slave);
};

} // namespace resoem::eni
