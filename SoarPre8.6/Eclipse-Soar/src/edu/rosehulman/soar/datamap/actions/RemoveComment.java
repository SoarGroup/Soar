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
 *
 * Action to remove the comment from this <code>DMItem</code>.
 * @author Tim Jasko
 */
public class RemoveComment extends Action {
	private DataMapEditor _parent;
	private DMItem _target;

	/**
	 * Constructor.
	 * 
	 * @param parent The DataMapEditor this item belongs to.
	 * @param target The item whose comment will be removed.
	 */
	public RemoveComment (DataMapEditor parent, DMItem target) {
		super();
		_parent = parent;
		_target = target;

		setText("Remove Comment");
		setImageDescriptor(PlatformUI.getWorkbench().
			getSharedImages().getImageDescriptor(ISharedImages.IMG_TOOL_DELETE));
	}
	
	public void run() {
		if (MessageDialog.openConfirm(_parent.getSite().getShell(), "Confirm Delete", 
				"Do you really want to delete this comment?")) {
					
			_target.setComment("");
			_parent.defecateUpon();
			_parent.getViewer().refresh();
		} // if
	}
}
