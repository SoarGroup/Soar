package edu.umich.visualsoar.statemap;
import edu.umich.visualsoar.graph.SoarVertex;

public class ProjectVertex extends SoarVertex {
	private String name;

	public ProjectVertex(int id,String inName) {
		super(id);
		name = inName;
	}
	
	public void write(java.io.Writer w) throws java.io.IOException {
		w.write("PROJECT " + number);
	}
	
	public boolean allowsEmanatingEdges() {
		return true;
	}
	
	public boolean isValid(String s) {
		return false;
	}
	
	public SoarVertex copy(int newId) {
		return new ProjectVertex(newId,name);
	}	

}
