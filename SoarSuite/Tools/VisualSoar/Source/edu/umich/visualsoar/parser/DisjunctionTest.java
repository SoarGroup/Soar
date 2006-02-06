package edu.umich.visualsoar.parser;

import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;

public final class DisjunctionTest {
	// Data Members
	private List d_constants = new LinkedList();
	
	// Constructors
	public DisjunctionTest() {}
	
	// Accessors
	public final void add(Constant constant) {
		d_constants.add(constant);
	}
	
	public final Iterator getConstants() {
		return d_constants.iterator();
	}
}
	
