/*
 * Created on Jan 19, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.actions;

import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.datamap.items.*;

import org.eclipse.jface.action.Action;


/**
 *
 * Action to change an item in the datamap to a different type.
 * The only information retained is the name. 
 * 
 * @author Tim Jasko
 */
public class ChangeType extends Action {
	private DataMapEditor _parent;
	private DMItem _itemType, _target;
	
	/**
	 * The sole constructor.
	 * 
	 * @param parent The DataMapEditor we are dealing with.
	 * @param itemType The type of {@link DMItem} we should be changing to.
	 * @param target The item which will be changed.
	 */
	public ChangeType (DataMapEditor parent, DMItem itemType, DMItem target) {
		super();
		_parent = parent;
		_itemType = itemType;
		_target = target;
		
		setText("to " + _itemType.getTypeName());
		setImageDescriptor(ItemImages.getImageDescriptor(itemType));
		
		if (target instanceof DMSpecial) {
			this.setEnabled(false);
		} // if
	}

	/**
	 * Change the target's type.
	 */
	public void run() {
		/*DMItem temp = _itemType.createNew(_parent.getFile());
	
		temp.setName(_target.getName());
		
		DMItem dmParent = _target.getParent();
		ArrayList siblings = dmParent.getChildren(); 
		
		//Get the exact position of the item in the child list so we 
		// can insert the replacement there.
		int loc = siblings.indexOf(_target);
		 
		//Make the new item recognize its parent.
		temp.setParent(dmParent);
		//Deal with the fact that it was automatically added to the 
		// end of the child list in the last call.
		siblings.remove(temp);
		//Replace the old child with a newer, better one.
		// Don't you wish you could do that with your kids?
		siblings.set(loc, temp); */
		
		_parent.getDataMap().replace(_target, _itemType);
		
		_parent.defecateUpon();
		_parent.getViewer().refresh();
	}
}
