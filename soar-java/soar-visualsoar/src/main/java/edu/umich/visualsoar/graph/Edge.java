package edu.umich.visualsoar.graph;

/**
 * This class represents an edge in a graph
 * it is based on Object-Oriented Design patterns in C++, later converted to Java
 * @author Brad Jones
 */

public class Edge implements java.io.Serializable {
////////////////////////////////////////
// DataMembers
////////////////////////////////////////
	/**
	 * v0 is the starting edge
	 * v1 is the ending edge
	 */
	protected Vertex v0;
	protected Vertex v1;

////////////////////////////////////////
// Constructors
////////////////////////////////////////
	// Deny default construction
	private Edge() {}

	/**
	 * constructs and edge between the first
	 * vertex and the second one
	 */
	public Edge(Vertex _v0, Vertex _v1) {
		v0 = _v0;
		v1 = _v1;
	}
	
////////////////////////////////////////
// Methods
////////////////////////////////////////

 	/**
 	 * @return the starting edge
 	 */
	public Vertex V0() {
		return v0;
	}
	
	/**
 	 * @return the ending edge
 	 */
	public Vertex V1() {
		return v1;
	}

	/**
	 * if v equals the starting edge then return the ending edge
	 * if v equal the ending edge then return the starting edge
	 * else throw and exception
	 * @throws IllegalArgumentException
 	 * @return the starting edge
 	 */
	public Vertex mate(Vertex v) {
		if (v.equals(v0)) 
			return v1;
		if (v.equals(v1))
			return v0;
		throw new IllegalArgumentException("Vertex passed in is not part of this edge");
	}
	
	/** 
	 * Produces string representation of the edge
	 * @return the representation
	 */
	public String toString() {
		return "This edge connects " + v0.toString() + " to " + v1.toString();
	}
}
