# Implementation Plan: Reflexive Web API & Networking (TSK-002, TSK-003, TSK-008)

**Date:** 2026-04-23  
**Status:** 🔄 **Active Execution Phase**  
**Objectives:** Deliver high-performance networking, a scalable discovery API, a strictly consistent induced API, and a real-time PUB/SUB dashboard.

## 1. Compliance Baseline
All work must strictly adhere to the following mandates:
- **CS-0010**: C++23 standards, NO `auto`, NO `new`, mandatory Doxygen, hard loop limits.
- **CS-0020**: Advanced safety, `std::expected` for errors, `[[nodiscard]]`, zero-copy views.
- **CS-0030**: Feature traceability (`@feature`, `@exposed`), mandatory stress testing (worst-case scenarios).

## 2. Phased Implementation

### Phase 1: The Typed Foundation (Core `named` & `datacodec`)
*Goal: Enable formal signatures without altering NamedObject base performance.*
- **Step 1.1: `TypedNamedMethod`**: ✅ **Completed**. Created subclass of `NamedMethod` with schema support.
- **Step 1.2: `JsonMapper` (Streaming)**: 🔄 **In Progress**. Designing high-performance, schema-driven parser using `jsoncons`.
- **Step 1.3: Performance Isolation Proof**: 🔲 **Not Started**.

### Phase 2: Async Networking & SSL (Plugin `net`)
*Goal: Finalize the communication stack.*
- **Step 2.1: WebSocket Core**: ✅ **Completed**. Implemented `WSClient` and `WSServer` with PIMPL isolation to resolve `CppServer` naming collisions. Lua bindings verified.
- **Step 2.2: SSL/TLS Hardening**: ✅ **Completed**. `SSLContextWrapper` supports root CAs and peer verification.
- **Step 2.3: Reconnection State Machine**: ✅ **Completed**. Exponential backoff implemented for `TCPClient` and `WSClient`.

### Phase 3: Base API & Tree Discovery (Service `webui`)
*Goal: Provide a scalable Discovery interface.*
- **Step 3.1: `WebUIService`**: ✅ **Completed**. `ActiveEntity` managing embedded HTTP infrastructure.
- **Step 3.2: Stateless Tree Walker**: 🔄 **In Progress**. implemented baseline `GET /api/v1/walk`. Chunking and `treeVersion` detection are pending.
- **Step 3.3: Metadata Registry**: ✅ **Completed**. `GET /api/v1/meta/{path}` returns schema and relationship descriptors.

### Phase 4: Induced API & OpenAPI (Plugin `net`)
*Goal: Direct method mapping and self-documentation.*
- **Step 4.1: `WebNamedMethod`**: 🔲 **Not Started**.
- **Step 4.2: Dynamic Induced Router**: 🔲 **Not Started**.
- **Step 4.3: OpenApiGenerator**: 🔲 **Not Started**.

### Phase 5: WebSocket PUB/SUB & Async Control
*Goal: Deliver real-time synchronization.*
- **Step 5.1: Wildcard Subscription Manager**: 🔲 **Not Started**.
- **Step 5.2: Throttled Delta Engine**: 🔲 **Not Started**.
- **Step 5.3: Acknowledged Async Set**: 🔲 **Not Started**.

### Phase 6: Frontend Development (Dashboard)
*Goal: Mission Control UI.*
- **Step 6.1: React/TS Scaffolding**: 🔲 **Not Started**.
- **Step 6.2: Recursive Tree Browser**: 🔲 **Not Started**.
- **Step 6.3: Dynamic Form Generator**: 🔲 **Not Started**.
- **Step 6.4: Dynamic API Proxy**: 🔲 **Not Started**.

## 3. Verification & Validation
- **Functional**: ✅ Baseline APIs and WebSocket Echo pass.
- **Industrial Stress**: 🔲 Pending Phase 5 completion.
- **Security Audit**: 🔲 Pending Phase 2 hardening validation.
- **Traceability**: ✅ All new methods follow Doxygen/CS-0030 standards.
