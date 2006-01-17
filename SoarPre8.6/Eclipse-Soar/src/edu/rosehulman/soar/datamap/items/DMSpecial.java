/*
 * Created on Jan 27, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items;

/**
 *
 * Represents a special identifier in the datamap. 
 * Used to make <i>io</i>, <i>input-link</i> and other such identifiers uneditable.
 * 
 * @author Tim Jasko 
 */
public class DMSpecial extends DMIdentifier {
	public DMSpecial() {
		super();
	}
	
	public DMSpecial(String name) {
		super(name);
	}
	
	public String getTypeName() {
		return "Special";
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
}
