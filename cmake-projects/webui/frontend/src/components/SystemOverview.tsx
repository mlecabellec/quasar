import { Activity, CheckCircle2 } from 'lucide-react';
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
 * @brief Compact vertical FSM visualizer for the sidebar detail pane.
 */
export const SystemOverview = ({ currentState = "STANDBY", health = 98.5 }: SystemOverviewProps) => {
  const states = ["STANDBY", "STARTING", "STARTED", "STOPPING"];
  const currentIndex = states.indexOf(currentState);

  return (
    <div className="flex flex-col gap-4">
      {/* Health bar */}
      <div className="flex items-center justify-between">
        <span className="text-[9px] font-bold text-slate-500 uppercase tracking-widest">Integrity</span>
        <span className="text-xs font-bold text-emerald-400 font-mono">{health.toFixed(1)}%</span>
      </div>
      <div className="w-full h-1.5 bg-industrial-800 rounded-full overflow-hidden">
        <div
          className="h-full rounded-full bg-gradient-to-r from-emerald-500 to-cyan-500 transition-all duration-1000"
          style={{ width: `${health}%` }}
        />
      </div>

      {/* FSM states — compact vertical */}
      <div className="flex flex-col gap-1 mt-2">
        {states.map((state, idx) => {
          const isActive = idx === currentIndex;
          const isPast = idx < currentIndex;
          return (
            <div key={state} className="relative">
              {/* Connector */}
              {idx < states.length - 1 && (
                <div className={cn(
                  "absolute left-[13px] top-8 w-px h-4",
                  isPast ? "bg-emerald-500/30" : "bg-industrial-700/40"
                )} />
              )}
              <div className={cn(
                "flex items-center gap-3 px-3 py-2 rounded-lg transition-all duration-300",
                isActive ? "bg-cyan-500/10 border border-cyan-500/20" : "border border-transparent"
              )}>
                <div className={cn(
                  "w-[10px] h-[10px] rounded-sm shrink-0 transition-all",
                  isActive ? "bg-cyan-500 shadow-[0_0_6px_rgba(0,242,255,0.6)]"
                    : isPast ? "bg-emerald-500/40" : "bg-industrial-700"
                )} />
                <span className={cn(
                  "text-[10px] font-bold uppercase tracking-widest",
                  isActive ? "text-cyan-400" : isPast ? "text-slate-500" : "text-slate-700"
                )}>{state}</span>
                {isPast && <CheckCircle2 size={10} className="text-emerald-500/40 ml-auto" />}
                {isActive && <Activity size={10} className="text-cyan-500 ml-auto animate-pulse" />}
              </div>
            </div>
          );
        })}
      </div>

      {/* Mini metrics */}
      <div className="grid grid-cols-2 gap-2 mt-2">
        <div className="bg-industrial-800/40 rounded-lg p-3 border border-white/5">
          <span className="text-[8px] text-slate-600 font-bold uppercase tracking-widest block">Cycle</span>
          <span className="text-[11px] font-bold text-slate-400 font-mono">50.0ms</span>
        </div>
        <div className="bg-industrial-800/40 rounded-lg p-3 border border-white/5">
          <span className="text-[8px] text-slate-600 font-bold uppercase tracking-widest block">Jitter</span>
          <span className="text-[11px] font-bold text-slate-400 font-mono">0.002ms</span>
        </div>
      </div>
    </div>
  );
};
