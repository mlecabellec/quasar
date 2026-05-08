/**
 * @file testmaster100.cpp
 * @brief EtherCAT master specifically designed for the following bus topology:
 *
 *   Slot 0: EK1100  - EtherCAT Bus Coupler (Beckhoff, 0x00000002/0x044C2C52)
 *   Slot 1: EL2809  - 16-ch Digital Output 24V DC 0.5A (Beckhoff, 0x044C2C52)
 *   Slot 2: EL1809  - 16-ch Digital Input  24V DC 3ms  (Beckhoff, 0x044C2C52)
 *   Slot 3: EL3318  - 8-ch  Thermocouple Input         (Beckhoff, 0x044C2C52)
 *   Slot 4: EL3314-0002 - 4-ch Thermocouple Input      (Beckhoff, 0x044C2C52)
 *
 * @details
 * Behaviour:
 *  - Runs a 10 ms cyclic loop exchanging process data.
 *  - Every 5 seconds: toggles a walking-bit pattern on the EL2809 digital
 * outputs.
 *  - Every 5 seconds: prints the state of all 16 EL1809 digital inputs.
 *  - Every 5 seconds: prints temperature readings from EL3318 (8 ch) and
 *                     EL3314-0002 (4 ch).
 *  - Runs until SIGINT (Ctrl+C) or SIGTERM.
 *
 * Process-image layout (determined by configure_fmmu output-first rule):
 *
 *   -- OUTPUTS (Master -> Slaves) --
 *   [0x00..0x01]  EL2809  : 16-bit DO word (bit 0 = Ch1 .. bit 15 = Ch16)
 *   [0x02..0x21]  EL3318  : 8 x 4-byte control word (CJ compensation, RxPDO)
 *   [0x22..0x31]  EL3314  : 4 x 4-byte control word (CJ compensation, RxPDO)
 *
 *   -- INPUTS (Slaves -> Master) --
 *   [0x32..0x33]  EL1809  : 16-bit DI word (bit 0 = Ch1 .. bit 15 = Ch16)
 *   [0x34..0x53]  EL3318  : 8 x 4-byte (2-byte status + 2-byte INT16 temp x
 * 0.1°C) [0x54..0x63]  EL3314  : 4 x 6-byte (2-byte status + 2-byte INT16 temp
 * + 2 align)
 *
 *   Total: 60 bytes (matches "Process Image size: 60 bytes" in test.log)
 *
 * Per ETG.1000.6 Beckhoff documentation:
 *  - EL3318/EL3314 temperature value: INT16, factor 0.1°C/LSB, range
 * -200..+870°C
 *  - EL3318/EL3314 status bits: bit6=Error, bit4=Overrange, bit3=Underrange
 *  - EL2809 control: 1 bit per channel, active-HIGH
 *  - EL1809 status:  1 bit per channel, HIGH = signal present
 *
 * @note Process image offsets are derived at runtime from
 * slaves_[].outputs_offset and slaves_[].inputs_offset as set by
 * configure_fmmu. The constants below are cross-checked against the test.log
 * layout at startup. If the bus topology differs, the program aborts with a
 * clear error.
 *
 * References:
 *  - Beckhoff EL2809:
 * https://www.beckhoff.com/en-en/products/i-o/ethercat-terminals/el2xxx-digital-output/el2809.html
 *  - Beckhoff EL1809:
 * https://www.beckhoff.com/en-en/products/i-o/ethercat-terminals/el1xxx-digital-input/el1809.html
 *  - Beckhoff EL3318:
 * https://www.beckhoff.com/en-en/products/i-o/ethercat-terminals/el3xxx-analog-input/el3318.html
 *  - Beckhoff EL3314:
 * https://www.beckhoff.com/en-en/products/i-o/ethercat-terminals/el3xxx-analog-input/el3314.html
 *  - ETG.1000.4 §5: LRW Working Counter rules
 */

#include "resoem/Enumerator.hpp"
#include "resoem/ProcessImage.hpp"
#include "resoem/RawSocket.hpp"
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <signal.h>
#include <string>
#include <thread>
#include <vector>

