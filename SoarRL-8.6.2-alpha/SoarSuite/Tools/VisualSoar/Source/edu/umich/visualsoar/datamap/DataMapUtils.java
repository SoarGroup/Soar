package edu.umich.visualsoar.datamap;

public class DataMapUtils {
// Deny Default Construction since there is no need
	public static boolean attributeNameIsValid(String theName) {	
		for (int i = 0; i < theName.length(); i++) {
			char testChar = theName.charAt(i);
			if (! (Character.isLetterOrDigit(testChar) || (testChar == '-'))) {
				return false;
			}
		}
		return true;
	}
}
