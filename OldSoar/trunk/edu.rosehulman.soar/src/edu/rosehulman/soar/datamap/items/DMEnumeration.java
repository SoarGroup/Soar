/*
 * Created on Jan 7, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items;

import edu.rosehulman.soar.datamap.items.dialogs.*;

import java.util.ArrayList;

import org.eclipse.ui.part.*;
import org.eclipse.jface.dialogs.*;

/**
 *
 * Represents an enumeration in the datamap.
 * 
 * @author Tim Jasko
 */
public class DMEnumeration extends DMItem {
	private ArrayList _enums = new ArrayList();

	public DMEnumeration() {
		super();
	}
	
	public DMEnumeration(String name) {
		super(name);
	}

	public String getTypeName() {
		return "Enumeration";
	}
	
	/**
	 * Gets the enumerations for this item.
	 * @return The enumerations
	 */
	public ArrayList getEnums() {
		return _enums;
	}
	
	/**
	 * Sets this item's enumerations
	 * @param enums The new enumerations
	 */
	public void setEnums(ArrayList enums) {
		_enums = enums;
	}
	
	public String toString() {
		String temp = getName() + ": [ ";
		for (int i=0; i<_enums.size(); i++) {
			temp += _enums.get(i) + " ";
		}
		temp += ']';
		
		String comment = getComment(); 
		
		if (comment != null && !comment.equals("")) {
			temp += "       * " + comment + " *";
		} // if
		
		return temp;
	}


	public int initValuesDialog(WorkbenchPart parent) {
		AddEnumerationDialog win = 
			new AddEnumerationDialog(parent.getSite().getShell());
		
		int res = win.open(); 
		
		if (res == Dialog.OK) {
			setName(win.getName());
			setEnums(win.getEnums());
		} // if
		
		return res;
	}

	public boolean canEditValues() {
		return true;
	}


	public int editValuesDialog(WorkbenchPart parent) {
		EditEnumerationDialog win =
			new EditEnumerationDialog(parent.getSite().getShell(), _enums);

		int res = win.open();

		if(res == Dialog.OK)
		{
			setEnums(win.getEnums());
		}
		
		return res;
	}

	public boolean acceptsChildren() {
		return false;
	}
	
	public DMItem createNew() {
		return new DMEnumeration();
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
			+ ">\n";
		
		//Pop each of the enumerations into there
		for (int i=0; i<getEnums().size(); i++) {
			ret += tabs + "\t<enum value=\"" + getEnums().get(i) + "\" />\n";
		} // for i
				
		ret += tabs + "</" + getTypeName() + ">\n";


		return ret;
	}

}
