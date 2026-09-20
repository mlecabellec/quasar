/**
 * @file EniDocument.hpp
 * @brief Strictly-typed modern C++23 AST and parser for ETG.2100 ENI files.
 * @details Contributes to [FE-0040] and [TSK-20260920-001].
 */

#pragma once

#include "resoem/EniConstants.hpp"
#include "resoem/common.hpp"
#include "quasar/named/NamedObject.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace resoem::eni {

/**
 * @brief Represents an InitCmd Validate clause in an ENI file.
 */
struct ValidateRule {
  bool signed_val = false;
  std::string type = "EQ";
  std::vector<std::uint8_t> data;
  std::vector<std::uint8_t> data_mask;
  std::uint32_t timeout_ms = 0;
};

/**
 * @brief Represents a register-level InitCmd in an ENI file.
 */
struct EniInitCmd {
  EsmTransition transition = EsmTransition::NONE;
  bool before_slave = false;
  std::string comment;
  std::string requirement;
  DatagramCmd cmd = DatagramCmd::NOP;
  std::uint16_t adp = 0;
  std::uint16_t ado = 0;
  std::uint32_t addr = 0;
  std::vector<std::uint8_t> data;
  std::size_t data_length = 0;
  std::uint32_t retries = DEFAULT_INIT_CMD_RETRIES;
  std::uint32_t timeout_ms = DEFAULT_INIT_CMD_TIMEOUT_MS;
  std::optional<ValidateRule> validate;
};

/**
 * @brief Represents a CoE (CANopen over EtherCAT) SDO InitCmd.
 */
struct EniCoEInitCmd {
  EsmTransition transition = EsmTransition::NONE;
  std::string comment;
  std::uint16_t index = 0;
  std::uint8_t subindex = 0;
  std::uint8_t ccs = 1; ///< 1=Download (write), 2=Upload (read)
  bool complete_access = false;
  bool fixed = false;
  std::uint32_t timeout_ms = 0;
  std::vector<std::uint8_t> data;
};

/**
 * @brief Defines the upstream port wiring for topology mapping.
 */
struct EniPreviousPort {
  char port = 'A'; ///< 'A', 'B', 'C', 'D'
  std::uint16_t phys_addr = 0;
  bool selected = true;
};

/**
 * @brief Represents Hot Connect group configuration and identification.
 */
struct EniHotConnect {
  std::string group_name;
  std::uint32_t group_member_count = 1;
  std::vector<EniInitCmd> identify_cmds;
};

/**
 * @brief Represents a single PDO entry mapping.
 */
struct EniPdoEntry {
  std::uint16_t index = 0;
  std::uint8_t subindex = 0;
  std::uint8_t bit_len = 0;
  std::string name;
  std::string data_type;
};

/**
 * @brief Represents a Process Data Object (RxPDO or TxPDO).
 */
struct EniPdo {
  std::uint16_t index = 0;
  std::string name;
  std::uint8_t sync_manager = 0;
  std::uint8_t sync_unit = 0;
  bool mandatory = false;
  bool is_tx = false; ///< false = RxPDO (outputs), true = TxPDO (inputs)
  std::vector<EniPdoEntry> entries;
};

/**
 * @brief Represents a SyncManager configuration.
 */
struct EniSyncManager {
  std::uint8_t index = 0;
  std::string type; ///< "Outputs", "Inputs", "MbxOut", "MbxIn"
  std::uint16_t start_address = 0;
  std::uint16_t length = 0;
  std::uint8_t control_byte = 0;
  bool enable = false;
  std::vector<std::uint16_t> assigned_pdos;
};

/**
 * @brief Distributed Clocks (DC) configuration for a slave.
 */
struct EniDC {
  bool enabled = false;
  bool potential_ref_clock = false;
  std::uint32_t cycle_time_0 = 0;
  std::uint32_t cycle_time_1 = 0;
  std::int32_t shift_time_0 = 0;
  std::int32_t shift_time_1 = 0;
};

/**
 * @brief Mailbox protocol parameters.
 */
struct EniMailbox {
  bool data_link_layer = true;
  std::uint16_t send_start = 0;
  std::uint16_t send_length = 0;
  std::uint16_t recv_start = 0;
  std::uint16_t recv_length = 0;
  std::uint16_t status_bit_addr = 0;
  std::vector<std::string> protocols; ///< "CoE", "FoE", "EoE", "AoE", "SoE"
  std::vector<EniCoEInitCmd> coe_init_cmds;
};

/**
 * @brief Complete configuration for a single EtherCAT slave in an ENI file.
 */
struct EniSlave {
  std::string name;
  std::uint16_t phys_addr = 0;
  std::int16_t auto_inc_addr = 0;
  std::string physics;
  std::uint32_t vendor_id = 0;
  std::uint32_t product_code = 0;
  std::uint32_t revision_no = 0;
  std::uint32_t serial_no = 0;

  std::optional<EniPreviousPort> previous_port;
  std::optional<EniHotConnect> hot_connect;
  std::vector<EniSyncManager> sync_managers;
  std::vector<EniPdo> rx_pdos;
  std::vector<EniPdo> tx_pdos;
  std::vector<EniInitCmd> init_cmds;
  EniMailbox mailbox;
  EniDC dc;
};

/**
 * @brief Master information and configuration.
 */
