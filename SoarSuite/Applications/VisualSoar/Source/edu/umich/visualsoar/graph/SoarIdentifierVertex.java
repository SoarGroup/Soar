package edu.umich.visualsoar.graph;

public class SoarIdentifierVertex extends SoarVertex  {
///////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////
	public SoarIdentifierVertex(int id) {
		super(id);
	}

///////////////////////////////////////////////////
// Accessors
///////////////////////////////////////////////////	
	public SoarVertex copy(int newId) {
		return new SoarIdentifierVertex(newId);
	}

	public boolean allowsEmanatingEdges() {
		return true;
	}
	
	public boolean isValid(String s) {
		return false;
	}

	public String toString() {
		return "";
	}
	
///////////////////////////////////////////////////
// Manipulators
///////////////////////////////////////////////////	
		public void write(java.io.Writer w) throws java.io.IOException {
		w.write("SOAR_ID " + number + '\n');
	}
	
}
