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
	public void copy(DMItem item) {
		_item = item;
		_status = COPIED;
	}
	
	
	/**
	 * Copies the DMItem to the clipboard, them delinks it from
	 *  its parent, effectively deleting it.
	 * @param item The item to cut.
	 */
	public void cut(DMItem item, DataMap dm) {
		_item = item;
		
		dm.remove(item);
		
		_status = CUT;
	}
	
	/**
	 * Pastes the item in the clipboard to the supplied parent.
	 * @param parent The lucky parent!
	 */
	public void pasteTo(DMItem parent, DataMap dm) {
		// we don't copy the item until it is pasted, so that we 
		// can paste multiple copies of the item if we want to.
		
		DMItem pastee = _item.copy();
		pastee.setID(dm.getCurrentID());
		dm.incrementCurrentID();
		
		parent.addChild( pastee );
	}
	
	
	public void pasteLinkTo(DMItem parent, DataMap dm) {
		DMPointer pnt = new DMPointer(_item.getName());
		
		pnt.setID(dm.getCurrentID());
		dm.incrementCurrentID();
		
		pnt.setTarget(_item);
		// we don't copy the item until it is pasted, so that we 
		// can paste multiple copies of the item if we want to.
		parent.addChild(pnt);
	}

}