struct EniMaster {
  std::string name;
  std::string destination_mac = "FFFFFFFFFFFF";
  std::string source_mac = "000000000000";
  std::uint16_t ethertype = ETHERCAT_ETHERTYPE_VAL;
  std::vector<EniInitCmd> init_cmds;
};

/**
 * @brief Represents a single datagram command inside a cyclic frame.
 */
struct EniCyclicCmd {
  std::vector<std::string> states;
  std::string comment;
  DatagramCmd cmd = DatagramCmd::NOP;
  std::uint32_t addr = 0;
  std::uint16_t adp = 0;
  std::uint16_t ado = 0;
  std::size_t data_length = 0;
  std::uint16_t expected_wkc = 0;
  std::size_t input_offset = 0;
  std::size_t output_offset = 0;
};

/**
 * @brief Represents a cyclic Ethernet frame composed of datagrams.
 */
struct EniCyclicFrame {
  std::vector<EniCyclicCmd> commands;
};

/**
 * @brief Process image variable definition.
 */
struct EniProcessImageVariable {
  std::string name;
  std::string data_type;
  std::size_t bit_size = 0;
  std::size_t bit_offset = 0;
  bool is_input = false; ///< true = input, false = output
};

/**
 * @brief Top-level ETG.2100 ENI Document AST.
 * @details Contributes to [TSK-20260920-001.1].
 */
class EniDocument {
public:
  /**
   * @brief Construct an empty ENI document.
   */
  EniDocument() = default;

  /**
   * @brief Load and parse an ENI XML file from the local filesystem.
   * @param filepath Absolute or relative path to the XML file.
   * @return Result<> success or ECError on failure.
   */
  [[nodiscard]] Result<> load_from_file(const std::string &filepath);

  /**
   * @brief Parse an ENI XML document from an in-memory string.
   * @param xml_content In-memory XML text.
   * @return Result<> success or ECError on failure.
   */
  [[nodiscard]] Result<> load_from_memory(std::string_view xml_content);

  /**
   * @brief Serialize the ENI document into an XML file.
   * @param filepath Destination file path.
   * @return Result<> success or ECError on failure.
   */
  [[nodiscard]] Result<> save_to_file(const std::string &filepath) const;

  /**
   * @brief Serialize the ENI document into an XML string.
   * @return Result<std::string> Serialized XML text.
   */
  [[nodiscard]] Result<std::string> save_to_string() const;

  /**
   * @brief Find a slave configuration by configured physical station address.
   * @param phys_addr Configured station address (e.g. 1001).
   * @return Pointer to slave or nullptr if not found.
   */
  [[nodiscard]] const EniSlave *find_slave_by_phys_addr(std::uint16_t phys_addr) const;

  /**
   * @brief Find a slave configuration by its Hot Connect station alias.
   * @param alias Station alias (Register 0x0012).
   * @return Pointer to slave or nullptr if not found.
   */
  [[nodiscard]] const EniSlave *find_slave_by_alias(std::uint16_t alias) const;

  /**
   * @brief Export the entire ENI document into Quasar's NamedObject hierarchical tree.
   * @param parent Optional parent NamedObject node.
   * @return Root NamedObject of the materialized ENI tree.
   */
  [[nodiscard]] std::shared_ptr<quasar::named::NamedObject>
  to_named_tree(std::shared_ptr<quasar::named::NamedObject> parent = nullptr) const;

  /**
   * @brief Access the Master configuration.
   */
  [[nodiscard]] const EniMaster &master() const noexcept { return master_; }
  [[nodiscard]] EniMaster &master() noexcept { return master_; }

  /**
   * @brief Access the list of configured slaves.
   */
  [[nodiscard]] const std::vector<EniSlave> &slaves() const noexcept { return slaves_; }
  [[nodiscard]] std::vector<EniSlave> &slaves() noexcept { return slaves_; }

  /**
   * @brief Access cyclic frames.
   */
  [[nodiscard]] const std::vector<EniCyclicFrame> &cyclic_frames() const noexcept { return cyclic_frames_; }
  [[nodiscard]] std::vector<EniCyclicFrame> &cyclic_frames() noexcept { return cyclic_frames_; }

  /**
   * @brief Access process image variable definitions.
   */
  [[nodiscard]] const std::vector<EniProcessImageVariable> &process_image_variables() const noexcept {
    return process_image_variables_;
  }
  [[nodiscard]] std::vector<EniProcessImageVariable> &process_image_variables() noexcept {
    return process_image_variables_;
  }

  /**
   * @brief Get cycle time in microseconds.
   */
  [[nodiscard]] std::uint32_t cycle_time_us() const noexcept { return cycle_time_us_; }
  void set_cycle_time_us(std::uint32_t us) noexcept { cycle_time_us_ = us; }

  /**
   * @brief Get document version.
   */
  [[nodiscard]] const std::string &version() const noexcept { return version_; }
  void set_version(std::string ver) noexcept { version_ = std::move(ver); }

private:
  std::string version_ = "1.3";
  EniMaster master_;
  std::vector<EniSlave> slaves_;
  std::uint32_t cycle_time_us_ = DEFAULT_CYCLE_TIME_US;
  std::vector<EniCyclicFrame> cyclic_frames_;
  std::vector<EniProcessImageVariable> process_image_variables_;
};

} // namespace resoem::eni
