# qlsh - Quasar Lua Shell

## Description
`qlsh` is a professional-grade, interactive Lua interpreter (REPL) for the Quasar framework. It provides real-time system introspection, reflexive orchestration, and interactive system exploration.

## Usage
```bash
./bin/qlsh [options]
```

### Options
*   `--plugin <path>`: Load a dynamic plugin shared library before starting the shell.
*   `--log <path>`: Initialize the global data logger and write events to the specified CSV file.
*   `--help, -h`: Display help message.

## Features
*   **Intelligent Autocompletion**: Tab-completion for Lua globals and live `NamedObject` hierarchy paths (e.g., `root.EcatMaster.`).
*   **Multi-line Input**: Accumulates statements until complete. Use **Alt+Enter** to force a newline without executing.
*   **Syntax Highlighting**: Real-time colorization of Lua keywords, strings, numbers, and comments.
*   **Persistent History**: Commands are saved to `~/.quasar_qlsh_history` across sessions.
*   **Safe Interruption**: Use **Ctrl+C** to interrupt long-running Lua scripts without terminating the shell.

## Built-in Commands
*   `ls(object)`: Visualizes the `NamedObject` sub-tree using a high-fidelity recursive renderer.
*   `help(object)`: Displays a formatted table of reflexive fields and methods for the target object.
*   `exit()`: Terminate the shell session.

## Keyboard Shortcuts
| Key | Action |
| :--- | :--- |
| **TAB** | Trigger autocompletion or cycle through suggestions. |
| **Alt+Enter** | Insert a newline (multi-line statement). |
| **Ctrl+C** | Interrupt execution / clear current line. |
| **Ctrl+D** | Exit the shell. |
| **Up/Down Arrow** | Navigate through command history. |

## Examples
Exploring the tree:
```lua
ls(root)
help(root.Service1)
```

Invoking a reflexive method:
```lua
root.Service1:start()
```
