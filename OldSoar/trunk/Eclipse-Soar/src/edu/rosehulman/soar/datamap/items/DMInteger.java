/*
 * Created on Dec 30, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items;

import edu.rosehulman.soar.datamap.items.dialogs.*;
import edu.rosehulman.soar.datamap.validators.*;

import org.eclipse.ui.part.*;
import org.eclipse.jface.dialogs.*;

/**
 * Represents an Integer on the datamap.
 * 
 * @author Tim Jasko
 */
public class DMInteger extends DMNumericItem {

	public DMInteger() {
		super();
	}
	
	public DMInteger(String name) {
		super(name);
	}

	public String getTypeName() {
		return "Integer";
	}

	
	public int initValuesDialog(WorkbenchPart parent) {
		AddNumericDialog win = 
			new AddNumericDialog(
				parent.getSite().getShell(), 
				new IntegerInputValidator());
		
		int res = win.open(); 
		
		if (res == Dialog.OK) {
			String lower = win.getLowerBound();
			if(lower == null || lower.equals(""))
			{
				setLowerBound(new Integer(Integer.MIN_VALUE));
			}
			else
			{
				setLowerBound(Integer.valueOf(lower));
			}

			String upper = win.getUpperBound();
			if(upper == null || upper.equals(""))
			{
				setUpperBound(new Integer(Integer.MAX_VALUE));
			}
			else
			{
				setUpperBound(Integer.valueOf(upper));
			}
			setName(win.getName());
		} // if
		
		return res;
	}


	public boolean canEditValues() {
		return true;
	}
	
	public int editValuesDialog(WorkbenchPart parent) {

		String lower = 
			(getLowerBound().intValue() == Integer.MIN_VALUE) 
				? "" : getLowerBound().toString();
		String upper = 
			(getUpperBound().intValue() == Integer.MAX_VALUE) 
				? "" : getUpperBound().toString();
		
		EditNumericDialog win =
			new EditNumericDialog(
				parent.getSite().getShell(), 
				new IntegerInputValidator(), 
				lower, upper);

		int res = win.open();

		if(res == Dialog.OK)
		{
			lower = win.getLowerBound().toString();
			if(lower == null || lower.equals(""))
			{
				setLowerBound(new Integer(Integer.MIN_VALUE));
			}
			else
			{
				setLowerBound(Integer.valueOf(lower));
			}

			upper = win.getUpperBound().toString();
			if(upper == null || upper.equals(""))
			{
				setUpperBound(new Integer(Integer.MAX_VALUE));
			}
			else
			{
				setUpperBound(Integer.valueOf(upper));
			}
		}
		
		return res;
	}

	public boolean acceptsChildren() {
		return false;
	}
	
	public DMItem createNew() {
		return new DMInteger();
	}
	
	public String getXML(int depth) {
		String ret = "";
		String tabs = "";
	
		for (int i=0; i<depth; i++) {
			tabs += '\t';
		} // for i
	
		ret += tabs;
	
		ret += "<" + getTypeName() + " name=\"" + getName() + "\""
			+ " comment=\"" + getComment() + "\"" 
			+ " lower=\"" + getLowerBound() + "\""
			+ " upper=\"" + getUpperBound() + "\"" + " />\n";
	
	
		return ret;
	}

}
