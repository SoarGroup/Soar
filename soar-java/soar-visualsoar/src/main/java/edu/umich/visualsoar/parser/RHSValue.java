package edu.umich.visualsoar.parser;

import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;

public final class RHSValue {
	// Data Members
	private Constant d_constant;
	private Pair d_variable;
	private FunctionCall d_functionCall;
	private boolean d_isConstant;
	private boolean d_isVariable;
	
	
	// Constructors
	private RHSValue() {}
	
	public RHSValue(Constant c) {
		d_constant = c;
		d_isConstant = true;
		d_isVariable = false;
	}
	
	public RHSValue(Pair variable) {
		d_variable = variable;
		d_isConstant = false;
		d_isVariable = true;
	}
	
	public RHSValue(FunctionCall functionCall) {
		d_functionCall = functionCall;
		d_isConstant = false;
		d_isVariable = false;
	}
	
	// Member Functions	
	public final boolean isConstant() {
		return d_isConstant;
	}
	
	public final boolean isVariable() {
		return d_isVariable;
	}
	
	public final boolean isFunctionCall() {
		return (!d_isConstant && !d_isVariable);
	}

	public final Constant getConstant() {
		if(!isConstant())
			throw new IllegalArgumentException("Not a Constant");
		else
			return d_constant;
	}
	
	public final Pair getVariable() {
		if(!isVariable()) 
			throw new IllegalArgumentException("Not a Variable");
		else
			return d_variable;
	}
	
	public final FunctionCall getFunctionCall() {
		if(!isFunctionCall())
			throw new IllegalArgumentException("Not a Function Call");
		else
			return d_functionCall;
	}
}
