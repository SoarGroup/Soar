package edu.rosehulman.soar.wizards.actions;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.ui.IActionDelegate;
import org.eclipse.ui.IWorkbench;

import edu.rosehulman.soar.SoarPlugin;
import edu.rosehulman.soar.wizards.AddImpasseWiz;

public class InvokeAddImpasseWizardActionDelegate
	implements IActionDelegate 
{
	IStructuredSelection _selection;

	public void selectionChanged(IAction action, ISelection selection)
	{
		if(selection instanceof IStructuredSelection)
		{
			_selection = (IStructuredSelection) selection;
		}
	}

	public void run(IAction action)
	{
		// Create the wizard
		AddImpasseWiz wiz = new AddImpasseWiz();
		wiz.init(getWorkbench(), _selection);

		// Create the wizard dialog
		WizardDialog dialog = new WizardDialog(
				getWorkbench().getActiveWorkbenchWindow().getShell(), wiz);

		// Open the dialog
		dialog.open();
	}

	public IWorkbench getWorkbench()
	{
		return SoarPlugin.getDefault().getWorkbench();
	}
}
