/**
 * @file SimulatedSlave.cpp
 * @brief Implementation of software EtherCAT Slave Controller emulation.
 * @details Contributes to [TSK-20260920-001.12].
 */

#include "resoem/virtual/SimulatedSlave.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <stdexcept>

namespace resoem::virtual_bus {

namespace {

constexpr std::size_t MAX_LOOP_LIMIT = 2048;

} // anonymous namespace

std::shared_ptr<SimulatedSlave> SimulatedSlave::create(
    const std::string &name,
    std::uint32_t vendor_id,
    std::uint32_t product_code,
    std::uint32_t revision_no,
    std::uint32_t serial_no,
    std::shared_ptr<quasar::named::NamedObject> parent) {
  struct MakeSharedEnabler : public SimulatedSlave {
    /// @brief Construct enabled SimulatedSlave instance.
    explicit MakeSharedEnabler(const std::string &n) : SimulatedSlave(n) {}
  };
  std::shared_ptr<SimulatedSlave> slave = std::make_shared<MakeSharedEnabler>(name);
  slave->initialize(vendor_id, product_code, revision_no, serial_no, parent);
  return slave;
}

std::shared_ptr<SimulatedSlave> SimulatedSlave::create_from_eni(
    const resoem::eni::EniSlave &slave_cfg,
    std::shared_ptr<quasar::named::NamedObject> parent) {
  std::shared_ptr<SimulatedSlave> slave = create(
      slave_cfg.name,
      slave_cfg.vendor_id,
      slave_cfg.product_code,
      slave_cfg.revision_no,
      slave_cfg.serial_no,
      parent);

  if (slave_cfg.physics.find('Y') != std::string::npos) {
    // Slave has physical Ethernet ports
  } else {
    // Closed
  }

  // Bind RxPDOs (Outputs)
  for (const resoem::eni::EniPdo &pdo : slave_cfg.rx_pdos) {
    for (const resoem::eni::EniPdoEntry &entry : pdo.entries) {
      std::size_t bit_len = entry.bit_len;
      if (bit_len == 0) {
        bit_len = 8;
      } else {
        // preserve valid bit_len
      }
      slave->add_output_variable(entry.name.empty() ? ("Output_0x" + std::to_string(entry.index)) : entry.name,
                                 0, bit_len, 0);
    }
  }

  // Bind TxPDOs (Inputs)
  for (const resoem::eni::EniPdo &pdo : slave_cfg.tx_pdos) {
    for (const resoem::eni::EniPdoEntry &entry : pdo.entries) {
      std::size_t bit_len = entry.bit_len;
      if (bit_len == 0) {
        bit_len = 8;
      } else {
        // preserve valid bit_len
      }
      slave->add_input_variable(entry.name.empty() ? ("Input_0x" + std::to_string(entry.index)) : entry.name,
                                0, bit_len, 0);
    }
  }

  return slave;
}

SimulatedSlave::SimulatedSlave(const std::string &name)
    : m_name{name} {
  m_eeprom.resize(SIMULATED_EEPROM_WORDS, 0);
}

void SimulatedSlave::initialize(
    std::uint32_t vendor_id,
    std::uint32_t product_code,
    std::uint32_t revision_no,
    std::uint32_t serial_no,
    std::shared_ptr<quasar::named::NamedObject> parent) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(500)};
  if (!lock.owns_lock()) {
    throw std::runtime_error("Failed to acquire mutex in SimulatedSlave::initialize");
  } else {
    // Lock acquired
  }

  m_vendorId = vendor_id;
  m_productCode = product_code;
  m_revisionNo = revision_no;
  m_serialNo = serial_no;

  m_escRam.fill(0);
  m_physicalRam.fill(0);

  // Initialize standard ESC register header
  m_escRam[regs::TYPE] = DEFAULT_ESC_TYPE;
  m_escRam[regs::REV] = DEFAULT_ESC_REVISION;
  std::memcpy(&m_escRam[regs::BUILD], &DEFAULT_ESC_BUILD, sizeof(std::uint16_t));
  m_escRam[regs::FMMUS] = DEFAULT_FMMU_COUNT;
  m_escRam[regs::SM] = DEFAULT_SM_COUNT;
  m_escRam[regs::PORTS] = DEFAULT_PORT_COUNT;
  std::memcpy(&m_escRam[regs::ESC_FEATURES], &DEFAULT_ESC_FEATURES, sizeof(std::uint16_t));

  // Initial AL status = INIT
  std::uint16_t init_state = states::INIT;
  std::memcpy(&m_escRam[regs::AL_STATUS], &init_state, sizeof(std::uint16_t));

  // Initial DL status: Port 0 and Port 1 links open with loop closed
  std::uint16_t dl_status = DL_STATUS_PORT0_LINK | DL_STATUS_PORT0_LOOP_CLOSED |
                            DL_STATUS_PORT1_LINK | DL_STATUS_PORT1_LOOP_CLOSED |
                            DL_STATUS_COMM_PORT0 | DL_STATUS_COMM_PORT1;
  std::memcpy(&m_escRam[regs::DL_STATUS], &dl_status, sizeof(std::uint16_t));

  // Build Quasar NamedObject hierarchy
  m_rootNode = quasar::named::NamedObject::create(m_name, parent);
  m_registersNode = quasar::named::NamedObject::create("Registers", m_rootNode);
  m_inputsNode = quasar::named::NamedObject::create("Inputs", m_rootNode);
  m_outputsNode = quasar::named::NamedObject::create("Outputs", m_rootNode);

  m_namedAlStatus = quasar::named::NamedInteger<std::uint16_t>::create("AL_Status", states::INIT, m_registersNode);
  m_namedAlControl = quasar::named::NamedInteger<std::uint16_t>::create("AL_Control", 0, m_registersNode);
  m_namedConfiguredAddress = quasar::named::NamedInteger<std::uint16_t>::create("ConfiguredAddress", 0, m_registersNode);
  m_namedStationAlias = quasar::named::NamedInteger<std::uint16_t>::create("StationAlias", 0, m_registersNode);
  m_namedDlStatus = quasar::named::NamedInteger<std::uint16_t>::create("DL_Status", dl_status, m_registersNode);
  m_namedLinkStatus = quasar::named::NamedInteger<std::uint8_t>::create("LinkStatus", 0x03, m_registersNode);

  init_default_eeprom();
}

