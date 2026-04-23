import { useEffect, useRef, useState, useCallback } from 'react';

/**
 * @hook useWebSocket
 * @brief Manages a persistent WebSocket connection to the Quasar backend.
 */
export function useWebSocket() {
  const [connected, setConnected] = useState(false);
  const [lastMessage, setLastMessage] = useState<any>(null);
  const wsRef = useRef<WebSocket | null>(null);

  const connect = useCallback(() => {
    // Port logic matching C++ WebUIService (m_port + 1)
    const port = parseInt(window.location.port) + 1;
    const ws = new WebSocket(`ws://${window.location.hostname}:${port}`);

    ws.onopen = () => {
      setConnected(true);
      console.log("[Quasar] Link established.");
    };

    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      setLastMessage(data);
    };

    ws.onclose = () => {
      setConnected(false);
      console.warn("[Quasar] Link lost. Retrying in 2s...");
      setTimeout(connect, 2000);
    };

    wsRef.current = ws;
  }, []);

  useEffect(() => {
    connect();
    return () => wsRef.current?.close();
  }, [connect]);

  const send = useCallback((action: string, payload: any) => {
    if (wsRef.current?.readyState === WebSocket.OPEN) {
      wsRef.current.send(JSON.stringify({ action, ...payload }));
    }
  }, []);

  return { connected, lastMessage, send };
}
