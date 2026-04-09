# 🌌 Quasar Lua Script Gallery

This directory contains a collection of Lua scripts demonstrating the capabilities of the Quasar project. These scripts are intended for testing, demonstration, and as pedagogical support for new users.

## 🚀 How to Run

Use the Quasar Script Runner (`sre`) located in your build directory:

```bash
./build/bin/sre your_script.lua
```

If the script requires plugins (like ZMQ or OPC UA), use the `--plugin` flag:

```bash
./build/bin/sre --plugin ./build/lib/quasar_zmq.so your_script.lua
```

## 📜 Script Index

| Script | Purpose | Features Tested |
| :--- | :--- | :--- |
| `01_hello_world.lua` | The classic entry point. | Basic execution |
| `02_named_object_basics.lua` | Creating and organizing the tree. | `NamedObject`, Hierarchy |
| `03_named_primitives.lua` | Storing values in the tree. | `NamedLong`, `NamedFloatingPoint`, etc. |
| `04_named_containers.lua` | Managing collections of objects. | `NamedArray`, `NamedMap` |
| `05_named_buffer.lua` | Working with raw binary data. | `NamedBuffer`, `write`, `read` |
| `06_traversal_basics.lua` | Navigating the object tree. | `getChild`, `getChildren`, `find` |
| `07_named_method_simple.lua` | Defining custom logic hooks. | `NamedLuaMethod`, `execute` |
| `08_named_service_lifecycle.lua` | Running background tasks. | `NamedService`, `onStart`, `run`, `onStop` |
| `09_named_service_params.lua` | Passing data to methods. | `NamedMethod` arguments |
| `10_tree_serialization.lua` | Saving and loading tree states. | `Serialization` (if available) |
| `11_logic_expression.lua` | Boolean algebra in action. | `LogicEngine`, `Expression` |
| `12_state_machine_simple.lua` | Modeling behavior with FSMs. | `StateMachine`, `State`, `Transition` |
| `13_sfc_simple.lua` | Sequential control flows. | `SFC`, Steps, Transitions |
| `14_cause_effect_matrix.lua` | Tabular logic mapping. | `CauseEffectMatrix` |
| `15_logic_engine_integration.lua` | Connecting logic to data. | `LogicEngine` + `NamedObject` |
| `16_zmq_pub_sub.lua` | Distributed tree updates. | `ZMQ` Plugin, `PUB/SUB`, `publishTree` |
| `17_zmq_req_rep.lua` | Synchronous communication. | `ZMQ` Plugin, `REQ/REP` |
| `18_opcua_server.lua` | Exposing data via OPC UA. | `OPC UA` Plugin, `ServerService` |
| `19_opcua_client.lua` | Consuming data from OPC UA. | `OPC UA` Plugin, `ClientService` |
| `20_net_plugin_basics.lua` | Raw networking. | `Net` Plugin, TCP/UDP |
| `21_calibration_linear.lua` | Simple sensor scaling. | `Calibration`, Linear transform |
| `22_calibration_polynomial.lua` | Non-linear corrections. | `Calibration`, Polynomial |
| `23_data_logger_simple.lua` | Recording history. | `DataLogger`, RingBuffer |
| `24_csv_exporter.lua` | Exporting for analysis. | `DataLogger`, `CsvFileWriter` |
| `25_datacodec_binary_mapping.lua` | Parsing binary protocols. | `DataCodec`, `BinaryMapper` |
| `26_lua_plugin_loading.lua` | Extending the environment. | `PluginLoader`, `PluginContract` |
| `27_error_handling.lua` | Robust script design. | `pcall`, Error propagation |
| `28_performance_stress.lua` | Pushing the limits. | Stress testing object creation |
| `29_concurrency_demo.lua` | Multiple threads, one tree. | Mutexes, Thread safety |
| `30_the_ultimate_answer.lua` | The grand finale. | ZMQ, Logic, NamedTree, Calibration |

---
"Don't Panic." - *The Hitchhiker's Guide to the Galaxy*