void SimulatedSlave::init_default_eeprom() {
  m_eeprom.assign(SIMULATED_EEPROM_WORDS, 0);

  // Word 0: PDI Control (0x0080 = On-chip bus / digital I/O)
  m_eeprom[0] = 0x0080;
  // Word 1: PDI Configuration
  m_eeprom[1] = 0x0000;
  // Word 2: Sync impulse length
  m_eeprom[2] = 0x0000;
  // Word 3: Extended PDI
  m_eeprom[3] = 0x0000;
  // Word 4: Configured Station Alias
  m_eeprom[4] = 0x0000;

  // Word 8-9: Vendor ID (32-bit Little Endian)
  m_eeprom[8] = static_cast<std::uint16_t>(m_vendorId & 0xFFFF);
  m_eeprom[9] = static_cast<std::uint16_t>((m_vendorId >> 16) & 0xFFFF);

  // Word 10-11: Product Code (32-bit Little Endian)
  m_eeprom[10] = static_cast<std::uint16_t>(m_productCode & 0xFFFF);
  m_eeprom[11] = static_cast<std::uint16_t>((m_productCode >> 16) & 0xFFFF);

  // Word 12-13: Revision Number
  m_eeprom[12] = static_cast<std::uint16_t>(m_revisionNo & 0xFFFF);
  m_eeprom[13] = static_cast<std::uint16_t>((m_revisionNo >> 16) & 0xFFFF);

  // Word 14-15: Serial Number
  m_eeprom[14] = static_cast<std::uint16_t>(m_serialNo & 0xFFFF);
  m_eeprom[15] = static_cast<std::uint16_t>((m_serialNo >> 16) & 0xFFFF);

  // Word 16: Mailbox Out (SM0: standard 0x1000 start, 128 bytes)
  m_eeprom[16] = 0x1000;
  m_eeprom[17] = 0x0080;

  // Word 18: Mailbox In (SM1: standard 0x1080 start, 128 bytes)
  m_eeprom[18] = 0x1080;
  m_eeprom[19] = 0x0080;

  // Word 20: Mailbox protocols (CoE = 0x0002)
  m_eeprom[20] = 0x0002;

  // Word 0x0040 (64): Categories header
  // Category Strings (Type 10, length 8 words)
  m_eeprom[64] = 10;
  m_eeprom[65] = 8;
  m_eeprom[66] = 1; // 1 string
  // Store name string
  std::size_t name_len = std::min(m_name.size(), static_cast<std::size_t>(14));
  m_eeprom[67] = static_cast<std::uint16_t>(name_len);
  std::memcpy(&m_eeprom[68], m_name.data(), name_len);

  // Category General (Type 30, length 12 words)
  std::size_t gen_offset = 64 + 2 + 8;
  m_eeprom[gen_offset] = 30;
  m_eeprom[gen_offset + 1] = 12;
  m_eeprom[gen_offset + 2] = 1; // Group string idx
  m_eeprom[gen_offset + 3] = 0; // Img idx
  m_eeprom[gen_offset + 4] = 1; // Order idx
  m_eeprom[gen_offset + 5] = 1; // Name idx

  // Category End (Type 0xFFFF)
  std::size_t end_offset = gen_offset + 2 + 12;
  m_eeprom[end_offset] = 0xFFFF;
  m_eeprom[end_offset + 1] = 0;
}

