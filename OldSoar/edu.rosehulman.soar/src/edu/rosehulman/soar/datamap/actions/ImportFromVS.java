/*
 * Created on Feb 4, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.actions;

import edu.rosehulman.soar.datamap.*;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IEditorActionDelegate;
import org.eclipse.ui.IEditorPart;
import org.eclipse.swt.*;
import org.eclipse.swt.widgets.FileDialog;

/**
 * 
 *
 * This is the menu item allowing the datamap to import from the 
 *  Visual Soar datamap format.
 *  The only problem is that it doesn't work particularly well.
 *  We need to scrap this and make a tool to import an entire Visual Soar Project.
 * 
 * @author Tim Jasko
 */
public class ImportFromVS implements IEditorActionDelegate {
	private DataMapEditor _editor;


	public void setActiveEditor(IAction action, IEditorPart targetEditor) {
		_editor = (DataMapEditor) targetEditor;
	}


	public void run(IAction action) {
		FileDialog win = new FileDialog(_editor.getSite().getShell(), SWT.OPEN);
		String filter[] = {"*.dm"};
		
		win.setFilterExtensions(filter);
		
		String file = win.open();
		
		if (file != null) {
			_editor.getContentProvider().importVS(file);
		} // if

	}


	public void selectionChanged(IAction action, ISelection selection) {
		// Like I care, sucker!
	}

}
