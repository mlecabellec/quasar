# Implementation Plan: Reflexive Web API & Networking (TSK-002, TSK-003, TSK-008)

**Date:** 2026-04-22  
**Status:** 🔄 **Draft / Refinement Phase**  
**Objectives:** Deliver high-performance networking, a scalable discovery API, a strictly consistent induced API, and a real-time PUB/SUB dashboard.

## 1. Compliance Baseline
All work must strictly adhere to the following mandates:
- **CS-0010**: C++23 standards, NO `auto`, NO `new`, mandatory Doxygen, hard loop limits.
- **CS-0020**: Advanced safety, `std::expected` for errors, `[[nodiscard]]`, zero-copy views.
- **CS-0030**: Feature traceability (`@feature`, `@exposed`), mandatory stress testing (worst-case scenarios).

## 2. Phased Implementation

### Phase 1: The Typed Foundation (Core `named` & `datacodec`)
*Goal: Enable formal signatures without altering NamedObject base performance.*
- **Step 1.1: `TypedNamedMethod`**: Create subclass of `NamedMethod` in `cmake-projects/named` with fields for `inputSchema` and `outputSchema` (using `datacodec::ContainerDef`).
- **Step 1.2: `JsonMapper` (Streaming)**: Implement high-performance, schema-driven JSON-to-NamedObject parser in `datacodec` using a one-pass streaming approach.
- **Step 1.3: Performance Isolation Proof**: Implement `TestMethodPerformance` to demonstrate that non-web nodes and standard tree operations maintain zero overhead.

### Phase 2: Async Networking & SSL (Plugin `net`)
*Goal: Finalize the communication stack.*
- **Step 2.1: WebSocket Core**: Implement `WSClient` and `WSServer` wrappers around `CppServer` with full Lua bindings.
- **Step 2.2: SSL/TLS Hardening**: Finalize `SSLContextWrapper` to support root CA paths and peer verification, mandatory for industrial HTTPS.
- **Step 2.3: Reconnection State Machine**: Implement exponential backoff for `TCPClient` and `WSClient` to handle network instability.

### Phase 3: Base API & Tree Discovery (Service `webui`)
*Goal: Provide a scalable Discovery interface.*
- **Step 3.1: `WebUIService`**: Create an `ActiveEntity` that manages the lifecycle of the Web infrastructure.
- **Step 3.2: Stateless Tree Walker**: Implement chunked `GET /api/v1/walk` using absolute path continuation tokens and a `treeVersion` conflict detection mechanism.
- **Step 3.3: Metadata Registry**: Implement `GET /api/v1/meta/{path}` returning schema descriptors and unit information.

### Phase 4: Induced API & OpenAPI (Plugin `net`)
*Goal: Direct method mapping and self-documentation.*
- **Step 4.1: `WebNamedMethod`**: Create subclass in `net` plugin for HTTP-specific metadata (Verb, Alias, OAS Summary).
- **Step 4.2: Dynamic Induced Router**: Implement a lookup engine that maps URI segments directly to `WebNamedMethod` nodes.
- **Step 4.3: OpenApiGenerator**: Implement a dynamic producer for `/openapi.json` that maps the live tree structure to OAS 3.0 schemas.

### Phase 5: WebSocket PUB/SUB & Async Control
*Goal: Deliver real-time synchronization.*
- **Step 5.1: Wildcard Subscription Manager**: Implement a Prefix-Trie based registry in `WebUIService` that uses the `IObserver` pattern to track branches (e.g., `system/sensors/*`) or individual leaves.
- **Step 5.1.1: Resource Guard**: Enforce a `MAX_SUBSCRIPTIONS_PER_SESSION` hard limit to prevent client-side resource exhaustion.
- **Step 5.2: Throttled Delta Engine**: Implement a background task that aggregates value changes into a single "Batch Delta" frame.
- **Step 5.2.1: Configurable Batching**: Allow clients to configure the batching latency (e.g., 16ms for 60Hz updates) to match their rendering capabilities.
- **Step 5.2.2: Base64 Binary Mapping**: Encode `NamedBuffer` and `NamedBitBuffer` payloads as Base64 strings within the JSON delta frames to simplify frontend ingestion.
- **Step 5.3: Acknowledged Async Set**: Implement a protocol where clients push values back to the tree via WS frames containing a unique `requestID`.
- **Step 5.3.1: Closed-Loop Confirmation**: The server returns an explicit "Ack" or "Error" frame for every `requestID` after passing through the `JsonMapper` consistency check.

### Phase 6: Frontend Development (Dashboard)
*Goal: Mission Control UI.*
- **Step 6.1: React/TS Scaffolding**: Setup Vite-based project in `cmake-projects/webui/frontend`.
- **Step 6.2: Recursive Tree Browser**: Build a high-performance visualizer capable of searching thousands of nodes via the Base API.
- **Step 6.3: Dynamic Form Generator**: Automatically build method invocation UI from the OpenAPI schema.
- **Step 6.4: Dynamic API Proxy**: Implement a runtime client that parses `openapi.json` at startup to build reflexive call interfaces.
- **Step 6.4.1: Live Schema Reload**: Integrate a WebSocket-driven listener that triggers a re-fetch of the OpenAPI definition when the `treeVersion` changes, ensuring the UI remains consistent with runtime-added Lua methods.

## 3. Verification & Validation
- **Functional**: Integration suite for every REST and WS endpoint.
- **Industrial Stress**: 24h stability test with 10+ concurrent clients walking a 10,000-node tree during active mutations.
- **Security Audit**: Verify that invalid SSL handshakes and malformed JSON payloads are strictly rejected.
- **Traceability**: Audit all new methods for mandatory Doxygen tags.
