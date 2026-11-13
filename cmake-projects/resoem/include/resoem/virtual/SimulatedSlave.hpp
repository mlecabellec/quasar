/**
 * @file SimulatedSlave.hpp
 * @brief Software emulation of an EtherCAT Slave Controller (ESC) with NamedObject synchronization.
 * @details Contributes to [FE-0040.8] Simulation & Stubs and [TSK-20260920-001.12].
 */

#pragma once

#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedObject.hpp"
#include "resoem/EniDocument.hpp"
#include "resoem/EtherCATTypes.hpp"
#include "resoem/common.hpp"
#include "resoem/virtual/VirtualBusConstants.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>

namespace resoem::virtual_bus {

/**
 * @struct SimulatedPdoVariable
 * @brief Metadata tracking a bound process data variable within simulated RAM.
 */
struct SimulatedPdoVariable {
  std::string name;                                        ///< Name of the variable
  std::size_t bit_offset;                                  ///< Bit offset within the process image
  std::size_t bit_length;                                  ///< Bit width of the variable
  bool is_input;                                           ///< True if TxPDO (Slave -> Master), false if RxPDO
  std::shared_ptr<quasar::named::NamedInteger<std::int64_t>> named_node; ///< Bound Quasar Named node
};

/**
 * @class SimulatedSlave
 * @brief Complete in-memory software emulation of an EtherCAT Slave Controller.
 * @details Emulates the 4KB ESC register file, SII EEPROM state machine, FMMU address
 * translation, SyncManager handshakes, and provides bidirectional synchronization with
 * the Quasar NamedObject hierarchy.
 */
class SimulatedSlave : public std::enable_shared_from_this<SimulatedSlave> {
public:
  /**
   * @brief Factory method creating a simulated slave.
   * @param name Name of the slave node.
   * @param vendor_id EtherCAT Vendor ID.
   * @param product_code EtherCAT Product Code.
   * @param revision_no Hardware revision number.
   * @param serial_no Serial number.
   * @param parent Optional parent NamedObject to attach the slave hierarchy to.
   * @return Shared pointer to the created SimulatedSlave.
   */
  [[nodiscard]] static std::shared_ptr<SimulatedSlave> create(
      const std::string &name,
      std::uint32_t vendor_id = 0x00000002,
      std::uint32_t product_code = 0x044C2C52,
      std::uint32_t revision_no = 0x00100000,
      std::uint32_t serial_no = 0x00000001,
      std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

  /**
   * @brief Factory method creating a simulated slave directly from an ENI slave definition.
   * @param slave_cfg ENI slave configuration structure.
   * @param parent Optional parent NamedObject.
   * @return Shared pointer to configured SimulatedSlave.
   */
  [[nodiscard]] static std::shared_ptr<SimulatedSlave> create_from_eni(
      const resoem::eni::EniSlave &slave_cfg,
      std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

  /// @brief Virtual destructor.
  virtual ~SimulatedSlave() noexcept = default;

  // Non-copyable via std::timed_mutex member

  /// @brief Get the root NamedObject node of this slave.
  [[nodiscard]] std::shared_ptr<quasar::named::NamedObject> get_root_node() const noexcept;

  /// @brief Get the configured station address (ESC reg 0x0010).
  [[nodiscard]] std::uint16_t get_configured_address() const noexcept;

  /// @brief Set the configured station address (ESC reg 0x0010).
  void set_configured_address(std::uint16_t address);

  /// @brief Get the configured station alias (ESC reg 0x0012).
  [[nodiscard]] std::uint16_t get_station_alias() const noexcept;

  /// @brief Set the station alias (ESC reg 0x0012).
  void set_station_alias(std::uint16_t alias);

  /// @brief Get current Application Layer (AL) state.
  [[nodiscard]] std::uint16_t get_al_state() const noexcept;

  /// @brief Set Application Layer state directly (for testing/fault injection).
  void set_al_state(std::uint16_t state);

  /**
   * @brief Add a TxPDO input variable (Slave -> Master) bound to NamedObject tree.
   * @param name Variable identifier.
   * @param bit_offset Offset in bits within the input process image.
   * @param bit_length Width in bits.
   * @param initial_value Initial numerical value.
   */
  void add_input_variable(const std::string &name, std::size_t bit_offset,
                          std::size_t bit_length, std::int64_t initial_value = 0);

  /**
   * @brief Add an RxPDO output variable (Master -> Slave) bound to NamedObject tree.
   * @param name Variable identifier.
   * @param bit_offset Offset in bits within the output process image.
   * @param bit_length Width in bits.
   * @param initial_value Initial numerical value.
   */
  void add_output_variable(const std::string &name, std::size_t bit_offset,
                           std::size_t bit_length, std::int64_t initial_value = 0);

  /**
   * @brief Read a value from an input variable in the Named tree.
   * @param name Variable name.
   * @return Current value.
   */
  [[nodiscard]] std::int64_t get_input_value(const std::string &name) const;

  /// @brief Set the value of an input variable (simulating sensor data change).
  void set_input_value(const std::string &name, std::int64_t value);

  /**
   * @brief Read a value from an output variable in the Named tree (set by Master).
   * @param name Variable name.
   * @return Current value.
   */
  [[nodiscard]] std::int64_t get_output_value(const std::string &name) const;

  /**
   * @brief Process an Auto-Increment Physical Read (APRD) datagram.
   * @param addr Auto-increment address (modified in-place by the slave).
   * @param offset ESC register offset.
   * @param data Payload span to write into.
   * @param wkc Working counter to increment if addressed.
   */
  void process_aprd(std::uint16_t &addr, std::uint16_t offset,
                    std::span<byte> data, std::uint16_t &wkc);

  /**
   * @brief Process an Auto-Increment Physical Write (APWR) datagram.
   * @param addr Auto-increment address (modified in-place by the slave).
   * @param offset ESC register offset.
   * @param data Payload span to read from.
   * @param wkc Working counter to increment if addressed.
   */
  void process_apwr(std::uint16_t &addr, std::uint16_t offset,
                    std::span<const byte> data, std::uint16_t &wkc);

  /**
   * @brief Process a Configured Address Physical Read (FPRD) datagram.
   * @param addr Configured station address.
   * @param offset ESC register offset.
   * @param data Payload span to write into.
   * @param wkc Working counter to increment if addressed.
   */
  void process_fprd(std::uint16_t addr, std::uint16_t offset,
                    std::span<byte> data, std::uint16_t &wkc);

  /**
   * @brief Process a Configured Address Physical Write (FPWR) datagram.
   * @param addr Configured station address.
   * @param offset ESC register offset.
   * @param data Payload span to read from.
   * @param wkc Working counter to increment if addressed.
   */
  void process_fpwr(std::uint16_t addr, std::uint16_t offset,
                    std::span<const byte> data, std::uint16_t &wkc);

  /// @brief Process a Broadcast Read (BRD) datagram.
  void process_brd(std::uint16_t offset, std::span<byte> data, std::uint16_t &wkc);

  /// @brief Process a Broadcast Write (BWR) datagram.
  void process_bwr(std::uint16_t offset, std::span<const byte> data, std::uint16_t &wkc);

  /// @brief Process a Logical ReadWrite (LRW) datagram.
  void process_lrw(std::uint32_t logical_addr, std::span<byte> data, std::uint16_t &wkc);

  /// @brief Process a Logical Read (LRD) datagram.
  void process_lrd(std::uint32_t logical_addr, std::span<byte> data, std::uint16_t &wkc);

  /// @brief Process a Logical Write (LWR) datagram.
  void process_lwr(std::uint32_t logical_addr, std::span<const byte> data, std::uint16_t &wkc);

  /**
   * @brief Synchronize internal TxPDO RAM buffer from current NamedObject input values.
   */
  void sync_inputs_from_named();

  /**
   * @brief Synchronize current RxPDO RAM buffer into NamedObject output values.
   */
  void sync_outputs_to_named();

  /// @brief Direct read access to simulated ESC register RAM.
  void read_esc_ram(std::size_t offset, std::span<byte> buffer) const;

  /// @brief Direct write access to simulated ESC register RAM.
  void write_esc_ram(std::size_t offset, std::span<const byte> buffer);

  /// @brief Convenience single-byte write to simulated ESC register.
  void write_register(std::uint16_t offset, std::uint8_t val);

  /// @brief Direct read access to simulated physical RAM (PDO memory).
  void read_physical_ram(std::size_t offset, std::span<byte> buffer) const;

  /// @brief Direct write access to simulated physical RAM (PDO memory).
  void write_physical_ram(std::size_t offset, std::span<const byte> buffer);

  /**
   * @brief Direct read access to simulated EEPROM word buffer.
   * @param word_addr Word offset.
   * @return 16-bit word.
   */
  [[nodiscard]] std::uint16_t read_eeprom_word(std::size_t word_addr) const;

  /// @brief Direct write access to simulated EEPROM word buffer.
  void write_eeprom_word(std::size_t word_addr, std::uint16_t value);

protected:
  /// @brief Construct a SimulatedSlave instance.
  explicit SimulatedSlave(const std::string &name);

  /**
   * @brief Initialize default ESC registers and NamedObject hierarchy.
   * @param vendor_id EtherCAT Vendor ID.
   * @param product_code EtherCAT Product Code.
   * @param revision_no Revision number.
   * @param serial_no Serial number.
   * @param parent Optional parent node.
   */
  void initialize(std::uint32_t vendor_id, std::uint32_t product_code,
                  std::uint32_t revision_no, std::uint32_t serial_no,
                  std::shared_ptr<quasar::named::NamedObject> parent);

private:
  mutable std::timed_mutex m_mutex; ///< Mutex protecting ESC state

  std::string m_name; ///< Slave name
  std::uint32_t m_vendorId = 0;       ///< Vendor ID
  std::uint32_t m_productCode = 0;    ///< Product Code
  std::uint32_t m_revisionNo = 0;     ///< Revision number
  std::uint32_t m_serialNo = 0;       ///< Serial number

  std::array<byte, SIMULATED_ESC_RAM_SIZE> m_escRam{};      ///< 4096-byte ESC register RAM
  std::vector<std::uint16_t> m_eeprom{};                    ///< Simulated SII EEPROM
  std::array<byte, SIMULATED_ESC_RAM_SIZE> m_physicalRam{}; ///< Physical memory (PDOs, Mailboxes)

  // NamedObject hierarchy
  std::shared_ptr<quasar::named::NamedObject> m_rootNode;      ///< Root slave node
  std::shared_ptr<quasar::named::NamedObject> m_registersNode; ///< Registers subtree
  std::shared_ptr<quasar::named::NamedObject> m_inputsNode;    ///< Inputs subtree (TxPDO)
  std::shared_ptr<quasar::named::NamedObject> m_outputsNode;   ///< Outputs subtree (RxPDO)

  // Registered Named primitives under Registers/
  std::shared_ptr<quasar::named::NamedInteger<std::uint16_t>> m_namedAlStatus;
  std::shared_ptr<quasar::named::NamedInteger<std::uint16_t>> m_namedAlControl;
  std::shared_ptr<quasar::named::NamedInteger<std::uint16_t>> m_namedConfiguredAddress;
  std::shared_ptr<quasar::named::NamedInteger<std::uint16_t>> m_namedStationAlias;
  std::shared_ptr<quasar::named::NamedInteger<std::uint16_t>> m_namedDlStatus;
  std::shared_ptr<quasar::named::NamedInteger<std::uint8_t>> m_namedLinkStatus;

  // Tracked PDO variables
  std::vector<SimulatedPdoVariable> m_variables;

  /// @brief Dispatch write to ESC register space.
  void handle_esc_write(std::uint16_t offset, std::span<const byte> data);
  /// @brief Handle EEPROM control register writes.
  void handle_eeprom_control_write(std::uint16_t control_word);
  /// @brief Handle AL Control register state machine transitions.
  void handle_al_control_write(std::uint16_t control_word);
  /// @brief Synchronize simulated ESC registers into Named tree nodes.
  void sync_registers_to_named();
  /// @brief Populate default SII EEPROM words.
  void init_default_eeprom();
};

} // namespace resoem::virtual_bus
