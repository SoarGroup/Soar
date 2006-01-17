/*
 * Created on Jan 7, 2004
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
 *
 * Represents a float in the datamap.
 * 
 * @author Tim Jasko
 */
public class DMFloat extends DMNumericItem {
	
	public DMFloat() {
		super();
	}
	
	public DMFloat(String name) {
		super(name);
	}

	public String getTypeName() {
		return "Float";
	}

	public int initValuesDialog(WorkbenchPart parent) {
		AddNumericDialog win = 
			new AddNumericDialog(
				parent.getSite().getShell(), 
				new FloatInputValidator());
		
		int res = win.open();
		
		if (res == Dialog.OK) {
			String lower = win.getLowerBound();
			if(lower == null || lower.equals(""))
			{
				setLowerBound(new Double(Double.MIN_VALUE));
			}
			else
			{
				setLowerBound(Double.valueOf(lower));
			}

			String upper = win.getUpperBound();
			if(upper == null || upper.equals(""))
			{
				setUpperBound(new Double(Double.MAX_VALUE));
			}
			else
			{
				setUpperBound(Double.valueOf(upper));
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
			(getLowerBound().doubleValue() == Double.MIN_VALUE) 
				? "" : getLowerBound().toString();
		String upper = 
			(getUpperBound().doubleValue() == Double.MAX_VALUE) 
				? "" : getUpperBound().toString();
		
		EditNumericDialog win =
			new EditNumericDialog(
				parent.getSite().getShell(), 
				new FloatInputValidator(), 
				lower, upper);

		int res = win.open();

		if(res == Dialog.OK)
		{
			lower = win.getLowerBound().toString();
			if(lower == null || lower.equals(""))
			{
				setLowerBound(new Double(Double.MIN_VALUE));
			}
			else
			{
				setLowerBound(Double.valueOf(lower));
			}

			upper = win.getUpperBound().toString();
			if(upper == null || upper.equals(""))
			{
				setUpperBound(new Double(Double.MAX_VALUE));
			}
			else
			{
				setUpperBound(Double.valueOf(upper));
			}
		}
		
		return res;

	}


	public boolean acceptsChildren() {
		return false;
	}
	
	public DMItem createNew() {
		DMFloat ret = new DMFloat();
		
		ret.setLowerBound(new Double(Double.MIN_VALUE));
		ret.setUpperBound(new Double(Double.MAX_VALUE));
		
		return ret;
	}

	public String getXML(int depth) {
		String ret = "";
		String tabs = "";

		for (int i=0; i<depth; i++) {
			tabs += '\t';
		} // for i

		ret += tabs;

		ret += "<" + getTypeName() + getSharedXML();
		
		if (getLowerBound().doubleValue() == Double.MIN_VALUE) {
			ret += " lower=\"-infinity\"";
		} else {
			ret += " lower=\"" + getLowerBound() + "\"";
		}
		
		if (getUpperBound().doubleValue() == Double.MAX_VALUE) {
			ret += " upper=\"infinity\" />\n";
		} else {
			ret += " upper=\"" + getUpperBound() + "\" />\n";
		}


		return ret;
	}
	
	public boolean isValidValue(String val) {
		try {
			double f = Double.parseDouble(val);
			double lower = getLowerBound().doubleValue();
			double upper = getUpperBound().doubleValue();
			
			if (f >= lower && f <= upper) {
				return true;
			} else {
				return false;
			}
		}
		catch(NumberFormatException nfe) {
			return false;
		}
	}
	
	public DMItem copy() {
		DMFloat ret = new DMFloat(getName());
		
		ret._comment = this._comment;
		ret._lowerBound = this._lowerBound;
		ret._upperBound = this._upperBound;
		
		return ret;
	}
}