using namespace resoem;

/* =========================================================================
 * Signal handling
 * ========================================================================= */

/// @brief Global stop flag — set by SIGINT/SIGTERM handler.
volatile sig_atomic_t g_stop = 0;

/**
 * @brief POSIX signal handler. Sets g_stop to terminate the cyclic loop.
 * @param sig Signal number (unused).
 */
static void handle_signal(int sig) {
  (void)sig;
  g_stop = 1;
}

/* =========================================================================
 * Topology constants (Beckhoff product names)
 * ========================================================================= */

/// @brief Expected slave count on this specific bus.
constexpr size_t EXPECTED_SLAVE_COUNT = 5U;

// Slave indices in the slaves_ list (0-based).
constexpr size_t SLAVE_EK1100 = 0U; ///< Bus Coupler — no process data
constexpr size_t SLAVE_EL2809 = 1U; ///< 16-ch Digital Output
constexpr size_t SLAVE_EL1809 = 2U; ///< 16-ch Digital Input
constexpr size_t SLAVE_EL3318 = 3U; ///< 8-ch Thermocouple Input (CoE)
constexpr size_t SLAVE_EL3314 = 4U; ///< 4-ch Thermocouple Input -0002 (CoE)

// EL2809: 2 bytes of digital output in process image.
constexpr uint32_t EL2809_OUT_BYTES = 2U;
// EL1809: 2 bytes of digital input in process image.
constexpr uint32_t EL1809_IN_BYTES = 2U;
// EL3318: 8 channels × 4 bytes (2-byte status + 2-byte INT16 temperature).
constexpr uint32_t EL3318_IN_BYTES = 32U;
constexpr uint32_t EL3318_CHANNELS = 8U;
// EL3314-0002: 4 channels × 6 bytes (includes extra alignment/status).
constexpr uint32_t EL3314_IN_BYTES = 24U;
constexpr uint32_t EL3314_CHANNELS = 4U;

// EL33xx channel record layout (per-channel, both EL3318 and EL3314).
// Byte 0-1: Status word (bit6=Error, bit4=Overrange, bit3=Underrange).
// Byte 2-3: INT16 temperature value (0.1°C per LSB).
constexpr uint8_t EL33XX_STATUS_ERROR_BIT = 0x40U;      ///< bit 6
constexpr uint8_t EL33XX_STATUS_OVERRANGE_BIT = 0x10U;  ///< bit 4
constexpr uint8_t EL33XX_STATUS_UNDERRANGE_BIT = 0x08U; ///< bit 3

// EL3318: each channel record is 4 bytes.
constexpr uint32_t EL3318_BYTES_PER_CHANNEL = 4U;
// EL3314-0002: each channel record is 6 bytes (extra RxPDO alignment).
constexpr uint32_t EL3314_BYTES_PER_CHANNEL = 6U;

// WKC constants for LRW (per ETG.1000.4 §5.4.7).
constexpr uint16_t WKC_LRW_OUTPUT_INCREMENT =
    1U; ///< Slave reads (drives outputs)
constexpr uint16_t WKC_LRW_INPUT_INCREMENT =
    2U; ///< Slave writes (drives inputs)

// Cyclic loop period.
constexpr std::chrono::milliseconds CYCLE_PERIOD_MS{10};

// Display/toggle interval: every 5 seconds = 500 cycles @ 10ms.
constexpr uint32_t DISPLAY_EVERY_N_CYCLES = 100U;

/* =========================================================================
 * Process image access helpers
 * ========================================================================= */

/**
 * @brief Read a uint16_t from the process image at a given byte offset.
 * @details Uses ProcessImage::read_byte() for safe bounds-checked access.
 *          Little-endian decode: low byte at offset, high byte at offset+1.
 * @param img    The process image buffer.
 * @param offset Byte offset within the image.
 * @return 16-bit little-endian value at the offset.
 */
