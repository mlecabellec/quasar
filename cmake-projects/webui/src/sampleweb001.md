# Quasar Integration Reference: sampleweb001

This executable serves as the primary integration reference for the Quasar industrial ecosystem. It demonstrates the seamless orchestration of core hierarchy, deterministic logic, and modern web interfaces.

## 🏗️ Architecture

The sample constructs a complete system stack:
1.  **Core Tree**: A `NamedObject` hierarchy representing sensors, system status, and command flags.
2.  **Logic Engine**: A `StateMachine` (HSM) that monitors tree variables (`ctx.Status.cmdStart`, etc.) to drive system state.
3.  **Industrial API**: An `OpcUaServerService` mapping the internal registry to a standard OPC UA endpoint (port 4840).
4.  **Web Dashboard**: A `WebUIService` (port 8086) serving the React/TypeScript frontend and providing REST/WebSocket control paths.

## 🚀 Execution & Verification

### Building the Sample
```bash
mkdir build && cd build
cmake ..
make sampleweb001
```

### Running the Integration Test
A specialized bash script is provided to verify the full-spectrum functionality (Tree -> FSM -> REST):
```bash
./cmake-projects/webui/test/VerifySampleWeb001.sh
```

### Manual Exploration
- **Web UI**: Visit `http://localhost:8086` to interact with the "Mission Control" dashboard.
- **REST API**: 
    - Walk the tree: `curl http://localhost:8086/api/v1/walk?path=/Registry`
    - Set a value: `curl -X POST http://localhost:8086/api/v1/set?path=/Registry/Status/cmdStart -d '{"value": true}'`
- **OPC UA**: Connect using `uaexpert` or similar to `opc.tcp://localhost:4840`.

## 🛡️ Engineering Standards
This sample strictly adheres to:
- **CS-0010/20**: Explicit typing, RAII thread management (`std::jthread`), and memory safety (Valgrind verified).
- **TSK-20260311-008**: Fulfills the requirements for high-performance web orchestration.

---
*Maintained by Quasar Engineering Agent.*
