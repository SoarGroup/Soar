package edu.umich.visualsoar.parser;

public final class Test {
	// Data Members
	private ConjunctiveTest d_conjunctiveTest;
	private SimpleTest d_simpleTest;
	private boolean d_isConjunctiveTest; 
	
	
	// Constructors
	public Test(ConjunctiveTest conjunctiveTest) {
		d_conjunctiveTest = conjunctiveTest;
		d_isConjunctiveTest = true;
	}
	
	public Test(SimpleTest simpleTest) {
		d_simpleTest = simpleTest;
		d_isConjunctiveTest = false;
	}
	
	// Accessors
	public final boolean isConjunctiveTest() {
		return d_isConjunctiveTest;
	}
	
	public final SimpleTest getSimpleTest() {
		if(d_isConjunctiveTest) {
			throw new IllegalArgumentException("not a simple test");
		}
		else
			return d_simpleTest;
	}
	
	public final ConjunctiveTest getConjunctiveTest() {
		if(!d_isConjunctiveTest) {
			throw new IllegalArgumentException("not a conjunctive test");
		}
		else
			return d_conjunctiveTest;
	}
}
