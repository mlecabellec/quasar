import React, { useState } from 'react';
import { Terminal, ShieldCheck, Package, Send, CheckCircle2, XCircle } from 'lucide-react';
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

  const handleSet = () => {
    if (!ws) return;
    setStatus('pending');
    ws.send("set", { path, value: parseFloat(inputValue), requestID: Date.now().toString() });
    // Mock success for UI feedback until full loop verified
    setTimeout(() => setStatus('success'), 500);
    setTimeout(() => setStatus('idle'), 3000);
  };

  return (
    <div className="space-y-6 animate-in fade-in duration-300">
      <div className="flex items-center gap-3">
        <div className="p-2 bg-amber-500/10 rounded border border-amber-500/20">
          <Terminal size={20} className="text-amber-400" />
        </div>
        <div>
          <h3 className="text-lg font-black text-slate-100 uppercase tracking-tighter italic">{name}</h3>
          <p className="text-[10px] text-slate-500 font-bold uppercase tracking-widest">{type}</p>
        </div>
      </div>

      <div className="grid grid-cols-2 gap-4">
        <div className="bg-industrial-950 p-6 rounded-2xl border border-industrial-700/30 flex flex-col items-center">
          <span className="text-[10px] text-slate-600 font-black uppercase mb-2">Live Value</span>
          <span className="text-3xl font-black text-amber-500 tracking-tighter">
            {value !== undefined ? value.toFixed(2) : "---"}
          </span>
        </div>

        <div className="bg-industrial-900/50 p-6 rounded-2xl border border-industrial-700/30 flex flex-col gap-3">
          <span className="text-[10px] text-slate-600 font-black uppercase">Command Input</span>
          <div className="flex gap-2">
            <input 
              type="number" 
              value={inputValue}
              onChange={(e) => setInputValue(e.target.value)}
              className="flex-1 bg-industrial-950 border border-industrial-700/50 rounded-lg px-3 py-2 text-sm text-slate-200 focus:outline-none focus:border-cyan-500/50 transition-colors"
              placeholder="0.00"
            />
            <button 
              onClick={handleSet}
              disabled={status === 'pending'}
              className={cn(
                "p-2 rounded-lg border transition-all flex items-center justify-center min-w-[40px]",
                status === 'success' ? "bg-green-500/20 border-green-500/40 text-green-400" :
                status === 'error' ? "bg-red-500/20 border-red-500/40 text-red-400" :
                "bg-cyan-500/10 border-cyan-500/30 text-cyan-400 hover:bg-cyan-500/20"
              )}
            >
              {status === 'pending' ? <RefreshCw className="animate-spin" size={16} /> :
               status === 'success' ? <CheckCircle2 size={16} /> :
               status === 'error' ? <XCircle size={16} /> :
               <Send size={16} />}
            </button>
          </div>
        </div>
      </div>
    </div>
  );
};

// --- Boolean Toggle Inspector ---
const BooleanInspector = ({ name, value, path, ws }: InspectorProps) => {
  const handleToggle = () => {
    if (!ws) return;
    ws.send("set", { path, value: !value, requestID: Date.now().toString() });
  };

  return (
    <div className="space-y-6">
      <div className="flex items-center gap-3">
         <ShieldCheck size={20} className="text-cyan-400" />
         <span className="font-black text-slate-100 uppercase tracking-tighter italic">{name}</span>
      </div>
      <div 
        onClick={handleToggle}
        className={cn(
          "w-full p-8 rounded-3xl border-2 cursor-pointer transition-all flex flex-col items-center gap-2",
          value ? "bg-cyan-500/10 border-cyan-500/30 shadow-[0_0_30px_rgba(0,242,255,0.05)]" : "bg-industrial-900/50 border-industrial-700/50 grayscale"
        )}
      >
        <span className={cn("text-4xl font-black italic tracking-tighter uppercase", value ? "text-cyan-400" : "text-slate-700")}>
          {value ? "TRUE // ENABLED" : "FALSE // DISABLED"}
        </span>
        <span className="text-[10px] text-slate-500 font-bold">CLICK_TO_TOGGLE_STATE</span>
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
  return registry[type] || (({ type }: InspectorProps) => (
    <div className="flex flex-col items-center justify-center h-full text-slate-700 space-y-2 opacity-30">
      <Package size={48} />
      <span className="text-xs uppercase font-bold tracking-widest">Generic Object View // {type}</span>
    </div>
  ));
}
