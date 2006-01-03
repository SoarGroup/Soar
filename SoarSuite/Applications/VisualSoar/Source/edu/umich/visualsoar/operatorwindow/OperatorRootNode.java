package edu.umich.visualsoar.operatorwindow;

import edu.umich.visualsoar.datamap.*;
import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.operatorwindow.*;
import edu.umich.visualsoar.MainFrame;

import java.io.*;
import java.awt.Component;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.JOptionPane;
import java.util.Vector;

/**
 * This is the root node for the operator window
 * @author Brad Jones
 * @version 0.5a 5 Aug 1999
 */
public class OperatorRootNode extends FolderNode implements java.io.Serializable {
///////////////////////////////////////////////////////////////////
// Data Members
///////////////////////////////////////////////////////////////////
	/**
	 * A that represents the file path to the datamap, must be intialized
	 * in the constructor
	 */
	private String fullPathStart;
	
///////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////
	/**
	 * This constructs the normal OperatorRootNode object
	 * @param name the name of the node
	 * @param folder the folder which the node is associated
	 * @param dataMap the path to the datamap
	 */
	public OperatorRootNode(String inName, int inId,String inFullPathStart, String inFolder) {
		super(inName,inId,inFolder);
		fullPathStart = inFullPathStart;
	}
	
	public OperatorRootNode(String inName,int inId,String inFolder) {
		super(inName,inId,inFolder);
	}
	
///////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////

  /**
   * @return whether an openDataMap() call on this node will work
   */
	public boolean hasDataMap()
    {
		return true;
	}
    
	/**
	 * Given a Writer this writes out a description of the root node
	 * that can be read back in later
	 * @param w the writer 
	 * @throws IOException if there is an error writing to the writer
	 */
	public void write(Writer w) throws IOException {
		w.write("ROOT " + name + " " + folderName + " " + id);
	}
	
	public void exportDesc(Writer w) throws IOException {
		w.write("ROOT " + name);
	}
	
	public void setFullPath(String s) {
		fullPathStart = s;
	}
	
	public String getFullPathStart() {
		return fullPathStart;
	}
	
	/**
	 * Adds a suboperator underneath this root node
	 * @param model the three model that this node is a part of
	 * @param swmm the Working Memory Model so that we can add corresponding entries to the datamap
	 * @param newOperatorName the name of the operator being added
	 */
	public OperatorNode addSuboperator(OperatorWindow operatorWindow, SoarWorkingMemoryModel swmm,String newOperatorName) throws IOException {
		OperatorNode newOperator = super.addSuboperator(operatorWindow,swmm,newOperatorName);

		SoarVertex oper = swmm.createNewSoarId();
		SoarVertex operName = swmm.createNewEnumeration(newOperatorName);
		swmm.addTriple(swmm.getTopstate(),"operator",oper);
		swmm.addTriple(oper,"name",operName);

		return this;
	}
	
	public String getProjectFile() {
		return fullPathStart + File.separator + name + ".vsa";
	}
	
	public String getDataMapFile() {
		return getFolderName() + File.separator + name + ".dm";
	}
	
	/**
	 * This returns the path of the project so that children can
	 * determine the full path
	 * @return the path of the project
	 */
	protected String getFullPathName() {
		return fullPathStart + File.separator + folderName;
	}
	
	/**
	 * This adjusts the context menu so that only the valid commands 
	 * are displayed
	 * @param c the owner of the context menu, should be the OperatorWindow
	 * @param x the horizontal position on the screen where the context menu should
	 * be displayed
	 * @param y the vertical position on the screen where the context menu should
	 * be displayed
	 */
	public void showContextMenu(Component c, int x, int y) {
		addSuboperatorItem.setEnabled(true);
		addFileItem.setEnabled(true);
		openRulesItem.setEnabled(false);
		openDataMapItem.setEnabled(true);
		deleteItem.setEnabled(false);
		renameItem.setEnabled(true);
		exportItem.setEnabled(false);
    impasseSubMenu.setEnabled(true);
		checkChildrenAgainstDataMapItem.setEnabled(true);
		contextMenu.show(c,x,y);
	}
	
	/**
	 * This opens/shows a dataMap with this nodes associated Data Map File
	 * @param pw the MainFrame 
	 */
	public void openDataMap(SoarWorkingMemoryModel swmm,MainFrame pw) {
		DataMap dataMap = new DataMap(swmm,swmm.getTopstate(),toString());
		dataMap.setVisible(true);
		pw.addDataMap(dataMap);
    pw.getDesktopPane().dmAddDataMap(swmm.getTopstate().getValue(), dataMap);
	}
	
