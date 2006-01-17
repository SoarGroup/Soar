/*
 * Created on Dec 15, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap;

import edu.rosehulman.soar.datamap.items.*;

import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.resources.*;
import org.eclipse.ui.part.*;



/**
 *
 * Wraps around a <code>DataMap</code> to provide content for the
 *  <code>DataMapEditor</code>.
 * 
 * @author Tim Jasko
 * @see DataMap
 * @see DataMapEditor
 * @see ITreeContentProvider
 */
public class DataMapContentProvider implements ITreeContentProvider {
	IProject _project;
	IFile _file;
	DataMapEditor _parent;
	DMItem root;
	DataMap _dm;
	
	Object[] rootArray = new Object[1];
	
	
	public DataMapContentProvider(DataMapEditor parent, IProject project,
		FileEditorInput file) {
		
		_parent = parent;
		_project = project;
		_file = file.getFile();
		
		try {
			_dm = new DataMap(_file);
			
			resetRoot();
			
		} catch (Exception e) {
			e.printStackTrace();
		}
		
	} // DataMapContentProvider( ... )
	
	public void save(IProgressMonitor monitor) {
		try {
			_dm.saveXML(monitor);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public void importVS(String file) {
		try {
			_dm.importVS(file);
			resetRoot();
			_parent.defecateUpon();
			
		} catch (Exception e) {
			e.printStackTrace();
			MessageDialog.openError(_parent.getSite().getShell(),
				"Error importing",
				"This may not be a valid Visual Soar datamap file.");
		} // catch
	}
	
	protected void resetRoot() {
		root = _dm.getRoot();
		rootArray[0] = root;
		_parent.getViewer().refresh();
	}
	
	
	public DMItem getRoot() {
		return root;
	}

	

	public Object[] getChildren(Object parentElement) {
		//System.out.println("getChildren()" + parentElement);
		if (parentElement == ResourcesPlugin.getWorkspace()) {
			//System.out.println(rootArray);
			return rootArray;
		} else {
		
			DMItem par = (DMItem)parentElement;
			
			if (par.acceptsChildren()) {
				//System.out.println(par.getChildren().toArray());
				return par.getChildren().toArray();
			} else {
				return null;
			}
		} // else

	}

	public Object getParent(Object element) {
		//System.out.println("getParent()");
		DMItem el = (DMItem) element;
		//System.out.println(el.getParent());
		return el.getParent();
	}

	public boolean hasChildren(Object element) {
		//System.out.println("hasChildren()");
		DMItem el = (DMItem) element;
		//System.out.println(el.hasChildren());
		return el.hasChildren();
	}

	public Object[] getElements(Object inputElement) {
		return getChildren(inputElement);
	}


	public void dispose() {

	}

	public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
	}

}
