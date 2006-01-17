/*
 * Created on Feb 9, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */

package edu.rosehulman.soar.wizards;

import edu.rosehulman.soar.*;
import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.datamap.items.*;

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
//for 3.0 compatibility
import org.eclipse.ui.ide.*;


/**
 * Adds a suboperator to the Soar heirarchy.
 * 
 * @author Tim Jasko
 */

public class AddSuboperatorWiz extends Wizard implements INewWizard {
	private AddSuboperatorWizPage1 page;
	private ISelection selection;

	/**
	 * Default constructor.
	 * You know. It constructs and stuff.
	 * @param selection - The resource that was selected when this wizard was called.
	 */
	public AddSuboperatorWiz() {
		super();
		setNeedsProgressMonitor(true);
	}
	
	/**
	 * Adds the page to the wizard.
	 */
	public void addPages() {
		page = new AddSuboperatorWizPage1(selection);
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
	 * Does all the work.
	 */
	private void doFinish( String parentName, String fileName,
		IProgressMonitor monitor) throws CoreException {
		//Crickey this code is a mess. You! Fix it!
		// Heck no. I ain't doing it.
		// Except I just did, somewhat. Disregard the previous comments.
		
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
			
			FileMarker.markResource(file, "operator");
			
			stream.close();
		} catch (IOException e) {
			e.printStackTrace();
		} // catch
		
		
		//Adds this guy to the parent's datamap
		try {
			
			DataMap dm = DataMap.getAssociatedDatamap(container);
			DMItem dmRoot = dm.getAssociatedVertex(container);
			
			// operator
			DMSpecial newOp = new DMSpecial( "operator" );
			newOp.setID( dm.getAndIncrementID() );
			
			// name
			DMEnumeration newEnum = new DMEnumeration("name");
			newEnum.getEnums().add( file.getName().split("\\.")[0] );
			newEnum.setID( dm.getAndIncrementID() );

			newOp.addChild(newEnum);
			dm.register(newEnum);
			
			// type
			DMEnumeration enumType = new DMEnumeration("type");
			enumType.getEnums().add( "state" );
			enumType.setID( dm.getAndIncrementID() );

			newOp.addChild(enumType);
			dm.register(enumType);
			
			// superstate
			
			DMPointer superstate = new DMPointer("superstate");
			superstate.setTarget( dmRoot );
			superstate.setID( dm.getAndIncrementID() );
			
			newOp.addChild(superstate);
			dm.register(superstate);
			
			// top-state
			DMPointer topstate = new DMPointer("top-state");
			topstate.setTarget( dm.getRoot() );
			topstate.setID( dm.getAndIncrementID() );
			
			newOp.addChild(topstate);
			dm.register(topstate);
			
			// add the whole lot of them to the datamap
			
			dmRoot.addChild(newOp);
			dm.register(newOp);
			
			
			Utility.associateID(file, newOp.getID());
			
			dm.saveXML(monitor);
			
		} catch (Exception e) {
			e.printStackTrace();
		} // catch
		
		
		
		/*try {
			if (! (container instanceof IProject)) {
				
				IFile dmFile = container.getFile(new Path("datamap.xdm"));
							
				if ( dmFile.exists()) {
					DataMap dm = new DataMap(dmFile);
							
					DMSpecial newOp = new DMSpecial(
						"operator: [" + file.getName() + "]");
								
					DMEnumeration newEnum = new DMEnumeration("name");
					newEnum.getEnums().add(container.getName());
								
					newOp.addChild(newEnum);
								
					dm.getRoot().addChild(newOp);
								
					dm.saveXML(monitor);
				} // if
			} // if

		} catch (Exception e) {
			e.printStackTrace();
		} // catch */
		
		
		
		
		monitor.worked(1);
		monitor.setTaskName("Opening file for editing...");
		
		getShell().getDisplay().asyncExec(new Runnable() {
			public void run() {
				IWorkbenchPage page =
					PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage();
				try {
					//page.openEditor(file);
					//new line for Eclipse 3.0 compatibility
					IDE.openEditor(page, file);
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