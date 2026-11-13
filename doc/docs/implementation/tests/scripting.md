# Scripting Module Tests

This document describes the testing suite for the `scripting` module, located in `cmake-projects/scripting/test/`.

## 1. Core Engine Tests (`TestLuaEngine.cpp`)
- **State Integrity**: Verifies that each `LuaEngine` instance is isolated and correctly initializes standard libraries.
- **Execution Safety**: Validates that `executeString` handles syntax errors and runtime exceptions without crashing the host process.
- **GC Determinism**: Measures the overhead of `gcStep` and verifies that memory is reclaimed correctly.

## 2. Service Lifecycle (`TestLuaService.cpp`)
- **Worker Isolation**: Verifies that each `LuaService` runs in its own thread and does not block others.
- **Task Posting**: Validates the thread-safe `postTask` mechanism for C++ to Lua communication.
- **State Persistence**: Ensures that Lua variables are preserved across multiple `onUpdate` cycles.

## 3. Bindings and Interop (`TestTypeBindings.cpp`)
- **NamedObject Bridge**: Verifies that all `coretypes` and `named` objects can be passed to and from Lua with full type fidelity.
- **Method Overrides**: Tests `ScriptableNamedObject` by overriding C++ virtual methods in Lua and verifying they are called correctly from C++.
- **Asynchronous Results**: Validates `LuaFuture` and the ability to return values from background script tasks.

## 4. Plugin System (`TestPluginLoading.cpp`)
- **Dynamic Registration**: Verifies that `PluginLoader` correctly loads shared libraries and registers new component types.
- **Hot-Reloading**: (Manual/Planned) Verification of system stability when plugins are swapped at runtime.

<!-- CS-0100-CATALOGUE-START -->
## Registered Test Case Pages (`CS-0100.22`)

