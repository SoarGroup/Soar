/**
 *
 * @file ProjectImages.java
 * @date Jun 10, 2004
 */
package edu.rosehulman.soar;


import edu.rosehulman.soar.datamap.items.*;

import org.eclipse.jface.resource.*;
import org.eclipse.swt.graphics.Image;

import java.net.*;


/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class SoarImages {
	public static final String IMAGE_SP = "IMAGE_SP";
	public static final String IMAGE_SOAR = "IMAGE_SOAR";
	
	//Those that are contained in ItemImages.
	public static final String DMITEM_ENUM = ItemImages.ITEM_ENUM;
	public static final String DMITEM_FLOAT = ItemImages.ITEM_FLOAT; 
	public static final String DMITEM_IDENTIFIER = ItemImages.ITEM_IDENTIFIER;
	public static final String DMITEM_INT = ItemImages.ITEM_INT;
	public static final String DMITEM_STRING = ItemImages.ITEM_STRING;
	public static final String DMITEM_POINTER = ItemImages.ITEM_POINTER;

	static {
		ImageRegistry imageReg = SoarPlugin.getDefault().getImageRegistry();
	
		imageReg.put(SoarImages.IMAGE_SP, loadImageDescriptor("sp.gif"));
		imageReg.put(SoarImages.IMAGE_SOAR, loadImageDescriptor("soar.gif"));
		
	}


	
	public static Image getImage(String id) {
		ImageRegistry ir = SoarPlugin.getDefault().getImageRegistry(); 
		return ir.get(id);
	}
	
	public static ImageDescriptor getImageDescriptor(String id) {
		ImageRegistry ir = SoarPlugin.getDefault().getImageRegistry();
		return ir.getDescriptor(id);
	}
	
	
	/**
	 * Loads an ImageDescriptor from a file in the icons directory
	 * 
	 * @param name - name of the file to be loaded
	 * @return An ImageDescriptor representing the image in the file
	 */
	private static ImageDescriptor loadImageDescriptor(String name) {
		try {
			 URL installURL = SoarPlugin.getDefault().getDescriptor().getInstallURL();
			 URL url = new URL(installURL, "icons/" + name);
			 return ImageDescriptor.createFromURL(url);
		} catch (MalformedURLException e) {
			 // should not happen
			 return ImageDescriptor.getMissingImageDescriptor();
		}
	}
}
