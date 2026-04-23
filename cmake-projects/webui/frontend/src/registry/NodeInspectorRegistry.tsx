import React, { useState, useEffect } from 'react';
import { Terminal, Shield, Package, Send, CheckCircle2, XCircle, Activity, Zap, Info, ArrowUpRight } from 'lucide-react';
import { clsx, type ClassValue } from 'clsx';
import { twMerge } from 'tailwind-merge';

function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs));
}

/**
 * @interface InspectorProps
 */
export interface InspectorProps {
  path: string;
  name: string;
  type: string;
  value?: any;
  ws?: { send: (a: string, p: any) => void };
}

// --- Primitive Number Inspector ---
const NumberInspector = ({ name, type, value, path, ws }: InspectorProps) => {
  const [inputValue, setInputValue] = useState(value?.toString() || "");
  const [status, setStatus] = useState<'idle' | 'pending' | 'success' | 'error'>('idle');

  useEffect(() => {
    if (status === 'idle') {
      setInputValue(value?.toString() || "");
    }
  }, [value, status]);

  const handleSet = () => {
    if (!ws) return;
    setStatus('pending');
    ws.send("set", { path, value: parseFloat(inputValue), requestID: Date.now().toString() });
    
    // UI Feedback cycle
    setTimeout(() => {
       setStatus('success');
       setTimeout(() => setStatus('idle'), 2000);
    }, 400);
  };

  return (
    <div className="space-y-8 animate-in fade-in zoom-in duration-500">
      <div className="flex items-center justify-between border-b border-industrial-700/30 pb-6">
        <div className="flex items-center gap-4">
          <div className="p-3 bg-amber-500/10 rounded-2xl border border-amber-500/30 shadow-[0_0_15px_rgba(245,158,11,0.1)]">
            <Terminal size={24} className="text-amber-400" />
          </div>
          <div>
            <h3 className="text-2xl font-black text-slate-100 uppercase tracking-tighter italic leading-none">{name}</h3>
            <div className="flex items-center gap-2 mt-2">
               <span className="text-[10px] text-slate-500 font-black uppercase tracking-widest bg-industrial-950 px-2 py-0.5 rounded border border-white/5">{type}</span>
               <span className="text-[10px] text-emerald-500/60 font-black uppercase tracking-widest flex items-center gap-1">
                  <Activity size={10} /> Active_Link
               </span>
            </div>
          </div>
        </div>
        <div className="flex gap-2">
           <div className="p-2 bg-industrial-950 rounded-xl border border-industrial-700/50 text-slate-600 hover:text-slate-400 cursor-pointer transition-colors">
              <Info size={16} />
           </div>
        </div>
      </div>

      <div className="grid grid-cols-12 gap-6">
        <div className="col-span-7 bg-industrial-950/60 rounded-[2rem] border border-industrial-700/30 p-8 flex flex-col relative overflow-hidden group">
          <div className="absolute top-0 right-0 p-4 opacity-10 group-hover:opacity-20 transition-opacity">
             <Activity size={120} className="text-amber-500" />
          </div>
          <span className="text-[11px] text-slate-600 font-black uppercase tracking-[0.3em] mb-4 flex items-center gap-2">
             <div className="w-2 h-2 rounded-full bg-amber-500/40 animate-pulse" />
             Instrumentation_Output
          </span>
          <div className="flex items-baseline gap-4 mt-2">
             <span className="text-7xl font-black text-amber-500 tracking-tighter drop-shadow-[0_0_20px_rgba(245,158,11,0.2)]">
               {value !== undefined ? (typeof value === 'number' ? value.toFixed(3) : value) : "---"}
             </span>
             <span className="text-xl font-bold text-slate-700 uppercase italic tracking-tighter">Units</span>
          </div>
          <div className="mt-8 pt-6 border-t border-industrial-800/50 flex justify-between">
             <div className="flex flex-col">
                <span className="text-[9px] text-slate-700 font-black uppercase tracking-widest">Signal_Fidelity</span>
                <span className="text-xs font-bold text-slate-400">99.8% OK</span>
             </div>
             <div className="flex flex-col items-end">
                <span className="text-[9px] text-slate-700 font-black uppercase tracking-widest">Last_Update</span>
                <span className="text-xs font-bold text-slate-500">Just Now</span>
             </div>
          </div>
        </div>

        <div className="col-span-5 bg-industrial-900/40 rounded-[2rem] border border-industrial-700/30 p-8 flex flex-col gap-6 relative overflow-hidden">
          <div className="absolute top-0 left-0 w-1 h-full bg-cyan-500/20" />
          <span className="text-[11px] text-slate-500 font-black uppercase tracking-[0.3em] flex items-center gap-2">
             <Zap size={14} className="text-cyan-500" />
             Control_Vector
          </span>
          <div className="flex flex-col gap-4">
            <div className="relative">
               <input 
                 type="number" 
                 value={inputValue}
                 onChange={(e) => setInputValue(e.target.value)}
                 className="w-full bg-industrial-950 border-2 border-industrial-700/50 rounded-2xl px-5 py-4 text-xl font-black text-slate-100 focus:outline-none focus:border-cyan-500/40 focus:ring-4 focus:ring-cyan-500/5 transition-all shadow-inner"
                 placeholder="0.000"
               />
               <div className="absolute right-4 top-1/2 -translate-y-1/2 text-[10px] font-black text-slate-700 uppercase tracking-widest pointer-events-none">Input_Val</div>
            </div>
            <button 
              onClick={handleSet}
              disabled={status === 'pending'}
              className={cn(
                "w-full py-4 rounded-2xl border-2 font-black uppercase tracking-[0.3em] text-[11px] transition-all flex items-center justify-center gap-3 shadow-2xl",
                status === 'success' ? "bg-emerald-500/20 border-emerald-500/40 text-emerald-400" :
                status === 'error' ? "bg-red-500/20 border-red-500/40 text-red-400" :
                "bg-cyan-500/10 border-cyan-500/30 text-cyan-400 hover:bg-cyan-500/20 hover:scale-[1.02] active:scale-95"
              )}
            >
              {status === 'pending' ? <RefreshCw className="animate-spin" size={16} /> :
               status === 'success' ? <><CheckCircle2 size={16} /> Command_Sent</> :
               status === 'error' ? <><XCircle size={16} /> Fail_Retry</> :
               <><Send size={16} /> Execute_Set</>}
            </button>
          </div>
          <div className="mt-auto bg-black/20 p-4 rounded-xl border border-white/5">
             <p className="text-[9px] text-slate-600 font-bold leading-relaxed uppercase tracking-widest">
               Safety_Lock: <span className="text-emerald-500/80">DISENGAGED</span><br/>
               The specified value will be injected directly into the deterministic core. Use caution.
             </p>
          </div>
        </div>
      </div>
    </div>
  );
};

