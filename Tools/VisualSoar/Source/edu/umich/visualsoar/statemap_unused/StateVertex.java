package edu.umich.visualsoar.statemap;
import edu.umich.visualsoar.graph.SoarVertex;
import java.util.*;
import java.io.File;

public class StateVertex extends SoarVertex {
	private String name;
	private Set files = new TreeSet();
	
	public StateVertex(int id,String inName) {
		super(id);
		name = inName;
	}
	
	public void write(java.io.Writer w) throws java.io.IOException {
		w.write("STATE " + number);
	}
	
	public boolean allowsEmanatingEdges() {
		return true;
	}
	
	public boolean isValid(String s) {
		return false;
	}
	
	public SoarVertex copy(int newId) {
		return new StateVertex(newId,new String(name));
	}
		
	public String toString() {
		return name;
	}
	
	public void addFile(File file) {
		files.add(file);
	}
	
	public void removeFile(File file) {
		files.remove(file);
	}
	
	public Iterator listFiles() {
		return files.iterator();
	}
}
