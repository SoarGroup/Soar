package edu.umich.visualsoar.parser;

public final class SimpleTest {
	// Data Members
	private DisjunctionTest d_disjunctionTest;
	private RelationalTest d_relationalTest;
	private boolean d_isDisjunctionTest;
	
	// NOT IMPLEMENTED
	private SimpleTest() {}
	
	// Constructor
	public SimpleTest(DisjunctionTest disjunctionTest) {
		d_disjunctionTest = disjunctionTest;
		d_isDisjunctionTest = true;
	}
	
	public SimpleTest(RelationalTest relationalTest) {
		d_relationalTest = relationalTest;
		d_isDisjunctionTest = false;
	}
	
	// Accessors
	public final boolean isDisjunctionTest() {
		return d_isDisjunctionTest;
	}
	
	public final DisjunctionTest getDisjunctionTest() {
		if(!d_isDisjunctionTest) 
			throw new IllegalArgumentException("Not Disjunction");
		else
			return d_disjunctionTest;
	}
	
	public final RelationalTest getRelationalTest() {
		if(d_isDisjunctionTest) 
			throw new IllegalArgumentException("Not Relation");
		else
			return d_relationalTest;
	}
}
