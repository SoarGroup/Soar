/*
 * Created on Jan 11, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.actions;

import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.datamap.items.*;


import org.eclipse.jface.action.Action;
import org.eclipse.jface.dialogs.*;


/**
 *
 * Action to add a new DMItem of the specified type to the datamap
 * @author Tim Jasko
 */
public class AddItem extends Action {
	private DataMapEditor mParent;
	private DMItem mItemType, mTarget;
	
	/**
	 * The sole constructor.
	 * 
	 * @param parent The DataMapEditor we will be adding to.
	 * @param itemType The type of {@link DMItem} we should be adding.
	 * @param target The item which will be the parent of the added child.
	 */
	public AddItem (DataMapEditor parent, DMItem itemType, DMItem target) {
		super();
		mParent = parent;
		mItemType = itemType;
		mTarget = target;
		
		setText("Add " + mItemType.getTypeName() + "...");
		setImageDescriptor(ItemImages.getImageDescriptor(itemType));
		
		if (!target.acceptsChildren()) {
			setEnabled(false);
		} // if
	}

	/**
	 * Displays an appropriate dialog to initialize the item's values to the
	 * user, then adds it to the target if OK was pressed. 
	 */
	public void run() {
		DMItem temp = mItemType.createNew();
	
		int res = temp.initValuesDialog(mParent);
		
		if (res == Dialog.OK) {
			
			DataMap dm = mParent.getContentProvider().getDataMap();
			
			temp.setID(dm.getCurrentID());
			dm.incrementCurrentID();
			dm.register(temp);
			
			mTarget.addChild(temp);
			mParent.getViewer().refresh();
			mParent.getViewer().expandToLevel(temp, 1);
			mParent.defecateUpon();
		} // if
	}

} // class
