/*******************************************************************************
 * Copyright (c) 2000, 2003 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials 
 * are made available under the terms of the Common Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/cpl-v10.html
 * 
 * Contributors:
 *     IBM Corporation - initial API and implementation
 *******************************************************************************/
package edu.rosehulman.soar.wizards;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.SWT;
import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.Path;
import org.eclipse.swt.events.*;
import org.eclipse.ui.dialogs.ContainerSelectionDialog;
import org.eclipse.jface.viewers.*;


/**
 * This wizard page is not currently in use.
 * 
 * @author Tim Jasko
 */

public class NewFileWizPage1 extends WizardPage {
	private Text containerText;
	private Text fileText;
	private ISelection selection;

	/**
	 * Constructor for SampleNewWizardPage.
	 * @param pageName
	 */
	public NewFileWizPage1(ISelection selection) {
		super("wizardPage");
		setTitle("Multi-page Editor File");
		setDescription("This wizard creates a new file with *.soar extension that can be opened by a multi-page editor.");
		this.selection = selection;
	}

	/**
	 * @see IDialogPage#createControl(Composite)
	 */
	public void createControl(Composite parent) {
		Composite container = new Composite(parent, SWT.NULL);
		GridLayout layout = new GridLayout();
		container.setLayout(layout);
		layout.numColumns = 3;
		layout.verticalSpacing = 9;
		Label label = new Label(container, SWT.NULL);
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
		});
		label = new Label(container, SWT.NULL);
		label.setText("&File name:");

		fileText = new Text(container, SWT.BORDER | SWT.SINGLE);
		gd = new GridData(GridData.FILL_HORIZONTAL);
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
	 * Tests if the current workbench selection is a suitable
	 * container to use.
	 */
	
	private void initialize() {
		if (selection!=null && selection.isEmpty()==false && selection instanceof IStructuredSelection) {
			IStructuredSelection ssel = (IStructuredSelection)selection;
			if (ssel.size()>1) return;
			Object obj = ssel.getFirstElement();
			if (obj instanceof IResource) {
				IContainer container;
				if (obj instanceof IContainer)
					container = (IContainer)obj;
				else
					container = ((IResource)obj).getParent();
				containerText.setText(container.getFullPath().toString());
			} // if
		} // if
		
		String fileName = "new_file.soar";
		
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		final String containerName = getContainerName();

		// Find an available file name to use as the default
		try {
			IResource resource = root.findMember(containerName);
			IContainer ict = (IContainer) resource;
			
			IFile file;
			int fileNum = 1; 
			do {
				fileName = "new_file" + fileNum + ".soar";
			
				file = ict.getFile(new Path(fileName));
				
				fileNum++;
			} while (file.exists() && fileNum <= 99);
				
		} catch (NullPointerException e) {
			e.printStackTrace();
		} // catch

		
		fileText.setText(fileName);
		fileText.setFocus();
	} // void initialize()
	
	/**
	 * Uses the standard container selection dialog to
	 * choose the new value for the container field.
	 */

	private void handleBrowse() {
		ContainerSelectionDialog dialog =
			new ContainerSelectionDialog(
				getShell(),
				ResourcesPlugin.getWorkspace().getRoot(),
				false,
				"Select new file container");
		if (dialog.open() == ContainerSelectionDialog.OK) {
			Object[] result = dialog.getResult();
			if (result.length == 1) {
				containerText.setText(((Path)result[0]).toOSString());
			}
		}
	}
	
	/**
	 * Ensures that both text fields are set.
	 */

	private void dialogChanged() {
		String container = getContainerName();
		String fileName = getFileName();
		
		// Characters which cannot be used in a resource name:
		char[] invalids = {'*', '\\', '/', '"', ':', '<', '>', '|', '?'};

		//Ensure the file has a container 
		if (container.length() == 0) {
			updateStatus("File container must be specified");
			return;
		} // if
		
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
		
		// Make sure that the folder does exist and that the file doesn't
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		final String containerName = getContainerName();
		
		try {
			IResource resource = root.findMember(containerName);
			
			IContainer ict = (IContainer) resource;
			final IFile file = ict.getFile(new Path(fileName));
			
			if (file.exists()) {
				updateStatus("File already exists. Use another name.");
				return;
			} // if
		} catch (NullPointerException e) {
			updateStatus("Container \"" + containerName + "\" does not exist");
			return;
		} // catch
		
		updateStatus(null);
	} // void dialogChanged()
	

	private void updateStatus(String message) {
		setErrorMessage(message);
		setPageComplete(message == null);
	}

	public String getContainerName() {
		return containerText.getText();
	}
	public String getFileName() {
		return fileText.getText();
	}
}