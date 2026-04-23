import { Activity, Database, ChevronRight, ChevronDown, Box, AlertCircle, RefreshCw } from 'lucide-react';
import { clsx, type ClassValue } from 'clsx';
import { twMerge } from 'tailwind-merge';

import { useWebSocket } from './hooks/useWebSocket';
import { useTree } from './hooks/useTree';
import type { QuasarNode } from './hooks/useTree';
import { getInspector } from './registry/NodeInspectorRegistry';
import { TelemetryGrid } from './components/TelemetryGrid';

// --- Utilities ---
function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs));
}

// --- Components ---

const TreeItem = ({ node, onSelect, onExpand, isSelected }: { 
  node: QuasarNode; 
  onSelect: (n: QuasarNode) => void;
  onExpand: (path: string) => void;
  isSelected: boolean;
}) => {
  return (
    <div className="space-y-1">
      <div 
        onClick={() => onSelect(node)}
        className={cn(
          "group flex items-center gap-2 p-2 rounded-lg cursor-pointer transition-all border",
          isSelected ? "bg-cyan-500/20 border-cyan-500/30 text-cyan-400" : "hover:bg-industrial-800/50 border-transparent text-slate-300"
        )}
      >
        <button 
          onClick={(e) => { e.stopPropagation(); onExpand(node.path); }}
          className="hover:text-cyan-400 transition-colors"
        >
          {node.isExpanded ? <ChevronDown size={14} /> : <ChevronRight size={14} />}
        </button>
        <div className="flex flex-col flex-1 overflow-hidden">
          <span className="text-sm font-bold truncate tracking-tight">{node.name}</span>
          <span className="text-[9px] text-slate-500 uppercase tracking-widest">{node.type}</span>
        </div>
      </div>
      
      {node.isExpanded && node.children && (
        <div className="ml-4 border-l border-industrial-700/50 pl-2 space-y-1">
          {node.children.map(child => (
            <TreeItem 
              key={child.path} 
              node={child} 
              onSelect={onSelect} 
              onExpand={onExpand}
              isSelected={isSelected}
            />
          ))}
        </div>
      )}
    </div>
  );
};

export default function App() {
  const { connected, send, lastMessage } = useWebSocket();
  const { tree, expandNode, selectedNode, setSelectedNode } = useTree(lastMessage);

  const Inspector = selectedNode ? getInspector(selectedNode.type) : null;

  return (
    <div className="min-h-screen bg-industrial-950 flex flex-col p-6 gap-6 font-mono selection:bg-cyan-500/30">
      {/* Header */}
      <header className="flex items-center justify-between border-b border-industrial-700/50 pb-6">
        <div className="flex items-center gap-4">
          <div className="p-3 bg-industrial-900 border border-industrial-700 shadow-inner rounded-xl">
            <Box size={24} className={cn("transition-colors", connected ? "text-cyan-400" : "text-slate-700")} />
          </div>
          <div>
            <h1 className="text-2xl font-black text-white tracking-tighter uppercase italic leading-none">Quasar Console</h1>
            <p className="text-slate-500 text-[10px] uppercase tracking-[0.2em] mt-1 italic">
              {connected ? "LINK_ESTABLISHED // STREAMING_ACTIVE" : "LINK_LOST // RECONNECTING..."}
            </p>
          </div>
        </div>
        
        <div className="flex items-center gap-3">
           <div className={cn(
             "px-4 py-1.5 rounded-full text-[10px] font-black uppercase tracking-tighter border flex items-center gap-2",
             connected ? "bg-cyan-500/10 text-cyan-400 border-cyan-500/20" : "bg-red-500/10 text-red-400 border-red-500/20"
           )}>
             <Activity size={14} className={connected ? "animate-pulse" : ""} />
             Live Environment
           </div>
        </div>
      </header>

      {/* Main Grid */}
      <main className="flex-1 grid grid-cols-12 gap-6 min-h-0">
        {/* Sidebar: Tree */}
        <aside className="col-span-3 bg-industrial-900/30 border border-industrial-700/50 rounded-2xl flex flex-col min-h-0">
          <div className="p-4 border-b border-industrial-700/30 flex items-center justify-between">
            <div className="flex items-center gap-2">
              <Database size={14} className="text-cyan-500" />
              <span className="text-xs font-black uppercase text-slate-400 tracking-wider">System Registry</span>
            </div>
            <RefreshCw size={12} className="text-slate-600 hover:text-cyan-500 cursor-pointer transition-colors" />
          </div>
          <div className="flex-1 overflow-y-auto p-4 custom-scrollbar">
            {tree.map(node => (
              <TreeItem 
                key={node.path} 
                node={node} 
                onSelect={setSelectedNode} 
                onExpand={expandNode}
                isSelected={selectedNode?.path === node.path}
              />
            ))}
          </div>
        </aside>

        {/* Central Stage: Inspector & Telemetry */}
        <section className="col-span-9 flex flex-col gap-6 min-h-0">
          <div className="flex-1 bg-industrial-900/50 border border-industrial-700/50 rounded-3xl overflow-hidden flex flex-col relative shadow-2xl">
            <div className="absolute top-4 left-4 z-10 flex gap-2">
              <div className="px-3 py-1 bg-industrial-800/80 backdrop-blur rounded border border-industrial-700/50 text-[10px] font-bold text-slate-400 uppercase">
                Viewport: {selectedNode ? selectedNode.name : "Registry"}
              </div>
            </div>

            {selectedNode && Inspector ? (
              <div className="flex-1 p-12">
                <Inspector 
                  path={selectedNode.path} 
                  name={selectedNode.name} 
                  type={selectedNode.type} 
                  value={selectedNode.value}
                  ws={{ send }}
                />
              </div>
            ) : (
              <div className="flex-1 flex flex-col items-center justify-center text-center p-12">
                 <div className="p-8 bg-industrial-800/50 rounded-full border border-industrial-700/30 mb-6 group hover:border-cyan-500/30 transition-all">
                   <AlertCircle size={64} className="text-slate-700 group-hover:text-cyan-500/50 transition-all" />
                 </div>
                 <h2 className="text-xl font-black text-slate-400 uppercase tracking-tighter italic">Ready for Discovery</h2>
                 <p className="text-slate-600 text-sm max-w-sm mt-2 font-medium">
                   Select an entity from the registry to initialize real-time instrumentation and control.
                 </p>
              </div>
            )}

            <div className="h-10 bg-industrial-950/80 border-t border-industrial-700/30 px-4 flex items-center justify-between text-[9px] text-slate-500 font-bold uppercase tracking-widest">
              <span>Telemetry Node: {selectedNode ? selectedNode.path : "N/A"}</span>
              <div className="flex gap-4">
                <span>Buffer: 50 Samples</span>
                <span>Interval: 50ms</span>
              </div>
            </div>
          </div>

          {/* Real-time Telemetry Dashboard Area */}
          <div className="h-64 bg-industrial-900/10 border border-industrial-700/50 rounded-3xl p-6">
             <TelemetryGrid lastMessage={lastMessage} />
          </div>
        </section>
      </main>

      <footer className="text-[9px] text-slate-600 font-bold flex justify-between uppercase tracking-[0.3em] opacity-40">
        <span>COLONY SEC-4 // INDUSTRIAL INFRASTRUCTURE MANAGEMENT</span>
        <span>© 2026 QUASAR CORE // DETERMINISTIC_REFLX_OS</span>
      </footer>
    </div>
  );
}