std::shared_ptr<quasar::named::NamedObject> SimulatedSlave::get_root_node() const noexcept {
  return m_rootNode;
}

std::uint16_t SimulatedSlave::get_configured_address() const noexcept {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return 0;
  } else {
    std::uint16_t addr = 0;
    std::memcpy(&addr, &m_escRam[regs::CONFIG_STATION_ADDR], sizeof(std::uint16_t));
    return addr;
  }
}

void SimulatedSlave::set_configured_address(std::uint16_t address) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock()) {
    std::memcpy(&m_escRam[regs::CONFIG_STATION_ADDR], &address, sizeof(std::uint16_t));
    if (m_namedConfiguredAddress != nullptr) {
      m_namedConfiguredAddress->setValue(address);
    } else {
      // no named node
    }
  } else {
    // Lock failed
  }
}

std::uint16_t SimulatedSlave::get_station_alias() const noexcept {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return 0;
  } else {
    std::uint16_t alias = 0;
    std::memcpy(&alias, &m_escRam[regs::CONFIG_STATION_ALIAS], sizeof(std::uint16_t));
    return alias;
  }
}

void SimulatedSlave::set_station_alias(std::uint16_t alias) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock()) {
    std::memcpy(&m_escRam[regs::CONFIG_STATION_ALIAS], &alias, sizeof(std::uint16_t));
    m_eeprom[4] = alias;
    if (m_namedStationAlias != nullptr) {
      m_namedStationAlias->setValue(alias);
    } else {
      // no named node
    }
  } else {
    // Lock failed
  }
}

std::uint16_t SimulatedSlave::get_al_state() const noexcept {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return states::INIT;
  } else {
    std::uint16_t st = 0;
    std::memcpy(&st, &m_escRam[regs::AL_STATUS], sizeof(std::uint16_t));
    return st & resoem::regs::al_status::STATE_MASK;
  }
}

