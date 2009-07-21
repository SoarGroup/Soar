package edu.umich.visualsoar.parser;

/**
 * @author Brad Jones
 * @version 0.75 3 Mar 2000
 */

public final class ValueTest {
	// Data Members
	private Test d_test;
	private boolean d_acceptablePreference = false;

	// Constructor
	public ValueTest(Test test) {
		d_test = test;
	}
	
	// Accessors
	public final Test getTest() {
		return d_test;
	}

	public final boolean hasAcceptablePreference() {
		return d_acceptablePreference;
	}
	
	// Modifiers
	public final void acceptablePreference() {
		d_acceptablePreference = true;
	}
}
