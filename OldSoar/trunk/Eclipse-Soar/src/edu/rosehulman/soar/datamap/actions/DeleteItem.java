/*
 * Created on Jan 15, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.actions;

import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.datamap.items.*;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.dialogs.*;
import org.eclipse.ui.*;

/**
 *
 * Action to removes an item from the DataMap.
 * 
 * @author Tim Jasko
 */
public class DeleteItem extends Action {
	private DataMapEditor _parent;
	private DMItem _target;
	
	
	/**
	 * Constructor.
	 * 
	 * @param parent The DataMapEditor this item belongs to.
	 * @param target The item to be baleeted.
	 */
	public DeleteItem (DataMapEditor parent, DMItem target) {
		super();
		_parent = parent;
		_target = target;

		setText("Delete Attribute...");
		setImageDescriptor(PlatformUI.getWorkbench().
			getSharedImages().getImageDescriptor(ISharedImages.IMG_TOOL_DELETE));
	}
	
	public void run() {
		if (MessageDialog.openConfirm(_parent.getSite().getShell(), "Confirm Delete", 
				"Do you really want to delete the attribute named \""
				+ _target.getName() + "\"?")) {

			_parent.getDataMap().remove(_target);
			
			_parent.defecateUpon();
			_parent.getViewer().refresh();
		} // if
		
	}

}
