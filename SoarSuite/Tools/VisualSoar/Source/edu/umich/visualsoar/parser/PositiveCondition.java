package edu.umich.visualsoar.parser;

import java.util.*;

public final class PositiveCondition {
	// Data Members
	private ConditionForOneIdentifier d_condition;
	private List d_conjunction;
	private boolean d_isConjunction;
	
	// Constructors
	public PositiveCondition() {
		d_isConjunction = true;
		d_conjunction = new LinkedList();
	}
	
	public PositiveCondition(ConditionForOneIdentifier cfoi) {
		d_condition = cfoi;
		d_isConjunction = false;
	}
	
	// Accessors
	public final boolean isConjunction() {
		return d_isConjunction;
	}
	
	public final void add(Condition c) {
		if(!d_isConjunction)
			throw new IllegalArgumentException("Not Conjunction");
		else
			d_conjunction.add(c);
	}
	
	public final Iterator getConjunction() {
		if(!d_isConjunction) 
			throw new IllegalArgumentException("Not Conjunction");
		else
			return d_conjunction.iterator();
	}
	
	public final ConditionForOneIdentifier getConditionForOneIdentifier() {
		if(d_isConjunction)
			throw new IllegalArgumentException("Not Condition For One Identifier");
		else
			return d_condition;
	}
}
