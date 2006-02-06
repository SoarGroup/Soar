package edu.umich.visualsoar.filemanager;

import edu.umich.visualsoar.ruleeditor.RuleEditor;
import edu.umich.visualsoar.MainFrame;
import java.io.*;
import javax.swing.JOptionPane;


public class FileNode extends FileManagerNode {
	private RuleEditor ruleEditor;

	private File file;
	private String name;
	
	// Deny Default Construction
	private FileNode() {}
	
	public FileNode(File inFile) {
		file = inFile;
		name = file.getName();
		if(!file.isFile()) 
			throw new IllegalArgumentException("Argument is not a file");
	}
	

	public boolean getAllowsChildren() {
		return false;
	}
	
	public void openRules() {
		if (ruleEditor == null || ruleEditor.isClosed()) {
			try {
				ruleEditor = new RuleEditor(file);
				MainFrame.getMainFrame().addRuleEditor(ruleEditor);			
			}
			catch(IOException ioe) {
				JOptionPane.showMessageDialog(MainFrame.getMainFrame(), "There was an error reading file: " + 
								file.getPath(), "I/O Error", JOptionPane.ERROR_MESSAGE); 
			}
		} 
		else {
			MainFrame.getMainFrame().showRuleEditor(ruleEditor);
		}
	}
	
	public String toString() {
		return name;
	}
	
	public void showContext(FileManager fm,int x,int y) {
		fm.showFileContextMenu(x,y);
	}
	
	public boolean isDragOk(int action) {
		if(action == java.awt.dnd.DnDConstants.ACTION_MOVE) {
			return true;
		}
		return false;
	}
	
	public boolean isDropOk(int action) {
		return false;
	}
	
}
