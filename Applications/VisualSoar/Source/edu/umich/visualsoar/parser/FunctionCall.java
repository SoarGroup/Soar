package edu.umich.visualsoar.parser;
import java.util.*;

public final class FunctionCall {
	private Pair d_functionName;
	private List d_rhsValues = new LinkedList();
	
	
	// Constructors
	private FunctionCall() {}
	
	public FunctionCall(Pair functionName) {
		d_functionName = functionName;
	}
	
	// Member Functions
	public final void add(RHSValue rhsValue) {
		d_rhsValues.add(rhsValue);
	}
	
	public final Iterator getRHSValues() {
		return d_rhsValues.iterator();
	}
	
	public final Pair getFunctionName() {
		return d_functionName;
	}
} 
