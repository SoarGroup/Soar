/**
 *
 * @file InvokeParseFile.java
 * @date Jun 2, 2004
 */
package edu.rosehulman.soar.wizards.actions;

import edu.rosehulman.soar.datamap.checker.*;
import edu.rosehulman.soar.datamap.*;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.*;
import org.eclipse.ui.*;
import org.eclipse.core.resources.*;

/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class InvokeParseFile implements IActionDelegate {

	IFile _target;

	public void run(IAction action) {
		DataMap dm = DataMap.getAssociatedDatamap(_target);
		
		DataMapChecker.matches(_target, dm);

	}


	public void selectionChanged(IAction action, ISelection selection) {
		if (selection!=null && selection.isEmpty()==false
			&& selection instanceof IStructuredSelection) {
				
			IStructuredSelection ssel = (IStructuredSelection)selection;
			if (ssel.size()>1) return;
			
			Object obj = ssel.getFirstElement();
			
			if (obj instanceof IFile) {
				_target = (IFile) obj;
			} // if
		} // if

	}

}
