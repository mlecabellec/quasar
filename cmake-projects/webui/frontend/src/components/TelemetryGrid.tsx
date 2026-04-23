import { useState, useEffect } from 'react';
import { TelemetryChart } from './TelemetryChart';

interface TelemetryGridProps {
  lastMessage: any;
}

/**
 * @component TelemetryGrid
 * @brief Manages a sliding window of telemetry data for the dashboard.
 */
export const TelemetryGrid = ({ lastMessage }: TelemetryGridProps) => {
  const [streams, setStreams] = useState<Record<string, { time: string, value: number }[]>>({});

  useEffect(() => {
    if (lastMessage?.action === "batch_update") {
      const timestamp = new Date().toLocaleTimeString();
      
      setStreams(prev => {
        const next = { ...prev };
        lastMessage.updates.forEach((u: any) => {
          if (typeof u.value === 'number') {
            if (!next[u.name]) next[u.name] = [];
            const newStream = [...next[u.name], { time: timestamp, value: u.value }];
            // Keep last 50 points (Sliding Window)
            next[u.name] = newStream.slice(-50);
          }
        });
        return next;
      });
    }
  }, [lastMessage]);

  const streamNames = Object.keys(streams);

  return (
    <div className="grid grid-cols-2 gap-6 h-full">
      {streamNames.length > 0 ? (
        streamNames.slice(0, 2).map((name) => (
          <div key={name} className="h-full">
             <TelemetryChart label={name} data={streams[name]} />
          </div>
        ))
      ) : (
        <div className="col-span-2 flex items-center justify-center border border-dashed border-industrial-700/50 rounded-2xl opacity-30 text-[10px] font-black uppercase tracking-[0.4em]">
          No Telemetry Active
        </div>
      )}
    </div>
  );
};
