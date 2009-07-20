package edu.umich.visualsoar.parser;
import java.util.*;

public final class VarAttrValMake {
	private Pair d_variable;
	private List d_attributeValueMakes = new LinkedList();
	
	private VarAttrValMake() {}
	
	public VarAttrValMake(Pair variable) {
		d_variable = variable;
	}
	
	// Member Functions
	public final void add(AttributeValueMake avm) {
		d_attributeValueMakes.add(avm);
	}
	
	public final Iterator getAttributeValueMakes() {
		return d_attributeValueMakes.iterator();
	}
	
	public final Pair getVariable() {
		return d_variable;
	}
}
