/*
 * Created on Jan 15, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.actions;

import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.datamap.items.*;
import edu.rosehulman.soar.datamap.validators.*;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.dialogs.*;
//import org.eclipse.ui.*;

/**
 *
 * Action to rename an item in the datamap.
 * 
 *  @author Tim Jasko
 */
public class RenameItem extends Action {
	private DataMapEditor _parent;
	private DMItem _target;

	/**
	 * Constructor.
	 * 
	 * @param parent The DataMapEditor this item belongs to.
	 * @param target The item to be renamed.
	 */
	public RenameItem (DataMapEditor parent, DMItem target) {
		super();
		_parent = parent;
		_target = target;

		setText("Rename Attribute...");
		//setImageDescriptor(PlatformUI.getWorkbench().
		//	getSharedImages().getImageDescriptor(ISharedImages.IMG_DEF_VIEW));
	}
	
	public void run() {
		InputDialog input = new InputDialog(_parent.getSite().getShell(),
			"Enter Attribute Name", "Attribute Name:", _target.getName(), new AttributeNameValidator());
		
		int res = input.open();
		if (res == Dialog.OK) {
			_target.setName(input.getValue());
			
			_parent.defecateUpon();
			_parent.getViewer().refresh();
		}
	
	}

}
