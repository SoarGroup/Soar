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
import org.eclipse.swt.events.*;
import org.eclipse.jface.viewers.*;
import org.eclipse.core.resources.*;

/**
 * Creates a new Soar Project.
 * 
 * @author Tim Jasko
 */

public class NewProjectWizPage1 extends WizardPage {
	private Text fileText;
	private ISelection selection;

	/**
	 * Constructor.
	 * @param selection The selection this wizard was called on.
	 */
	public NewProjectWizPage1(ISelection selection) {
		super("wizardPage");
		setTitle("New Soar Project");
		setDescription("This wizard creates a new Soar project");
		this.selection = selection;
	}

	public void createControl(Composite parent) {
		Composite container = new Composite(parent, SWT.NULL);
		GridLayout layout = new GridLayout();
		container.setLayout(layout);
		layout.numColumns = 3;
		layout.verticalSpacing = 9;
		
		GridData gd = new GridData(GridData.FILL_HORIZONTAL);
		
		Label label = new Label(container, SWT.NULL);
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
	 * Initializes the project name to one that is available.
	 */
	private void initialize() {
		
		String projectName;
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		IProject project;
		int projectNum = 1;
		
		// Find an available project name to use as the default
		do {
			projectName = "new-soar" + projectNum;
			
			project = root.getProject(projectName);
			
			projectNum++;
		} while (project.exists() && projectNum <= 99);
				
		fileText.setText(projectName);
		fileText.setFocus();
	} // void initialize()
	
	/**
	 * Ensures that the values entered are valid.
	 */
	private void dialogChanged() {
		String projectName = getProjectName();
		// Characters which cannot be used in a resource name:
		char[] invalids = {'*', '\\', '/', '"', ':', '<', '>', '|', '?'};
		
		if (projectName.length() == 0) {
			updateStatus("Project name must be specified");
			return;
		} // if
		
		for (int i=0; i<invalids.length; i++) {
			if (projectName.indexOf(invalids[i]) != -1 ) {
				updateStatus(invalids[i] +
				  " is an invalid character in the project name " + projectName);
				return;
			} // if
		} // for i
		
		if (projectName.charAt(projectName.length() - 1) == '.') {
			updateStatus("Resource name cannot end in a period.");
			return;
		} // if
		
		//Make sure the project doesn't already exist
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		IProject newProject = root.getProject(projectName);
		if (newProject.exists()) {
			updateStatus("A project with that name already exists.");
			return;	
		} // if

		updateStatus(null);
	} // void dialogChanged()

	/**
	 * Updates the wizard status message.
	 * @param message The new status message.
	 */
	private void updateStatus(String message) {
		setErrorMessage(message);
		setPageComplete(message == null);
	}

	/**
	 * Gets the name to be used for this project.
	 * @return The new project name.
	 */
	public String getProjectName() {
		return fileText.getText();
	}
}