	/**
	 * This returns the associated datamap entry for the root node
	 * which is going to be the top-state
	 */
	public SoarIdentifierVertex getStateIdVertex() {
		return MainFrame.getMainFrame().getOperatorWindow().getDatamap().getTopstate();
	}
	
	public void rename(OperatorWindow operatorWindow, String newName, String newPath) throws IOException {
		DefaultTreeModel model = (DefaultTreeModel)operatorWindow.getModel();
		File oldFolder = new File(getFolderName());		
		File newFolder = new File(newPath + File.separator + newName);
		// Rename Folder
		if (!oldFolder.renameTo(newFolder)) {
			JOptionPane.showMessageDialog(MainFrame.getMainFrame(),"Folder Rename Failed!","Rename Error",JOptionPane.ERROR_MESSAGE);
			return;
		}
		folderName = newFolder.getName();
		name = newName;
    fullPathStart = newPath;

		model.nodeChanged(this);
	}
	
	public void importFunc(Reader r,OperatorWindow operatorWindow,SoarWorkingMemoryModel swmm) throws IOException, NumberFormatException {
			VSEImporter.read(r,this,operatorWindow,swmm,VSEImporter.HLOPERATOR | VSEImporter.OPERATOR);
	}
	
	public void renameAndBackup(OperatorWindow operatorWindow,String newName, String newPath) {

    String oldFullPathStart = fullPathStart;
    String oldFolder = folderName;

    if( new File(newPath + File.separator + newName + ".vsa").exists()) {
      JOptionPane.showMessageDialog(MainFrame.getMainFrame(),"An agent with this name already exists at this location.","Naming Error",JOptionPane.ERROR_MESSAGE);
			return;
    }
    /*
		if(name.equals(newName)) {
			JOptionPane.showMessageDialog(MainFrame.getMainFrame(),"The new name must be different from the current name.","Naming Error",JOptionPane.ERROR_MESSAGE);
			return;
		}
      */
		File oldFolderName = new File(getFolderName());
		try {
			rename(operatorWindow,newName, newPath);
			oldFolderName.mkdir();

			FileWriter graphWriter = new FileWriter(oldFolderName.getPath() + File.separator + oldFolderName.getName() + ".dm");
			operatorWindow.getDatamap().write(graphWriter);
			graphWriter.close();

			for(int i = 0; i < getChildCount(); ++i) {
				((OperatorNode)getChildAt(i)).copyStructures(oldFolderName);
			}
		}
		catch(IOException ioe) {
			JOptionPane.showMessageDialog(MainFrame.getMainFrame(),"Save As Failed!","Save As Failed",JOptionPane.ERROR_MESSAGE);
		}
	}

	public void startSourcing() throws IOException {
		Writer w = new FileWriter(fullPathStart + File.separator + folderName + ".soar");
		source(w);
		w.close();
		sourceRecursive();		
	}

  public void searchTestDataMap(SoarWorkingMemoryModel swmm, Vector errors) {
    DataMap dataMap = new DataMap(swmm,swmm.getTopstate(), "");
    errors.addAll(dataMap.searchTestDataMap(swmm.getTopstate(),  toString() ));
  }

  public void searchCreateDataMap(SoarWorkingMemoryModel swmm, Vector errors) {
    DataMap dataMap = new DataMap(swmm,swmm.getTopstate(), "");
    errors.addAll(dataMap.searchCreateDataMap(swmm.getTopstate(),  toString() ));
  }

  public void searchTestNoCreateDataMap(SoarWorkingMemoryModel swmm, Vector errors) {
    DataMap dataMap = new DataMap(swmm,swmm.getTopstate(), "");
    errors.addAll(dataMap.searchTestNoCreateDataMap(swmm.getTopstate(), toString() ));
  }

  public void searchCreateNoTestDataMap(SoarWorkingMemoryModel swmm, Vector errors) {
    DataMap dataMap = new DataMap(swmm,swmm.getTopstate(), "");
    errors.addAll(dataMap.searchCreateNoTestDataMap(swmm.getTopstate(), toString() ));
  }

  public void searchNoTestNoCreateDataMap(SoarWorkingMemoryModel swmm, Vector errors) {
    DataMap dataMap = new DataMap(swmm,swmm.getTopstate(), "");
    errors.addAll(dataMap.searchNoTestNoCreateDataMap(swmm.getTopstate(), toString() ));
  }
}
	
