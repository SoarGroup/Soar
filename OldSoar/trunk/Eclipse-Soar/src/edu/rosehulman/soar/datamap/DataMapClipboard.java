/**
 *
 * @file DataMapClipboard.java
 * @date Jun 4, 2004
 */
package edu.rosehulman.soar.datamap;



import edu.rosehulman.soar.datamap.items.*;


/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class DataMapClipboard {
	private DMItem _item;
	
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
	
	/**
	 * Copies the DMItem to the clipboard.
	 * @param item The DMItem to copy.
	 */
	public void copy(DMItem item) {
		_item = item;
	}
	
	
	/**
	 * Copies the DMItem to the clipboard, them delinks it from
	 *  its parent, effectively deleting it.
	 * @param item The item to cut.
	 */
	public void cut(DMItem item) {
		_item = item;
		item.setParent(null);
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

}
