import { ResponsiveContainer, AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip } from 'recharts';

interface TelemetryPoint {
  time: string;
  value: number;
}

interface TelemetryChartProps {
  data: TelemetryPoint[];
  color?: string;
  label: string;
}

/**
 * @component TelemetryChart
 * @brief High-fidelity industrial time-series visualizer.
 */
export const TelemetryChart = ({ data, color = "#00f2ff", label }: TelemetryChartProps) => {
  return (
    <div className="w-full h-full flex flex-col">
      <div className="flex justify-between items-center mb-2 px-1">
         <span className="text-[9px] font-black text-slate-500 uppercase tracking-widest">{label} // REAL_TIME_STREAM</span>
         <span className="text-[10px] font-mono text-cyan-400 font-bold">
           {data.length > 0 ? data[data.length - 1].value.toFixed(2) : "0.00"}
         </span>
      </div>
      <div className="flex-1 bg-industrial-950/50 rounded-lg border border-industrial-700/30 overflow-hidden shadow-inner">
        <ResponsiveContainer width="100%" height="100%">
          <AreaChart data={data} margin={{ top: 10, right: 10, left: -20, bottom: 0 }}>
            <defs>
              <linearGradient id={`colorValue-${label}`} x1="0" y1="0" x2="0" y2="1">
                <stop offset="5%" stopColor={color} stopOpacity={0.3}/>
                <stop offset="95%" stopColor={color} stopOpacity={0}/>
              </linearGradient>
            </defs>
            <CartesianGrid strokeDasharray="3 3" stroke="#2a2a2f" vertical={false} />
            <XAxis dataKey="time" hide />
            <YAxis 
                domain={['auto', 'auto']} 
                tick={{ fontSize: 9, fill: '#475569' }} 
                axisLine={false} 
                tickLine={false}
            />
            <Tooltip 
              contentStyle={{ backgroundColor: '#121214', border: '1px solid #2a2a2f', borderRadius: '8px', fontSize: '10px' }}
              itemStyle={{ color: color }}
            />
            <Area 
              type="monotone" 
              dataKey="value" 
              stroke={color} 
              fillOpacity={1} 
              fill={`url(#colorValue-${label})`} 
              strokeWidth={2}
              isAnimationActive={false}
            />
          </AreaChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
};
