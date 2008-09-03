package edu.umich.visualsoar.parser;

public class PreferenceSpecifier {
	// Data Members
	private boolean d_isUnaryPreference;
	private RHSValue d_rhs;
	private int d_specType;
	
	// Enumeration
	public static final int ACCEPTABLE = 0;
	public static final int REJECT = 1;
	public static final int EQUAL = 2;
	public static final int GREATER = 3;
	public static final int LESS = 4;
	public static final int REQUIRE = 5;
	public static final int PROHIBIT = 6;
	public static final int AMPERSAND = 7;
	public static final int ATSIGN = 8;
	              
	// Constructors
	private PreferenceSpecifier() {}
	protected PreferenceSpecifier(int type) {
		d_specType = type;
		d_isUnaryPreference = true;
	}
	
	protected PreferenceSpecifier(int type,RHSValue rhsval) {
		d_specType = type;
		d_isUnaryPreference = false;
		d_rhs = rhsval;
	}
	
	// Accessors
	public final boolean isUnaryPreference() {
		return d_isUnaryPreference;
	}
	
	public final int getPreferenceSpecifierType() {
		return d_specType;
	}
	
	public final RHSValue getRHS() {
		if(isUnaryPreference()) 
			throw new IllegalArgumentException("Not a binary preference");
		else
			return d_rhs;
	}
}
