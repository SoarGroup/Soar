/*
 * Created on Dec 15, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap;

import edu.rosehulman.soar.datamap.editor.*;
import edu.rosehulman.soar.datamap.items.*;

import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.resources.*;
import org.eclipse.ui.*;

import java.util.*;

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
	DatamapEditorInput _fileInput;
	IFile _file;
	DataMapEditor _parent;
	DMItem _root;
	DataMap _dm;
	
	Object[] rootArray = new Object[1];
	
	
	public DataMapContentProvider(DataMapEditor parent, IProject project,
		IFileEditorInput fileInput) {
		
		_parent = parent;
		_project = project;
		_fileInput = (DatamapEditorInput) fileInput;
		_file = fileInput.getFile();
		
		try {
			_dm = DataMap.getAssociatedDatamap(_file);
			
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
		DMItem root;
		
		
		int rootID = _fileInput.getRootID();
		
		root = _dm.getItem(rootID);
		
		if (root == null) {
			root = _dm.getRoot();
		}
		
		_root = root;
		rootArray[0] = _root;
		
		_parent.getViewer().refresh();
	}
	
	private DMItem findOpNode(ArrayList path, String name) {
		if (! name.equals("")) {
			ArrayList ops = _dm.find(path);
				
			for (int i=0; i<ops.size(); ++i) {
				DMItem op = (DMItem) ops.get(i);
					
				if (op.acceptsChildren()) {
					ArrayList children = op.getChildren();
					for (int i2=0; i2<children.size(); i2++) {
						DMItem kid = (DMItem) children.get(i2); 
						if (kid instanceof DMEnumeration && kid.getName().equals("name")
							&& ((DMEnumeration) kid).getEnums().get(0).equals(name) ) {
							
							return op;
						}
					} // for i2
				}
			} // for i
		}
		
		return null;
	}
	
	public DMItem getRoot() {
		return _root;
	}

	public DataMap getDataMap() {
		return _dm;
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
				
				if (par != _root && par instanceof DMIdentifier
					&& par.getName().equals("operator")) {
					ArrayList children = par.getChildren();
					
					for (int i=0; i<children.size(); ++i) {
						if ( ((DMItem)children.get(i)).getName().equals("name") ) {
							Object[] ret = new Object[1];
							ret[0] = children.get(i);
							
							return ret;
						}
					}
					
					return null;
				}
				
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
