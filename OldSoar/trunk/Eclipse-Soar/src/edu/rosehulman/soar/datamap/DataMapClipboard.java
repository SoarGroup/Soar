/**
 *
 * @file DataMapClipboard.java
 * @date Jun 4, 2004
 */
package edu.rosehulman.soar.datamap;



import edu.rosehulman.soar.datamap.items.*;

import org.eclipse.core.resources.*;

/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class DataMapClipboard {
	private DMItem _item;
	private IFile _file;
	private int _status = 0;
	
	public static final int EMPTY = 0;
	public static final int CUT = 1;
	public static final int COPIED = 2;
	
	/**
	 * Tells if something is in the clipboard or not.
	 * @return <code>true</code> if nothing is in the clipboard,
	 *  <code>false</code> if something is.
	 */
	public boolean isEmpty() {
		return _item == null;
	}
	
	/**
	 * Gets the item currently in the clipboard.
	 * @return The item in the clipboard.
	 */
	public DMItem getBuffer() {
		return _item;
	}
	
	public int getBufferStatus() {
		return _status;
	}
	
	/**
	 * Copies the DMItem to the clipboard.
	 * @param item The DMItem to copy.
	 */
	public void copy(DMItem item, IFile file) {
		_item = item;
		_file = file;
		_status = COPIED;
	}
	
	
	/**
	 * Copies the DMItem to the clipboard, them delinks it from
	 *  its parent, effectively deleting it.
	 * @param item The item to cut.
	 */
	public void cut(DMItem item, IFile file) {
		_item = item;
		_file = file;
		item.setParent(null);
		_status = CUT;
	}
	
	/**
	 * Pastes the item in the clipboard to the supplied parent.
	 * @param parent The lucky parent!
	 */
	public void pasteTo(DMItem parent) {
		// we don't copy the item until it is pasted, so that we 
		// can paste multiple copies of the item if we want to.
		parent.addChild(_item.copy());
	}
	
	
	public void pasteLinkTo(DMItem parent) {
		DMPointer pnt = new DMPointer(_item.getName());
		pnt.setTarget(_item);
		// we don't copy the item until it is pasted, so that we 
		// can paste multiple copies of the item if we want to.
		parent.addChild(pnt);
	}

}
