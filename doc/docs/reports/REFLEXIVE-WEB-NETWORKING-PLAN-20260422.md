# Implementation Plan: Reflexive Web API & Networking (TSK-002, TSK-003, TSK-008)

**Date:** 2026-04-23  
**Status:** ✅ **Baseline Implementation Complete**  
**Objectives:** Deliver high-performance networking, a scalable discovery API, a strictly consistent induced API, and a real-time PUB/SUB dashboard.

## 1. Compliance Baseline
All work must strictly adhere to the following mandates:
- **CS-0010**: C++23 standards, NO `auto`, NO `new`, mandatory Doxygen, hard loop limits.
- **CS-0020**: Advanced safety, `std::expected` for errors, `[[nodiscard]]`, zero-copy views.
- **CS-0030**: Feature traceability (`@feature`, `@exposed`), mandatory stress testing (worst-case scenarios).

## 2. Phased Implementation

### Phase 1: The Typed Foundation (Core `named` & `datacodec`)
*Goal: Enable formal signatures without altering NamedObject base performance.*
- **Step 1.1: `TypedNamedMethod`**: ✅ **Completed**.
- **Step 1.2: `JsonMapper` (Streaming)**: ✅ **Completed**. Integrated `jsoncons` into `WebUIService`.
- **Step 1.3: Performance Isolation Proof**: 🔲 **Pending**.

### Phase 2: Async Networking & SSL (Plugin `net`)
*Goal: Finalize the communication stack.*
- **Step 2.1: WebSocket Core**: ✅ **Completed**. PIMPL isolation and handshake resolved.
- **Step 2.2: SSL/TLS Hardening**: ✅ **Completed**. `SSLContextWrapper` integrated.
- **Step 2.3: Reconnection State Machine**: ✅ **Completed**. Exponential backoff in client wrappers.

### Phase 3: Base API & Tree Discovery (Service `webui`)
*Goal: Provide a scalable Discovery interface.*
- **Step 3.1: `WebUIService`**: ✅ **Completed**.
- **Step 3.2: Stateless Tree Walker**: ✅ **Completed**. `GET /api/v1/walk` supports chunking.
- **Step 3.3: Metadata Registry**: ✅ **Completed**. `GET /api/v1/meta/{path}` implemented.

### Phase 4: Induced API & OpenAPI (Plugin `net`)
*Goal: Direct method mapping and self-documentation.*
- **Step 4.1: `WebNamedMethod`**: ✅ **Completed**. Metadata for HTTP verbs/aliases added.
- **Step 4.2: Dynamic Induced Router**: ✅ **Completed**. Recursive resolver implemented in `WebUIService`.
- **Step 4.3: OpenApiGenerator**: ✅ **Completed**. `/openapi.json` derived from hierarchy.

### Phase 5: WebSocket PUB/SUB & Async Control
*Goal: Deliver real-time synchronization.*
- **Step 5.1: Wildcard Subscription Manager**: ✅ **Completed**. Thread-safe registry in `WebUIService`.
- **Step 5.2: Throttled Delta Engine**: ✅ **Completed**. Background worker thread with batching.
- **Step 5.3: Acknowledged Async Set**: ✅ **Completed**. Closed-loop JSON command protocol.

### Phase 6: Frontend Development (Dashboard)
*Goal: Mission Control UI.*
- **Step 6.1: React/TS Scaffolding**: ✅ **Completed**. Integrated with CMake build chain.
- **Step 6.2: Recursive Tree Browser**: ✅ **Completed**. Lazy-loading React components.
- **Step 6.3: Dynamic Form Generator**: ✅ **Completed**. Registry-based node inspectors.
- **Step 6.4: Dynamic API Proxy**: ✅ **Completed**. Vite proxy configured for dev mode.

## 3. Verification & Validation
- **Functional**: ✅ Baseline APIs and WebSocket Echo pass.
- **Industrial Stress**: ✅ Valgrind audit confirmed 0 leaks during stress tests.
- **Security Audit**: 🔲 Pending detailed SSL handshake penalty analysis.
- **Traceability**: ✅ All methods follow Doxygen/CS-0030 standards.
