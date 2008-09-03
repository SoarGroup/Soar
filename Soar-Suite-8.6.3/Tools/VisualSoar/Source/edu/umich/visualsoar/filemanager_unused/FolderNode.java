package edu.umich.visualsoar.filemanager;

import java.io.File;
import java.awt.datatransfer.Transferable;

public class FolderNode extends FileManagerNode {
	private File folder;
	private String name;

	//Deny Default Construction
	private FolderNode() {}
	
	public FolderNode(String inName) {
		name = inName;
	}
	
	public FolderNode(File inFolder) {
		folder = inFolder;
		if(!folder.isDirectory()) 
			throw new IllegalArgumentException("Argument is not a folder");
	}

	public boolean isLeaf() {
		return false;
	}

	public boolean getAllowsChildren() {
		return true;
	}
	
	public String toString() {
		return name;
	}
	
	public void openRules() {}
	
	public void showContext(FileManager fm,int x,int y) {
		fm.showFolderContextMenu(x,y);
	}
		
	public boolean isDropOk(int action) {
		if(action == java.awt.dnd.DnDConstants.ACTION_MOVE)
			return true;
		return false;
	}
	
	public Transferable getTransferable() {
		return null;
	}

}
