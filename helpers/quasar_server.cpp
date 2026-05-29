#include "quasar/opcua/OpcUaServerService.hpp"
#include "quasar/named/NamedInteger.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

using namespace quasar::named;
using namespace quasar::opcua;

bool running = true;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    running = false;
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "Starting Quasar OPC UA Server..." << std::endl;

    auto root = NamedObject::create("Root");
    auto data = NamedObject::create("Data", root);
    auto myInt = NamedInteger<int32_t>::create("MyInt", 42, data);

    auto server = OpcUaServerService::create("OpcUaServer");
    server->setPort(4840);
    server->setRootObject(root);

    server->start();

    std::cout << "Server is running on port 4840. Press Ctrl+C to stop." << std::endl;

    while (running && server->isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Stopping server..." << std::endl;
    server->stop();

    return 0;
}
