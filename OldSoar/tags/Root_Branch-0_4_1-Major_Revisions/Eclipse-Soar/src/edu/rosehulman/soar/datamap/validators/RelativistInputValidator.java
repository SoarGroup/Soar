/*
 * Created on Dec 26, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.validators;

import org.eclipse.jface.dialogs.IInputValidator;

/**
 *
 * Darn relativists. Just going around accepting everything.
 * Performs no validation on the input string. It simply accepts everything.
 * For use with the InputDialog where no validation is necessary. 
 * 
 * @author Tim Jasko
 */
public class RelativistInputValidator implements IInputValidator {

	public String isValid(String newText) {
		return null;
	}

}