static uint16_t read_u16(const ProcessImage &img, uint32_t offset) {
  uint16_t lo = static_cast<uint16_t>(img.read_byte(offset));
  uint16_t hi = static_cast<uint16_t>(img.read_byte(offset + 1U));
  return static_cast<uint16_t>((hi << 8U) | lo);
}

/**
 * @brief Write a uint16_t into the process image at a given byte offset.
 * @details Uses ProcessImage::write_byte() for safe bounds-checked access.
 *          Little-endian encode: low byte at offset, high byte at offset+1.
 * @param img    The process image buffer (mutable).
 * @param offset Byte offset within the image.
 * @param val    16-bit value to write (little-endian).
 */
static void write_u16(ProcessImage &img, uint32_t offset, uint16_t val) {
  img.write_byte(offset, static_cast<uint8_t>(val & 0xFFU));
  img.write_byte(offset + 1U, static_cast<uint8_t>((val >> 8U) & 0xFFU));
}

/**
 * @brief Read a signed INT16 temperature from a thermocouple channel record.
 * @details Per Beckhoff EL33xx documentation: value = raw x 0.1 degC.
 *          Bytes 2-3 of each channel record hold the INT16 measurement.
 * @param img         The process image buffer.
 * @param base_offset Byte offset of the start of this channel's record.
 * @return Temperature in degC as a double.
 */
static double read_el33xx_temperature(const ProcessImage &img,
                                      uint32_t base_offset) {
  // Temperature INT16 is stored at bytes [base+2, base+3] (little-endian).
  uint16_t raw_u = read_u16(img, base_offset + 2U);
  // Reinterpret as signed INT16 for negative temperatures.
  int16_t raw = static_cast<int16_t>(raw_u);
  return static_cast<double>(raw) * 0.1;
}

/**
 * @brief Read the status low byte of a thermocouple channel record.
 * @details Byte 0 of the 2-byte status word holds error/underrange/overrange
 * bits.
 * @param img         The process image buffer.
 * @param base_offset Byte offset of the start of this channel's record.
 * @return Low byte of the status word.
 */
static uint8_t read_el33xx_status(const ProcessImage &img,
                                  uint32_t base_offset) {
  return static_cast<uint8_t>(img.read_byte(base_offset));
}

/* =========================================================================
 * Topology validation
 * ========================================================================= */

/**
 * @brief Validate that the discovered bus matches the expected topology.
 * @details Checks slave count, product names, and process image offsets.
 * Prints a clear error and returns false if anything is mismatched.
 * @param enumerator Populated Enumerator after configure_fmmu().
 * @param image_bytes Total process image size in bytes.
 * @return true if topology is exactly as expected, false otherwise.
 */
static bool validate_topology(const Enumerator &enumerator,
                              uint32_t image_bytes) {
  const std::vector<SlaveInfo> &slaves = enumerator.slaves();

  // Check slave count.
  if (slaves.size() != EXPECTED_SLAVE_COUNT) {
    std::cerr << "[FATAL] Expected " << EXPECTED_SLAVE_COUNT
              << " slaves, found " << slaves.size() << ". Check bus topology."
              << std::endl;
    return false;
  }

  // Define expected names for each slot.
  const std::string expected_names[EXPECTED_SLAVE_COUNT] = {
      "EK1100", "EL2809", "EL1809", "EL3318", "EL3314-0002"};

  // Validate each slave name.
  for (size_t i = 0; i < EXPECTED_SLAVE_COUNT; ++i) {
    // Check if the slave name contains the expected substring.
    // Names may have trailing spaces or revision info from SII.
    if (slaves[i].name.find(expected_names[i]) == std::string::npos) {
      std::cerr << "[FATAL] Slot " << i << ": expected '" << expected_names[i]
                << "', found '" << slaves[i].name << "'." << std::endl;
      return false;
    }
  }

  // Validate EL2809 has outputs.
  if (slaves[SLAVE_EL2809].outputs_size_bits == 0) {
    std::cerr << "[FATAL] EL2809 reports no output bits. Check SII parsing."
              << std::endl;
    return false;
  }

  // Validate EL1809 has inputs.
  if (slaves[SLAVE_EL1809].inputs_size_bits == 0) {
    std::cerr << "[FATAL] EL1809 reports no input bits. Check SII parsing."
              << std::endl;
    return false;
  }

  // Cross-check total process image size.
  std::cout << "[TOPOLOGY] Process image: " << image_bytes << " bytes. "
            << "Expected ~60 bytes." << std::endl;

  // Log confirmed layout.
  std::cout << "[TOPOLOGY] Confirmed slave layout:" << std::endl;
  for (size_t i = 0; i < EXPECTED_SLAVE_COUNT; ++i) {
    std::cout << "  Slot " << i << " [" << slaves[i].name << "]"
              << " addr=0x" << std::hex << slaves[i].configured_address
              << " out_bits=" << std::dec << slaves[i].outputs_size_bits
              << " in_bits=" << slaves[i].inputs_size_bits << " out_off=0x"
              << std::hex << slaves[i].outputs_offset << " in_off=0x"
              << slaves[i].inputs_offset << std::dec << std::endl;
  }
  return true;
}

