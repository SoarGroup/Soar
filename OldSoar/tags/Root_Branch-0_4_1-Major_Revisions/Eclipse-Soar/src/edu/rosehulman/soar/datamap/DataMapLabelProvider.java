/*
 * Created on Dec 18, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap;

import edu.rosehulman.soar.datamap.items.*;

import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.swt.graphics.Image;



/**
 *
 * Makes the datamap pretty.
 * 
 * @author Tim Jasko
 * @see LabelProvider
 */
public class DataMapLabelProvider extends LabelProvider {
	
	public DataMapLabelProvider() {
	}
	
	/*public String getColumnText(Object obj, int index) {
		return getText(obj);
	}
	
	public Image getColumnImage(Object obj, int index) {
		return getImage(obj);
	} */
	
	public Image getImage(Object obj) {
		return ItemImages.getImage(obj);
	}
} // class
