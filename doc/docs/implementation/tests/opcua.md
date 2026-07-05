# OPC UA Module Tests

This document describes the testing suite for the `opcua` module, located in `cmake-projects/opcua/test/`.

## 1. Server Tests (`TestOpcUaServer.cpp`)
- **Tree Mapping**: Verifies that a Quasar `NamedObject` hierarchy is correctly transformed into OPC UA Variable and Object nodes.
- **Method Exposure**: Validates that `NamedMethod` objects are callable as OPC UA Methods.
- **Value Sync**: Verifies that changing a Quasar object's value (e.g., `NamedInteger`) automatically updates the corresponding OPC UA node.
- **Concurrent Access**: Tests multiple OPC UA clients connecting and reading data simultaneously.

## 2. Client Tests (`TestOpcUaClient.cpp`)
- **Recursive Mirroring**: Verifies that the client correctly browses and mirrors a remote address space into a local `NamedObject` tree.
- **Subscription Latency**: Measures the time between a server-side change and the corresponding update in the mirrored local object.
- **Method Forwarding**: Validates that calling `execute()` on a mirrored local method correctly triggers the remote OPC UA method.
- **Reconnection**: Tests the client's ability to recover from network interruptions.

## 3. Security and Auth
- **X.509 Validation**: Verifies that the `OpcUaSecurityManager` correctly rejects clients with untrusted or expired certificates.
- **Encryption**: Validates data integrity and confidentiality under `SignAndEncrypt` policies.
- **User Auth**: Tests Username/Password authentication logic.
