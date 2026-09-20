/**
 * @file EniComparator.hpp
 * @brief Multi-level comparison engine between reference ENI files and scanned EtherCAT topologies.
 * @details Contributes to [TSK-20260920-001.4] and [FE-0040.8].
 */

#pragma once

#include "resoem/EniDocument.hpp"
#include "resoem/IRawSocket.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace resoem::eni {

/**
 * @enum ComparisonLevel
 * @brief Granularity and scope of ENI / topology comparison.
 */
enum class ComparisonLevel : std::uint8_t {
  TOPOLOGY = 0,    ///< Verify slave count, order, physical addresses, and port wiring
  TYPE = 1,        ///< Verify vendor IDs, product codes, revision and serial numbers
  MAPPING = 2,     ///< Verify SyncManagers, FMMUs, and PDO entries
  HOT_CONNECT = 3, ///< Verify Hot Connect groups, station aliases, and group members
  ALL = 4          ///< Perform exhaustive comparison across all levels
};

/**
 * @struct Mismatch
 * @brief Describes a single difference or mismatch detected during comparison.
 */
struct Mismatch {
  ComparisonLevel level{ComparisonLevel::ALL}; ///< Mismatch category
  std::size_t slave_index{0};                  ///< Slave index in reference document
  std::string slave_name;                      ///< Reference slave name
  std::string item_name;                       ///< Compared field or item name
  std::string expected_value;                  ///< Expected value from reference
  std::string actual_value;                    ///< Actual value detected
  std::string details;                         ///< Diagnostic description
};

/**
 * @struct ComparisonReport
 * @brief Complete diagnostic outcome of an ENI comparison pass.
 */
struct ComparisonReport {
  bool matched{true};                          ///< True if no mismatches detected for selected level
  std::vector<Mismatch> mismatches;            ///< Detailed list of mismatches
  std::size_t reference_slave_count{0};        ///< Number of slaves in reference
  std::size_t actual_slave_count{0};           ///< Number of slaves in tested bus/document

  /**
   * @brief Format a human-readable summary of all mismatches.
   * @return Formatted report string.
   */
  [[nodiscard]] std::string format_summary() const;
};

/**
 * @class EniComparator
 * @brief Compares reference ENI documents against live buses or candidate ENI files.
 */
class EniComparator {
public:
  /**
   * @brief Compare two EniDocument AST models.
   * @param reference Expected ENI document (golden reference).
   * @param actual Candidate ENI document (actual/generated).
   * @param level Scope of comparison (TOPOLOGY, TYPE, MAPPING, HOT_CONNECT, ALL).
   * @return Detailed ComparisonReport.
   */
  [[nodiscard]] static ComparisonReport compare_documents(
      const EniDocument &reference, const EniDocument &actual,
      ComparisonLevel level = ComparisonLevel::ALL);

  /**
   * @brief Compare a reference EniDocument directly against a live connected EtherCAT bus.
   * @param reference Expected ENI document.
   * @param socket Transport socket connected to the live network.
   * @param level Scope of comparison.
   * @return Detailed ComparisonReport.
   */
  [[nodiscard]] static ComparisonReport compare_live_bus(
      const EniDocument &reference, IRawSocket &socket,
      ComparisonLevel level = ComparisonLevel::ALL);
};

} // namespace resoem::eni
