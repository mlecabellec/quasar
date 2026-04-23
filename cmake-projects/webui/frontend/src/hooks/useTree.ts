import { useState, useCallback, useEffect, useMemo } from 'react';

export interface QuasarNode {
  name: string;
  type: string;
  path: string;
  value?: any;
  children?: QuasarNode[];
  isExpanded?: boolean;
}

/**
 * @hook useTree
 * @brief Manages the state and real-time discovery of the Quasar hierarchy.
 * Now supports fuzzy filtering for high-speed navigation.
 */
export function useTree(lastWsMessage: any) {
  const [tree, setTree] = useState<QuasarNode[]>([]);
  const [selectedNode, setSelectedNode] = useState<QuasarNode | null>(null);
  const [filter, setFilter] = useState("");

  const fetchNodes = useCallback(async (path: string = "/") => {
    try {
      const response = await fetch(`/api/v1/walk?path=${path}&limit=100`);
      const data = await response.json();
      if (data.status === "ok") {
        return data.nodes.map((n: any) => ({
          ...n,
          path: path === "/" ? `/${n.name}` : `${path}/${n.name}`,
          isExpanded: false
        }));
      }
    } catch (e) {
      console.error("[Tree] Walk failed", e);
    }
    return [];
  }, []);

  const expandNode = useCallback(async (path: string) => {
    const children = await fetchNodes(path);
    setTree(prev => {
      const updateChildren = (list: QuasarNode[]): QuasarNode[] => {
        return list.map(node => {
          if (node.path === path) return { ...node, children, isExpanded: true };
          if (node.children) return { ...node, children: updateChildren(node.children) };
          return node;
        });
      };
      return updateChildren(prev);
    });
  }, [fetchNodes]);

  // --- Filter Logic ---
  const filteredTree = useMemo(() => {
    if (!filter) return tree;
    const lowerFilter = filter.toLowerCase();
    
    const filterNodes = (list: QuasarNode[]): QuasarNode[] => {
      return list.reduce((acc: QuasarNode[], node) => {
        const matches = node.name.toLowerCase().includes(lowerFilter) || node.type.toLowerCase().includes(lowerFilter);
        const filteredChildren = node.children ? filterNodes(node.children) : [];
        
        if (matches || filteredChildren.length > 0) {
          acc.push({ ...node, children: filteredChildren, isExpanded: matches ? node.isExpanded : true });
        }
        return acc;
      }, []);
    };
    
    return filterNodes(tree);
  }, [tree, filter]);

  // --- Real-time Sync Logic ---
  useEffect(() => {
    if (lastWsMessage?.action === "batch_update") {
      const updates = lastWsMessage.updates;
      setTree(prev => {
        const applyUpdates = (list: QuasarNode[]): QuasarNode[] => {
          return list.map(node => {
            const update = updates.find((u: any) => u.name === node.name);
            let newNode = update ? { ...node, value: update.value } : node;
            if (newNode.children) newNode.children = applyUpdates(newNode.children);
            return newNode;
          });
        };
        return applyUpdates(prev);
      });
    }
  }, [lastWsMessage]);

  useEffect(() => {
    fetchNodes("/").then(setTree);
  }, [fetchNodes]);

  return { tree: filteredTree, selectedNode, setSelectedNode, expandNode, filter, setFilter };
}
