/**
 *
 * @file SoarNavigator.java
 * @date Mar 21, 2004
 */
package edu.rosehulman.soar.navigator;

import org.eclipse.ui.views.navigator.*;
import org.eclipse.jface.viewers.*;
import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.Path;
import org.eclipse.ui.*;
import org.eclipse.jface.dialogs.*;

/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class SoarNavigator extends ResourceNavigator {
	
	protected void initFilters(TreeViewer viewer) {
		viewer.addFilter(new SoarResourceFilter());	
	}
	
	
	protected void handleDoubleClick(DoubleClickEvent event) {
		
		
		IStructuredSelection sel = (IStructuredSelection)
			event.getSelection();
		
		Object obj = sel.getFirstElement();
		
		//If this folder is an operator, open the associated source file.
		if (obj instanceof IFolder) {
			IFolder folder = (IFolder) obj;
			
			String fileName = folder.getName() + ".soar";
			
			IContainer parent = folder.getParent();
			
			IFile operatorSource = parent.getFile(new Path(fileName));
			
			if (operatorSource.exists()) {
				IWorkbenchPage page =
					PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage();
				try {
					page.openEditor(operatorSource);
				} catch (PartInitException e) {
				}
			}
			
		}
		
		super.handleDoubleClick(event);
	}

}