// --- Boolean Toggle Inspector ---
const BooleanInspector = ({ name, type, value, path, ws }: InspectorProps) => {
  const handleToggle = () => {
    if (!ws) return;
    ws.send("set", { path, value: !value, requestID: Date.now().toString() });
  };

  return (
    <div className="space-y-10 animate-in fade-in slide-in-from-bottom-4 duration-500">
      <div className="flex items-center justify-between border-b border-industrial-700/30 pb-6">
        <div className="flex items-center gap-4">
          <div className="p-3 bg-cyan-500/10 rounded-2xl border border-cyan-500/30 shadow-[0_0_15px_rgba(0,242,255,0.1)]">
            <Shield size={24} className="text-cyan-400" />
          </div>
          <div>
            <h3 className="text-2xl font-black text-slate-100 uppercase tracking-tighter italic leading-none">{name}</h3>
            <p className="text-[10px] text-slate-500 font-black uppercase tracking-widest mt-2 bg-industrial-950 px-2 py-0.5 rounded border border-white/5 inline-block">{type}</p>
          </div>
        </div>
      </div>

      <div 
        onClick={handleToggle}
        className={cn(
          "w-full p-16 rounded-[3rem] border-4 cursor-pointer transition-all duration-700 flex flex-col items-center gap-6 relative overflow-hidden group/toggle",
          value 
            ? "bg-cyan-500/10 border-cyan-500/40 shadow-[0_0_50px_rgba(0,242,255,0.1)]" 
            : "bg-industrial-900/40 border-industrial-800/80 grayscale hover:grayscale-0 hover:border-industrial-700"
        )}
      >
        <div className={cn(
          "absolute inset-0 bg-gradient-to-tr from-cyan-500/5 to-transparent transition-opacity duration-1000",
          value ? "opacity-100" : "opacity-0"
        )} />
        
        <div className={cn(
          "p-8 rounded-full border-2 transition-all duration-1000 group-hover/toggle:scale-110",
          value ? "bg-cyan-500/20 border-cyan-500/40 shadow-inner animate-pulse-cyan" : "bg-industrial-950 border-industrial-800"
        )}>
           <Zap size={60} className={cn("transition-all duration-700", value ? "text-cyan-400 drop-shadow-[0_0_15px_rgba(0,242,255,0.8)]" : "text-slate-800")} />
        </div>

        <div className="text-center relative z-10">
          <span className={cn("text-6xl font-black italic tracking-tighter uppercase block", value ? "text-cyan-400" : "text-slate-700")}>
            {value ? "ACTIVE" : "INACTIVE"}
          </span>
          <span className="text-[12px] text-slate-500 font-black uppercase tracking-[0.4em] mt-2 block opacity-60">
            {value ? "SYS_GATE_OPEN // FLOW_OK" : "SYS_GATE_CLOSED // LINK_HALT"}
          </span>
        </div>

        <div className="mt-8 px-6 py-2 bg-industrial-950/80 rounded-full border border-industrial-700/50 text-[10px] font-black text-slate-500 uppercase tracking-widest flex items-center gap-3">
           Toggle_Request <ArrowUpRight size={12} />
        </div>
      </div>
    </div>
  );
};

const RefreshCw = ({ className, size }: { className?: string, size?: number }) => (
  <svg className={className} width={size} height={size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="3" strokeLinecap="round" strokeLinejoin="round">
    <path d="M21 2v6h-6"/><path d="M3 12a9 9 0 0 1 15-6.7L21 8"/><path d="M3 22v-6h6"/><path d="M21 12a9 9 0 0 1-15 6.7L3 16"/>
  </svg>
);

// --- Component Registry ---
const registry: Record<string, React.FC<InspectorProps>> = {
  "NamedInteger": NumberInspector,
  "NamedFloatingPoint": NumberInspector,
  "NamedBoolean": BooleanInspector,
};

export function getInspector(type: string): React.FC<InspectorProps> {
  return registry[type] || (({ type, name }: InspectorProps) => (
    <div className="flex flex-col items-center justify-center h-full text-slate-700 space-y-6 opacity-30 animate-pulse">
      <Package size={120} strokeWidth={1} />
      <div className="text-center">
        <h3 className="text-2xl font-black uppercase italic tracking-tighter text-slate-600">{name}</h3>
        <p className="text-xs uppercase font-black tracking-[0.4em] mt-2">Generic_System_Node // {type}</p>
      </div>
      <div className="px-6 py-2 border border-dashed border-slate-800 rounded-full text-[10px] font-black uppercase">
         No_Custom_Inspector_Mapped
      </div>
    </div>
  ));
}
