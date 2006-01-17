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


import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.ui.*;
import org.eclipse.core.runtime.*;
import org.eclipse.jface.operation.*;
import java.lang.reflect.InvocationTargetException;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.CoreException;

import edu.rosehulman.soar.*;
import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.natures.*;
import edu.rosehulman.soar.sourcing.*;

/**
 * The wizard to create a new Soar project.
 * 
 * @author Tim Jasko
 */

public class NewProjectWiz extends Wizard implements INewWizard {
	private NewProjectWizPage1 page;
	private ISelection selection;
	private IWorkbench workbench;

	/**
	 * Constructor for NewFileWiz.
	 */
	public NewProjectWiz() {
		super();
		setNeedsProgressMonitor(true);
	}
	
	/**
	 * Adding the page to the wizard.
	 */
	public void addPages() {
		page = new NewProjectWizPage1(selection);
		addPage(page);
	}

	/**
	 * This method is called when 'Finish' button is pressed in
	 * the wizard. We will create an operation and run it
	 * using wizard as execution context.
	 */
	public boolean performFinish() {
		final String fileName = page.getProjectName();
		IRunnableWithProgress op = new IRunnableWithProgress() {
			public void run(IProgressMonitor monitor) throws InvocationTargetException {
				try {
					doFinish(fileName, monitor);
				} catch (CoreException e) {
					throw new InvocationTargetException(e);
				} finally {
					monitor.done();
				}
			}
		};
		try {
			getContainer().run(true, false, op);
			workbench.showPerspective(
				"edu.rosehulman.soar.perspective.SoarPerspectiveFactory",
				workbench.getWorkbenchWindows()[0]);
		} catch (InterruptedException e) {
			return false;
		} catch (InvocationTargetException e) {
			Throwable realException = e.getTargetException();
			MessageDialog.openError(getShell(), "Error", realException.getMessage());
			return false;
		} catch (WorkbenchException e) {
			MessageDialog.openError(getShell(), "Error", e.getMessage());
			return false;
		}
		return true;
		
	}
	
	/**
	 * The worker method. It will create a new project, then
	 * create the appropriate Soar heirarchy and files.
	 */
	private void doFinish( String projectName, IProgressMonitor monitor)
	  throws CoreException {
		
		monitor.beginTask("Creating " + projectName, 7);
		
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		IProject newProject = root.getProject(projectName);
		
		//creation of the project
		if (newProject.exists()) {
			throwCoreException("Project \"" + projectName + "\" already exists");
		} else {
			
			newProject.create(monitor);
			newProject.open(monitor);
			
			try {

				IProjectDescription description = newProject.getDescription();
				String[] natures = description.getNatureIds();
				String[] newNatures = new String[natures.length + 1];
				System.arraycopy(natures, 0, newNatures, 0, natures.length);
				newNatures[natures.length] = SoarProjectNature.NATURE_ID;
				description.setNatureIds(newNatures);
				
				newProject.setDescription(description, IResource.FORCE ,monitor);
				
				newProject.setPersistentProperty(DataMap.VERTEX_ID, "0");
				
			} catch (CoreException e) {
				e.printStackTrace();
			} // catch
			
		} // else
		
		monitor.worked(2);
		
		
		// Create the contents of the project's root directory
		IFolder folderAll = newProject.getFolder("all");
		if (!folderAll.exists()) {
			folderAll.create(true, true, monitor);
		} // if
		FileMarker.markResource(folderAll, "file");
		
		IFolder folderElaborations = newProject.getFolder("elaborations");
		if (!folderElaborations.exists()) {
			folderElaborations.create(true, true, monitor);
		} // if
		FileMarker.markResource(folderElaborations, "file");
		
		IFile file_firstload = newProject.getFile("_firstload.soar");
		if (!file_firstload.exists()) {
			file_firstload.create(
			  Utility.getFileTemplate(file_firstload, "_firstload.soar"),
			  true, monitor);
		} // if
		FileMarker.markResource(file_firstload, "file");
		
		IFile filedatamap = newProject.getFile("datamap.xdm");
		if (!filedatamap.exists()) {
			filedatamap.create(
			  Utility.getFileTemplate(filedatamap, "datamap.xdm"),
			  true, monitor);
		} // if
		
		monitor.worked(3);
		
		// Create the contents of the elaborations folder
		IFile file_all = folderElaborations.getFile("_all.soar");
		if (!file_all.exists()) {
			file_all.create(
			  Utility.getFileTemplate(file_all, "_all.soar"),
			  true, monitor);
		} // if
		FileMarker.markResource(file_all, "file");
		
		IFile filetopstate = folderElaborations.getFile("top-state.soar");
		if (!filetopstate.exists()) {
			filetopstate.create(
			  Utility.getFileTemplate(filetopstate, "top-state.soar"),
			  true, monitor);
		} // if
		FileMarker.markResource(filetopstate, "file");
		
		SourcingFile.createSourcingFile(newProject, monitor);
		
		
		newProject.close(monitor);
		newProject.open(monitor);
		
		monitor.worked(2);
		monitor.done();
		
	} // void doFinish(...)
	

	private void throwCoreException(String message) throws CoreException {
		IStatus status =
			new Status(IStatus.ERROR, "edu.rosehulman.soar.wizards", IStatus.OK, message, null);
		throw new CoreException(status);
	}

	/**
	 * We will accept the selection in the workbench to see if
	 * we can initialize from it.
	 */
	public void init(IWorkbench workbench, IStructuredSelection selection) {
		this.workbench = workbench;
		this.selection = selection;
	}
}