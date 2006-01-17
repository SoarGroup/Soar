/*
 * Created on Jan 7, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.validators;

import org.eclipse.jface.dialogs.IInputValidator;

/**
 * Validates for float values.
 * 
 * @author Tim Jasko
 */
public class FloatInputValidator implements IInputValidator {
	
	public String isValid(String newText) {
		
		// a blank field indicates infinity
		if (newText == null || newText.equals("")) {
			return null;
		} // if
		
		try {
			Double.valueOf(newText);
			
			//the string was valid if we get this far
			
			return null;
			
		} catch (NumberFormatException e) { // the string is not valid
			return "You must enter a float";
		}
	}

}
