/**
 *
 * @file PasteItem.java
 * @date Jun 4, 2004
 */
package edu.rosehulman.soar.datamap.actions;

import edu.rosehulman.soar.*;
import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.datamap.items.*;


import org.eclipse.jface.action.Action;
import org.eclipse.ui.*;

/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class PasteItem extends Action {
	private DataMapEditor _parent;
	private DMItem _target;
	
	
	/**
	 * Constructor.
	 * 
	 * @param parent The DataMapEditor this item belongs to.
	 * @param target The item fated to become the parent of
	 *   the item in the clipboard.
	 */
	public PasteItem (DataMapEditor parent, DMItem target) {
		super();
		_parent = parent;
		_target = target;

		setText("Paste");
		setImageDescriptor(PlatformUI.getWorkbench().
			getSharedImages().getImageDescriptor(ISharedImages.IMG_TOOL_PASTE));

		if ( SoarPlugin.getDataMapClipboard().isEmpty() 
			|| ! (target instanceof DMIdentifier) ) {
			setEnabled(false);
		}
	}
	
	
	public void run() {
		DataMapClipboard dmc = SoarPlugin.getDataMapClipboard();
		
		dmc.pasteTo(_target, _parent.getDataMap());
		
		_parent.defecateUpon();
		_parent.getViewer().refresh();
	}
}
