package edu.umich.visualsoar.graph;
import java.util.Enumeration;
import edu.umich.visualsoar.util.*;

/**
 * This class is a graph class that provides an interface for different kinds of graphs
 * it is based on Object-Oriented Design patterns in C++, later converted to Java
 * @author Brad Jones
 */


public abstract class Graph {
///////////////////////////////////////////////////////////////////
// Data Members
///////////////////////////////////////////////////////////////////
	protected int numberOfVertices = 0;
	protected int numberOfEdges = 0;

///////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////
	protected void depthFirstTraversal(PrePostVisitor visitor, Vertex vertex, boolean[] visited) {
		if (visitor.isDone())
			return;
		visitor.preVisit(vertex);
		visited[vertex.getValue()] = true;
		Enumeration e = emanatingEdges(vertex);
		while(e.hasMoreElements()) {
			Edge edge = (Edge)e.nextElement();
			Vertex to = edge.mate(vertex);
			if (!visited[to.getValue()]) 
				depthFirstTraversal(visitor,to,visited);
		}
		visitor.postVisit(vertex);
	}	
	
	/**
	 * @return the numberOfEdges
	 */
	public int numberOfEdges() {
		return numberOfEdges;
	}
	
	/**
	 * @return the numberOfVertices
	 */
	public int numberOfVertices() {
		return numberOfVertices;
	}
	
	/**
	 * Adds a vertex to the graph 
	 */
	public abstract void addVertex(Vertex v);
	public abstract Vertex selectVertex(int id);
	public Vertex get(int id) {
		return selectVertex(id);
	}
	
	public abstract void addEdge(Edge e);
	public abstract void removeEdge(Edge e);
	
	/**
	 * If you have two vertices, get the edge between them
	 * if the edge exists in the graph it returns that edge
	 * otherwise it returns null
	 */
	public abstract Edge selectEdge(int v0, int v1);
	
	/**
	 * If you have two vertices, tests whether there is an
	 * edge between them, if there is return true otherwise
	 * false
	 */
	public abstract boolean isEdge(int v0, int v1);
	public boolean isConnected() {
		CountingVisitor visitor = new CountingVisitor();
		depthFirstTraversal(new PreOrder(visitor),selectVertex(0));
		return (visitor.count() == numberOfVertices);
	}
	
	/**
	 * Tests the graph for cycles
	 * returns true if there is a cycle, false otherwise
	 */
	public boolean isCyclic() {
		return true;
	}
	
	public abstract Enumeration vertices();
	public abstract Enumeration edges();
	public abstract Enumeration incidentEdges(Vertex v);
	public abstract Enumeration emanatingEdges(Vertex v);
	
	public void depthFirstTraversal(PrePostVisitor visitor, Vertex start) {
		boolean[] visited = new boolean[numberOfVertices];
		for(int i = 0; i < numberOfVertices; ++i)
			visited[i] = false;
		depthFirstTraversal(visitor,start,visited);
	} 
	
	public void breadthFirstTraversal(Visitor visitor, Vertex start) {
		boolean[] enqueued = new boolean[numberOfVertices];
		for(int i = 0; i < numberOfVertices; ++i) 
			enqueued[i] = false;
		Queue queue = new QueueAsLinkedList();
		
		enqueued[start.getValue()] = true;
		queue.enqueue(start);
		while(!queue.isEmpty() && !visitor.isDone()) {
			Vertex vertex = (Vertex)queue.dequeue();
			visitor.visit(vertex);
			Enumeration e = emanatingEdges(vertex);
			while(e.hasMoreElements()) {
				Edge edge = (Edge)e.nextElement();
				Vertex to = edge.mate(vertex);
				if (!enqueued[to.getValue()]) {
					enqueued[to.getValue()] = true;
					queue.enqueue(to);
				}
			}
		}
	}
}