void SimulatedSlave::set_al_state(std::uint16_t state) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock()) {
    std::memcpy(&m_escRam[regs::AL_STATUS], &state, sizeof(std::uint16_t));
    if (m_namedAlStatus != nullptr) {
      m_namedAlStatus->setValue(state);
    } else {
      // no named node
    }
  } else {
    // Lock failed
  }
}

void SimulatedSlave::add_input_variable(const std::string &name, std::size_t bit_offset,
                                       std::size_t bit_length, std::int64_t initial_value) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock()) {
    std::shared_ptr<quasar::named::NamedInteger<std::int64_t>> node =
        quasar::named::NamedInteger<std::int64_t>::create(name, initial_value, m_inputsNode);

    SimulatedPdoVariable var;
    var.name = name;
    var.bit_offset = bit_offset;
    var.bit_length = bit_length;
    var.is_input = true;
    var.named_node = node;
    m_variables.push_back(var);
  } else {
    // Lock failed
  }
}

void SimulatedSlave::add_output_variable(const std::string &name, std::size_t bit_offset,
                                        std::size_t bit_length, std::int64_t initial_value) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock()) {
    std::shared_ptr<quasar::named::NamedInteger<std::int64_t>> node =
        quasar::named::NamedInteger<std::int64_t>::create(name, initial_value, m_outputsNode);

    SimulatedPdoVariable var;
    var.name = name;
    var.bit_offset = bit_offset;
    var.bit_length = bit_length;
    var.is_input = false;
    var.named_node = node;
    m_variables.push_back(var);
  } else {
    // Lock failed
  }
}

std::int64_t SimulatedSlave::get_input_value(const std::string &name) const {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return 0;
  } else {
    for (const SimulatedPdoVariable &var : m_variables) {
      if (var.is_input && var.name == name && var.named_node != nullptr) {
        return var.named_node->value();
      } else {
        // continue search
      }
    }
    return 0;
  }
}

void SimulatedSlave::set_input_value(const std::string &name, std::int64_t value) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock()) {
    for (SimulatedPdoVariable &var : m_variables) {
      if (var.is_input && var.name == name && var.named_node != nullptr) {
        var.named_node->setValue(value);
        break;
      } else {
        // continue search
      }
    }
  } else {
    // Lock failed
  }
}

std::int64_t SimulatedSlave::get_output_value(const std::string &name) const {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return 0;
  } else {
    for (const SimulatedPdoVariable &var : m_variables) {
      if (!var.is_input && var.name == name && var.named_node != nullptr) {
        return var.named_node->value();
      } else {
        // continue search
      }
    }
    return 0;
  }
}

void SimulatedSlave::process_aprd(std::uint16_t &addr, std::uint16_t offset,
                                  std::span<byte> data, std::uint16_t &wkc) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return;
  } else {
    if (addr == 0) {
      // Target slave matched
      if (offset + data.size() <= SIMULATED_ESC_RAM_SIZE) {
        std::memcpy(data.data(), &m_escRam[offset], data.size());
        wkc += WKC_INC_READ;
      } else {
        // Out of bounds read
      }
      addr += 1;
    } else {
      // Forward downstream: increment address
      addr += 1;
    }
  }
}

void SimulatedSlave::process_apwr(std::uint16_t &addr, std::uint16_t offset,
                                  std::span<const byte> data, std::uint16_t &wkc) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return;
  } else {
    if (addr == 0) {
      // Target slave matched
      if (offset + data.size() <= SIMULATED_ESC_RAM_SIZE) {
        std::memcpy(&m_escRam[offset], data.data(), data.size());
        handle_esc_write(offset, data);
        wkc += WKC_INC_WRITE;
      } else {
        // Out of bounds write
      }
      addr += 1;
    } else {
      // Forward downstream: increment address
      addr += 1;
    }
  }
}

