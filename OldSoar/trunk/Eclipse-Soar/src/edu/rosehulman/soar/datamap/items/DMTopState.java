/*
 * Created on Jan 18, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items;

/**
 *
 * Represents the link to the top-state of the datamap.
 * 
 * @author Tim Jasko
 */
public class DMTopState extends DMSpecial {
	public DMTopState() {
		super();
	}
	
	public DMTopState(String name) {
		super(name);
	}
	
	public String getTypeName() {
		return "TopState";
	}
	
	public String getXML(int depth) {
		String ret = "";
		String tabs = "";

		for (int i=0; i<depth; i++) {
			tabs += '\t';
		} // for i

		ret += tabs;

		ret += "<" + getTypeName() + getSharedXML()
			+ " />\n";


		return ret;
	}
	
	
	public DMItem copy() {
		DMIdentifier ret = new DMIdentifier(getName());
		
		ret._comment = this._comment;
		ret._children = this._children;
		
		return ret;
	}

}
