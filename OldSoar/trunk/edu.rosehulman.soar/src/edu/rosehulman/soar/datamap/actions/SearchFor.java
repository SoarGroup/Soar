/*
 * Created on Jan 12, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.actions;

import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.datamap.items.*;
import edu.rosehulman.soar.datamap.actions.dialogs.*;

import org.eclipse.jface.action.Action;
//import org.eclipse.ui.*;

/**
 *
 * Action to preform a search on the datamap.
 * 
 * @author Tim Jasko
 */
public class SearchFor extends Action {
	private DataMapEditor _parent;
	private DMItem _target;
	
	
	/**
	 * Constructor.
	 * 
	 * @param parent The DataMapEditor this item belongs to.
	 * @param target The item from which the search will start.
	 */
	public SearchFor (DataMapEditor parent, DMItem target) {
		super();
		_parent = parent;
		_target = target;
		
		setText("Search For...");
		//setImageDescriptor(PlatformUI.getWorkbench().getSharedImages().
		//	getImageDescriptor(ISharedImages.IMG_OBJS_TASK_TSK));
		
		if (!target.acceptsChildren()) {
			setEnabled(false);
		} // if
	}


	public void run() {
		
		SearchForDialog win = new SearchForDialog(_parent, _target);
		
		win.open();
		
		
	}
}
