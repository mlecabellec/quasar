/**
 * @file EniComparator.cpp
 * @brief Implementation of multi-level ENI and EtherCAT topology comparator.
 * @details Contributes to [TSK-20260920-001.4] and [FE-0040.8].
 */

#include "resoem/EniComparator.hpp"
#include "resoem/EniGenerator.hpp"
#include <algorithm>
#include <sstream>

namespace resoem::eni {

std::string ComparisonReport::format_summary() const {
  std::ostringstream oss;
  oss << "=== ENI COMPARISON REPORT ===\n";
  oss << "Reference Slaves: " << reference_slave_count
      << " | Actual Slaves: " << actual_slave_count << "\n";
  oss << "Result: " << (matched ? "MATCHED (Identical)" : "MISMATCH DETECTED") << "\n";
  oss << "Total Mismatches: " << mismatches.size() << "\n";

  if (!mismatches.empty()) {
    oss << "--- Mismatch Details ---\n";
    for (std::size_t i = 0; i < mismatches.size(); ++i) {
      const Mismatch &m = mismatches[i];
      oss << "[" << (i + 1) << "] Slave #" << m.slave_index << " (" << m.slave_name << ")\n";
      oss << "    Field:    " << m.item_name << "\n";
      oss << "    Expected: " << m.expected_value << "\n";
      oss << "    Actual:   " << m.actual_value << "\n";
      if (!m.details.empty()) {
        oss << "    Note:     " << m.details << "\n";
      } else {
        // No extra note
      }
    }
  } else {
    // No mismatches
  }

  return oss.str();
}

ComparisonReport EniComparator::compare_documents(const EniDocument &reference,
                                                  const EniDocument &actual,
                                                  ComparisonLevel level) {
  ComparisonReport report;
  report.reference_slave_count = reference.slaves().size();
  report.actual_slave_count = actual.slaves().size();
  report.matched = true;

  const std::vector<EniSlave> &ref_slaves = reference.slaves();
  const std::vector<EniSlave> &act_slaves = actual.slaves();

  // 1. Topology check: Slave Count
  if (level == ComparisonLevel::TOPOLOGY || level == ComparisonLevel::ALL) {
    if (ref_slaves.size() != act_slaves.size()) {
      report.matched = false;
      Mismatch m;
      m.level = ComparisonLevel::TOPOLOGY;
      m.slave_index = 0;
      m.slave_name = "Network";
      m.item_name = "SlaveCount";
      m.expected_value = std::to_string(ref_slaves.size());
      m.actual_value = std::to_string(act_slaves.size());
      m.details = "Physical slave count differs from ENI definition";
      report.mismatches.push_back(m);
    } else {
      // Slave counts match
    }
  } else {
    // Skip slave count check
  }

  // 2. Per-Slave Comparisons
  std::size_t common_count = std::min(ref_slaves.size(), act_slaves.size());
  for (std::size_t i = 0; i < common_count; ++i) {
    const EniSlave &ref = ref_slaves[i];
    const EniSlave &act = act_slaves[i];

    // TOPOLOGY Level
    if (level == ComparisonLevel::TOPOLOGY || level == ComparisonLevel::ALL) {
      if (ref.phys_addr != act.phys_addr) {
        report.matched = false;
        Mismatch m;
        m.level = ComparisonLevel::TOPOLOGY;
        m.slave_index = i;
        m.slave_name = ref.name;
        m.item_name = "PhysAddr";
        m.expected_value = "0x" + std::to_string(ref.phys_addr);
        m.actual_value = "0x" + std::to_string(act.phys_addr);
        m.details = "Configured station address mismatch";
        report.mismatches.push_back(m);
      } else {
        // PhysAddr matched
      }
    } else {
      // Skip topology
    }

    // TYPE Level
    if (level == ComparisonLevel::TYPE || level == ComparisonLevel::ALL) {
      if (ref.vendor_id != act.vendor_id) {
        report.matched = false;
        Mismatch m;
        m.level = ComparisonLevel::TYPE;
        m.slave_index = i;
        m.slave_name = ref.name;
        m.item_name = "VendorId";
        m.expected_value = "0x" + std::to_string(ref.vendor_id);
        m.actual_value = "0x" + std::to_string(act.vendor_id);
        m.details = "Hardware Vendor ID mismatch";
        report.mismatches.push_back(m);
      } else {
        // Vendor ID matched
      }

      if (ref.product_code != act.product_code) {
        report.matched = false;
        Mismatch m;
        m.level = ComparisonLevel::TYPE;
        m.slave_index = i;
        m.slave_name = ref.name;
        m.item_name = "ProductCode";
        m.expected_value = "0x" + std::to_string(ref.product_code);
        m.actual_value = "0x" + std::to_string(act.product_code);
        m.details = "Hardware Product Code mismatch";
        report.mismatches.push_back(m);
      } else {
        // Product Code matched
      }

      if (ref.revision_no != act.revision_no) {
        report.matched = false;
        Mismatch m;
        m.level = ComparisonLevel::TYPE;
        m.slave_index = i;
        m.slave_name = ref.name;
        m.item_name = "RevisionNo";
        m.expected_value = "0x" + std::to_string(ref.revision_no);
        m.actual_value = "0x" + std::to_string(act.revision_no);
        m.details = "Hardware Revision Number mismatch";
        report.mismatches.push_back(m);
      } else {
        // Revision matched
      }
    } else {
      // Skip type
    }

    // MAPPING Level
    if (level == ComparisonLevel::MAPPING || level == ComparisonLevel::ALL) {
      if (ref.sync_managers.size() != act.sync_managers.size()) {
        report.matched = false;
        Mismatch m;
        m.level = ComparisonLevel::MAPPING;
        m.slave_index = i;
        m.slave_name = ref.name;
        m.item_name = "SyncManagerCount";
        m.expected_value = std::to_string(ref.sync_managers.size());
        m.actual_value = std::to_string(act.sync_managers.size());
        m.details = "Number of active SyncManagers differs";
        report.mismatches.push_back(m);
      } else {
        // SM count matched
      }
    } else {
      // Skip mapping
    }

    // HOT_CONNECT Level
    if (level == ComparisonLevel::HOT_CONNECT || level == ComparisonLevel::ALL) {
      bool ref_hc = ref.hot_connect.has_value();
      bool act_hc = act.hot_connect.has_value();
      if (ref_hc != act_hc) {
        // Record capability mismatch
        report.matched = false;
        Mismatch m;
        m.level = ComparisonLevel::HOT_CONNECT;
        m.slave_index = i;
        m.slave_name = ref.name;
        m.item_name = "HotConnectStatus";
        m.expected_value = ref_hc ? "Enabled" : "Disabled";
        m.actual_value = act_hc ? "Enabled" : "Disabled";
        m.details = "Hot Connect capability assignment mismatch";
        report.mismatches.push_back(m);
      } else if (ref_hc && act_hc) {
        // Compare group names when both have hot connect configured
        if (ref.hot_connect->group_name != act.hot_connect->group_name) {
          report.matched = false;
          Mismatch m;
          m.level = ComparisonLevel::HOT_CONNECT;
          m.slave_index = i;
          m.slave_name = ref.name;
          m.item_name = "HotConnectGroupName";
          m.expected_value = ref.hot_connect->group_name;
          m.actual_value = act.hot_connect->group_name;
          m.details = "Hot Connect Group Name mismatch";
          report.mismatches.push_back(m);
        } else {
          // Group name matched
        }
      } else {
        // Neither has Hot Connect
      }
    } else {
      // Skip hot connect
    }
  }

  return report;
}

ComparisonReport EniComparator::compare_live_bus(const EniDocument &reference,
                                                IRawSocket &socket,
                                                ComparisonLevel level) {
  // Generate ENI topology directly from live bus scanning
  EniGenerator generator(socket);
  Result<EniDocument> live_doc = generator.generate("LiveMaster");
  if (!live_doc.has_value()) {
    // Record scan failure mismatch
    ComparisonReport rep;
    rep.matched = false;
    Mismatch m;
    m.level = ComparisonLevel::TOPOLOGY;
    m.slave_name = "Network";
    m.item_name = "LiveBusScan";
    m.expected_value = "Success";
    m.actual_value = "Failed";
    m.details = "Live bus interrogation failed";
    rep.mismatches.push_back(m);
    return rep;
  } else {
    // Compare live scanned document against reference
    return compare_documents(reference, live_doc.value(), level);
  }
}

} // namespace resoem::eni
