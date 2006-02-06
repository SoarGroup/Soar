package edu.umich.visualsoar.parser;

import java.util.List;
import java.util.Iterator;
import java.util.LinkedList;

public final class ConjunctiveTest {
	// Data Members
	private List d_simpleTests = new LinkedList();
	
	// Constructors
	public ConjunctiveTest() {}
	
	// Methods
	public final void add(SimpleTest simpleTest) {
		d_simpleTests.add(simpleTest);
	}
	
	public final Iterator getSimpleTests() {
		return d_simpleTests.iterator();
	}
}
