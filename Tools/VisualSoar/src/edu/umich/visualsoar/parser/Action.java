package edu.umich.visualsoar.parser;
import java.util.*;

public final class Action {
	// Data Members
	private VarAttrValMake d_varAttrValMake;
	private FunctionCall d_functionCall;
	private boolean d_isVarAttrValMake;
	
	// NOT IMPLEMENTED
	private Action() {}

	// Constructors
	public Action(VarAttrValMake varAttrValMake) {
		d_varAttrValMake = varAttrValMake;
		d_isVarAttrValMake = true;
	}
	
	public Action(FunctionCall functionCall) {
		d_functionCall = functionCall;
		d_isVarAttrValMake = false;
	}
	// Accessors
	public final boolean isVarAttrValMake() {
		return d_isVarAttrValMake;
	}
	
	public final VarAttrValMake getVarAttrValMake() {
		if(!d_isVarAttrValMake)
			throw new IllegalArgumentException("Not Variable Attribute Value Make");
		else
			return d_varAttrValMake;
	}
	
	public final FunctionCall getFunctionCall() {
		if(d_isVarAttrValMake) 
			throw new IllegalArgumentException("Not a Function Call");
		else
			return d_functionCall;
	}
}
