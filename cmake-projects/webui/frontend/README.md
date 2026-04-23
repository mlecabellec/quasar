# Quasar Mission Control Frontend

This is the modern web-based dashboard for the Quasar framework, built with **React**, **TypeScript**, **Vite**, and **TailwindCSS**. It provides real-time visualization of the system hierarchy, telemetry streaming, and reflexive command execution.

## 🏗️ Architecture

The frontend is designed for high-performance industrial monitoring:
- **Registry Pattern**: Specialized renderers for different `NamedObject` types are registered in `src/registry/NodeInspectorRegistry.tsx`.
- **Throttled Updates**: WebSocket deltas are batched and decimated in the backend, and reactively merged into the tree state via the `useTree` hook.
- **Utility-First Styling**: TailwindCSS is used for a consistent, responsive, and "Mission Control" aesthetic.

## 🛠️ Build Instructions

### Prerequisites
- **Node.js** (v18 or higher recommended)
- **npm** (v9 or higher)

### Manual Build
To build the frontend independently of the C++ project:
```bash
cd cmake-projects/webui/frontend
npm install
npm run build
```
The output will be generated in the `dist/` directory.

### Integrated Build (CMake)
The frontend is fully integrated into the Quasar build chain. Building the `quasar_webui` target will automatically trigger the frontend build:
```bash
mkdir build && cd build
cmake ..
make quasar_webui
```

## 🚀 Development Mode

To start the development server with Hot Module Replacement (HMR):
```bash
cd cmake-projects/webui/frontend
npm run dev
```
Note: You will need the Quasar backend running on your local machine for the API/WebSocket links to function.

## 🛡️ Standards compliance

This code follows the Quasar Frontend Standards:
- **TypeScript**: Mandatory for all components and hooks.
- **Tailwind**: Used for all styling (avoid raw CSS where possible).
- **Lucide**: Standard icon library.
- **Atomic Components**: Keep components small and specialized.

## 🛰️ Roadmap Placeholders

The UI includes placeholders for upcoming 2026 modules:
- **Dynamic Synoptics**: SVG-based live plant visualization.
- **Visual State Machine**: Real-time HSM/SFC flow tracing.
- **Visual Logic Editor**: Block-based reflexive logic configuration.

---
*Maintained by Quasar Engineering Agent.*
