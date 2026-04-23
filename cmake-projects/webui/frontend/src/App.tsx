import { Activity, Database, ChevronRight, Box, Layers, Cpu, Terminal, Shield, Zap, LayoutDashboard, Settings, User, Folder, FolderOpen, Menu, Bell, Info, Search } from 'lucide-react';
import { useState, useEffect, useMemo } from 'react';
import { clsx, type ClassValue } from 'clsx';
import { twMerge } from 'tailwind-merge';

import { useWebSocket } from './hooks/useWebSocket';
import { useTree } from './hooks/useTree';
import type { QuasarNode } from './hooks/useTree';
import { getInspector } from './registry/NodeInspectorRegistry';
import { TelemetryGrid } from './components/TelemetryGrid';
import { SystemOverview } from './components/SystemOverview';

// --- Utilities ---
function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs));
}

// --- Components ---

const NodeIcon = ({ type, isExpanded }: { type: string, isExpanded?: boolean }) => {
  if (type === "NamedInteger" || type === "NamedFloatingPoint") return <Activity size={14} className="text-amber-400" />;
  if (type === "NamedBoolean") return <Shield size={14} className="text-cyan-400" />;
  if (type === "NamedString") return <Layers size={14} className="text-purple-400" />;
  if (type === "WebUIService") return <Box size={14} className="text-pink-400" />;
  if (type === "LogicEngine" || type === "StateMachine") return <Cpu size={14} className="text-emerald-400" />;
  
  if (isExpanded) return <FolderOpen size={14} className="text-industrial-400" />;
  return <Folder size={14} className="text-industrial-500" />;
};

const TreeItem = ({ node, onSelect, onExpand, isSelected }: { 
  node: QuasarNode; 
  onSelect: (n: QuasarNode) => void;
  onExpand: (path: string) => void;
  isSelected: boolean;
}) => {
  const isLeaf = ["NamedInteger", "NamedBoolean", "NamedFloatingPoint", "NamedString"].includes(node.type);
  
  return (
    <div className="flex flex-col select-none animate-slide-up" style={{ animationDelay: '50ms' }}>
      <div 
        onClick={() => onSelect(node)}
        className={cn(
          "group flex items-center gap-2 px-3 py-1.5 rounded-xl cursor-pointer transition-all duration-500 border relative",
          isSelected 
            ? "bg-cyan-500/10 border-cyan-500/40 text-cyan-400 shadow-[0_0_25px_rgba(0,242,255,0.1)] scale-[1.03] z-20" 
            : "hover:bg-industrial-800/30 border-transparent text-slate-400 hover:text-slate-100"
        )}
      >
        {!isLeaf ? (
          <button 
            onClick={(e) => { e.stopPropagation(); onExpand(node.path); }}
            className="p-1 hover:bg-industrial-700/50 rounded-lg transition-all duration-300 transform active:scale-90"
          >
            <ChevronRight size={12} className={cn(
              "text-slate-600 transition-transform duration-500",
              node.isExpanded && "rotate-90 text-cyan-500"
            )} />
          </button>
        ) : (
          <div className="w-4 h-4 flex items-center justify-center">
            <div className={cn(
              "w-1 h-1 rounded-full transition-all duration-700",
              isSelected ? "bg-cyan-500 scale-150 shadow-[0_0_8px_rgba(0,242,255,1)]" : "bg-slate-800 group-hover:bg-cyan-500/40"
            )} />
          </div>
        )}
        
        <div className={cn(
          "p-1.5 rounded-lg border transition-all duration-700 shadow-inner",
          isSelected ? "bg-cyan-500/20 border-cyan-500/30" : "bg-black/20 border-white/5"
        )}>
          <NodeIcon type={node.type} isExpanded={node.isExpanded} />
        </div>
        
        <div className="flex items-baseline gap-2 flex-1 overflow-hidden">
          <span className={cn(
            "text-[13px] truncate tracking-tight transition-all duration-500",
            !isLeaf ? "font-black uppercase tracking-widest" : "font-bold",
            isSelected ? "text-cyan-400" : "text-slate-300"
          )}>
            {node.name}
          </span>
          <span className="text-[7px] text-slate-600 font-black uppercase tracking-[0.3em] opacity-0 group-hover:opacity-100 transition-all duration-700 translate-x-2 group-hover:translate-x-0 whitespace-nowrap">
            // {node.type}
          </span>
        </div>

        {node.value !== undefined && (
          <span className="text-[11px] font-mono text-amber-500/90 bg-black/40 px-2 py-0.5 rounded-lg border border-white/5 font-bold shadow-inner">
            {typeof node.value === 'number' ? node.value.toFixed(2) : String(node.value)}
          </span>
        )}
      </div>
      
      <div className={cn(
        "grid transition-all duration-700 ease-in-out overflow-hidden ml-[1.35rem]",
        node.isExpanded ? "grid-rows-[1fr] mt-1 opacity-100" : "grid-rows-[0fr] opacity-0"
      )}>
        <div className="min-h-0 border-l border-industrial-700/20 pl-3 space-y-1 relative">
          {isSelected && (
            <div className="absolute left-[-1px] top-0 w-px h-full bg-gradient-to-b from-cyan-500 to-transparent shadow-[0_0_8px_rgba(0,242,255,0.3)]" />
          )}
          
          {node.children && node.children.map(child => (
            <TreeItem 
              key={child.path} 
              node={child} 
              onSelect={onSelect} 
              onExpand={onExpand}
              isSelected={isSelected && child.path === node.path} 
            />
          ))}
        </div>
      </div>
    </div>
  );
};

