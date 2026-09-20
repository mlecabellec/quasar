/**
 * @file ecat_topocheck.cpp
 * @brief CLI tool to validate live EtherCAT bus against reference ENI configuration file.
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

int main(int argc, char *argv[]) {
  optparse::OptionParser parser =
      optparse::OptionParser().description("Quasar EtherCAT Topology & ENI Validation Tool");

  parser.add_option("-i", "--interface")
      .dest("interface")
      .set_default("eth0")
      .help("Network interface (default: eth0)");

  parser.add_option("-c", "--config")
      .dest("config")
      .help("Reference ENI XML configuration file path (required)");

  parser.add_option("-l", "--level")
      .dest("level")
      .set_default("all")
      .help("Comparison level: topology, type, mapping, hotconnect, all (default: all)");

  optparse::Values options = parser.parse_args(argc, argv);

  if (!options.is_set("config")) {
    std::cerr << "Error: Reference ENI configuration file (-c / --config) is required." << std::endl;
    parser.print_help();
    return 1;
  } else {
    // Config argument present
  }

  std::string iface = options["interface"];
  std::string config_file = options["config"];
  std::string level_str = options["level"];
  ComparisonLevel level = parse_level(level_str);

  EniDocument reference_doc;
  Result<void> load_res = reference_doc.load_from_file(config_file);
  if (!load_res.has_value()) {
    std::cerr << "Error: Failed to load reference ENI file: " << config_file << std::endl;
    return 1;
  } else {
    std::cout << "Loaded reference ENI with " << reference_doc.slaves().size()
              << " slaves." << std::endl;
  }

  try {
    std::cout << "Verifying live bus on " << iface << " against reference..." << std::endl;
    RawSocket socket(iface);

    ComparisonReport report = EniComparator::compare_live_bus(reference_doc, socket, level);
    std::cout << report.format_summary() << std::endl;

    if (report.matched) {
      return 0;
    } else {
      return 1;
    }
  } catch (const SocketError &e) {
    std::cerr << "Socket error on interface " << iface << ": " << e.what() << std::endl;
    std::cerr << "Note: Raw packet capture usually requires root privileges (sudo)." << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Unexpected error: " << e.what() << std::endl;
    return 1;
  }
}
