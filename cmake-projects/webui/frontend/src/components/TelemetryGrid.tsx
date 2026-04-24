import { useState, useEffect } from 'react';
import { Activity, Search } from 'lucide-react';
import { TelemetryChart } from './TelemetryChart';

interface TelemetryGridProps {
  lastMessage: any;
}

/**
 * @component TelemetryGrid
 * @brief Data table + inline sparklines for live telemetry signals.
 * Includes a search filter and a sliding-window buffer per signal.
 */
export const TelemetryGrid = ({ lastMessage }: TelemetryGridProps) => {
  const [streams, setStreams] = useState<Record<string, { time: string; value: number; type: string }[]>>({});
  const [filter, setFilter] = useState("");

  useEffect(() => {
    if (lastMessage?.action === "batch_update") {
      const timestamp = new Date().toLocaleTimeString();
      setStreams(prev => {
        const next = { ...prev };
        lastMessage.updates.forEach((u: any) => {
          if (typeof u.value === 'number') {
            if (!next[u.name]) next[u.name] = [];
            next[u.name] = [...next[u.name], { time: timestamp, value: u.value, type: u.type }].slice(-50);
          }
        });
        return next;
      });
    }
  }, [lastMessage]);

  const signals = Object.entries(streams)
    .map(([name, data]) => ({ name, last: data[data.length - 1], history: data }))
    .filter(s => s.name.toLowerCase().includes(filter.toLowerCase()));

  return (
    <div className="flex flex-col h-full gap-3">
      {/* Header row */}
      <div className="flex items-center justify-between shrink-0">
        <div className="flex items-center gap-2">
          <Activity size={14} className="text-cyan-500" />
          <span className="text-[10px] font-bold uppercase tracking-widest text-slate-300">Telemetry</span>
          <span className="text-[9px] text-slate-600 font-mono ml-2">{signals.length} signals</span>
        </div>
        <div className="flex items-center gap-2 bg-industrial-800/60 rounded-lg border border-white/5 px-3 py-1.5 focus-within:border-cyan-500/30 transition-colors">
          <Search size={11} className="text-slate-600" />
          <input
            type="text"
            placeholder="Filter..."
            value={filter}
            onChange={e => setFilter(e.target.value)}
            className="bg-transparent border-none focus:outline-none text-[10px] text-slate-400 w-28 placeholder:text-slate-700"
          />
        </div>
      </div>

      {/* Table */}
      <div className="flex-1 overflow-y-auto custom-scrollbar rounded-xl border border-white/5 bg-industrial-900/40">
        <table className="w-full text-left border-collapse">
          <thead className="sticky top-0 z-10 bg-industrial-900/90 backdrop-blur-sm">
            <tr className="border-b border-white/5">
              <th className="px-4 py-2.5 text-[9px] font-bold text-slate-500 uppercase tracking-widest">Signal</th>
              <th className="px-4 py-2.5 text-[9px] font-bold text-slate-500 uppercase tracking-widest">Type</th>
              <th className="px-4 py-2.5 text-[9px] font-bold text-slate-500 uppercase tracking-widest text-right">Value</th>
              <th className="px-4 py-2.5 text-[9px] font-bold text-slate-500 uppercase tracking-widest w-40">Trend</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-white/[0.03]">
            {signals.length > 0 ? signals.map(sig => (
              <tr key={sig.name} className="hover:bg-white/[0.02] transition-colors group">
                <td className="px-4 py-3">
                  <span className="text-[11px] font-bold text-slate-300 group-hover:text-cyan-400 transition-colors">{sig.name}</span>
                </td>
                <td className="px-4 py-3">
                  <span className="text-[9px] font-mono text-slate-600 bg-industrial-800/60 px-2 py-0.5 rounded">{sig.last.type}</span>
                </td>
                <td className="px-4 py-3 text-right">
                  <span className="text-[12px] font-mono font-bold text-amber-400">{sig.last.value.toFixed(3)}</span>
                </td>
                <td className="px-4 py-3">
                  <div className="h-8 w-full opacity-50 group-hover:opacity-100 transition-opacity">
                    <TelemetryChart label={sig.name} data={sig.history} isMini />
                  </div>
                </td>
              </tr>
            )) : (
              <tr>
                <td colSpan={4} className="px-4 py-12 text-center text-[10px] text-slate-700 italic">
                  {filter ? "No signals matching filter" : "Awaiting telemetry data..."}
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
};
