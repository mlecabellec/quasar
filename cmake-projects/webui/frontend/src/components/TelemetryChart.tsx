import { ResponsiveContainer, AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip } from 'recharts';
import { Activity } from 'lucide-react';
import { useState, useEffect } from 'react';

interface TelemetryPoint {
  time: string;
  value: number;
}

interface TelemetryChartProps {
  data?: TelemetryPoint[];
  value?: number; // Real-time value for mini mode
  color?: string;
  label: string;
  isMini?: boolean;
}

/**
 * @component TelemetryChart
 * @brief High-fidelity industrial time-series visualizer.
 * Supports full dashboard view and minimalist "mini" table view.
 */
export const TelemetryChart = ({ data, value, color = "#00f2ff", label, isMini = false }: TelemetryChartProps) => {
  const [internalData, setInternalData] = useState<TelemetryPoint[]>([]);

  // Internal buffering for mini mode
  useEffect(() => {
    if (isMini && value !== undefined) {
      const timestamp = new Date().toLocaleTimeString();
      setInternalData(prev => [...prev.slice(-19), { time: timestamp, value }]);
    }
  }, [value, isMini]);

  const displayData = isMini ? internalData : (data || []);
  const lastValue = displayData.length > 0 ? displayData[displayData.length - 1].value : (value || 0);

  if (isMini) {
    return (
      <div className="w-full h-full opacity-60 hover:opacity-100 transition-opacity duration-500">
        <ResponsiveContainer width="100%" height="100%">
          <AreaChart data={displayData}>
            <defs>
              <linearGradient id={`miniGradient-${label}`} x1="0" y1="0" x2="0" y2="1">
                <stop offset="5%" stopColor={color} stopOpacity={0.3}/>
                <stop offset="95%" stopColor={color} stopOpacity={0}/>
              </linearGradient>
            </defs>
            <Area 
              type="monotone" 
              dataKey="value" 
              stroke={color} 
              fill={`url(#miniGradient-${label})`} 
              strokeWidth={2}
              dot={false}
              isAnimationActive={false}
            />
          </AreaChart>
        </ResponsiveContainer>
      </div>
    );
  }

  return (
    <div className="w-full h-full flex flex-col group/chart">
      <div className="flex justify-between items-end mb-3 px-2">
         <div className="flex flex-col">
            <div className="flex items-center gap-2">
               <Activity size={10} className="text-slate-600" />
               <span className="text-[10px] font-black text-slate-400 uppercase tracking-[0.2em]">{label}</span>
            </div>
            <span className="text-[8px] text-slate-700 font-bold uppercase tracking-tighter mt-0.5">Reflexive_Stream // Operational</span>
         </div>
         <div className="flex flex-col items-end">
            <span className="text-[14px] font-mono text-cyan-400 font-black drop-shadow-[0_0_8px_rgba(0,242,255,0.4)]">
              {lastValue.toFixed(3)}
            </span>
         </div>
      </div>
      <div className="flex-1 bg-black/20 rounded-2xl border border-white/5 overflow-hidden shadow-inner relative group-hover/chart:border-cyan-500/20 transition-colors duration-500">
        <div className="absolute inset-0 bg-gradient-to-b from-cyan-500/5 to-transparent pointer-events-none" />
        <ResponsiveContainer width="100%" height="100%">
          <AreaChart data={displayData} margin={{ top: 15, right: 10, left: -25, bottom: 0 }}>
            <defs>
              <linearGradient id={`colorValue-${label}`} x1="0" y1="0" x2="0" y2="1">
                <stop offset="5%" stopColor={color} stopOpacity={0.2}/>
                <stop offset="95%" stopColor={color} stopOpacity={0}/>
              </linearGradient>
            </defs>
            <CartesianGrid strokeDasharray="4 4" stroke="#1e293b" vertical={false} opacity={0.2} />
            <XAxis dataKey="time" hide />
            <YAxis 
                domain={['auto', 'auto']} 
                tick={{ fontSize: 9, fill: '#475569', fontWeight: 'bold' }} 
                axisLine={false} 
                tickLine={false}
            />
            <Tooltip 
              contentStyle={{ 
                backgroundColor: 'rgba(15, 23, 42, 0.95)', 
                border: '1px solid rgba(255, 255, 255, 0.1)', 
                borderRadius: '12px', 
                fontSize: '10px',
                backdropFilter: 'blur(12px)',
                boxShadow: '0 10px 30px rgba(0,0,0,0.5)'
              }}
              itemStyle={{ color: color, fontWeight: 'bold' }}
              cursor={{ stroke: 'rgba(255,255,255,0.1)', strokeWidth: 1 }}
            />
            <Area 
              type="monotone" 
              dataKey="value" 
              stroke={color} 
              fillOpacity={1} 
              fill={`url(#colorValue-${label})`} 
              strokeWidth={3}
              dot={false}
              activeDot={{ r: 4, strokeWidth: 0, fill: color, className: "animate-pulse" }}
              isAnimationActive={false}
            />
          </AreaChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
};