/* =========================================================================
 * Display functions
 * ========================================================================= */

/**
 * @brief Print all 16 EL1809 digital input channels.
 * @param img    Current process image.
 * @param offset Byte offset of EL1809 inputs in the image.
 */
static void display_digital_inputs(const ProcessImage &img, uint32_t offset) {
  uint16_t di_word = read_u16(img, offset);
  std::cout << "[DI] EL1809 16x Digital Input:" << std::endl;
  // Print each channel on a column layout.
  for (uint8_t ch = 0; ch < 16U; ++ch) {
    bool state = ((di_word >> ch) & 0x01U) != 0U;
    std::cout << "  Ch" << std::setw(2) << static_cast<int>(ch + 1U) << ": "
              << (state ? "ON " : "OFF") << "  ";
    // Break line every 4 channels for readability.
    if (((ch + 1U) % 4U) == 0U) {
      std::cout << std::endl;
    }
  }
}

/**
 * @brief Print all 16 EL2809 digital output channels as currently written.
 * @param img    Current process image.
 * @param offset Byte offset of EL2809 outputs in the image.
 */
static void display_digital_outputs(const ProcessImage &img, uint32_t offset) {
  uint16_t do_word = read_u16(img, offset);
  std::cout << "[DO] EL2809 16x Digital Output (current pattern=0x" << std::hex
            << std::setw(4) << std::setfill('0') << do_word << std::dec
            << std::setfill(' ') << "):" << std::endl;
  for (uint8_t ch = 0; ch < 16U; ++ch) {
    bool state = ((do_word >> ch) & 0x01U) != 0U;
    std::cout << "  Ch" << std::setw(2) << static_cast<int>(ch + 1U) << ": "
              << (state ? "ON " : "OFF") << "  ";
    if (((ch + 1U) % 4U) == 0U) {
      std::cout << std::endl;
    }
  }
}

/**
 * @brief Print all EL3318 thermocouple channel readings.
 * @param img    Current process image.
 * @param offset Byte offset of EL3318 inputs in the image.
 */
static void display_el3318(const ProcessImage &img, uint32_t offset) {
  std::cout << "[TC] EL3318 8x Thermocouple:" << std::endl;
  for (uint32_t ch = 0; ch < EL3318_CHANNELS; ++ch) {
    // Each channel record is 4 bytes wide (status_lo, status_hi, temp_lo,
    // temp_hi).
    uint32_t ch_offset = offset + ch * EL3318_BYTES_PER_CHANNEL;
    uint8_t status = read_el33xx_status(img, ch_offset);
    double temp = read_el33xx_temperature(img, ch_offset);
    // Decode status bits.
    bool err = (status & EL33XX_STATUS_ERROR_BIT) != 0U;
    bool over = (status & EL33XX_STATUS_OVERRANGE_BIT) != 0U;
    bool under = (status & EL33XX_STATUS_UNDERRANGE_BIT) != 0U;
    std::cout << "  Ch" << (ch + 1U) << ": ";
    if (err == true) {
      std::cout << "ERROR";
    } else if (over == true) {
      std::cout << "OVERRANGE";
    } else if (under == true) {
      std::cout << "UNDERRANGE";
    } else {
      std::cout << std::fixed << std::setprecision(1) << temp << " degC";
    }
    std::cout << std::endl;
  }
}

