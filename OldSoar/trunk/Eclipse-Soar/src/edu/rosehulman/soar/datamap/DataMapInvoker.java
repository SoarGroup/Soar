/**
 *
 * @file InvokeParseFile.java
 * @date Jun 13, 2004
 */
package edu.rosehulman.soar.datamap;

import edu.rosehulman.soar.datamap.editor.*;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.*;
import org.eclipse.ui.*;
import org.eclipse.core.resources.*;

import java.util.*;

/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class DataMapInvoker implements IActionDelegate {

	IContainer _target;

	public void run(IAction action) {
		IFile dmFile = _target.getProject().getFile("datamap.xdm");
		DatamapEditorInput fei;
		
		if (_target.equals(_target.getProject())) {
			fei = new DatamapEditorInput(dmFile, new ArrayList(), "");
		} else {
			String name = _target.getName();
			 
			ArrayList path = new ArrayList();
			IContainer con = _target.getParent();
			
			while (! con.equals(_target.getProject()) ) {
				path.add(0, con.getName());
				con = con.getParent();
			}
			
			fei = new DatamapEditorInput(dmFile, path, name);
		}
		
		
		
		IWorkbenchPage page =
			PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage();
		try {
			page.openEditor(fei, DataMapEditor.ID);
		} catch (PartInitException e) {
			e.printStackTrace();
		}
	}


	public void selectionChanged(IAction action, ISelection selection) {
		if (selection!=null && selection.isEmpty()==false
			&& selection instanceof IStructuredSelection) {
				
			IStructuredSelection ssel = (IStructuredSelection)selection;
			if (ssel.size()>1) return;
			
			Object obj = ssel.getFirstElement();
			
			if (obj instanceof IContainer) {
				_target = (IContainer) obj;
			} // if
		} // if

	}

}
