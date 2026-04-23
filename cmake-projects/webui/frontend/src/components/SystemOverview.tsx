import { Zap, Activity, Shield, CheckCircle2 } from 'lucide-react';
import { clsx, type ClassValue } from 'clsx';
import { twMerge } from 'tailwind-merge';

function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs));
}

interface SystemOverviewProps {
  currentState: string;
  health: number;
}

/**
 * @component SystemOverview
 * @brief Overhauled state machine visualizer inspired by Svar's clean technical layouts.
 */
export const SystemOverview = ({ currentState = "STANDBY", health = 98.5 }: SystemOverviewProps) => {
  const states = ["STANDBY", "STARTING", "STARTED", "STOPPING"];
  const currentIndex = states.indexOf(currentState);

  return (
    <div className="flex flex-col gap-6 h-full">
      {/* Header Info */}
      <div className="flex justify-between items-start">
         <div className="flex flex-col gap-1">
            <span className="text-[14px] font-black text-white uppercase tracking-tighter italic">Machine_Status</span>
            <div className="flex items-center gap-2">
               <div className={cn("w-1.5 h-1.5 rounded-full", health > 95 ? "bg-emerald-500 shadow-[0_0_8px_rgba(16,185,129,0.5)]" : "bg-amber-500")} />
               <span className="text-[9px] font-black text-slate-500 uppercase tracking-widest">Core_Reflexive_Integrity</span>
            </div>
         </div>
         <div className="flex flex-col items-end">
            <span className="text-xl font-black text-emerald-400 font-mono tracking-tighter">{health.toFixed(1)}%</span>
            <span className="text-[8px] font-bold text-slate-600 uppercase tracking-tighter">System_Nominal</span>
         </div>
      </div>

      {/* FSM Visualization: Vertical Stack (Svar style) */}
      <div className="flex-1 flex flex-col gap-3">
         {states.map((state, index) => {
           const isActive = index === currentIndex;
           const isPast = index < currentIndex;
           
           return (
             <div key={state} className="relative group">
                {/* Connector Line */}
                {index < states.length - 1 && (
                  <div className={cn(
                    "absolute left-[1.45rem] top-10 w-px h-8 transition-colors duration-1000",
                    isPast ? "bg-emerald-500/30" : "bg-industrial-800"
                  )} />
                )}
                
                <div className={cn(
                  "flex items-center gap-4 p-4 rounded-2xl border transition-all duration-700",
                  isActive 
                    ? "bg-cyan-500/10 border-cyan-500/40 shadow-[0_0_20px_rgba(0,242,255,0.1)] translate-x-2" 
                    : "bg-black/20 border-white/5 opacity-40 hover:opacity-100 hover:bg-black/40"
                )}>
                   <div className={cn(
                     "w-12 h-12 rounded-xl border flex items-center justify-center transition-all duration-700 shadow-inner",
                     isActive ? "bg-industrial-950 border-cyan-500/50 shadow-[0_0_15px_rgba(0,242,255,0.2)]" : "bg-black/40 border-white/5"
                   )}>
                      {state === "STANDBY" && <Shield size={18} className={isActive ? "text-cyan-400" : "text-slate-600"} />}
                      {state === "STARTING" && <Zap size={18} className={isActive ? "text-amber-400" : "text-slate-600"} />}
                      {state === "STARTED" && <Activity size={18} className={isActive ? "text-emerald-400" : "text-slate-600"} />}
                      {state === "STOPPING" && <Shield size={18} className={isActive ? "text-rose-400" : "text-slate-600"} />}
                   </div>
                   
                   <div className="flex-1 flex flex-col">
                      <div className="flex items-center justify-between">
                         <span className={cn(
                           "text-[11px] font-black uppercase tracking-widest transition-colors",
                           isActive ? "text-white" : "text-slate-500"
                         )}>{state}</span>
                         {isPast && <CheckCircle2 size={12} className="text-emerald-500/50" />}
                      </div>
                      <div className="w-full h-1 bg-black/40 rounded-full mt-2 overflow-hidden border border-white/5">
                         {isActive && (
                           <div className="h-full bg-cyan-500 shadow-[0_0_8px_rgba(0,242,255,0.5)] animate-progress" style={{ width: '100%' }} />
                         )}
                         {isPast && <div className="h-full bg-emerald-500/20" style={{ width: '100%' }} />}
                      </div>
                   </div>
                </div>
             </div>
           );
         })}
      </div>

      {/* Mini Technical Details */}
      <div className="grid grid-cols-2 gap-4 mt-4">
         <div className="p-4 bg-black/40 rounded-2xl border border-white/5 flex flex-col gap-1 shadow-inner group hover:border-cyan-500/20 transition-all">
            <span className="text-[8px] text-slate-700 font-black uppercase tracking-widest group-hover:text-cyan-500/50">Jitter_Index</span>
            <span className="text-xs font-mono font-bold text-slate-400">0.002ms</span>
         </div>
         <div className="p-4 bg-black/40 rounded-2xl border border-white/5 flex flex-col gap-1 shadow-inner group hover:border-cyan-500/20 transition-all">
            <span className="text-[8px] text-slate-700 font-black uppercase tracking-widest group-hover:text-cyan-500/50">Core_Load</span>
            <span className="text-xs font-mono font-bold text-slate-400">4.2%</span>
         </div>
      </div>
    </div>
  );
};
