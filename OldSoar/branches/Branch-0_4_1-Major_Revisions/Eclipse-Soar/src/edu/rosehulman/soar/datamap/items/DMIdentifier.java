/*
 * Created on Dec 25, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items;

import java.util.*;

import org.eclipse.jface.dialogs.*;
import org.eclipse.ui.part.*;

import edu.rosehulman.soar.datamap.validators.*;


/**
 *
 * Represents an identifier in the datamap.
 * 
 * @author Tim Jasko
 */
public class DMIdentifier extends DMItem {
	ArrayList _children = new ArrayList(1);
	
	public DMIdentifier() {
		super();
	}
	
	public DMIdentifier(String name) {
		super(name);
	}
	
	public String toString() {
		String temp = getName();
		
		if (getName().equals("operator")) {
			for (int i=0; i<_children.size(); i++) {
				DMItem kid = (DMItem) _children.get(i); 
				if (kid instanceof DMEnumeration && kid.getName().equals("name")) {
					temp += ": " + ((DMEnumeration) kid).getEnums().get(0);
					break;
				}
			} // for i
		} // if
		
		String comment = getComment(); 
		
		if (comment != null && !comment.equals("")) {
			temp += "       * " + comment + " *";
		} // if
		
		return temp;
	}
	
	
	public ArrayList getChildren() {
		return _children;
	}
	
	/**
	 * Sets the children to the given ArrayList.
	 * 
	 * @param children The new ArrayList of children.
	 */
	public void setChildren(ArrayList children) {
		_children = children;
	}
	
	public boolean hasChildren() {
		return !_children.isEmpty();
	}
	

	public String getTypeName() {
		return "Identifier";
	}

	
	public int initValuesDialog(WorkbenchPart parent) {
		InputDialog input = new InputDialog(parent.getSite().getShell(),
			"Enter Attribute Name", "Attribute name:", "",
			new AttributeNameValidator());
		
		int res = input.open();
		
		setName(input.getValue());
		
		return res;
	}


	public boolean canEditValues() {
		return false;
	}

	public int editValuesDialog(WorkbenchPart parent) {
		// no editing of values here!
		return 0;
	}

	public boolean acceptsChildren() {
		return true;
	}
	
	public DMItem createNew() {
		DMItem ret = new DMIdentifier();
		
		
		return ret;
	}
	
	public String getXML(int depth) {
		String ret = "";
		String tabs = "";

		for (int i=0; i<depth; i++) {
			tabs += '\t';
		} // for i

		ret += tabs;

		ret += "<" + getTypeName() + getSharedXML() 
			+ " >\n";
		
		for (int i=0; i<getChildren().size(); i++) {
			ret += ((DMItem) getChildren().get(i)).getXML( depth + 1 );
		} // for i
		
		ret += tabs + "</" + getTypeName() + ">\n";


		return ret;
	}
	
	public boolean isValidValue(String val) {
		return false;
	}
	
	public DMItem copy() {
		DMIdentifier ret = new DMIdentifier(getName());
		
		ret._comment = this._comment;
		
		ArrayList newKids = (ArrayList) this._children.clone();
		for (int i=0; i<newKids.size(); ++i) {
			newKids.set(i, ((DMItem) newKids.get(i)).copy() );
		}
		
		ret._children = newKids;
		
		return ret;
	}

} // class DMIdentifier
