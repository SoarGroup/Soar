/*
 * Created on Feb 9, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
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
 * Adds a new Soar file.
 * 
 * @author Tim Jasko
 */

public class AddFileWizPage1 extends WizardPage {
	//private Text containerText;
	private Text fileText;
	private ISelection selection;
	private String containerName;

	/**
	 * Default constructor
	 * @param selection - The resource that was selected when this wizard was called.
	 */
	public AddFileWizPage1(ISelection selection) {
		super("wizardPage");
		setTitle("Add File");
		setDescription("This wizard adds a file.");
		this.selection = selection;
	}


	public void createControl(Composite parent) {
		Composite container = new Composite(parent, SWT.NULL);
		GridLayout layout = new GridLayout();
		container.setLayout(layout);
		layout.numColumns = 3;
		layout.verticalSpacing = 9;
		/*Label label = new Label(container, SWT.NULL);
		label.setText("&Container:");

		containerText = new Text(container, SWT.BORDER | SWT.SINGLE);
		GridData gd = new GridData(GridData.FILL_HORIZONTAL);
		containerText.setLayoutData(gd);
		containerText.addModifyListener(new ModifyListener() {
			public void modifyText(ModifyEvent e) {
				dialogChanged();
			}
		});

		Button button = new Button(container, SWT.PUSH);
		button.setText("Browse...");
		button.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				handleBrowse();
			}
		});*/
		Label label = new Label(container, SWT.NULL);
		label.setText("&File name:");

		fileText = new Text(container, SWT.BORDER | SWT.SINGLE);
		GridData gd = new GridData(GridData.FILL_HORIZONTAL);
		fileText.setLayoutData(gd);
		fileText.addModifyListener(new ModifyListener() {
			public void modifyText(ModifyEvent e) {
				dialogChanged();
			}
		});
		initialize();
		dialogChanged();
		setControl(container);
	}
	

	/**
	 * Initializes the file name to one that is not currently in use.
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
		

		String fileName = "file.soar";
		
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		final String containerName = getContainerName();


		// Find an available file name to use as the default
		try {
			IResource resource = root.findMember(containerName);
			
			fileName = Utility.getAvailableName(resource, "file");
			
		} catch (NullPointerException e) {
			e.printStackTrace();
		} // catch

		
		fileText.setText(fileName);
		fileText.setFocus();
		
	} // void initialize()
	
	/**
	 * Ensures that entered values are valid.
	 */
	private void dialogChanged() {
		String fileName = getFileName();
		
		// Characters which cannot be used in a resource name:
		char[] invalids = {'*', '\\', '/', '"', ':', '<', '>', '|', '?'};

		
		// Ensure the file has a name
		if (fileName.length() == 0) {
			updateStatus("File name must be specified");
			return;
		} // if
		
		// Ensure the extension is .soar
		int dotLoc = fileName.indexOf('.');
		if (dotLoc != -1) {
			String ext = fileName.substring(dotLoc + 1);
			if (ext.equalsIgnoreCase("soar") == false) {
				updateStatus("File extension must be \"soar\"");
				return;
			} // if
		} else {
			updateStatus("File must have a \".soar\" extension");
			return;
		} // else
		
		for (int i=0; i<invalids.length; i++) {
			if (fileName.indexOf(invalids[i]) != -1 ) {
				updateStatus(invalids[i] +
				  " is an invalid character in the file name " + fileName);
				return;
			} // if
		} // for i
		
		
		
		
		
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		final String containerName = getContainerName();
		
		try {
			IResource res = root.findMember(containerName);
			if (Utility.isFolderFile(res)) {
				IContainer ict = Utility.getFileFolder(res);
				
				final IFile file = ict.getFile(new Path(fileName));
				
				if (file.exists()) {
					updateStatus("File already exists. Use another name.");
					return;
				} // if
			} else if (res instanceof IContainer) {
				IContainer ict = (IContainer) res;
				final IFile file = ict.getFile(new Path(fileName));
				
				if (file.exists()) {
					updateStatus("File already exists. Use another name.");
					return;
				} // if
			} // if
		
			
		} catch (NullPointerException e) {
			//updateStatus("Operator \"" + containerName + "\" does not exist");
			return;
		} // catch
		
		updateStatus(null);
	} // void dialogChanged()
	

	/**
	 * Updates the status message.
	 * @param message The new status message (<code>null</code> if all is well).
	 */
	private void updateStatus(String message) {
		setErrorMessage(message);
		setPageComplete(message == null);
	}

	/**
	 * The container that this file will be added to.
	 * @return The container name.
	 */
	public String getContainerName() {
		//return containerText.getText();
		return containerName;
	}
	
	
	/**
	 * The name of the new file.
	 * @return The file name.
	 */
	public String getFileName() {
		return fileText.getText();
	}
}
