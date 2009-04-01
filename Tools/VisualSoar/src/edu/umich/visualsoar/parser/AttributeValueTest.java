package edu.umich.visualsoar.parser;
import java.util.*;

public final class AttributeValueTest {
	// Data Members
	boolean d_isNegated = false;
	List d_attributeTests = new LinkedList();
	List d_valueTests = new LinkedList();
	
	// Accessors
	public final void negate() {
		d_isNegated = true;
	}
	
	public final void add(AttributeTest at) {
		d_attributeTests.add(at);
	}
	
	public final void add(ValueTest vt) {
		d_valueTests.add(vt);
	}
	
	public final Iterator getAttributeTests() {
		return d_attributeTests.iterator();
	}
	
	public final Iterator getValueTests() {
		return d_valueTests.iterator();
	}
	
	public final boolean isNegated() {
		return d_isNegated;
	}
}