export default function App() {
  const { connected, send, lastMessage } = useWebSocket();
  const { tree, expandNode, selectedNode, setSelectedNode, filter, setFilter } = useTree(lastMessage);
  const [logs, setLogs] = useState<{ id: string, msg: string, type: 'info' | 'warn' | 'error' }[]>([]);

  const Inspector = selectedNode ? getInspector(selectedNode.type) : null;

  const systemState = useMemo(() => {
    const findInTree = (nodes: QuasarNode[], name: string): any => {
      for (const node of nodes) {
        if (node.name === name) return node.value;
        if (node.children) {
          const val = findInTree(node.children, name);
          if (val !== undefined) return val;
        }
      }
      return undefined;
    };
    return findInTree(tree, "systemState") || "STANDBY";
  }, [tree]);

  useEffect(() => {
    if (lastMessage) {
      setLogs(prev => [
        { id: Math.random().toString(36), msg: JSON.stringify(lastMessage).slice(0, 80), type: 'info' },
        ...prev.slice(0, 49)
      ]);
    }
  }, [lastMessage]);

  return (
    <div className="min-h-screen bg-industrial-950 flex font-mono selection:bg-cyan-500/30 relative overflow-hidden">
      <div className="crt-overlay" />
      <div className="scanline" />

      {/* Sidebar: Icon-centric (Svar style) */}
      <nav className="w-20 bg-industrial-900 border-r border-white/5 flex flex-col items-center py-8 gap-8 z-50">
         <div className="w-12 h-12 bg-cyan-500/10 rounded-2xl flex items-center justify-center border border-cyan-500/30 shadow-[0_0_20px_rgba(0,242,255,0.2)] mb-4">
            <Zap size={24} className="text-cyan-400" />
         </div>
         
         <div className="flex flex-col gap-4">
            {[LayoutDashboard, Terminal, Settings, Database, Shield].map((Icon, i) => (
              <button key={i} className={cn(
                "p-4 rounded-2xl transition-all duration-300 group relative",
                i === 0 ? "bg-cyan-500 text-industrial-950 shadow-[0_0_20px_rgba(0,242,255,0.4)]" : "text-slate-500 hover:text-slate-100 hover:bg-white/5"
              )}>
                <Icon size={20} />
                {i !== 0 && (
                  <div className="absolute left-full ml-4 px-3 py-1.5 bg-industrial-800 rounded-lg text-[10px] font-black uppercase tracking-widest opacity-0 group-hover:opacity-100 pointer-events-none transition-all translate-x-[-10px] group-hover:translate-x-0 whitespace-nowrap z-50 shadow-2xl border border-white/5">
                    {Icon.name}
                  </div>
                )}
              </button>
            ))}
         </div>

         <div className="mt-auto flex flex-col gap-4">
            <button className="p-4 text-slate-600 hover:text-slate-100 transition-colors">
               <User size={20} />
            </button>
         </div>
      </nav>

      {/* Main Container */}
      <div className="flex-1 flex flex-col min-w-0">
        {/* Top Header Bar */}
        <header className="h-20 bg-industrial-900/50 backdrop-blur-md border-b border-white/5 flex items-center justify-between px-10 z-40">
           <div className="flex items-center gap-8">
              <div className="flex flex-col">
                 <div className="flex items-center gap-3">
                    <h1 className="text-lg font-black text-white tracking-tighter uppercase italic leading-none">Quasar_Mission_Control</h1>
                    <span className="text-[10px] bg-cyan-500/10 text-cyan-500 px-2 py-0.5 rounded-full font-black border border-cyan-500/20">V2.5</span>
                 </div>
                 {/* Breadcrumbs (Svar style) */}
                 <div className="flex items-center gap-2 mt-1 text-[9px] font-bold text-slate-500 uppercase tracking-widest">
                    <span>Root</span>
                    <ChevronRight size={10} />
                    <span className="text-slate-400">{selectedNode ? selectedNode.path.split('/').filter(Boolean).join(' > ') : 'Explorer'}</span>
                 </div>
              </div>
           </div>

           <div className="flex items-center gap-8">
              <div className="hidden lg:flex items-center gap-6 px-6 border-x border-white/5">
                 <div className="flex flex-col items-end">
                    <span className="text-[8px] text-slate-600 uppercase font-black tracking-widest">Sys_Health</span>
                    <span className="text-xs font-bold text-emerald-400">99.98%</span>
                 </div>
                 <div className="flex flex-col items-end">
                    <span className="text-[8px] text-slate-600 uppercase font-black tracking-widest">Bus_Load</span>
                    <span className="text-xs font-bold text-amber-400">12.4%</span>
                 </div>
              </div>

              <div className="flex items-center gap-3">
                 <button className="p-2.5 bg-white/5 hover:bg-white/10 rounded-xl transition-all border border-white/5 relative">
                    <Bell size={18} className="text-slate-400" />
                    <div className="absolute top-2 right-2 w-2 h-2 bg-rose-500 rounded-full border-2 border-industrial-900" />
                 </button>
                 <div className={cn(
                   "px-5 py-2 rounded-xl text-[10px] font-black uppercase tracking-[0.2em] border flex items-center gap-3 transition-all",
                   connected ? "bg-cyan-500/10 text-cyan-400 border-cyan-500/20 shadow-[0_0_15px_rgba(0,242,255,0.1)]" : "bg-red-500/10 text-red-500 border-red-500/20"
                 )}>
                   <div className={cn("w-2 h-2 rounded-full", connected ? "bg-cyan-400 animate-pulse" : "bg-red-500")} />
                   {connected ? "Stream_Live" : "Offline"}
                 </div>
              </div>
           </div>
        </header>

        {/* Dashboard Grid */}
        <main className="flex-1 overflow-hidden grid grid-cols-12 gap-px bg-white/5">
           {/* Navigation Pane */}
           <div className="col-span-3 bg-industrial-950 flex flex-col min-h-0">
              <div className="p-6 border-b border-white/5 bg-industrial-900/30 flex flex-col gap-4">
                 <div className="flex items-center justify-between">
                    <div className="flex items-center gap-3">
                       <Database size={16} className="text-cyan-500" />
                       <span className="text-[11px] font-black uppercase tracking-widest text-slate-100">Registry_Explorer</span>
                    </div>
                    <button className="p-1.5 hover:bg-white/5 rounded-lg text-slate-500 transition-colors">
                       <Menu size={16} />
                    </button>
                 </div>
                 {/* Integrated Search (Svar style) */}
                 <div className="flex items-center gap-3 bg-black/40 rounded-xl border border-white/5 px-4 py-2 group focus-within:border-cyan-500/30 transition-all shadow-inner">
                    <Search size={12} className="text-slate-700 group-focus-within:text-cyan-500 transition-colors" />
                    <input 
                      type="text" 
                      placeholder="SEARCH_NODES..." 
                      value={filter}
                      onChange={(e) => setFilter(e.target.value)}
                      className="bg-transparent border-none focus:outline-none text-[9px] font-black text-slate-400 uppercase tracking-widest w-full placeholder:text-slate-800" 
                    />
                 </div>
              </div>
              <div className="flex-1 overflow-y-auto p-4 custom-scrollbar space-y-1">
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
           </div>

           {/* Central Instrumentation Pane */}
           <div className="col-span-6 bg-industrial-900/20 backdrop-blur-sm flex flex-col min-h-0 relative">
              <div className="absolute inset-0 hex-grid opacity-10 pointer-events-none" />
              
              <div className="p-8 flex-1 overflow-y-auto custom-scrollbar">
                {selectedNode && Inspector ? (
                  <div className="animate-zoom-in">
                    <div className="mb-10 flex items-center justify-between">
                       <div className="flex items-center gap-6">
                          <div className="p-4 bg-industrial-950 rounded-2xl border border-white/5 shadow-2xl">
                             <NodeIcon type={selectedNode.type} isExpanded />
                          </div>
                          <div className="flex flex-col">
                             <h2 className="text-3xl font-black text-white uppercase tracking-tighter italic">{selectedNode.name}</h2>
                             <span className="text-[10px] text-slate-500 font-black uppercase tracking-widest mt-1 opacity-60">ID: {selectedNode.path}</span>
                          </div>
                       </div>
                       <div className="flex gap-2">
                          <button className="px-6 py-2.5 bg-cyan-500 text-industrial-950 rounded-xl text-[10px] font-black uppercase tracking-widest shadow-xl hover:scale-105 transition-transform active:scale-95">Execute_Command</button>
                       </div>
                    </div>
                    
                    <div className="glass-panel rounded-[2.5rem] p-10 border border-white/5">
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
                  <div className="h-full flex flex-col items-center justify-center text-center p-20 animate-slide-up">
                     <div className="relative mb-12 group">
                        <div className="absolute inset-0 bg-cyan-500/10 blur-[80px] rounded-full" />
                        <div className="relative p-16 bg-industrial-950/40 rounded-full border border-white/10 shadow-2xl group-hover:scale-110 transition-transform duration-1000">
                          <Cpu size={80} className="text-slate-800 group-hover:text-cyan-500/20" />
                        </div>
                     </div>
                     <h3 className="text-xl font-black text-slate-300 uppercase tracking-[0.2em] italic mb-4">No_Active_Instrumentation</h3>
                     <p className="text-[10px] text-slate-500 max-w-sm uppercase tracking-widest leading-loose font-bold">Select a node from the registry to initialize the reflexive orchestration bridge.</p>
                  </div>
                )}
              </div>
           </div>

           {/* Detail / Info Pane (Svar Detail Pane style) */}
           <div className="col-span-3 bg-industrial-950 border-l border-white/5 flex flex-col min-h-0">
              <div className="p-8 flex flex-col gap-8 h-full">
                 <div className="flex flex-col gap-6 animate-slide-up" style={{ animationDelay: '100ms' }}>
                    <div className="flex items-center gap-3">
                       <Info size={16} className="text-cyan-500" />
                       <span className="text-[11px] font-black uppercase tracking-widest text-slate-100">System_Diagnostics</span>
                    </div>
                    <SystemOverview currentState={systemState} health={connected ? 99.8 : 0} />
                 </div>

                 <div className="flex-1 flex flex-col gap-4 animate-slide-up" style={{ animationDelay: '200ms' }}>
                    <div className="flex items-center justify-between">
                       <div className="flex items-center gap-3">
                          <Terminal size={16} className="text-amber-500" />
                          <span className="text-[11px] font-black uppercase tracking-widest text-slate-100">Event_Log</span>
                       </div>
                       <button className="text-[8px] font-black uppercase text-slate-600 hover:text-slate-400 transition-colors">Clear_All</button>
                    </div>
                    <div className="flex-1 bg-black/30 rounded-3xl border border-white/5 p-4 custom-scrollbar overflow-y-auto space-y-2 font-mono text-[9px]">
                       {logs.length > 0 ? logs.map(log => (
                         <div key={log.id} className="p-2 hover:bg-white/5 rounded-lg border border-transparent hover:border-white/5 transition-all group">
                            <div className="flex justify-between items-center mb-1">
                               <span className="text-slate-700 font-black">[{new Date().toLocaleTimeString().split(' ')[0]}]</span>
                               <span className="text-[7px] text-slate-600 font-bold uppercase tracking-tighter opacity-0 group-hover:opacity-100 transition-opacity">PID: 4820</span>
                            </div>
                            <span className="text-slate-400 italic block leading-relaxed">{log.msg}</span>
                         </div>
                       )) : (
                         <div className="h-full flex items-center justify-center italic text-slate-800 uppercase tracking-widest">Awaiting_Events...</div>
                       )}
                    </div>
                 </div>
              </div>
           </div>
        </main>

        {/* Global Telemetry Table (Svar Grid style) - Toggleable or fixed at bottom */}
        <footer className="h-1/3 bg-industrial-950 border-t border-white/5 p-8 overflow-hidden z-30">
           <TelemetryGrid lastMessage={lastMessage} />
        </footer>
      </div>
    </div>
  );
}
