/*
 * Created on Feb 9, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */

package edu.rosehulman.soar.wizards;

import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.*;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;
import org.eclipse.core.runtime.*;
import org.eclipse.jface.operation.*;
import java.lang.reflect.InvocationTargetException;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.CoreException;
import java.io.*;
import org.eclipse.ui.*;


/**
 * Adds a file to the Soar heirarchy.
 * 
 * @author Tim Jasko
 */

public class AddFileWiz extends Wizard implements INewWizard {
	private AddFileWizPage1 page;
	private ISelection selection;

	/**
	 * Default constructor.
	 */
	public AddFileWiz() {
		super();
		setNeedsProgressMonitor(true);
	}
	
	/**
	 * Adding the page to the wizard.
	 */

	public void addPages() {
		page = new AddFileWizPage1(selection);
		addPage(page);
	}

	/**
	 * This method is called when 'Finish' button is pressed in
	 * the wizard. We will create an operation and run it
	 * using wizard as execution context.
	 */
	public boolean performFinish() {
		final String containerName = page.getContainerName();
		final String fileName = page.getFileName();
		
		IRunnableWithProgress op = new IRunnableWithProgress() {
			public void run(IProgressMonitor monitor) throws InvocationTargetException {
				try {
					doFinish(containerName, fileName, monitor);
				} catch (CoreException e) {
					throw new InvocationTargetException(e);
				} finally {
					monitor.done();
				}
			}
		};
		
		try {
			getContainer().run(true, false, op);
		} catch (InterruptedException e) {
			return false;
		} catch (InvocationTargetException e) {
			Throwable realException = e.getTargetException();
			MessageDialog.openError(getShell(), "Error", realException.getMessage());
			return false;
		}
		
		return true;
	}
	
	/**
	 * The worker method. Makes the file.
	 */
	private void doFinish( String parentName, String fileName,
		IProgressMonitor monitor) throws CoreException {
		//Yup. Big mess of code here, too. Did I mention that it's
		// nearly identical to that found in AddSuboperatorWiz?
		// Almost, but not quite in several places. Sucks, don't it?
		// Somebody should refactor this whole section.
		// Somebody else, of course. 
		
		monitor.beginTask("Creating " + fileName, 2);
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		IResource parent = root.findMember(new Path(parentName));
		
		
		if (!parent.exists() ) {
			throwCoreException("Parent operator \"" + parentName + "\" does not exist.");
		} // if
		
		
		IContainer container = Utility.folderify(parent, monitor);
		
		
		final IFile file = container.getFile(new Path(fileName));
		
		try {
			InputStream stream =
			  Utility.getFileTemplate(file, "new_soar.soar");
			  
			if (file.exists()) {
				throwCoreException("File \"" + fileName + "\" already exists");
			} else {
				file.create(stream, true, monitor);
			} // else
			
			
			Utility.markResource(file, "file");
			
			stream.close();
		} catch (IOException e) {
			e.printStackTrace();
		} // catch
		
		monitor.worked(1);
		monitor.setTaskName("Opening file for editing...");
		
		getShell().getDisplay().asyncExec(new Runnable() {
			public void run() {
				IWorkbenchPage page =
					PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage();
				try {
					page.openEditor(file);
				} catch (PartInitException e) {
				}
			}
		});
		
		monitor.worked(1);
		
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
		this.selection = selection;
	}
}