/**
 *
 * @file DatamapEditorInput.java
 * @date Jun 14, 2004
 */
package edu.rosehulman.soar.datamap.editor;


import edu.rosehulman.soar.*;

import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.IPersistableElement;

import java.util.*;


/**
 * 
 * 
 * @author Tim Jasko (tj9582@yahoo.com)
 */
public class DatamapEditorInput implements IFileEditorInput {
	private IFile _file;
	private ArrayList _path;
	private String _opName;
	
	public DatamapEditorInput(IFile file, ArrayList path, String opName) {
		_file = file;
		_path = path;
		_opName = opName;
	}

	public IFile getFile() {
		return _file;
	}
	
	public ArrayList getPath() {
		return _path;
	}
	
	public boolean equals( Object o) {
		if (o instanceof DatamapEditorInput) {
			DatamapEditorInput temp = (DatamapEditorInput) o;
			
			if (! _opName.equals(temp.getName()))
				return false;
			
			if (! _file.equals(temp.getFile()))
				return false;
			
			if (! _path.equals(temp.getPath()))
				return false;
			
			return true;
		}
		
		return false;	 
	}
	
	public String getOperatorName() {
		return _opName;
	}


	public IStorage getStorage() throws CoreException {
		return _file;
	}


	public boolean exists() {
		return true;
	}


	public ImageDescriptor getImageDescriptor() {
		return SoarImages.getImageDescriptor(SoarImages.IMAGE_SOAR);
	}


	public String getName() {
		return _opName;
	}


	public IPersistableElement getPersistable() {
		return new DatamapInputPersistable(this);
	}


	public String getToolTipText() {
		return _opName;
	}


	public Object getAdapter(Class adapter) {
		return null;
	}

}
