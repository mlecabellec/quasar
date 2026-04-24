import { Activity, Database, ChevronRight, Box, Layers, Cpu, Terminal, Shield, Zap, Search, Folder, FolderOpen, Info, Bell } from 'lucide-react';
import { useState, useEffect, useMemo } from 'react';
import { clsx, type ClassValue } from 'clsx';
import { twMerge } from 'tailwind-merge';

import { useWebSocket } from './hooks/useWebSocket';
import { useTree } from './hooks/useTree';
import type { QuasarNode } from './hooks/useTree';
import { getInspector } from './registry/NodeInspectorRegistry';
import { TelemetryGrid } from './components/TelemetryGrid';
import { TelemetryChart } from './components/TelemetryChart';
import { SystemOverview } from './components/SystemOverview';

/* ─── Utilities ─── */
function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs));
}

/* ─── Node Icon ─── */
const NodeIcon = ({ type, isExpanded }: { type: string; isExpanded?: boolean }) => {
  if (type === "NamedInteger" || type === "NamedFloatingPoint") return <Activity size={13} className="text-amber-400" />;
  if (type === "NamedBoolean") return <Shield size={13} className="text-cyan-400" />;
  if (type === "NamedString") return <Layers size={13} className="text-purple-400" />;
  if (type === "WebUIService") return <Box size={13} className="text-pink-400" />;
  if (type === "LogicEngine" || type === "StateMachine") return <Cpu size={13} className="text-emerald-400" />;
  return isExpanded ? <FolderOpen size={13} className="text-slate-500" /> : <Folder size={13} className="text-slate-600" />;
};

/* ─── Recursive Tree Item ─── */
const TreeItem = ({ node, depth, onSelect, onExpand, selectedPath }: {
  node: QuasarNode;
  depth: number;
  onSelect: (n: QuasarNode) => void;
  onExpand: (path: string) => void;
  selectedPath: string | null;
}) => {
  const isLeaf = ["NamedInteger", "NamedBoolean", "NamedFloatingPoint", "NamedString"].includes(node.type);
  const isSelected = selectedPath === node.path;

  return (
    <>
      <div
        onClick={() => onSelect(node)}
        style={{ paddingLeft: `${12 + depth * 16}px` }}
        className={cn(
          "group flex items-center gap-2 py-[5px] pr-3 cursor-pointer transition-colors duration-150 border-l-2",
          isSelected
            ? "bg-cyan-500/8 border-l-cyan-500 text-white"
            : "border-l-transparent hover:bg-white/[0.03] text-slate-400 hover:text-slate-200"
        )}
      >
        {/* Expand toggle or leaf indicator */}
        {!isLeaf ? (
          <button
            onClick={e => { e.stopPropagation(); onExpand(node.path); }}
            className="p-0.5 hover:bg-white/10 rounded transition-colors"
          >
            <ChevronRight size={11} className={cn(
              "text-slate-600 transition-transform duration-200",
              node.isExpanded && "rotate-90 text-cyan-500"
            )} />
          </button>
        ) : (
          <span className="w-4" />
        )}

        {/* Icon */}
        <NodeIcon type={node.type} isExpanded={node.isExpanded} />

        {/* Label */}
        <span className={cn(
          "text-[11px] truncate flex-1",
          !isLeaf ? "font-bold" : "font-normal",
          isSelected ? "text-cyan-300" : ""
        )}>
          {node.name}
        </span>

        {/* Value badge */}
        {node.value !== undefined && (
          <span className="text-[10px] font-mono text-amber-400/80 bg-amber-500/8 px-1.5 py-0.5 rounded shrink-0">
            {typeof node.value === 'number' ? node.value.toFixed(1) : String(node.value)}
          </span>
        )}
      </div>

      {/* Children */}
      {node.isExpanded && node.children && (
        <div className="relative">
          {/* Vertical guide line */}
          <div
            className="absolute top-0 bottom-0 border-l border-white/5"
            style={{ left: `${20 + depth * 16}px` }}
          />
          {node.children.map(child => (
            <TreeItem
              key={child.path}
              node={child}
              depth={depth + 1}
              onSelect={onSelect}
              onExpand={onExpand}
              selectedPath={selectedPath}
            />
          ))}
        </div>
      )}
    </>
  );
};

/* ══════════════════════════════════════════════════
   Main Application
   ══════════════════════════════════════════════════ */
