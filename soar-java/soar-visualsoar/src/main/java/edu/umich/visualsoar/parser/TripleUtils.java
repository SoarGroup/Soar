package edu.umich.visualsoar.parser;

public class TripleUtils {
	// Deny Default Construction
	private TripleUtils() {}

	// Static methods
	public static boolean isVariable(String possibleVar) {
		return possibleVar.startsWith("<") && possibleVar.endsWith(">"); 
	}

  public static boolean isInteger(String possibleInt) {
    for(int i = 0; i < possibleInt.length(); i++) {
      char c = possibleInt.charAt(i);
      if(! ((Character.isDigit(c)) || ( i==0 && c=='-') ))
        return false;
    }
    return true;
  }

  public static boolean isFloat(String possibleFloat) {
    for(int i = 0; i < possibleFloat.length(); i++) {
      char c = possibleFloat.charAt(i);
      if(! ( Character.isDigit(c) || (c == '.')  || ( i==0 && c == '-') ) )
        return false;
      }
      return true;
  }
}

