/**
 *
 * @file DMPointer.java
 * @date Jun 9, 2004
 */
package edu.rosehulman.soar.datamap.items;

import edu.rosehulman.soar.datamap.*;

import org.eclipse.ui.part.WorkbenchPart;
import org.eclipse.core.resources.*;

import java.util.*;

/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class DMPointer extends DMItem {
	private DMItem _target;
	private IFile _targetFile;
	private int _targetID;
	
	
	public DMPointer() {
		super();
	}
	
	public DMPointer(String name) {
		super(name);
	}
	
	public int getTargetID() {
		return _targetID;
	}
	
	public void setTargetID(int targetID) {
		_targetID = targetID;
	}
	
	public IFile getTargetFile() {
		return _targetFile;
	}
	
	public void setTargetFile(IFile file) {
		_targetFile = file;
	}
	
	/**
	 * Sets the attribute pointed to.
	 * @param target
	 */
	public void setTarget(DMItem target) {
		_target = target;
		
		setTargetID(target._id);
	}
	
	/**
	 * Gets the attribute pointed to.
	 * @return
	 */
	public DMItem getTarget() {
		return _target;
	}

	public boolean isValidValue(String val) {
		return _target.isValidValue(val);
	}


	public DMItem createNew(IFile file) {
		DMItem ret = new DMPointer();
		
		ret._id = DataMap.getCurrentID(file);
		DataMap.incrementCurrentID(file);
		
		return ret;
	}


	public String getTypeName() {
		return "Pointer";
	}


	public int initValuesDialog(WorkbenchPart parent) {
		return _target.initValuesDialog(parent);
	}


	public boolean canEditValues() {
		return _target.canEditValues();
	}


	public int editValuesDialog(WorkbenchPart parent) {
		return _target.editValuesDialog(parent);
	}
	
	public ArrayList getChildren() {
		return _target.getChildren();
	}
	
	public void setChildren(ArrayList children) {
		_target.setChildren(children);
	}
	
	public boolean hasChildren() {
		return _target.hasChildren();
	}
	
	public boolean acceptsChildren() {
		return _target.acceptsChildren();
	}
	
	public String toString() {
		return getName() + " -> " + _target.toString();
	}


	public String getXML(int depth) {
		
		String ret = "";
		String tabs = "";
		
		for (int i=0; i<depth; i++) {
			tabs += '\t';
		} // for i
		
		ret += tabs;
		
		ret += "<" + getTypeName() + getSharedXML()
			+ " targetID=\"" + _target._id + "\"" 
			+ " targetFile=\"" + _targetFile.getFullPath() + "\""
			+ " />\n";
		
		
		return ret;
	}


	public DMItem copy() {
		// TODO copy
		return null;
	}

}
