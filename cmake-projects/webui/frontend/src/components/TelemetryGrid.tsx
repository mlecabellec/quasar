import { useState, useEffect } from 'react';
import { Thermometer, Gauge, Activity, Zap, Search } from 'lucide-react';
import { TelemetryChart } from './TelemetryChart';


interface TelemetryGridProps {
  lastMessage: any;
}

/**
 * @component TelemetryGrid
 * @brief Overhauled high-fidelity instrumentation grid inspired by Svar and Shadcn.
 */
export const TelemetryGrid = ({ lastMessage }: TelemetryGridProps) => {
  const [streams, setStreams] = useState<Record<string, { time: string, value: number, type: string }[]>>({});
  const [filter, setFilter] = useState("");

  useEffect(() => {
    if (lastMessage?.action === "batch_update") {
      const timestamp = new Date().toLocaleTimeString();
      
      setStreams(prev => {
        const next = { ...prev };
        lastMessage.updates.forEach((u: any) => {
          if (typeof u.value === 'number') {
            if (!next[u.name]) next[u.name] = [];
            const newStream = [...next[u.name], { time: timestamp, value: u.value, type: u.type }];
            next[u.name] = newStream.slice(-50);
          }
        });
        return next;
      });
    }
  }, [lastMessage]);

  const allSignals = Object.entries(streams).map(([name, data]) => ({
    name,
    lastValue: data[data.length - 1].value,
    type: data[data.length - 1].type,
    history: data
  }));

  const filteredSignals = allSignals.filter(s => 
    s.name.toLowerCase().includes(filter.toLowerCase()) || 
    s.type.toLowerCase().includes(filter.toLowerCase())
  );

  return (
    <div className="flex flex-col h-full gap-4">
      <div className="flex items-center justify-between px-2">
         <div className="flex items-center gap-3">
            <Activity size={18} className="text-cyan-500" />
            <div className="flex flex-col">
               <span className="text-[11px] font-black uppercase tracking-[0.2em] text-slate-100">Telemetry_Instrumentation</span>
               <span className="text-[8px] font-bold text-slate-600 uppercase tracking-tighter">Real-Time Data Flux</span>
            </div>
         </div>
         <div className="flex items-center gap-4 bg-black/40 rounded-xl border border-white/5 px-4 py-1.5 shadow-inner group focus-within:border-cyan-500/30 transition-all">
            <Search size={14} className="text-slate-700 group-focus-within:text-cyan-500 transition-colors" />
            <input 
              type="text" 
              placeholder="FILTER_SIGNALS..." 
              value={filter}
              onChange={(e) => setFilter(e.target.value)}
              className="bg-transparent border-none focus:outline-none text-[10px] font-black text-slate-400 uppercase tracking-widest w-40 placeholder:text-slate-800" 
            />
         </div>
      </div>

      <div className="flex-1 overflow-hidden glass-panel rounded-[2rem] border border-white/5 flex flex-col">
        <div className="overflow-y-auto custom-scrollbar flex-1">
          <table className="w-full text-left border-collapse">
            <thead>
              <tr className="border-b border-white/5 bg-white/5 sticky top-0 z-10 backdrop-blur-md">
                <th className="px-8 py-4 text-[10px] font-black text-slate-500 uppercase tracking-widest">Signal_ID</th>
                <th className="px-8 py-4 text-[10px] font-black text-slate-500 uppercase tracking-widest">Type</th>
                <th className="px-8 py-4 text-[10px] font-black text-slate-500 uppercase tracking-widest text-right">Value</th>
                <th className="px-8 py-4 text-[10px] font-black text-slate-500 uppercase tracking-widest">Flux_Analysis</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-white/5 font-mono text-[12px]">
              {filteredSignals.length > 0 ? filteredSignals.map((signal) => (
                <tr key={signal.name} className="hover:bg-white/[0.03] transition-colors group">
                  <td className="px-8 py-5">
                    <div className="flex items-center gap-4">
                      <div className="w-10 h-10 rounded-xl bg-industrial-950 border border-white/5 flex items-center justify-center group-hover:border-cyan-500/30 transition-all shadow-lg">
                         {signal.name.toLowerCase().includes('temp') && <Thermometer size={16} className="text-amber-500" />}
                         {signal.name.toLowerCase().includes('press') && <Gauge size={16} className="text-cyan-500" />}
                         {!signal.name.toLowerCase().includes('temp') && !signal.name.toLowerCase().includes('press') && <Zap size={16} className="text-slate-600 group-hover:text-cyan-500/50" />}
                      </div>
                      <div className="flex flex-col">
                         <span className="font-black text-slate-200 group-hover:text-cyan-400 transition-colors uppercase tracking-tight">{signal.name}</span>
                         <span className="text-[8px] text-slate-700 font-bold uppercase tracking-tighter">Status: Nominal</span>
                      </div>
                    </div>
                  </td>
                  <td className="px-8 py-5">
                    <span className="text-[9px] font-black uppercase tracking-widest px-2 py-1 bg-black/40 rounded border border-white/5 text-slate-600">
                      {signal.type}
                    </span>
                  </td>
                  <td className="px-8 py-5 text-right">
                     <div className="flex flex-col items-end gap-1">
                        <span className="text-sm font-black text-amber-500 drop-shadow-[0_0_8px_rgba(245,158,11,0.3)]">
                           {signal.lastValue.toFixed(3)}
                        </span>
                        <div className="w-16 h-1 bg-industrial-800 rounded-full overflow-hidden">
                           <div className="h-full bg-amber-500/50 shadow-[0_0_5px_rgba(245,158,11,0.5)]" style={{ width: `${Math.min(100, (signal.lastValue / 100) * 100)}%` }} />
                        </div>
                     </div>
                  </td>
                  <td className="px-8 py-5 w-64">
                     <div className="h-14 w-full">
                        <TelemetryChart label={signal.name} data={signal.history} isMini />
                     </div>
                  </td>
                </tr>
              )) : (
                <tr>
                  <td colSpan={4} className="px-8 py-32 text-center">
                    <div className="flex flex-col items-center gap-4 opacity-20">
                       <Search size={48} className="text-slate-700" />
                       <span className="text-xs italic uppercase tracking-[0.4em] font-black text-slate-600">
                         No instrumentation found matching filter
                       </span>
                    </div>
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
};
