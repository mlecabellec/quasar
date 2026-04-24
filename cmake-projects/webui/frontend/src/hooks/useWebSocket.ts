import { useEffect, useRef, useState, useCallback } from 'react';

/**
 * @hook useWebSocket
 * @brief Manages a persistent WebSocket connection to the Quasar backend.
 * Supports dev mode redirection to the industrial backend port.
 */
export function useWebSocket() {
  const [connected, setConnected] = useState(false);
  const [lastMessage, setLastMessage] = useState<any>(null);
  const wsRef = useRef<WebSocket | null>(null);

  const connect = useCallback(() => {
    // Port logic:
    // If running on a Vite dev server (5173+), connect to default Quasar WS port (8087)
    // Otherwise, use relative port logic (m_port + 1)
    const currentPort = parseInt(window.location.port);
    let port = currentPort + 1;
    if (currentPort >= 5173 && currentPort < 5200) {
      port = 8087;
    }
    
    const ws = new WebSocket(`ws://${window.location.hostname}:${port}`);

    ws.onopen = () => {
      setConnected(true);
      console.log(`[Quasar] Link established on port ${port}.`);
    };

    ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        setLastMessage(data);
      } catch (e) {
        console.error("[Quasar] WS Parse Error:", e);
      }
    };

    ws.onclose = () => {
      setConnected(false);
      console.warn("[Quasar] Link lost. Retrying in 2s...");
      setTimeout(connect, 2000);
    };

    ws.onerror = (err) => {
      console.error("[Quasar] WS Error:", err);
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
