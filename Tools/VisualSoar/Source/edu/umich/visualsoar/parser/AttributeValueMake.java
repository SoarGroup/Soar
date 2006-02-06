package edu.umich.visualsoar.parser;
import java.util.*;

public final class AttributeValueMake {
	// Data Members
	private List d_valueMakes = new LinkedList();
	private List d_rhsValues = new LinkedList();
	
	// Constructors
	public AttributeValueMake() {}
	
	
	// Accessors
	public final void add(RHSValue rhsv) {
		d_rhsValues.add(rhsv);
	}
	
	public final void add(ValueMake vm) {
		d_valueMakes.add(vm);
	}
	
	public final Iterator getRHSValues() {
		return d_rhsValues.iterator();
	}
	
	public final Iterator getValueMakes() {
		return d_valueMakes.iterator();
	}
	
}