void SimulatedSlave::process_fprd(std::uint16_t addr, std::uint16_t offset,
                                  std::span<byte> data, std::uint16_t &wkc) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return;
  } else {
    std::uint16_t cfg_addr = 0;
    std::memcpy(&cfg_addr, &m_escRam[regs::CONFIG_STATION_ADDR], sizeof(std::uint16_t));
    if (addr == cfg_addr && cfg_addr != 0) {
      if (offset + data.size() <= SIMULATED_ESC_RAM_SIZE) {
        std::memcpy(data.data(), &m_escRam[offset], data.size());
        wkc += WKC_INC_READ;
      } else {
        // Out of bounds
      }
    } else {
      // Address mismatch
    }
  }
}

void SimulatedSlave::process_fpwr(std::uint16_t addr, std::uint16_t offset,
                                  std::span<const byte> data, std::uint16_t &wkc) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return;
  } else {
    std::uint16_t cfg_addr = 0;
    std::memcpy(&cfg_addr, &m_escRam[regs::CONFIG_STATION_ADDR], sizeof(std::uint16_t));
    if (addr == cfg_addr && cfg_addr != 0) {
      if (offset + data.size() <= SIMULATED_ESC_RAM_SIZE) {
        std::memcpy(&m_escRam[offset], data.data(), data.size());
        handle_esc_write(offset, data);
        wkc += WKC_INC_WRITE;
      } else {
        // Out of bounds
      }
    } else {
      // Address mismatch
    }
  }
}

void SimulatedSlave::process_brd(std::uint16_t offset, std::span<byte> data, std::uint16_t &wkc) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return;
  } else {
    if (offset + data.size() <= SIMULATED_ESC_RAM_SIZE) {
      // Broadcast read: logical OR with existing data in frame
      for (std::size_t i = 0; i < data.size() && i < MAX_LOOP_LIMIT; ++i) {
        data[i] = static_cast<byte>(data[i] | m_escRam[offset + i]);
      }
      wkc += WKC_INC_READ;
    } else {
      // Out of bounds
    }
  }
}

void SimulatedSlave::process_bwr(std::uint16_t offset, std::span<const byte> data, std::uint16_t &wkc) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return;
  } else {
    if (offset + data.size() <= SIMULATED_ESC_RAM_SIZE) {
      std::memcpy(&m_escRam[offset], data.data(), data.size());
      handle_esc_write(offset, data);
      wkc += WKC_INC_WRITE;
    } else {
      // Out of bounds
    }
  }
}

