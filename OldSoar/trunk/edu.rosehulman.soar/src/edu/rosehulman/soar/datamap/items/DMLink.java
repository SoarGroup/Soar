/*
 * Created on Feb 11, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items;

import edu.rosehulman.soar.datamap.*;

import org.eclipse.core.resources.*;

/**
 *
 * Links to another DataMap file and loads its children from there.
 * 
 * @author Tim Jasko 
 */
public class DMLink extends DMSpecial {
	private IFile _file;
	private DataMap _dm;
	
	
	/**
	 * Constructs a DMLink with the given name.
	 * 
	 * @param name The DMLink's name.
	 */
	public DMLink(String name) {
		super(name);
	}
	
	/**
	 * Default constructor
	 *
	 */
	public DMLink() {
		super();
	}
	
	public String getTypeName() {
		return "Link";
	}
	
	/**
	 * Sets the DMLink to load itself from this DataMap file.
	 * 
	 * @param file The Datamap this file is linked to.
	 */
	public void setFile(IFile file) {
		_file = file;
		try {
			_dm = new DataMap(file);
			
			this.setChildren(_dm.getRoot().getChildren());
			
		} catch (Exception e) {
			e.printStackTrace();
		} // catch
	} // void setFile(IFile file)
	
	
	/**
	 * The DataMap file this item loads from.
	 * 
	 * @return The DataMap file.
	 */
	public IFile getFile() {
		return _file;
	} // IFile getFile()
	
	
	public DMItem createNew() {
		return new DMLink();
	}
	
	
	/**
	 * Gets a string of XML representing this Item.
	 *  Please note: Calling this method will cause the DataMap this node links
	 *  to to automatically be saved.
	 * 
	 * @param depth How deep in the tree this item is (used for indenting purposes).
	 */
	public String getXML(int depth) {
		
		try {
			_dm.saveXML(null);
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		
		String ret = "";
		String tabs = "";

		for (int i=0; i<depth; i++) {
			tabs += '\t';
		} // for i

		ret += tabs;

		ret += "<" + getTypeName() + " name=\"" + getName() + "\""
			+ " comment=\"" + getComment() + "\""
			+ " path=\"" + getFile().getFullPath() + "\""
			+ " />\n";

		return ret;
	} // String getXML(int depth)
	
	
} // class
