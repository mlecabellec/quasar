/**
 * @file ecat_scan.cpp
 * @brief CLI tool to scan physical EtherCAT bus and export ETG.2100 ENI XML configuration.
 * @details Contributes to [TSK-20260920-001.3] and [FE-0040.8].
 */

#include "OptionParser.h"
#include "resoem/EniGenerator.hpp"
#include "resoem/RawSocket.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace resoem;
using namespace resoem::eni;

int main(int argc, char *argv[]) {
  optparse::OptionParser parser =
      optparse::OptionParser().description("Quasar EtherCAT ENI Generator & Bus Scanner");

  parser.add_option("-i", "--interface")
      .dest("interface")
      .set_default("eth0")
      .help("Network interface (default: eth0)");

  parser.add_option("-o", "--output")
      .dest("output")
      .set_default("generated_eni.xml")
      .help("Output ENI XML file path (default: generated_eni.xml)");

  parser.add_option("-m", "--master")
      .dest("master")
      .set_default("QuasarMaster")
      .help("Master device identifier (default: QuasarMaster)");

  optparse::Values options = parser.parse_args(argc, argv);

  std::string iface = options["interface"];
  std::string output_file = options["output"];
  std::string master_name = options["master"];

  std::cout << "Starting EtherCAT scan on interface: " << iface << "..." << std::endl;

  try {
    RawSocket socket(iface);
    EniGenerator generator(socket);

    Result<EniDocument> gen_res = generator.generate(master_name);
    if (!gen_res.has_value()) {
      std::cerr << "Error: Failed to scan EtherCAT bus." << std::endl;
      return 1;
    } else {
      // Scan succeeded
    }

    const EniDocument &doc = gen_res.value();
    std::cout << "Successfully scanned " << doc.slaves().size() << " slaves." << std::endl;

    Result<void> save_res = generator.save_to_xml_file(output_file, doc);
    if (!save_res.has_value()) {
      std::cerr << "Error: Failed to write ENI XML file to: " << output_file << std::endl;
      return 1;
    } else {
      std::cout << "ETG.2100 ENI XML successfully exported to: " << output_file << std::endl;
    }

    return 0;
  } catch (const SocketError &e) {
    std::cerr << "Socket error on interface " << iface << ": " << e.what() << std::endl;
    std::cerr << "Note: Raw packet capture usually requires root privileges (sudo)." << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Unexpected error: " << e.what() << std::endl;
    return 1;
  }
}
