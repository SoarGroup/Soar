package edu.rosehulman.soar.wizards.actions;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.ui.*;
import org.eclipse.core.resources.*;

import edu.rosehulman.soar.*;
import edu.rosehulman.soar.sourcing.*;

/**
 * Invokes the Sourcing File generator on a project.
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */

public class InvokeGenerateSourcingFile implements IActionDelegate 
{
	IProject project;

	public void selectionChanged(IAction action, ISelection selection)
	{	
		if (selection!=null && selection.isEmpty()==false
			&& selection instanceof IStructuredSelection) {
				
			IStructuredSelection ssel = (IStructuredSelection)selection;
			if (ssel.size()>1) return;
			
			Object obj = ssel.getFirstElement();
			
			if (obj instanceof IProject) {
				project = (IProject) obj;
			} // if
		} // if
	}

	public void run(IAction action)
	{
		SourcingFile.createSourcingFile(project, null);
	}

	public IWorkbench getWorkbench()
	{
		return SoarPlugin.getDefault().getWorkbench();
	}
} // class
