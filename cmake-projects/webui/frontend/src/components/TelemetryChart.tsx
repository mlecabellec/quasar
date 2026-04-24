import { ResponsiveContainer, AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip } from 'recharts';
import { Activity } from 'lucide-react';
import { useState, useEffect } from 'react';

interface TelemetryPoint {
  time: string;
  value: number;
}

interface TelemetryChartProps {
  data?: TelemetryPoint[];
  value?: number;
  color?: string;
  label: string;
  isMini?: boolean;
}

/**
 * @component TelemetryChart
 * @brief Recharts-based time-series area chart.
 * Supports full and mini (inline table sparkline) modes.
 */
export const TelemetryChart = ({ data, value, color = "#00f2ff", label, isMini = false }: TelemetryChartProps) => {
  const [internalData, setInternalData] = useState<TelemetryPoint[]>([]);

  useEffect(() => {
    if (isMini && value !== undefined) {
      const timestamp = new Date().toLocaleTimeString();
      setInternalData(prev => [...prev.slice(-24), { time: timestamp, value }]);
    }
  }, [value, isMini]);

  const displayData = isMini ? internalData : (data || []);
  const lastValue = displayData.length > 0 ? displayData[displayData.length - 1].value : (value || 0);

  /* Mini sparkline for table rows */
  if (isMini) {
    return (
      <ResponsiveContainer width="100%" height="100%">
        <AreaChart data={displayData}>
          <defs>
            <linearGradient id={`mini-${label}`} x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor={color} stopOpacity={0.25}/>
              <stop offset="95%" stopColor={color} stopOpacity={0}/>
            </linearGradient>
          </defs>
          <Area
            type="monotone" dataKey="value" stroke={color}
            fill={`url(#mini-${label})`} strokeWidth={1.5}
            dot={false} isAnimationActive={false}
          />
        </AreaChart>
      </ResponsiveContainer>
    );
  }

  /* Full dashboard chart */
  return (
    <div className="w-full h-full flex flex-col">
      <div className="flex justify-between items-end mb-2 px-1">
        <div className="flex items-center gap-2">
          <Activity size={10} className="text-slate-600" />
          <span className="text-[10px] font-bold text-slate-400 uppercase tracking-widest">{label}</span>
        </div>
        <span className="text-sm font-mono text-cyan-400 font-bold">
          {lastValue.toFixed(3)}
        </span>
      </div>
      <div className="flex-1 bg-industrial-900/40 rounded-xl border border-white/5 overflow-hidden">
        <ResponsiveContainer width="100%" height="100%">
          <AreaChart data={displayData} margin={{ top: 10, right: 8, left: -20, bottom: 0 }}>
            <defs>
              <linearGradient id={`full-${label}`} x1="0" y1="0" x2="0" y2="1">
                <stop offset="5%" stopColor={color} stopOpacity={0.15}/>
                <stop offset="95%" stopColor={color} stopOpacity={0}/>
              </linearGradient>
            </defs>
            <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.04)" vertical={false} />
            <XAxis dataKey="time" hide />
            <YAxis
              domain={['auto', 'auto']}
              tick={{ fontSize: 9, fill: '#475569' }}
              axisLine={false} tickLine={false}
            />
            <Tooltip
              contentStyle={{
                backgroundColor: 'rgba(10,10,12,0.95)',
                border: '1px solid rgba(255,255,255,0.1)',
                borderRadius: '8px',
                fontSize: '10px'
              }}
              itemStyle={{ color: color, fontWeight: 'bold' }}
            />
            <Area
              type="monotone" dataKey="value" stroke={color}
              fillOpacity={1} fill={`url(#full-${label})`}
              strokeWidth={2} dot={false}
              activeDot={{ r: 3, fill: color }}
              isAnimationActive={false}
            />
          </AreaChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
};
