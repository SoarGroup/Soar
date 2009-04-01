package edu.umich.visualsoar.parser;

import java.util.*;

public final class ConditionForOneIdentifier {
	// Data Members
	private boolean d_hasState;
	private Pair d_variable;
	private List d_attributeValueTests = new LinkedList();
	
	// Constructor
	public ConditionForOneIdentifier(boolean hasState,Pair variable) {
		d_variable = variable;
		d_hasState = hasState;
	}
		
	// Accessor
	public final boolean hasState() {
		return d_hasState;
	}
	
	public final Pair getVariable() {
		return d_variable;
	}
	
	public final void add(AttributeValueTest avt) {
		d_attributeValueTests.add(avt);
	}
	
	public final Iterator getAttributeValueTests() {
		return d_attributeValueTests.iterator();
	}
}
