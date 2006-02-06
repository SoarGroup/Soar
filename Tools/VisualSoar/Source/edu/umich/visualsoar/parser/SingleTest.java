package edu.umich.visualsoar.parser;

public class SingleTest {
	// Data Members
	private Constant d_constant;
	private Pair d_variable;
	private boolean d_isConstant;
	
	// NOT IMPLEMENTED
	private SingleTest() {}
	
	// Constructors
	public SingleTest(Constant constant) {
		d_constant = constant;
		d_isConstant = true;
	}
	
	public SingleTest(Pair variable) {
		d_variable = variable;
		d_isConstant = false;
	}
	
	// Accessors
	public final boolean isConstant() {
		return d_isConstant;
	}
	
	public final Constant getConstant() {
		if(!d_isConstant) 
			throw new IllegalArgumentException("Not Constant");
		else
			return d_constant;
	}
	
	public final Pair getVariable() {
		if(d_isConstant)
			throw new IllegalArgumentException("Not Variable");
		else
			return d_variable;
	}
}
