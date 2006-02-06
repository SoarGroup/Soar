package edu.umich.visualsoar.graph;
import java.util.*;

public class OrderedDirectedGraphAsAdjacencyLists extends DirectedGraphAsAdjacencyLists {
	public void addEdge(Edge e) {
		NamedEdge ne = (NamedEdge)e;
		String name = ne.getName();
		Vertex start = e.V0();
		LinkedList emanatingEdges = (LinkedList)adjacencyLists.get(start.getValue());
		// Find where to place it in the list
		// Keeps the list in alphabetical order
		boolean found = false;
		ListIterator i = emanatingEdges.listIterator(emanatingEdges.size());
		while(i.hasPrevious() && !found) {
			NamedEdge current = (NamedEdge)i.previous();
			if (current.getName().compareTo(ne.getName()) <= 0) 
				found = true;
		}

		// Put it in the list
		if (!found)
			emanatingEdges.addFirst(e);
		else {
			i.next();
			i.add(e);
		}
		++numberOfEdges;
	}
}