| Test Case | Documentation Page |
| :--- | :--- |
| `ExtendableClassesTest.MethodOverride` | [ExtendableClassesTest.MethodOverride](scripting/ExtendableClassesTest.MethodOverride.md) |
| `ExtendableClassesTest.HookAddChild` | [ExtendableClassesTest.HookAddChild](scripting/ExtendableClassesTest.HookAddChild.md) |
| `IndustrialStressTest.HighConcurrencyLockContention` | [IndustrialStressTest.HighConcurrencyLockContention](scripting/IndustrialStressTest.HighConcurrencyLockContention.md) |
| `LuaEngineTest.BasicExecution` | [LuaEngineTest.BasicExecution](scripting/LuaEngineTest.BasicExecution.md) |
| `LuaEngineTest.FunctionDefinition` | [LuaEngineTest.FunctionDefinition](scripting/LuaEngineTest.FunctionDefinition.md) |
| `LuaEngineTest.SandboxExecution` | [LuaEngineTest.SandboxExecution](scripting/LuaEngineTest.SandboxExecution.md) |
| `LuaEngineTest.PanicHandling` | [LuaEngineTest.PanicHandling](scripting/LuaEngineTest.PanicHandling.md) |
| `LuaServiceTest.BasicLifecycle` | [LuaServiceTest.BasicLifecycle](scripting/LuaServiceTest.BasicLifecycle.md) |
| `LuaServiceTest.ScriptExecution` | [LuaServiceTest.ScriptExecution](scripting/LuaServiceTest.ScriptExecution.md) |
| `LuaServiceTest.ObjectTracking` | [LuaServiceTest.ObjectTracking](scripting/LuaServiceTest.ObjectTracking.md) |
| `LuaServiceTest.ErrorHandling` | [LuaServiceTest.ErrorHandling](scripting/LuaServiceTest.ErrorHandling.md) |
| `LuaServiceTest.WorstCaseTermination` | [LuaServiceTest.WorstCaseTermination](scripting/LuaServiceTest.WorstCaseTermination.md) |
| `NamedLuaMethodTest.Execution` | [NamedLuaMethodTest.Execution](scripting/NamedLuaMethodTest.Execution.md) |
| `NamedLuaMethodTest.Invalidation` | [NamedLuaMethodTest.Invalidation](scripting/NamedLuaMethodTest.Invalidation.md) |
| `QlshTest.BasicLuaExecution` | [QlshTest.BasicLuaExecution](scripting/QlshTest.BasicLuaExecution.md) |
| `QlshTest.NamedObjectInteraction` | [QlshTest.NamedObjectInteraction](scripting/QlshTest.NamedObjectInteraction.md) |
| `QlshTest.ZmqPluginInteraction` | [QlshTest.ZmqPluginInteraction](scripting/QlshTest.ZmqPluginInteraction.md) |
| `QlshTest.MultiLineAccumulation` | [QlshTest.MultiLineAccumulation](scripting/QlshTest.MultiLineAccumulation.md) |
| `QlshTest.ShellUtilities` | [QlshTest.ShellUtilities](scripting/QlshTest.ShellUtilities.md) |
| `QlshTest.NamedServiceInShell` | [QlshTest.NamedServiceInShell](scripting/QlshTest.NamedServiceInShell.md) |
| `QlshTest.VariablePersistence` | [QlshTest.VariablePersistence](scripting/QlshTest.VariablePersistence.md) |
| `QlshTest.ExitCommand` | [QlshTest.ExitCommand](scripting/QlshTest.ExitCommand.md) |
| `QlshTest.UserReportedSequenceFix` | [QlshTest.UserReportedSequenceFix](scripting/QlshTest.UserReportedSequenceFix.md) |
| `QlshTest.MultipleGlobalPersistence` | [QlshTest.MultipleGlobalPersistence](scripting/QlshTest.MultipleGlobalPersistence.md) |
| `QlshTest.EvaluateAndPrint` | [QlshTest.EvaluateAndPrint](scripting/QlshTest.EvaluateAndPrint.md) |
| `QlshTest.TableManagement` | [QlshTest.TableManagement](scripting/QlshTest.TableManagement.md) |
| `QlshTest.LexicalScopingCompliance` | [QlshTest.LexicalScopingCompliance](scripting/QlshTest.LexicalScopingCompliance.md) |
| `QlshTest.HighVolumeStressTest` | [QlshTest.HighVolumeStressTest](scripting/QlshTest.HighVolumeStressTest.md) |
| `ScriptExecutorTest.ExecuteOnce` | [ScriptExecutorTest.ExecuteOnce](scripting/ScriptExecutorTest.ExecuteOnce.md) |
| `ScriptExecutorTest.ExecuteSync` | [ScriptExecutorTest.ExecuteSync](scripting/ScriptExecutorTest.ExecuteSync.md) |
| `ScriptExecutorTest.ExecuteAsync` | [ScriptExecutorTest.ExecuteAsync](scripting/ScriptExecutorTest.ExecuteAsync.md) |
| `ScriptManagerTest.MemoryStress` | [ScriptManagerTest.MemoryStress](scripting/ScriptManagerTest.MemoryStress.md) |
| `ScriptManagerTest.SandboxIsolation` | [ScriptManagerTest.SandboxIsolation](scripting/ScriptManagerTest.SandboxIsolation.md) |
| `TypeBindingsTest.IntegerPrecision` | [TypeBindingsTest.IntegerPrecision](scripting/TypeBindingsTest.IntegerPrecision.md) |
| `TypeBindingsTest.UnsignedIntegerPrecision` | [TypeBindingsTest.UnsignedIntegerPrecision](scripting/TypeBindingsTest.UnsignedIntegerPrecision.md) |
| `TypeBindingsTest.QuantityMath` | [TypeBindingsTest.QuantityMath](scripting/TypeBindingsTest.QuantityMath.md) |
| `TypeBindingsTest.HierarchyResolution` | [TypeBindingsTest.HierarchyResolution](scripting/TypeBindingsTest.HierarchyResolution.md) |
| `TypeBindingsTest.VariantHandling` | [TypeBindingsTest.VariantHandling](scripting/TypeBindingsTest.VariantHandling.md) |
| `TypeBindingsTest.CppVariantHandling` | [TypeBindingsTest.CppVariantHandling](scripting/TypeBindingsTest.CppVariantHandling.md) |
| `TypeBindingsTest.IsolatedCreation` | [TypeBindingsTest.IsolatedCreation](scripting/TypeBindingsTest.IsolatedCreation.md) |
| `ScriptingTutorialTest.Tutorial1_LuaEngineAndScriptExecutorModes` | [ScriptingTutorialTest.Tutorial1_LuaEngineAndScriptExecutorModes](scripting/ScriptingTutorialTest.Tutorial1_LuaEngineAndScriptExecutorModes.md) |
| `ScriptingTutorialTest.Tutorial2_LuaFutureQueuedObserverAndTracker` | [ScriptingTutorialTest.Tutorial2_LuaFutureQueuedObserverAndTracker](scripting/ScriptingTutorialTest.Tutorial2_LuaFutureQueuedObserverAndTracker.md) |
| `ScriptingTutorialTest.Tutorial3_ShellUtilitiesAndPluginBoundaries` | [ScriptingTutorialTest.Tutorial3_ShellUtilitiesAndPluginBoundaries](scripting/ScriptingTutorialTest.Tutorial3_ShellUtilitiesAndPluginBoundaries.md) |
| `ScriptingCoverageTest.LuaErrorHandlingAndObserverInvalidation` | [ScriptingCoverageTest.LuaErrorHandlingAndObserverInvalidation](scripting/ScriptingCoverageTest.LuaErrorHandlingAndObserverInvalidation.md) |
| `ScriptingFuzzStressTest.DeterministicLuaFuzzAndConcurrentServiceStress` | [ScriptingFuzzStressTest.DeterministicLuaFuzzAndConcurrentServiceStress](scripting/ScriptingFuzzStressTest.DeterministicLuaFuzzAndConcurrentServiceStress.md) |
| `ZmqPluginTest` | [ZmqPluginTest](scripting/ZmqPluginTest.md) |
