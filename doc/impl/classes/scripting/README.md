# Scripting Classes

This directory contains the implementation details of the Lua scripting engine and its management layer.

## Core Environment
- [LuaEngine](LuaEngine.md): Secure wrapper for the Lua state and global `ScriptManager`.
- [PluginLoader](PluginLoader.md): Dynamic library loading and component registration system.

## Operational Scripts
- [LuaService](LuaService.md): Persistent background script execution via `ScriptComponent` lifecycle.
- [ScriptableNamedObject](ScriptableNamedObject.md): Hybrid C++/Lua objects with script-driven method overrides.