void SimulatedSlave::process_lrw(std::uint32_t logical_addr, std::span<byte> data, std::uint16_t &wkc) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return;
  } else {
    bool had_read = false;
    bool had_write = false;

    // Scan configured FMMUs (FMMU 0..7)
    for (std::size_t i = 0; i < DEFAULT_FMMU_COUNT && i < MAX_LOOP_LIMIT; ++i) {
      std::size_t fmmu_base = regs::FMMU0 + (i * 16);
      std::uint8_t activate = m_escRam[fmmu_base + 0x0C];
      if ((activate & 0x01) == 0) {
        continue;
      } else {
        // FMMU is active
      }

      std::uint32_t fmmu_log_start = 0;
      std::memcpy(&fmmu_log_start, &m_escRam[fmmu_base + 0x00], sizeof(std::uint32_t));
      std::uint16_t fmmu_len = 0;
      std::memcpy(&fmmu_len, &m_escRam[fmmu_base + 0x04], sizeof(std::uint16_t));
      std::uint16_t fmmu_phys_start = 0;
      std::memcpy(&fmmu_phys_start, &m_escRam[fmmu_base + 0x08], sizeof(std::uint16_t));
      std::uint8_t fmmu_type = m_escRam[fmmu_base + 0x0B];

      std::uint32_t fmmu_log_end = fmmu_log_start + fmmu_len;
      std::uint32_t dgram_end = logical_addr + static_cast<std::uint32_t>(data.size());

      if (logical_addr < fmmu_log_end && dgram_end > fmmu_log_start) {
        std::uint32_t overlap_start = std::max(logical_addr, fmmu_log_start);
        std::uint32_t overlap_end = std::min(dgram_end, fmmu_log_end);
        std::size_t overlap_len = overlap_end - overlap_start;

        std::size_t dgram_offset = overlap_start - logical_addr;
        std::size_t phys_offset = fmmu_phys_start + (overlap_start - fmmu_log_start);

        if ((fmmu_type & 0x01) != 0) {
          // Read direction (TxPDO: Slave -> Master)
          if (phys_offset + overlap_len <= SIMULATED_ESC_RAM_SIZE) {
            std::memcpy(data.data() + dgram_offset, &m_physicalRam[phys_offset], overlap_len);
            had_read = true;
          } else {
            // Buffer overflow prevented
          }
        } else {
          // Not read
        }

        if ((fmmu_type & 0x02) != 0) {
          // Write direction (RxPDO: Master -> Slave)
          if (phys_offset + overlap_len <= SIMULATED_ESC_RAM_SIZE) {
            std::memcpy(&m_physicalRam[phys_offset], data.data() + dgram_offset, overlap_len);
            had_write = true;
          } else {
            // Buffer overflow prevented
          }
        } else {
          // Not write
        }
      } else {
        // No overlap with this FMMU
      }
    }

    if ((had_read != false) && (had_write != false)) {
      wkc += WKC_INC_READWRITE;
    } else if (had_read != false) {
      wkc += WKC_INC_READ;
    } else if (had_write != false) {
      wkc += WKC_INC_WRITE;
    } else {
      // Neither
    }

    if (had_write != false) {
      sync_outputs_to_named();
    } else {
      // No outputs updated
    }
  }
}

void SimulatedSlave::process_lrd(std::uint32_t logical_addr, std::span<byte> data, std::uint16_t &wkc) {
  process_lrw(logical_addr, data, wkc);
}

void SimulatedSlave::process_lwr(std::uint32_t logical_addr, std::span<const byte> data, std::uint16_t &wkc) {
  std::vector<byte> non_const_copy{data.begin(), data.end()};
  process_lrw(logical_addr, std::span<byte>(non_const_copy.data(), non_const_copy.size()), wkc);
}

void SimulatedSlave::handle_esc_write(std::uint16_t offset, std::span<const byte> data) {
  // Check if Station Address was written (0x0010)
  if (offset <= regs::CONFIG_STATION_ADDR && (offset + data.size()) >= (regs::CONFIG_STATION_ADDR + 2)) {
    std::uint16_t addr = 0;
    std::memcpy(&addr, &m_escRam[regs::CONFIG_STATION_ADDR], sizeof(std::uint16_t));
    if (m_namedConfiguredAddress != nullptr) {
      m_namedConfiguredAddress->setValue(addr);
    } else {
      // No node
    }
  } else {
    // Other reg
  }

  // Check if Station Alias was written (0x0012)
  if (offset <= regs::CONFIG_STATION_ALIAS && (offset + data.size()) >= (regs::CONFIG_STATION_ALIAS + 2)) {
    std::uint16_t alias = 0;
    std::memcpy(&alias, &m_escRam[regs::CONFIG_STATION_ALIAS], sizeof(std::uint16_t));
    if (m_namedStationAlias != nullptr) {
      m_namedStationAlias->setValue(alias);
    } else {
      // No node
    }
  } else {
    // Other reg
  }

  // Check if AL Control was written (0x0120)
  if (offset <= regs::AL_CONTROL && (offset + data.size()) >= (regs::AL_CONTROL + 2)) {
    std::uint16_t al_ctrl = 0;
    std::memcpy(&al_ctrl, &m_escRam[regs::AL_CONTROL], sizeof(std::uint16_t));
    handle_al_control_write(al_ctrl);
  } else {
    // Other reg
  }

  // Check if EEPROM Control was written (0x0502)
  if (offset <= regs::EEPROM_CONTROL && (offset + data.size()) >= (regs::EEPROM_CONTROL + 2)) {
    std::uint16_t eep_ctrl = 0;
    std::memcpy(&eep_ctrl, &m_escRam[regs::EEPROM_CONTROL], sizeof(std::uint16_t));
    handle_eeprom_control_write(eep_ctrl);
  } else {
    // Other reg
  }
}

