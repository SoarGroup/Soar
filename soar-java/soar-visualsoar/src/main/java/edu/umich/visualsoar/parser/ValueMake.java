package edu.umich.visualsoar.parser;

import java.util.*;

public final class ValueMake {
	// Data Members
	private RHSValue d_rhsValue;
	private List preferenceSpecifiers = new LinkedList();

	// Constructors
	private ValueMake() {}
	public ValueMake(RHSValue rhsValue) {
		d_rhsValue = rhsValue;
	}
	
	public final void add(PreferenceSpecifier ps) {
		preferenceSpecifiers.add(ps);
	}
	
	// Accessors
	public final RHSValue getRHSValue() {
		return d_rhsValue;
	}
	
	public final Iterator getPreferenceSpecifiers() {
		return preferenceSpecifiers.iterator();
	}
}