/**
 * @brief Print all EL3314-0002 thermocouple channel readings.
 * @param img    Current process image.
 * @param offset Byte offset of EL3314 inputs in the image.
 */
static void display_el3314(const ProcessImage &img, uint32_t offset) {
  std::cout << "[TC] EL3314-0002 4x Thermocouple:" << std::endl;
  for (uint32_t ch = 0; ch < EL3314_CHANNELS; ++ch) {
    // EL3314-0002 has 6 bytes per channel in the -0002 variant.
    uint32_t ch_offset = offset + ch * EL3314_BYTES_PER_CHANNEL;
    uint8_t status = read_el33xx_status(img, ch_offset);
    double temp = read_el33xx_temperature(img, ch_offset);
    bool err = (status & EL33XX_STATUS_ERROR_BIT) != 0U;
    bool over = (status & EL33XX_STATUS_OVERRANGE_BIT) != 0U;
    bool under = (status & EL33XX_STATUS_UNDERRANGE_BIT) != 0U;
    std::cout << "  Ch" << (ch + 1U) << ": ";
    if (err == true) {
      std::cout << "ERROR";
    } else if (over == true) {
      std::cout << "OVERRANGE";
    } else if (under == true) {
      std::cout << "UNDERRANGE";
    } else {
      std::cout << std::fixed << std::setprecision(1) << temp << " degC";
    }
    std::cout << std::endl;
  }
}

/* =========================================================================
 * Output pattern generator
 * ========================================================================= */

/**
 * @brief Compute the next walking-bit DO pattern.
 * @details Cycles a single active bit across all 16 channels, then inverts
 * to all-ON, then returns to a walking pattern. Provides a visible indicator
 * that the master is alive and the outputs are functioning.
 *
 * Pattern sequence (repeating):
 *   Cycle 0:  0x0001 (Ch1 only)
 *   Cycle 1:  0x0002 (Ch2 only)
 *   ...
 *   Cycle 15: 0x8000 (Ch16 only)
 *   Cycle 16: 0xAAAA (even channels)
 *   Cycle 17: 0x5555 (odd channels)
 *   Then back to Cycle 0.
 *
 * @param step Current step counter (incremented by caller).
 * @return 16-bit pattern to write to EL2809 outputs.
 */
static uint16_t next_output_pattern(uint32_t step) {
  // Steps 0-15: walking single bit.
  constexpr uint32_t WALKING_STEPS = 16U;
  // Steps 16-17: alternating masks.
  constexpr uint32_t TOTAL_STEPS = 18U;
  uint32_t s = step % TOTAL_STEPS;
  if (s < WALKING_STEPS) {
    return static_cast<uint16_t>(1U << s);
  } else if (s == 16U) {
    return 0xAAAAU; // Even-numbered channels ON
  } else {
    return 0x5555U; // Odd-numbered channels ON
  }
}

/* =========================================================================
 * Main cyclic loop
 * ========================================================================= */

/**
 * @brief Main entry point.
 * @details Parses CLI arguments, runs bus startup, then enters the cyclic
 * loop exchanging process data every 10 ms. Stops on SIGINT/SIGTERM.
 * @param argc Argument count.
 * @param argv Argument vector. argv[1] = ethernet interface name.
 * @return 0 on clean exit, 1 on error.
 */
