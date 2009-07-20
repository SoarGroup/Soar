package edu.umich.visualsoar.graph;
import edu.umich.visualsoar.util.*;
import java.util.*;

/**
 * This class is an implementation of the DirectedGraph class, using Adjacency Lists
 * it is based on Object-Oriented Design patterns in C++, later converted to Java
 * @author Brad Jones
 */

public class DirectedGraphAsAdjacencyLists extends DirectedGraph {
//////////////////////////////////////////////////////////
// Data Members
//////////////////////////////////////////////////////////
	protected Vector vertices = new Vector();
	protected Vector adjacencyLists = new Vector();

/////////////////////////////////////////////////////////
// Methods
/////////////////////////////////////////////////////////	
	public void addVertex(Vertex v) {
		if (v.getValue() == numberOfVertices) {
			vertices.add(v);
			adjacencyLists.add(new LinkedList());		
			++numberOfVertices;
		}
		else {
			vertices.set(v.getValue(),v);
			adjacencyLists.set(v.getValue(),new LinkedList());
		}
		
	}
	
	public Vertex selectVertex(int id) {
		return (Vertex)vertices.get(id);
	}
	
	public void addEdge(Edge e) {
		Vertex start = e.V0();
		LinkedList emanatingEdges = (LinkedList)adjacencyLists.get(start.getValue());
		emanatingEdges.add(e);
		++numberOfEdges;
	}
	
	public void removeEdge(Edge e) {
		Vertex start = e.V0();
		LinkedList emanatingEdges = (LinkedList)adjacencyLists.get(start.getValue());
		emanatingEdges.remove(e);
		--numberOfEdges;
	}
	
	public Edge selectEdge(int v0, int v1) {
		LinkedList emanatingEdges = (LinkedList)adjacencyLists.get(v0);
		Iterator i = emanatingEdges.iterator();
		while(i.hasNext()) {
			Edge edge = (Edge)i.next();
			if (edge.V1().getValue() == v1)
				return edge;
		}
		return null;
	}
	
	public boolean isEdge(int v0, int v1) {
		LinkedList emanatingEdges = (LinkedList)adjacencyLists.get(v0);
		Iterator i = emanatingEdges.iterator();
		while(i.hasNext()) {
			Edge edge = (Edge)i.next();
			if (edge.V1().getValue() == v1)
				return true;
		}
		return false;
	}
	
	public Enumeration vertices() {
		return vertices.elements();
	}
	
	public Enumeration edges() {
		LinkedList allEdges = new LinkedList();
		for(int i = 0; i < numberOfVertices; ++i) 
			allEdges.addAll(((LinkedList)(LinkedList)adjacencyLists.get(i)));
		return new EnumerationIteratorWrapper(allEdges.iterator());			
	}
	
	public Enumeration emanatingEdges(Vertex v) {
		LinkedList emanatingEdges = (LinkedList)adjacencyLists.get(v.getValue());
		Iterator i = emanatingEdges.iterator();
		return new EnumerationIteratorWrapper(i);
	}
	
	public Enumeration incidentEdges(Vertex v) {
		LinkedList incidentEdges = new LinkedList();
		Enumeration e = edges();
		while(e.hasMoreElements()) {
			Edge edge = (Edge)e.nextElement();
			if (v.getValue() == edge.V1().getValue())
				incidentEdges.add(edge);
		}
		return new EnumerationIteratorWrapper(incidentEdges.iterator());
	}		
	
	public void reduce(List listOfStartVertices) {
		// This code finds all the unvisited nodes
		boolean[] visited = new boolean[numberOfVertices()];
		for(int i = 0; i < visited.length; ++i)
			visited[i] = false;
		Visitor doNothing = new DoNothingVisitor();
		PrePostVisitor dnPreVisitor = new PreOrder(doNothing);
		Enumeration e = new EnumerationIteratorWrapper(listOfStartVertices.iterator());
		while(e.hasMoreElements()) {
			Vertex startVertex = (Vertex)e.nextElement();
			if(!visited[startVertex.getValue()])
				depthFirstTraversal(dnPreVisitor, startVertex,visited);
		}
		
		//This code maps visted nodes to new ids
		Hashtable ht = new Hashtable();
		int newNumberOfVertices = 0;
		for(int i = 0; i < visited.length; ++i) {
			if(visited[i]) 
				ht.put(new Integer(i),new Integer(newNumberOfVertices++));
		}
		
		// Make up the new vertices
		Vector newVertices = new Vector();
		for(int i = 0; i < numberOfVertices(); ++i) {
			Integer newId = (Integer)ht.get(new Integer(i));
			if(newId != null) {
				Vertex v = (Vertex)vertices.get(i);
				v.setValue(newId.intValue());
				newVertices.add(v);
			}
		}
		
		// Make up the new edges
		Vector newAdjacencyLists = new Vector();
		int newNumberOfEdges = 0;
		for(int i = 0; i < numberOfVertices; ++i) {
			Integer newId = (Integer)ht.get(new Integer(i));
			if(newId != null) {
				newAdjacencyLists.add(adjacencyLists.get(i));
				newNumberOfEdges += ((List)adjacencyLists.get(i)).size();		
			}						
		}
		
		// Update the Working Memory
		vertices = newVertices;
		adjacencyLists = newAdjacencyLists;
		numberOfVertices = newNumberOfVertices;
		numberOfEdges = newNumberOfEdges;
	}
	
	public void resolve() {
		for(int i = 0; i < adjacencyLists.size(); ++i) {
			for(int j = 0; j < ((List)adjacencyLists.get(i)).size(); ++j) {
				LinkedList linkedList = (LinkedList)adjacencyLists.get(i);
				NamedEdge e = (NamedEdge)linkedList.get(j);
				if(e.V0() != selectVertex(e.V0().getValue()) ||
				   e.V1() != selectVertex(e.V1().getValue())) {
				 	NamedEdge ne = new NamedEdge(selectVertex(e.V0().getValue()),selectVertex(e.V1().getValue()),e.getName());
				 	linkedList.set(j,ne);
				}
			}
		}
	}
}
