package edu.umich.visualsoar.parser;

import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;

public final class ConditionSide {
	private List d_conditions = new LinkedList();
	
	// Constructor
	public ConditionSide() {
	}
	
	// Accessors
	public final void add(Condition condition) {
		d_conditions.add(condition);
	}
	
	public final Iterator getConditions() {
		return d_conditions.iterator();
	}
}