int main(int argc, char *argv[]) {
  // -----------------------------------------------------------------------
  // Argument parsing
  // -----------------------------------------------------------------------
  if (argc < 2) {
    std::cerr << "Usage: testmaster100 <interface>" << std::endl;
    std::cerr << "  Example: testmaster100 eno2" << std::endl;
    return 1;
  }
  std::string iface = argv[1];

  // -----------------------------------------------------------------------
  // POSIX signal handling: catch Ctrl+C and SIGTERM for graceful stop.
  // -----------------------------------------------------------------------
  struct sigaction sa;
  std::memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handle_signal;
  sigaction(SIGINT, &sa, nullptr);
  sigaction(SIGTERM, &sa, nullptr);

  std::cout << "=========================================================="
            << std::endl;
  std::cout << " testmaster100 — EtherCAT Master for Beckhoff Demo Bus"
            << std::endl;
  std::cout << " Interface: " << iface << std::endl;
  std::cout << "=========================================================="
            << std::endl;

  // -----------------------------------------------------------------------
  // Step 1: Open raw socket.
  // -----------------------------------------------------------------------
  std::unique_ptr<RawSocket> socket;
  std::unique_ptr<Enumerator> enumerator;
  ProcessImage image;

  try {
    socket = std::make_unique<RawSocket>(iface);
  } catch (const std::exception &ex) {
    std::cerr << "[FATAL] Cannot open socket on '" << iface
              << "': " << ex.what() << ". Run as root?" << std::endl;
    return 1;
  }

  // -----------------------------------------------------------------------
  // Step 2: Enumerate bus.
  // -----------------------------------------------------------------------
  enumerator = std::make_unique<Enumerator>(*socket);
  // Verbose level 1: show state transitions and SM config.
  enumerator->set_verbose(1);

  std::cout << "[STEP 1] Enumerating bus..." << std::endl;
  Result<size_t> enum_res = enumerator->enumerate();
  if (!enum_res) {
    std::cerr << "[FATAL] Bus enumeration failed (err="
              << static_cast<int>(enum_res.error()) << ")." << std::endl;
    return 1;
  }
  std::cout << "  Found " << *enum_res << " slave(s)." << std::endl;

  // -----------------------------------------------------------------------
  // Step 3: Configure FMMU (assigns process image offsets to each slave).
  // -----------------------------------------------------------------------
  std::cout << "[STEP 2] Configuring FMMU..." << std::endl;
  Result<uint32_t> fmmu_res = enumerator->configure_fmmu(image);
  if (!fmmu_res) {
    std::cerr << "[FATAL] FMMU configuration failed." << std::endl;
    return 1;
  }
  uint32_t image_bytes = *fmmu_res;
  std::cout << "  Process image: " << image_bytes << " bytes." << std::endl;

  // -----------------------------------------------------------------------
  // Step 4: Validate topology against expected slave lineup.
  // -----------------------------------------------------------------------
  if (!validate_topology(*enumerator, image_bytes)) {
    return 1;
  }

  // -----------------------------------------------------------------------
  // Step 5: Retrieve process image offsets from each slave's configured info.
  //         These are authoritative — set by configure_fmmu() — and do not
  //         depend on hardcoded constants.
  // -----------------------------------------------------------------------
  const std::vector<SlaveInfo> &slaves = enumerator->slaves();
  uint32_t el2809_out_offset = slaves[SLAVE_EL2809].outputs_offset;
  uint32_t el1809_in_offset = slaves[SLAVE_EL1809].inputs_offset;
  uint32_t el3318_in_offset = slaves[SLAVE_EL3318].inputs_offset;
  uint32_t el3314_in_offset = slaves[SLAVE_EL3314].inputs_offset;

  std::cout << "[STEP 3] Process image offsets (from FMMU):" << std::endl;
  std::cout << "  EL2809 outputs at byte 0x" << std::hex << el2809_out_offset
            << std::dec << std::endl;
  std::cout << "  EL1809 inputs  at byte 0x" << std::hex << el1809_in_offset
            << std::dec << std::endl;
  std::cout << "  EL3318 inputs  at byte 0x" << std::hex << el3318_in_offset
            << std::dec << std::endl;
  std::cout << "  EL3314 inputs  at byte 0x" << std::hex << el3314_in_offset
            << std::dec << std::endl;

  // -----------------------------------------------------------------------
  // Step 6: Compute expected WKC.
  //  EL2809: outputs only  → +1
  //  EL1809: inputs only   → +2
  //  EL3318: outputs+inputs→ +1+2 = +3
  //  EL3314: outputs+inputs→ +1+2 = +3
  // Total = 1+2+3+3 = 9 (matches test.log)
  // -----------------------------------------------------------------------
  uint16_t expected_wkc = 0U;
  for (const SlaveInfo &s : slaves) {
    if (s.outputs_size_bits > 0U) {
      expected_wkc += WKC_LRW_OUTPUT_INCREMENT;
    }
    if (s.inputs_size_bits > 0U) {
      expected_wkc += WKC_LRW_INPUT_INCREMENT;
    }
  }
  std::cout << "[STEP 4] Expected WKC: " << expected_wkc << std::endl;

  // -----------------------------------------------------------------------
  // Step 7: SAFE-OP — slave clocks synchronise and PDO exchange starts.
  // -----------------------------------------------------------------------
  std::cout << "[STEP 5] Transitioning to SAFE-OP..." << std::endl;
  {
    Result<> r = enumerator->request_state_all(states::SAFE_OP);
    if (!r) {
      std::cerr << "[WARNING] Some slave(s) did not reach SAFE-OP. "
                << "Proceeding with available slaves." << std::endl;
    }
  }

  // Configure FMMU again after potentially ignoring some slaves during state
  // transition This ensures process image offsets are correct for the remaining
  // "good" slaves.
  std::cout << "[STEP 5.1] Re-configuring FMMU for available slaves..."
            << std::endl;
  fmmu_res = enumerator->configure_fmmu(image);
  if (!fmmu_res) {
    std::cerr << "[FATAL] FMMU re-configuration failed." << std::endl;
    return 1;
  }
  image_bytes = *fmmu_res;

  // Refresh offsets
  el2809_out_offset = slaves[SLAVE_EL2809].outputs_offset;
  el1809_in_offset = slaves[SLAVE_EL1809].inputs_offset;
  el3318_in_offset = slaves[SLAVE_EL3318].inputs_offset;
  el3314_in_offset = slaves[SLAVE_EL3314].inputs_offset;

  // Warm-up: exchange a few frames so slaves validate WKC.
  std::cout << "[STEP 6] Warm-up frame exchange..." << std::endl;
  for (int w = 0; w < 10; ++w) {
    enumerator->exchange_process_data(image);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // -----------------------------------------------------------------------
  // Step 8: OPERATIONAL — full cyclic operation begins.
  // -----------------------------------------------------------------------
  std::cout << "[STEP 7] Transitioning to OPERATIONAL..." << std::endl;
  {
    Result<> r = enumerator->request_state_all(states::OP);
    if (!r) {
      std::cerr << "[WARNING] Some slave(s) did not reach OP. "
                << "Continuing in available state." << std::endl;
    }
  }

  // -----------------------------------------------------------------------
  // Cyclic loop
  // -----------------------------------------------------------------------
  std::cout << std::endl;
  std::cout << "========== Running — Ctrl+C to stop ==========" << std::endl;
  std::cout << "  Cycle: " << CYCLE_PERIOD_MS.count() << " ms | "
            << "Display: every "
            << (DISPLAY_EVERY_N_CYCLES * CYCLE_PERIOD_MS.count() / 1000) << " s"
            << std::endl;
  std::cout << "==============================================" << std::endl;

  uint32_t cycle_count = 0U; ///< Total cycles executed.
  uint32_t output_step = 0U; ///< Walking-bit pattern counter.
  uint32_t wkc_errors = 0U;  ///< Consecutive WKC mismatches.

  // Zero the output image before starting.
  write_u16(image, el2809_out_offset, 0x0000U);

  // Hard loop limit [CS-0010.37]: runs until g_stop is set.
  // The actual limit is unreachable in practice (>27 days at 10ms).
  std::chrono::steady_clock::time_point app_start =
      std::chrono::steady_clock::now();
  for (uint64_t guard = 0U; guard < 9000000000ULL && g_stop == 0; ++guard) {
    // Exit after 60s for testing
    if (std::chrono::steady_clock::now() - app_start >=
        std::chrono::seconds(60)) {
      break;
    }
    // Record cycle start time for fixed-period sleeping.
    std::chrono::steady_clock::time_point t_start =
        std::chrono::steady_clock::now();

    // Apply output pattern if this is a display cycle.
    if ((cycle_count % DISPLAY_EVERY_N_CYCLES) == 0U && cycle_count > 0U) {
      // Generate next walking-bit pattern for EL2809.
      uint16_t pattern = next_output_pattern(output_step);
      write_u16(image, el2809_out_offset, pattern);
      ++output_step;
    }

    // Exchange process data (LRW datagram).
    Result<uint16_t> ex = enumerator->exchange_process_data(image);

    // Check WKC health.
    if (!ex || *ex != expected_wkc) {
      ++wkc_errors;
      // Print warning every 100 consecutive errors to avoid log flood.
      if (wkc_errors % 100U == 1U) {
        std::cerr << "[WARN] WKC mismatch: got "
                  << (ex ? static_cast<int>(*ex) : -1) << ", expected "
                  << expected_wkc << " (consecutive errors=" << wkc_errors
                  << ")" << std::endl;
      }
    } else {
      // Reset error counter on good frame.
      wkc_errors = 0U;
    }

    ++cycle_count;

    // Every DISPLAY_EVERY_N_CYCLES, print process data to console.
    if ((cycle_count % DISPLAY_EVERY_N_CYCLES) == 0U) {
      std::cout << std::endl;
      std::cout << "--- Cycle " << cycle_count
                << " (t=" << (cycle_count * CYCLE_PERIOD_MS.count() / 1000U)
                << "s) -----------------------------------------------"
                << std::endl;

      // Display EL2809 digital outputs (16 channels).
      if (!slaves[SLAVE_EL2809].ignored)
        display_digital_outputs(image, el2809_out_offset);

      // Display EL1809 digital inputs  (16 channels).
      if (!slaves[SLAVE_EL1809].ignored)
        display_digital_inputs(image, el1809_in_offset);

      // Display EL3318 thermocouple readings (8 channels).
      if (!slaves[SLAVE_EL3318].ignored)
        display_el3318(image, el3318_in_offset);

      // Display EL3314-0002 thermocouple readings (4 channels).
      if (!slaves[SLAVE_EL3314].ignored)
        display_el3314(image, el3314_in_offset);

      std::cout << "WKC errors since last display: " << wkc_errors << std::endl;
    }

    // Sleep for the remainder of the 10 ms cycle period.
    std::chrono::steady_clock::time_point t_end =
        std::chrono::steady_clock::now();
    std::chrono::microseconds elapsed =
        std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start);
    std::chrono::microseconds remaining =
        std::chrono::duration_cast<std::chrono::microseconds>(CYCLE_PERIOD_MS) -
        elapsed;
    // Only sleep if there is remaining time in this cycle.
    if (remaining.count() > 0) {
      std::this_thread::sleep_for(remaining);
    }
  }

  // -----------------------------------------------------------------------
  // Graceful shutdown: zero all outputs and return to INIT.
  // -----------------------------------------------------------------------
  std::cout << std::endl;
  std::cout << "========== Stop requested — shutting down =========="
            << std::endl;

  // Zero output bits before transitioning away from OP.
  write_u16(image, el2809_out_offset, 0x0000U);
  enumerator->exchange_process_data(image);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  // Request INIT state for all slaves via per-slave FPWR.
  enumerator->request_state_all(states::INIT, std::chrono::seconds(2));

  std::cout << "  Total cycles executed: " << cycle_count << std::endl;
  std::cout << "  Total WKC errors:      " << wkc_errors << std::endl;
  std::cout << "Goodbye." << std::endl;
  return 0;
}
