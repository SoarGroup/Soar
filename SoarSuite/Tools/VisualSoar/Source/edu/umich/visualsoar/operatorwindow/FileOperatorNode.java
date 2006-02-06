package edu.umich.visualsoar.operatorwindow;

import edu.umich.visualsoar.datamap.*;
import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.ruleeditor.RuleEditor;
import javax.swing.tree.*;
import java.io.*;
import java.awt.Component;
import edu.umich.visualsoar.MainFrame;
import edu.umich.visualsoar.parser.ParseException;
import java.awt.datatransfer.DataFlavor;
import java.util.*;
import javax.swing.JOptionPane;


/**
 *  FileOperatorNode class is for File Operators.
 *  Similar to SoarOperatorNode in every way other than
 *  writing to disk and does not have a datamap associated
 *  with itself, instead, it uses the datamap of its' Operator parent
 *  Supports sub-filing.
 */
class FileOperatorNode extends SoarOperatorNode {

	/////////////////////////////////////////
	// Constructors
	/////////////////////////////////////////
	/**
	 * this creates a low-level operator with the given name and file
	 */
	public FileOperatorNode(String inName,int inId,String inFileName) {
		super(inName,inId,inFileName);
	}

	/**
	 * this creates a highlevel operator with the given name, file, folder and
	 * dataMapId
	 */
	public FileOperatorNode(String inName,int inId,String inFileName,String inFolderName,SoarIdentifierVertex inDataMapId) {
		this(inName,inId,inFileName);
		folderName = inFolderName;
		dataMapId = inDataMapId;
		dataMapIdNumber = inDataMapId.getValue();
		isHighLevel = true;
	}

	/**
	 * This will construct a high-level operator node, this one supports serialization,
	 * restoreId must be called to get this object into a good state
	 */
	public FileOperatorNode(String inName,int inId,String inFileName,String inFolderName,int inDataMapIdNumber) {
		this(inName,inId,inFileName);
		folderName = inFolderName;
		dataMapIdNumber = inDataMapIdNumber;
		isHighLevel = true;
	}


  /**
   *  File Operator Nodes do not own their own datamaps, so therefore, it is
   *  redundant to search the datamap associated with a FileOperatorNode.
   */
  public void searchTestDataMap(SoarWorkingMemoryModel swmm, Vector errors) {}
  public void searchCreateDatamap(SoarWorkingMemoryModel swmm, Vector errors) {}
  public void searchTestNoCreateDatamap(SoarWorkingMemoryModel swmm, Vector errors) {}
  public void searchCreateNoTestDatamap(SoarWorkingMemoryModel swmm, Vector errors) {}
  public void searchNoTestNoCreateDatamap(SoarWorkingMemoryModel swmm, Vector errors) {}

		
	/**
	 * This adjusts the context menu so that only the valid commands 
	 * are displayed
	 * @param c the owner of the context menu, should be the OperatorWindow
	 * @param x the horizontal position on the screen where the context menu should
	 * be displayed
	 * @param y the vertical position on the screen where the context menu should
	 * be displayed
	 */
	public void showContextMenu(Component c, int x, int y)
    {
		if(isHighLevel)
        {
			addSuboperatorItem.setEnabled(true);
			addFileItem.setEnabled(true);
			openRulesItem.setEnabled(true);
			openDataMapItem.setEnabled(false);
			deleteItem.setEnabled(true);
			renameItem.setEnabled(true);
			exportItem.setEnabled(true);
            impasseSubMenu.setEnabled(true);
			checkChildrenAgainstDataMapItem.setEnabled(true);
		}
		else {
			addSuboperatorItem.setEnabled(true);
			addFileItem.setEnabled(true);
			openRulesItem.setEnabled(true);
			openDataMapItem.setEnabled(false);
			deleteItem.setEnabled(true);
			renameItem.setEnabled(true);
			exportItem.setEnabled(true);
            impasseSubMenu.setEnabled(true);
			checkChildrenAgainstDataMapItem.setEnabled(false);
		}
		contextMenu.show(c,x,y);
	}


    /*
     * This represents the set of actions that should be preformed when an
     * operator becomes a high-level operator
     */
    public void firstTimeAdd(OperatorWindow operatorWindow,
                             SoarWorkingMemoryModel swmm) throws IOException 
    {
        OperatorNode parent = (OperatorNode)getParent();
        
        // Create the Folder
        File folder = new File(parent.getFullPathName() + File.separator + name);
        if (!okayToCreate(folder, true))
        return;
        if (!folder.mkdir()) throw new IOException();

        // Determine the datamap id
        if (parent instanceof SoarOperatorNode)
        {
            dataMapId = (parent).getStateIdVertex();
        }
        else
        {
            dataMapId = swmm.getTopstate();
        }
        dataMapIdNumber = dataMapId.getValue();

        // Make this node highlevel
        isHighLevel = true;

        //Add the folder
        folderName = folder.getName();
    }

    
	/**
	 * Given a Writer this writes out a description of the soar operator node
	 * that can be read back in later
	 * @param w the writer 
	 * @throws IOException if there is an error writing to the writer
	 */
	public void exportDesc(Writer w) throws IOException {
		if(isHighLevel) 
			w.write("HLFOPERATOR " + name + " " + dataMapIdNumber);
		else
			w.write("FOPERATOR " + name);
	}

	public void write(Writer w) throws IOException {
		if(isHighLevel) {
			w.write("HLFOPERATOR " + name + " " + fileAssociation + " " + folderName + " " + dataMapId.getValue() + " " + id);
		}
		else {
			w.write("FOPERATOR " + name  + " " + fileAssociation + " " + id);
		}
	}




} // end of FileOperatorNode
