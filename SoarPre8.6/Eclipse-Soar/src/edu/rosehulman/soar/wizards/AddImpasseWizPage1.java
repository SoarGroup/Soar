
package edu.rosehulman.soar.wizards;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.SWT;
import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.Path;
import org.eclipse.swt.events.*;
//import org.eclipse.ui.dialogs.*;
import org.eclipse.jface.viewers.*;


/**
 * Adds a new Soar impasse.
 * 
 * @author Tim Jasko
 */

public class AddImpasseWizPage1 extends WizardPage {
	private ISelection selection;
	private String containerName;
	private Button btnConflict, btnConstraint, btnTie, btnNoChange;

	/**
	 * Constructor
	 * @param selection The selection this wizard was called on.
	 */
	public AddImpasseWizPage1(ISelection selection) {
		super("wizardPage");
		setTitle("New Impasse");
		setDescription("This wizard creates a new impasse.");
		this.selection = selection;
	}

	public void createControl(Composite parent) {
		Composite comp = new Composite(parent, SWT.NULL);
		GridLayout layout = new GridLayout();
		comp.setLayout(layout);
		layout.numColumns = 1;
		layout.verticalSpacing = 9;
		
		btnTie = new Button(comp, SWT.RADIO);
		btnTie.setText("Operator Tie Impasse");
		btnTie.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				dialogChanged();
			}
		});
		btnTie.setSelection(true);
		
		btnConflict = new Button(comp, SWT.RADIO);
		btnConflict.setText("Operator Conflict Impasse");
		btnConflict.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				dialogChanged();
			}
		});
		
		btnConstraint = new Button(comp, SWT.RADIO);
		btnConstraint.setText("Operator Constraint-Failure Impasse");
		btnConstraint.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				dialogChanged();
			}
		});
		
		btnNoChange = new Button(comp, SWT.RADIO);
		btnNoChange.setText("State No-Change Impasse");
		btnNoChange.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				dialogChanged();
			}
		});
		

		initialize();
		dialogChanged();
		setControl(comp);
	}
	

	/**
	 * Performs initialization of values.
	 *
	 */
	private void initialize() {

		if (selection!=null && selection.isEmpty()==false
			&& selection instanceof IStructuredSelection) {
				
			IStructuredSelection ssel = (IStructuredSelection)selection;
			if (ssel.size()>1) return;
			
			Object obj = ssel.getFirstElement();
			
			if (obj instanceof IResource) {
				IResource container = (IResource)obj;
				containerName = container.getFullPath().toString();
				//containerText.setText(container.getFullPath().toString());
			} // if
		} // if

	} // void initialize()

	


	/**
	 * Ensures that the selected impasse does not already exist.
	 *
	 */
	private void dialogChanged() {

		IResource res = ResourcesPlugin.getWorkspace().getRoot().findMember(
			new Path(containerName));
		
		String suggestion = getFileExtensionless();
		
		if (! Utility.getAvailableName(res, suggestion).equals(getFileName())) {
			updateStatus("That impasse already exists.");
			return;
		} // if
		
		 
		
		updateStatus(null);
	} // void dialogChanged()
	

	/**
	 * Updates the status message.
	 * @param message The new status message (<code>null</code> if all is well in the world).
	 */
	private void updateStatus(String message) {
		setErrorMessage(message);
		setPageComplete(message == null);
	}
	
	/**
	 * Gets the container this impasse will be added to.
	 * @return The container name.
	 */
	public String getContainerName() {
		return containerName;
	}
	
	/**
	 * Gets the name of the new impasse, including extension.
	 * @return The file name.
	 */
	public String getFileName() {
		
		return getFileExtensionless() + ".soar";
		
	} // String getFileName()
	
	/**
	 * Gets the name of the new impasse, minus the extension.
	 * @return The file name/
	 */
	public String getFileExtensionless() {
		
		if (btnConflict.getSelection()) {
			return "Impasse__Operator_Conflict";
		} else if (btnConstraint.getSelection()) {
			return "Impasse__Operator_Constraint-Failure";
		} else if (btnTie.getSelection()) {
			return "Impasse__Operator_Tie";
		} else if (btnNoChange.getSelection()) {
			return "Impasse__State_No-Change";
		} else { // We should never get here
			return "Impasse";
		}
	
	} // String getFileExtensionless()

}
