package edu.umich.visualsoar.filemanager;

import javax.swing.tree.DefaultMutableTreeNode;
import java.awt.datatransfer.Transferable;

/*
 To stop all the switches on type
 */

public abstract class FileManagerNode extends DefaultMutableTreeNode {
	public abstract void openRules();
	
	public abstract void showContext(FileManager fm,int x,int y);
	
	public boolean isDragOk(int action) {
		return false;
	}
	
	public Transferable getTransferable() {
		return null;
	}
	
	public boolean isDropOk(int action) {
		return false;
	}
}
