/*
 * Created on Jan 27, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.validators;

import org.eclipse.jface.dialogs.IInputValidator;

/**
 *
 * Ensures that the name of a Datamap attribute is valid.
 * 
 * @author Tim Jasko
 */
public class AttributeNameValidator implements IInputValidator {

	public String isValid(String newText) {
	
		//This way is better
		for (int i=0; i<newText.length(); i++) {
			char c = newText.charAt(i);
			
			if (! (Character.isLetterOrDigit(c) || c=='-')) {
				return "Attribute names may contain only letters, numbers, and hyphens";
			}
		} // for i
		
		return null;
	}

}
