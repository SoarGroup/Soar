/*
 * Created on Jan 13, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items;

import edu.rosehulman.soar.SoarPlugin;

import org.eclipse.core.runtime.Platform;
import org.eclipse.jface.resource.*;
import org.eclipse.swt.graphics.Image;
import org.eclipse.ui.*;

import java.net.*;

/**
 *
 * This class serves to store the Images used by the datamap.
 * 
 * @author Tim Jasko
 */
public class ItemImages {
	public static final String ITEM_ENUM = "ITEM_ENUM";
	public static final String ITEM_FLOAT = "ITEM_FLOAT"; 
	public static final String ITEM_IDENTIFIER = "ITEM_IDENTIFIER";
	public static final String ITEM_INT = "ITEM_INT";
	public static final String ITEM_STRING = "ITEM_STRING";
	public static final String ITEM_POINTER = "ITEM_POINTER";
	
	/**
	 * Loads the required images into the image registry
	 */
	static {
		ImageRegistry imageReg = SoarPlugin.getDefault().getImageRegistry();
	
		imageReg.put(ItemImages.ITEM_ENUM, getImageDescriptor("enum.gif"));
		imageReg.put(ItemImages.ITEM_FLOAT, getImageDescriptor("float.gif"));
		imageReg.put(ItemImages.ITEM_IDENTIFIER, getImageDescriptor("id.gif"));
		imageReg.put(ItemImages.ITEM_INT, getImageDescriptor("int.gif"));
		imageReg.put(ItemImages.ITEM_STRING, getImageDescriptor("string.gif"));
		imageReg.put(ItemImages.ITEM_POINTER, getImageDescriptor("pointer.gif"));
	}
	
	/**
	 * Returns the icon for the given type of Datamap Item
	 * 
	 * @param obj	The DMItem to be represented
	 * @return	An Image representing the item's type
	 */
	public static Image getImage(Object obj) {
		ImageRegistry ir = SoarPlugin.getDefault().getImageRegistry(); 
		if (obj instanceof DMIdentifier) {
			return ir.get(ItemImages.ITEM_IDENTIFIER);
		} else if (obj instanceof DMFloat) {
			return ir.get(ItemImages.ITEM_FLOAT);
		} else if (obj instanceof DMEnumeration) {
			return ir.get(ItemImages.ITEM_ENUM);
		} else if (obj instanceof DMInteger) {
			return ir.get(ItemImages.ITEM_INT);
		} else if (obj instanceof DMString) {
			return ir.get(ItemImages.ITEM_STRING);
		} else if (obj instanceof DMPointer) {
			return ir.get(ItemImages.ITEM_POINTER);
		} else {
			return PlatformUI.getWorkbench().
				getSharedImages().getImage(ISharedImages.IMG_OBJ_ELEMENT);		
		} // else
	}
	
	/**
	 * Returns an ImageDescriptor of the icon for the given type of Datamap Item
	 * 
	 * @param obj The DMItem to be represented
	 * @return	An ImageDescriptor representing the item's type
	 */
	public static ImageDescriptor getImageDescriptor(Object obj) {
		ImageRegistry ir = SoarPlugin.getDefault().getImageRegistry(); 
		if (obj instanceof DMIdentifier) {
			return ir.getDescriptor(ItemImages.ITEM_IDENTIFIER);
		} else if (obj instanceof DMFloat) {
			return ir.getDescriptor(ItemImages.ITEM_FLOAT);
		} else if (obj instanceof DMEnumeration) {
			return ir.getDescriptor(ItemImages.ITEM_ENUM);
		} else if (obj instanceof DMInteger) {
			return ir.getDescriptor(ItemImages.ITEM_INT);
		} else if (obj instanceof DMString) {
			return ir.getDescriptor(ItemImages.ITEM_STRING);
		} else if (obj instanceof DMPointer) {
			return ir.getDescriptor(ItemImages.ITEM_POINTER);
		} else {
			return PlatformUI.getWorkbench().
				getSharedImages().getImageDescriptor(ISharedImages.IMG_OBJ_ELEMENT);		
		} // else
	}
	
	/**
	 * Loads an ImageDescriptor from a file in the icons directory
	 * 
	 * @param name - name of the file to be loaded
	 * @return An ImageDescriptor representing the image in the file
	 */
	private static ImageDescriptor getImageDescriptor(String name) {
		try {
			 URL installURL = Platform.getBundle("edu.rosehulman.soar").getEntry("/");
			 URL url = new URL(installURL, "icons/" + name);
			 return ImageDescriptor.createFromURL(url);
		} catch (MalformedURLException e) {
			 // should not happen
			 return ImageDescriptor.getMissingImageDescriptor();
		}
	}
}
