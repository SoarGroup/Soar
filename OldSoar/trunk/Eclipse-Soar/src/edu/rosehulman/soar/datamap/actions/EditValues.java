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
import org.eclipse.jface.dialogs.*;
import org.eclipse.ui.*;


/**
 *
 * Action to call up the <code>editValuesDialog</code> on the given <code>DMItem</code>.
 * 
 * @author Tim Jasko 
 */
public class EditValues extends Action {
	private DataMapEditor _parent;
	private DMItem _target;

	/**
	 * Constructor.
	 * 
	 * @param parent The DataMapEditor this item belongs to.
	 * @param target The item whose values we intend to edit.
	 */
	public EditValues (DataMapEditor parent, DMItem target) {
		super();
		_parent = parent;
		_target = target;

		setText("Edit Value(s)...");
		setImageDescriptor(PlatformUI.getWorkbench().
			getSharedImages().getImageDescriptor(ISharedImages.IMG_TOOL_COPY_HOVER));
	}
	
	public void run() {
		if (_target.editValuesDialog(_parent) == Dialog.OK) {
		
			_parent.defecateUpon();
			_parent.getViewer().refresh();
		} // if
	}
}