void SimulatedSlave::handle_eeprom_control_write(std::uint16_t control_word) {
  std::uint16_t cmd = control_word & 0x0700;
  if (cmd == eeprom::CMD_READ) {
    std::uint16_t word_addr = 0;
    std::memcpy(&word_addr, &m_escRam[regs::EEPROM_ADDRESS], sizeof(std::uint16_t));
    if (word_addr < m_eeprom.size()) {
      std::uint32_t val = m_eeprom[word_addr];
      if (static_cast<std::size_t>(word_addr) + 1 < m_eeprom.size()) {
        val |= (static_cast<std::uint32_t>(m_eeprom[word_addr + 1]) << 16);
      } else {
        // Single word
      }
      std::memcpy(&m_escRam[regs::EEPROM_DATA], &val, sizeof(std::uint32_t));
    } else {
      // Out of bounds read
      std::uint32_t ff = 0xFFFFFFFF;
      std::memcpy(&m_escRam[regs::EEPROM_DATA], &ff, sizeof(std::uint32_t));
    }
    // Clear BUSY bit
    std::uint16_t status = 0x0000;
    std::memcpy(&m_escRam[regs::EEPROM_CONTROL], &status, sizeof(std::uint16_t));
  } else if (cmd == eeprom::CMD_WRITE) {
    std::uint16_t word_addr = 0;
    std::memcpy(&word_addr, &m_escRam[regs::EEPROM_ADDRESS], sizeof(std::uint16_t));
    if (word_addr < m_eeprom.size()) {
      std::uint16_t val = 0;
      std::memcpy(&val, &m_escRam[regs::EEPROM_DATA], sizeof(std::uint16_t));
      m_eeprom[word_addr] = val;
    } else {
      // Out of bounds write
    }
    // Clear BUSY bit
    std::uint16_t status = 0x0000;
    std::memcpy(&m_escRam[regs::EEPROM_CONTROL], &status, sizeof(std::uint16_t));
  } else if (cmd == eeprom::CMD_RELOAD) {
    init_default_eeprom();
    std::uint16_t status = 0x0000;
    std::memcpy(&m_escRam[regs::EEPROM_CONTROL], &status, sizeof(std::uint16_t));
  } else {
    // NOP or unknown command
  }
}

void SimulatedSlave::handle_al_control_write(std::uint16_t control_word) {
  if (m_namedAlControl != nullptr) {
    m_namedAlControl->setValue(control_word);
  } else {
    // No node
  }

  std::uint16_t req_state = control_word & resoem::regs::al_status::STATE_MASK;
  bool ack_bit = (control_word & states::ACK) != 0;

  if (ack_bit != false) {
    // Clear error bit in AL Status and clear AL Status Code
    std::uint16_t current_al = 0;
    std::memcpy(&current_al, &m_escRam[regs::AL_STATUS], sizeof(std::uint16_t));
    current_al &= ~resoem::regs::al_status::ERROR_BIT;
    std::memcpy(&m_escRam[regs::AL_STATUS], &current_al, sizeof(std::uint16_t));
    std::uint16_t zero = 0;
    std::memcpy(&m_escRam[regs::AL_STATUS_CODE], &zero, sizeof(std::uint16_t));
  } else {
    // Normal request
  }

  if (req_state == states::INIT || req_state == states::PRE_OP ||
      req_state == states::BOOT || req_state == states::SAFE_OP ||
      req_state == states::OP) {
    std::memcpy(&m_escRam[regs::AL_STATUS], &req_state, sizeof(std::uint16_t));
    if (m_namedAlStatus != nullptr) {
      m_namedAlStatus->setValue(req_state);
    } else {
      // No node
    }
  } else {
    // Invalid state requested
  }
}