export default function App() {
  const { connected, send, lastMessage } = useWebSocket();
  const { tree, expandNode, selectedNode, setSelectedNode, filter, setFilter } = useTree(lastMessage);
  const [logs, setLogs] = useState<{ id: string; msg: string; ts: string }[]>([]);

  /* Chart data buffer: keep last 50 points per stream */
  const [streams, setStreams] = useState<Record<string, { time: string; value: number }[]>>({});

  const Inspector = selectedNode ? getInspector(selectedNode.type) : null;

  const systemState = useMemo(() => {
    const find = (nodes: QuasarNode[], name: string): any => {
      for (const n of nodes) {
        if (n.name === name) return n.value;
        if (n.children) { const v = find(n.children, name); if (v !== undefined) return v; }
      }
      return undefined;
    };
    return find(tree, "systemState") || "STANDBY";
  }, [tree]);

  /* Log + stream accumulator */
  useEffect(() => {
    if (!lastMessage) return;
    const ts = new Date().toLocaleTimeString();

    setLogs(prev => [
      { id: Math.random().toString(36), msg: JSON.stringify(lastMessage).slice(0, 100), ts },
      ...prev.slice(0, 99)
    ]);

    if (lastMessage.action === "batch_update") {
      setStreams(prev => {
        const next = { ...prev };
        lastMessage.updates.forEach((u: any) => {
          if (typeof u.value === 'number') {
            if (!next[u.name]) next[u.name] = [];
            next[u.name] = [...next[u.name], { time: ts, value: u.value }].slice(-50);
          }
        });
        return next;
      });
    }
  }, [lastMessage]);

  const chartNames = Object.keys(streams).slice(0, 2);

  /* ─── Render ─── */
  return (
    <div className="h-screen flex flex-col bg-industrial-950 font-mono text-slate-300 overflow-hidden relative">
      <div className="crt-overlay" />
      <div className="scanline" />

      {/* ━━━ Top Bar ━━━ */}
      <header className="h-14 shrink-0 bg-industrial-900/80 backdrop-blur-md border-b border-white/5 flex items-center justify-between px-6 z-40">
        <div className="flex items-center gap-5">
          <div className="flex items-center gap-3">
            <Zap size={18} className="text-cyan-400" />
            <span className="text-sm font-bold text-white tracking-tight">Quasar Mission Control</span>
          </div>
          <span className="text-[9px] bg-cyan-500/10 text-cyan-400 px-2 py-0.5 rounded font-bold border border-cyan-500/20">v2.5</span>

          {/* Breadcrumb */}
          <div className="hidden md:flex items-center gap-1.5 text-[9px] text-slate-500 font-mono ml-4">
            <span>root</span>
            {selectedNode && selectedNode.path.split('/').filter(Boolean).map((seg, i) => (
              <span key={i} className="flex items-center gap-1.5">
                <ChevronRight size={9} className="text-slate-700" />
                <span className="text-slate-400">{seg}</span>
              </span>
            ))}
          </div>
        </div>

        <div className="flex items-center gap-4">
          <div className="hidden lg:flex items-center gap-5 text-[9px] font-mono mr-2">
            <span className="text-slate-500">Health <span className="text-emerald-400 ml-1">99.9%</span></span>
            <span className="text-slate-500">Load <span className="text-amber-400 ml-1">12%</span></span>
          </div>
          <button className="p-2 hover:bg-white/5 rounded-lg transition-colors relative">
            <Bell size={15} className="text-slate-500" />
            <div className="absolute top-1.5 right-1.5 w-1.5 h-1.5 bg-rose-500 rounded-full" />
          </button>
          <div className={cn(
            "px-3 py-1.5 rounded-lg text-[9px] font-bold uppercase tracking-wider border flex items-center gap-2",
            connected
              ? "bg-cyan-500/8 text-cyan-400 border-cyan-500/15"
              : "bg-red-500/8 text-red-400 border-red-500/15"
          )}>
            <div className={cn("w-1.5 h-1.5 rounded-full", connected ? "bg-cyan-400 animate-pulse" : "bg-red-500")} />
            {connected ? "Live" : "Offline"}
          </div>
        </div>
      </header>

      {/* ━━━ Main Content ━━━ */}
      <div className="flex-1 flex min-h-0">

        {/* ── Left: Registry Explorer ── */}
        <aside className="w-72 shrink-0 bg-industrial-900/40 border-r border-white/5 flex flex-col">
          {/* Search */}
          <div className="p-3 border-b border-white/5">
            <div className="flex items-center gap-2 bg-industrial-800/50 rounded-lg border border-white/5 px-3 py-2 focus-within:border-cyan-500/30 transition-colors">
              <Search size={12} className="text-slate-600 shrink-0" />
              <input
                type="text"
                placeholder="Search nodes..."
                value={filter}
                onChange={e => setFilter(e.target.value)}
                className="bg-transparent border-none focus:outline-none text-[10px] text-slate-300 w-full placeholder:text-slate-700"
              />
            </div>
          </div>

          {/* Section label */}
          <div className="px-4 py-2 flex items-center justify-between">
            <div className="flex items-center gap-2">
              <Database size={12} className="text-cyan-500" />
              <span className="text-[9px] font-bold text-slate-500 uppercase tracking-widest">Registry</span>
            </div>
            <span className="text-[9px] text-slate-600 font-mono">{tree.length}</span>
          </div>

          {/* Tree */}
          <div className="flex-1 overflow-y-auto custom-scrollbar">
            {tree.map(node => (
              <TreeItem
                key={node.path}
                node={node}
                depth={0}
                onSelect={setSelectedNode}
                onExpand={expandNode}
                selectedPath={selectedNode?.path || null}
              />
            ))}
          </div>
        </aside>

        {/* ── Center: Inspector / Charts ── */}
        <main className="flex-1 flex flex-col min-w-0">
          {/* Inspector area */}
          <div className="flex-1 overflow-y-auto custom-scrollbar">
            {selectedNode && Inspector ? (
              <div className="p-8 animate-zoom-in">
                {/* Inspector header */}
                <div className="flex items-center justify-between mb-8">
                  <div className="flex items-center gap-4">
                    <div className="p-3 bg-industrial-800/50 rounded-xl border border-white/5">
                      <NodeIcon type={selectedNode.type} isExpanded />
                    </div>
                    <div>
                      <h2 className="text-xl font-bold text-white tracking-tight">{selectedNode.name}</h2>
                      <span className="text-[9px] text-slate-500 font-mono">{selectedNode.path}</span>
                    </div>
                  </div>
                  <button className="px-4 py-2 bg-cyan-500 text-industrial-950 rounded-lg text-[10px] font-bold uppercase tracking-wider hover:bg-cyan-400 active:scale-95 transition-all">
                    Execute
                  </button>
                </div>

                {/* Inspector content */}
                <div className="glass-panel rounded-2xl p-8">
                  <Inspector
                    path={selectedNode.path}
                    name={selectedNode.name}
                    type={selectedNode.type}
                    value={selectedNode.value}
                    ws={{ send }}
                  />
                </div>
              </div>
            ) : (
              <div className="h-full flex flex-col items-center justify-center text-center p-16 opacity-40">
                <Cpu size={64} className="text-slate-800 mb-6" />
                <h3 className="text-lg font-bold text-slate-500 tracking-tight mb-2">No Node Selected</h3>
                <p className="text-[10px] text-slate-600 max-w-xs leading-relaxed">
                  Select a node from the registry to inspect its properties and control its state.
                </p>
              </div>
            )}
          </div>

          {/* Chart strip (2 charts side by side) */}
          <div className="h-44 shrink-0 border-t border-white/5 bg-industrial-900/30 grid grid-cols-2 gap-px">
            {chartNames.length > 0 ? chartNames.map(name => (
              <div key={name} className="p-4">
                <TelemetryChart label={name} data={streams[name]} />
              </div>
            )) : (
              <div className="col-span-2 flex items-center justify-center text-[10px] text-slate-700 italic">
                Awaiting telemetry streams...
              </div>
            )}
          </div>
        </main>

        {/* ── Right: Diagnostics + Logs + Telemetry Table ── */}
        <aside className="w-80 shrink-0 bg-industrial-900/30 border-l border-white/5 flex flex-col">
          {/* System Diagnostics */}
          <div className="p-5 border-b border-white/5">
            <div className="flex items-center gap-2 mb-4">
              <Info size={12} className="text-cyan-500" />
              <span className="text-[9px] font-bold text-slate-500 uppercase tracking-widest">Diagnostics</span>
            </div>
            <SystemOverview currentState={systemState} health={connected ? 99.8 : 0} />
          </div>

          {/* Event Log */}
          <div className="flex-1 flex flex-col min-h-0 p-5 border-b border-white/5">
            <div className="flex items-center justify-between mb-3">
              <div className="flex items-center gap-2">
                <Terminal size={12} className="text-amber-500" />
                <span className="text-[9px] font-bold text-slate-500 uppercase tracking-widest">Event Log</span>
              </div>
              <button
                onClick={() => setLogs([])}
                className="text-[8px] font-bold text-slate-600 hover:text-slate-400 transition-colors uppercase"
              >
                Clear
              </button>
            </div>
            <div className="flex-1 overflow-y-auto custom-scrollbar space-y-1 font-mono text-[9px]">
              {logs.length > 0 ? logs.slice(0, 30).map(log => (
                <div key={log.id} className="py-1 hover:bg-white/[0.02] rounded px-1 transition-colors">
                  <span className="text-slate-700 mr-2">{log.ts}</span>
                  <span className="text-slate-500 break-all">{log.msg}</span>
                </div>
              )) : (
                <div className="h-full flex items-center justify-center text-slate-700 italic">Awaiting events...</div>
              )}
            </div>
          </div>

          {/* Telemetry Data Table */}
          <div className="h-64 shrink-0 p-4">
            <TelemetryGrid lastMessage={lastMessage} />
          </div>
        </aside>
      </div>
    </div>
  );
}
