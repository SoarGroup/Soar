package edu.umich.visualsoar.operatorwindow;

import java.awt.*;
import edu.umich.visualsoar.datamap.*;
import javax.swing.tree.*;
import java.io.*;
import java.util.Enumeration;
import java.util.Map;

public class LinkNode extends FileNode {
	SoarOperatorNode linkedToNode;
	int linkedToNodeId;

	public LinkNode(String inName,int inId,String inFileName,int inLinkToNodeId) {
		super(inName,inId,inFileName);
		linkedToNodeId = inLinkToNodeId;
	}

	public LinkNode(int inId,SoarOperatorNode inHighLevelOperator) {
		super(inHighLevelOperator.toString(),inId,inHighLevelOperator.toString() + ".soar");
		linkedToNode = inHighLevelOperator;
		linkedToNode.registerLink(this);
	}
	
	public void restore(Map persistentIds) {
		linkedToNode = (SoarOperatorNode)persistentIds.get(new Integer(linkedToNodeId));
		linkedToNode.registerLink(this);
	}
	
	public final SoarOperatorNode getLinkToNode() {
		return linkedToNode;
	}
	
	public String getFolderName() {
		return linkedToNode.getFolderName();
	}
		
	public javax.swing.tree.TreeNode getChildAt(int childIndex) {
		return linkedToNode.getChildAt(childIndex);
	}
	
	public int getChildCount() {
		return linkedToNode.getChildCount();
	}
	
	public boolean getAllowsChildren() {
		return linkedToNode.getAllowsChildren();
	}
	
	public boolean isLeaf() {
		return linkedToNode.isLeaf();
	}
	
	public Enumeration children() {
		return EMPTY_ENUMERATION;
	}
	
	
	public OperatorNode addSuboperator(OperatorWindow operatorWindow, SoarWorkingMemoryModel swmm,String newOperatorName) throws IOException {
		return linkedToNode.addSuboperator(operatorWindow,swmm,newOperatorName);
	}
	
	public void addFile(OperatorWindow operatorWindow, String newFileName) throws IOException {
		linkedToNode.addFile(operatorWindow,newFileName);
	}
	
	public void delete(OperatorWindow operatorWindow) {
		OperatorNode parent = (OperatorNode)getParent();
		renameToDeleted(new File(getFileName()));
		operatorWindow.removeNode(this);
		parent.notifyDeletionOfChild(operatorWindow,this);		
		linkedToNode.removeLink(this);
	}
	
	public String toString() {
		if(linkedToNode == null)
			return name;
		else
			return name + " @ " + linkedToNode.getUniqueName();
	}
	
	public void write(Writer w) throws IOException {
		w.write("LINK " + name + " " + fileAssociation + " " + linkedToNode.getId() + " " + id);
	}
	
	public void showContextMenu(Component c,int x, int y) {
		addSuboperatorItem.setEnabled(true);
		addFileItem.setEnabled(false);
		openRulesItem.setEnabled(true);
		openDataMapItem.setEnabled(true);
		deleteItem.setEnabled(true);
		renameItem.setEnabled(false);
		exportItem.setEnabled(true);
    impasseSubMenu.setEnabled(false);
		checkChildrenAgainstDataMapItem.setEnabled(true);
		contextMenu.show(c,x,y);
	}
	
	public void exportDesc(Writer w) throws IOException {
		w.write("OPERATOR " + name);
	}

}
