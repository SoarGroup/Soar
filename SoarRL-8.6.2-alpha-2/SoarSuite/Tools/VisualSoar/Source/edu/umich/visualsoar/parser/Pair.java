package edu.umich.visualsoar.parser;

public final class Pair implements Comparable {
/////////////////////////////////////
// Data Members
/////////////////////////////////////
	private String d_string;
	private int d_line;
	
/////////////////////////////////////
// Constructors
/////////////////////////////////////
	// Deny Default Construction
	private Pair() {}
	
	public Pair(String string,int line) {
		d_string = string;
		d_line = line;
	}
	
/////////////////////////////////////
// Accessors
/////////////////////////////////////
	public final String getString() {
		return d_string;
	}
	
	public final int getLine() {
		return d_line;
	}
	
	public final boolean equals(Object o) {
		if(o instanceof Pair) {
			Pair p = (Pair)o;
			return d_string.equals(p.getString());
		}
		return false;
	}
	
	public final int compareTo(Object o) {
		Pair p = (Pair)o;
		return d_string.compareTo(p.getString());
	}

}
