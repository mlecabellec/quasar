import { Activity, Database, ChevronRight, ChevronDown, Box, AlertCircle, RefreshCw, Layers, Cpu } from 'lucide-react';
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

const NodeIcon = ({ type, isExpanded }: { type: string, isExpanded?: boolean }) => {
  if (type === "NamedInteger" || type === "NamedFloatingPoint") return <Activity size={14} className="text-amber-400" />;
  if (type === "NamedBoolean") return <ShieldCheck size={14} className="text-cyan-400" />;
  if (type === "NamedString") return <Layers size={14} className="text-purple-400" />;
  if (type === "WebUIService") return <Box size={14} className="text-pink-400" />;
  if (type === "LogicEngine" || type === "StateMachine") return <Cpu size={14} className="text-green-400" />;
  return isExpanded ? <ChevronDown size={14} className="text-slate-500" /> : <ChevronRight size={14} className="text-slate-500" />;
};

const TreeItem = ({ node, onSelect, onExpand, isSelected }: { 
  node: QuasarNode; 
  onSelect: (n: QuasarNode) => void;
  onExpand: (path: string) => void;
  isSelected: boolean;
}) => {
  const isLeaf = ["NamedInteger", "NamedBoolean", "NamedFloatingPoint", "NamedString"].includes(node.type);
  
  return (
    <div className="space-y-0.5 select-none">
      <div 
        onClick={() => onSelect(node)}
        className={cn(
          "group flex items-center gap-2 px-3 py-1.5 rounded-md cursor-pointer transition-all duration-200 border",
          isSelected 
            ? "bg-cyan-500/10 border-cyan-500/40 text-cyan-400 shadow-[0_0_15px_rgba(0,242,255,0.1)]" 
            : "hover:bg-industrial-800/40 border-transparent text-slate-400 hover:text-slate-100"
        )}
      >
        {!isLeaf ? (
          <button 
            onClick={(e) => { e.stopPropagation(); onExpand(node.path); }}
            className="p-0.5 hover:bg-industrial-700/50 rounded transition-colors"
          >
            {node.isExpanded ? <ChevronDown size={12} className="text-cyan-500" /> : <ChevronRight size={12} className="text-slate-600" />}
          </button>
        ) : (
          <div className="w-4 h-4 flex items-center justify-center">
            <div className="w-1 h-1 rounded-full bg-slate-700 group-hover:bg-cyan-500/50 transition-colors" />
          </div>
        )}
        
        <NodeIcon type={node.type} />
        
        <div className="flex items-baseline gap-2 flex-1 overflow-hidden">
          <span className="text-[13px] font-bold truncate tracking-tight">{node.name}</span>
          <span className="text-[8px] text-slate-600 font-black uppercase tracking-widest opacity-0 group-hover:opacity-100 transition-opacity whitespace-nowrap">
            // {node.type}
          </span>
        </div>

        {node.value !== undefined && (
          <span className="text-[11px] font-mono text-amber-500/80 bg-black/20 px-1.5 py-0.5 rounded border border-white/5">
            {typeof node.value === 'number' ? node.value.toFixed(2) : String(node.value)}
          </span>
        )}
      </div>
      
      {node.isExpanded && node.children && (
        <div className="ml-4 border-l border-industrial-700/30 pl-2 mt-0.5">
          {node.children.map(child => (
            <TreeItem 
              key={child.path} 
              node={child} 
              onSelect={onSelect} 
              onExpand={onExpand}
              isSelected={isSelected && child.path === node.path} // Placeholder for recursive selection
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
    <div className="min-h-screen bg-industrial-950 flex flex-col p-4 gap-4 font-mono selection:bg-cyan-500/30">
      {/* Dynamic Header */}
      <header className="flex items-center justify-between bg-industrial-900/40 border border-industrial-700/30 rounded-2xl px-6 py-4 backdrop-blur-md shadow-2xl">
        <div className="flex items-center gap-5">
          <div className="relative">
            <div className={cn(
              "absolute inset-0 rounded-xl blur-lg transition-colors duration-1000",
              connected ? "bg-cyan-500/20" : "bg-red-500/20"
            )} />
            <div className="relative p-3 bg-industrial-950 border border-industrial-700 shadow-inner rounded-xl">
              <Box size={24} className={cn("transition-all duration-500", connected ? "text-cyan-400 drop-shadow-[0_0_8px_rgba(0,242,255,0.5)]" : "text-slate-700")} />
            </div>
          </div>
          <div>
            <div className="flex items-center gap-2">
              <h1 className="text-xl font-black text-white tracking-tighter uppercase italic leading-none">Quasar_Mission_Control</h1>
              <span className="text-[10px] bg-cyan-500/10 text-cyan-500 px-1.5 py-0.5 rounded font-black border border-cyan-500/20">V2.4</span>
            </div>
            <p className="text-slate-500 text-[9px] uppercase tracking-[0.3em] mt-1 font-bold">
              {connected ? "SYS_ACTIVE // SYNC_LOCK_OK" : "SYS_CRITICAL // LINK_INTERRUPTED"}
            </p>
          </div>
        </div>
        
        <div className="flex items-center gap-4">
           <div className={cn(
             "px-4 py-2 rounded-xl text-[10px] font-black uppercase tracking-widest border flex items-center gap-3 transition-all duration-500",
             connected ? "bg-cyan-500/5 text-cyan-400 border-cyan-500/20 glow-cyan" : "bg-red-500/10 text-red-500 border-red-500/40 animate-pulse"
           )}>
             <div className={cn("w-2 h-2 rounded-full", connected ? "bg-cyan-400 animate-ping" : "bg-red-500")} />
             {connected ? "Stream Live" : "Link Lost"}
           </div>
           <button className="p-2.5 bg-industrial-800/50 hover:bg-industrial-700 border border-industrial-700/50 rounded-xl text-slate-500 hover:text-cyan-400 transition-all">
             <RefreshCw size={18} />
           </button>
        </div>
      </header>

      {/* Main Orchestration Grid */}
      <main className="flex-1 grid grid-cols-12 gap-4 min-h-0">
        {/* Fancy Sidebar: Tree */}
        <aside className="col-span-3 bg-industrial-900/20 border border-industrial-700/30 rounded-3xl flex flex-col min-h-0 shadow-xl">
          <div className="p-5 border-b border-industrial-700/20 flex items-center justify-between">
            <div className="flex items-center gap-2.5">
              <Database size={15} className="text-cyan-500" />
              <span className="text-[11px] font-black uppercase text-slate-400 tracking-[0.15em]">Registry_Explorer</span>
            </div>
            <div className="text-[9px] font-bold text-slate-600 bg-black/30 px-2 py-0.5 rounded-full border border-white/5 uppercase">
               Nodes: {tree.length}
            </div>
          </div>
          <div className="flex-1 overflow-y-auto p-4 custom-scrollbar space-y-0.5">
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

        {/* Central Stage */}
        <section className="col-span-9 flex flex-col gap-4 min-h-0">
          <div className="flex-1 bg-industrial-900/40 border border-industrial-700/30 rounded-[2.5rem] overflow-hidden flex flex-col relative shadow-2xl group">
            
            {/* Stage Decor */}
            <div className="absolute top-0 left-0 w-full h-1 bg-gradient-to-r from-transparent via-cyan-500/20 to-transparent" />
            
            <div className="absolute top-6 left-6 z-10 flex gap-3">
              <div className="px-4 py-1.5 bg-industrial-950/80 backdrop-blur-xl rounded-full border border-industrial-700/50 text-[10px] font-black text-cyan-500/80 uppercase tracking-widest shadow-lg">
                <span className="text-slate-600 mr-2">Target:</span> 
                {selectedNode ? selectedNode.name : "AWAITING_SELECTION"}
              </div>
            </div>

            {selectedNode && Inspector ? (
              <div className="flex-1 p-16 overflow-y-auto">
                <Inspector 
                  path={selectedNode.path} 
                  name={selectedNode.name} 
                  type={selectedNode.type} 
                  value={selectedNode.value}
                  ws={{ send }}
                />
              </div>
            ) : (
              <div className="flex-1 flex flex-col items-center justify-center text-center p-16">
                 <div className="relative mb-8">
                    <div className="absolute inset-0 bg-cyan-500/20 blur-3xl animate-pulse" />
                    <div className="relative p-10 bg-industrial-800/40 rounded-full border-2 border-industrial-700/50 shadow-2xl transition-all duration-700 group-hover:scale-110">
                      <AlertCircle size={80} className="text-slate-700 transition-colors duration-700 group-hover:text-cyan-500/40" />
                    </div>
                 </div>
                 <h2 className="text-2xl font-black text-slate-300 uppercase tracking-tighter italic drop-shadow-md">Registry Discovery Required</h2>
                 <p className="text-slate-500 text-sm max-w-xs mt-3 font-medium leading-relaxed uppercase text-[11px] tracking-wide">
                   Initialize a high-fidelity instrumentation session by selecting a terminal entity from the system tree.
                 </p>
              </div>
            )}

            {/* Bottom Status bar */}
            <div className="h-12 bg-industrial-950/40 border-t border-industrial-700/20 px-8 flex items-center justify-between text-[10px] text-slate-500 font-bold uppercase tracking-widest">
              <div className="flex gap-6">
                <span className="flex items-center gap-2">
                   <div className="w-1.5 h-1.5 rounded-full bg-cyan-500/50" />
                   Node: {selectedNode ? selectedNode.path : "---"}
                </span>
                <span className="flex items-center gap-2">
                   <div className="w-1.5 h-1.5 rounded-full bg-amber-500/50" />
                   Type: {selectedNode ? selectedNode.type : "---"}
                </span>
              </div>
              <div className="flex gap-6 items-center">
                <span className="opacity-50 tracking-tighter italic">Quasar_Reflexive_Link_v4.2</span>
                <div className="flex items-center gap-1.5 px-3 py-1 bg-cyan-500/10 rounded-md border border-cyan-500/20 text-cyan-400 leading-none">
                   50Hz
                </div>
              </div>
            </div>
          </div>

          {/* Telemetry Dashboard Area */}
          <div className="h-72 bg-industrial-900/20 border border-industrial-700/30 rounded-[2.5rem] p-8 shadow-inner overflow-hidden relative">
             <div className="absolute top-4 right-8 flex items-center gap-2 text-[9px] font-black text-slate-600 uppercase tracking-widest">
                <div className="w-2 h-2 rounded-sm bg-cyan-500/20 border border-cyan-500/40" />
                Live Telemetry Output
             </div>
             <TelemetryGrid lastMessage={lastMessage} />
          </div>
        </section>
      </main>

      <footer className="text-[9px] text-slate-600 font-black flex justify-between uppercase tracking-[0.4em] opacity-30 px-2">
        <div className="flex gap-8">
           <span>MARS_COLONY_SEC_4 // INFRA_MANAGEMENT</span>
           <span className="text-cyan-500">DETERMINISTIC_REFLX_OS_2026</span>
        </div>
        <span>© QUASAR_CORE_SYSTEMS</span>
      </footer>
    </div>
  );
}

const ShieldCheck = ({ size, className }: { size?: number, className?: string }) => (
  <svg className={className} width={size} height={size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
    <path d="M20 13c0 5-3.5 7.5-7.66 8.95a1 1 0 0 1-.67-.01C7.5 20.5 4 18 4 13V6a1 1 0 0 1 1-1c2 0 4.5-1.2 6.24-2.72a1.17 1.17 0 0 1 1.52 0C14.5 3.8 17 5 19 5a1 1 0 0 1 1 1z"/>
    <path d="m9 12 2 2 4-4"/>
  </svg>
);
