package edu.umich.visualsoar.parser;

public class Constant {
	// Data Members
	private int d_beginLine;
	private int d_constType;
	private int d_intConst;
	private float d_floatConst;
	private String d_symConst;

	// Enumeration
	public static final int INTEGER_CONST = 0;
	public static final int SYMBOLIC_CONST = 1;
	public static final int FLOATING_CONST = 2;

	// Deny Default Construction
	private Constant() {}
		
	// Constructors
	public Constant(String symConst,int beginLine) {
		d_beginLine = beginLine;
		d_symConst = symConst;
		d_constType = SYMBOLIC_CONST;
	}
	
	public Constant(int intConst,int beginLine) {
		d_beginLine = beginLine;
		d_intConst = intConst;
		d_constType = INTEGER_CONST;
	}
	
	public Constant(float floatConst,int beginLine) {
		d_beginLine = beginLine;
		d_floatConst = floatConst;
		d_constType = FLOATING_CONST;
	}
	
	
	// Accessor Functions
	public int getConstantType() {
		return d_constType;
	}
	
	public int getBeginLine() {
		return d_beginLine;
	}
	
	public int getIntConst() {
		if(d_constType != INTEGER_CONST)
			throw new IllegalArgumentException("Not a Integer Constant");
		else
			return d_intConst;
	}
	
	public float getFloatConst() {
		if(d_constType != FLOATING_CONST)
			throw new IllegalArgumentException("Not a Floating Point Constant");
		else
			return d_floatConst;
	}
	
	public String getSymConst() {
		if(d_constType != SYMBOLIC_CONST)
			throw new IllegalArgumentException("Not a Symbolic Constant");
		else
			return d_symConst;
	}
	
	public Pair toPair() {
		return new Pair(toString(),d_beginLine);
	}
	
	public String toString() {
		switch(d_constType) {
			case INTEGER_CONST:
				return "" + d_intConst;
			case FLOATING_CONST:
				return "" + d_floatConst;
			case SYMBOLIC_CONST:
				return new String(d_symConst);
			default:
				return new String("Unknown Type");
		}	
	}
}
