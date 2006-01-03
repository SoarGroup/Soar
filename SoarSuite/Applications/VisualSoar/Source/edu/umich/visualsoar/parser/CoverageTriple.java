package edu.umich.visualsoar.parser;

/**
 * This adds the extra information needed to do the coverage checking
 * for a particular triple
 * @author Brad Jones
 */

public class CoverageTriple extends Triple {
//////////////////////////////////////////////////////////
// DataMembers
//////////////////////////////////////////////////////////
	boolean d_isChecking;
	
//////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////
	public CoverageTriple(Pair variable,Pair attribute,Pair value,boolean isChecking) {
		super(variable,attribute,value);
		d_isChecking = isChecking;
	}
	
	public CoverageTriple(Pair variable,Pair attribute,Pair value,boolean hasState,boolean isChecking) {
		super(variable,attribute,value,hasState);
		d_isChecking = isChecking;
	}
	
	public CoverageTriple(Triple triple,boolean isChecking) {
		super(triple.getVariable(),triple.getAttribute(),triple.getValue(),triple.hasState());
		d_isChecking = isChecking;
	}
	
////////////////////////////////////////////////////////////
// Accessors
////////////////////////////////////////////////////////////
	public boolean isChecking() {
		return d_isChecking;
	}
	
	public String toString() {
		return (d_isChecking) ? "Checking " + super.toString() : "Creating " + super.toString();
	}
}
