/**
 * @file ecat_cmp.cpp
 * @brief CLI tool to compare two ENI files or compare an ENI file with a live bus.
 * @details Contributes to [TSK-20260920-001.4] and [FE-0040.8].
 */

#include "OptionParser.h"
#include "resoem/EniComparator.hpp"
#include "resoem/EniDocument.hpp"
#include "resoem/RawSocket.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace resoem;
using namespace resoem::eni;

namespace {

/// @brief Parse CLI comparison strictness level string into enum value.
ComparisonLevel parse_level(const std::string &str) {
  if (str == "topology") {
    return ComparisonLevel::TOPOLOGY;
  } else if (str == "type") {
    return ComparisonLevel::TYPE;
  } else if (str == "mapping") {
    return ComparisonLevel::MAPPING;
  } else if (str == "hotconnect") {
    return ComparisonLevel::HOT_CONNECT;
  } else {
    return ComparisonLevel::ALL;
  }
}

} // anonymous namespace

/// @brief Entry point for comparing live EtherCAT bus topology against an ENI file.
int main(int argc, char *argv[]) {
  optparse::OptionParser parser =
      optparse::OptionParser().description("Quasar EtherCAT Multi-Level ENI Comparator");

  parser.add_option("-r", "--reference")
      .dest("reference")
      .help("Golden reference ENI XML configuration file path (required)");

  parser.add_option("-c", "--candidate")
      .dest("candidate")
      .help("Candidate ENI XML configuration file path to compare against reference");

  parser.add_option("-i", "--interface")
      .dest("interface")
      .help("Live network interface to compare directly against reference");

  parser.add_option("-l", "--level")
      .dest("level")
      .set_default("all")
      .help("Comparison level: topology, type, mapping, hotconnect, all (default: all)");

  optparse::Values options = parser.parse_args(argc, argv);

  if (!options.is_set("reference")) {
    std::cerr << "Error: Reference ENI file (-r / --reference) is required." << std::endl;
    parser.print_help();
    return 1;
  } else {
    // Reference present
  }

  if (!options.is_set("candidate") && !options.is_set("interface")) {
    std::cerr << "Error: Must specify either a candidate ENI file (-c) or a live interface (-i)." << std::endl;
    parser.print_help();
    return 1;
  } else {
    // Target present
  }

  std::string ref_file = options["reference"];
  std::string level_str = options["level"];
  ComparisonLevel level = parse_level(level_str);

  EniDocument ref_doc;
  Result<void> ref_res = ref_doc.load_from_file(ref_file);
  if (!ref_res.has_value()) {
    std::cerr << "Error: Failed to load reference ENI file: " << ref_file << std::endl;
    return 1;
  } else {
    std::cout << "Loaded reference ENI: " << ref_file << " (" << ref_doc.slaves().size()
              << " slaves)" << std::endl;
  }

  ComparisonReport report;

  if (options.is_set("candidate")) {
    std::string cand_file = options["candidate"];
    EniDocument cand_doc;
    Result<void> cand_res = cand_doc.load_from_file(cand_file);
    if (!cand_res.has_value()) {
      std::cerr << "Error: Failed to load candidate ENI file: " << cand_file << std::endl;
      return 1;
    } else {
      std::cout << "Loaded candidate ENI: " << cand_file << " (" << cand_doc.slaves().size()
                << " slaves)" << std::endl;
    }

    report = EniComparator::compare_documents(ref_doc, cand_doc, level);
  } else {
    std::string iface = options["interface"];
    std::cout << "Comparing reference ENI against live bus on " << iface << "..." << std::endl;
    try {
      RawSocket socket{iface};
      report = EniComparator::compare_live_bus(ref_doc, socket, level);
    } catch (const SocketError &e) {
      std::cerr << "Socket error on interface " << iface << ": " << e.what() << std::endl;
      return 1;
    }
  }

  std::cout << report.format_summary() << std::endl;
  if (report.matched) {
    return 0;
  } else {
    return 1;
  }
}
