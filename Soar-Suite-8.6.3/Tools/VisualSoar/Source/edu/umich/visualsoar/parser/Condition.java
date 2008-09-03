package edu.umich.visualsoar.parser;

public final class Condition {
	// Data Members
	private boolean d_isNegated;
	private PositiveCondition d_positiveCondition;
	
	// Constructors
	public Condition(boolean isNegated,PositiveCondition positiveCondition) {
		d_isNegated = isNegated;
		d_positiveCondition = positiveCondition;
	}
	
	// Accessors
	public final boolean isNegated() {
		return d_isNegated;
	}
	
	public final PositiveCondition getPositiveCondition() {
		return d_positiveCondition;
	}
}