void SimulatedSlave::sync_inputs_from_named() {
  for (const SimulatedPdoVariable &var : m_variables) {
    if (var.is_input && var.named_node != nullptr) {
      std::int64_t val = var.named_node->value();
      std::size_t byte_idx = var.bit_offset / 8;
      std::size_t bytes_to_copy = std::min((var.bit_length + 7) / 8, sizeof(std::int64_t));
      if (byte_idx + bytes_to_copy <= SIMULATED_ESC_RAM_SIZE) {
        std::memcpy(&m_physicalRam[byte_idx], &val, bytes_to_copy);
      } else {
        // Prevent overflow
      }
    } else {
      // Skip outputs
    }
  }
}

void SimulatedSlave::sync_outputs_to_named() {
  for (const SimulatedPdoVariable &var : m_variables) {
    if (!var.is_input && var.named_node != nullptr) {
      std::size_t byte_idx = var.bit_offset / 8;
      std::size_t bytes_to_copy = std::min((var.bit_length + 7) / 8, sizeof(std::int64_t));
      if (byte_idx + bytes_to_copy <= SIMULATED_ESC_RAM_SIZE) {
        std::int64_t val = 0;
        std::memcpy(&val, &m_physicalRam[byte_idx], bytes_to_copy);
        var.named_node->setValue(val);
      } else {
        // Prevent overflow
      }
    } else {
      // Skip inputs
    }
  }
}

void SimulatedSlave::read_esc_ram(std::size_t offset, std::span<byte> buffer) const {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock() && offset + buffer.size() <= SIMULATED_ESC_RAM_SIZE) {
    std::memcpy(buffer.data(), &m_escRam[offset], buffer.size());
  } else {
    // Lock failed or out of bounds
  }
}

void SimulatedSlave::write_esc_ram(std::size_t offset, std::span<const byte> buffer) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock() && offset + buffer.size() <= SIMULATED_ESC_RAM_SIZE) {
    std::memcpy(&m_escRam[offset], buffer.data(), buffer.size());
  } else {
    // Lock failed or out of bounds
  }
}

void SimulatedSlave::write_register(std::uint16_t offset, std::uint8_t val) {
  byte b = static_cast<byte>(val);
  std::array<byte, 1> arr{b};
  write_esc_ram(offset, arr);
}

void SimulatedSlave::read_physical_ram(std::size_t offset, std::span<byte> buffer) const {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock() && offset + buffer.size() <= SIMULATED_ESC_RAM_SIZE) {
    std::memcpy(buffer.data(), &m_physicalRam[offset], buffer.size());
  } else {
    // Lock failed or out of bounds
  }
}

void SimulatedSlave::write_physical_ram(std::size_t offset, std::span<const byte> buffer) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock() && offset + buffer.size() <= SIMULATED_ESC_RAM_SIZE) {
    std::memcpy(&m_physicalRam[offset], buffer.data(), buffer.size());
  } else {
    // Lock failed or out of bounds
  }
}

std::uint16_t SimulatedSlave::read_eeprom_word(std::size_t word_addr) const {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock() && word_addr < m_eeprom.size()) {
    return m_eeprom[word_addr];
  } else {
    return 0xFFFF;
  }
}

void SimulatedSlave::write_eeprom_word(std::size_t word_addr, std::uint16_t value) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (lock.owns_lock() && word_addr < m_eeprom.size()) {
    m_eeprom[word_addr] = value;
  } else {
    // Out of bounds
  }
}

} // namespace resoem::virtual_bus
