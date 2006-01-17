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
import org.eclipse.ui.*;


/**
 *
 * Action to edit the comment associated with this datamap item.
 * 
 * @author Tim Jasko 
 */
public class EditComment extends Action {
	private DataMapEditor _parent;
	private DMItem _target;


	/**
	 * Constructor.
	 * 
	 * @param parent The DataMapEditor this item belongs to.
	 * @param target The item whose comment we intend to edit.
	 */
	public EditComment (DataMapEditor parent, DMItem target) {
		super();
		_parent = parent;
		_target = target;

		setText("Add/Edit Comment...");
		setImageDescriptor(PlatformUI.getWorkbench().
			getSharedImages().getImageDescriptor(ISharedImages.IMG_TOOL_COPY));
	}
	
	public void run() {
		InputDialog input = new InputDialog(_parent.getSite().getShell(),
			"Enter Comment", "Comment:", _target.getComment(), new RelativistInputValidator());
		
		int res = input.open();
		
		if (res == Dialog.OK) {
			_target.setComment(input.getValue().replace('\"', '\''));
		}
		
		_parent.defecateUpon();
		_parent.getViewer().refresh();
	}
}
