package edu.umich.visualsoar.statemap;
import edu.umich.visualsoar.graph.SoarVertex;

public class FileVertex extends SoarVertex {
	private String name;

	public FileVertex(int id,String inName) {
		super(id);
		name = inName;
	}
	
	public void write(java.io.Writer w) throws java.io.IOException {
		w.write("FILE " + number);
	}
	
	public boolean allowsEmanatingEdges() {
		return false;
	}
	
	public boolean isValid(String s) {
		return false;
	}
	
	public SoarVertex copy(int newId) {
		return new FileVertex(newId,new String(name));
	}
}
