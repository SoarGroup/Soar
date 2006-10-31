package edu.umich.visualsoar.operatorwindow;

import edu.umich.visualsoar.datamap.SoarWorkingMemoryModel;
import edu.umich.visualsoar.MainFrame;
import java.io.*;
import java.awt.datatransfer.DataFlavor;
import java.awt.Component;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.JOptionPane;
import java.util.*;

/**
 * This is the Folder node for the operator window
 * @author Brad Jones
 * @version 0.5a 5 Aug 1999
 */
public class FolderNode extends OperatorNode implements java.io.Serializable 
{

///////////////////////////////////////////////////////////////////
// Data Members
///////////////////////////////////////////////////////////////////
    /**
     * a string that is the path to the folder which is associated with this
     * node
     */
    protected String folderName;
    
///////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////
    /**
     * This constructs a folder node for the Operator Window
     * @param name the name of the node
     * @param folder the folder for which this node is associated
     */
    public FolderNode(String inName,int inId,String inFolderName) 
    {

        super(inName,inId);
        folderName = inFolderName;
    }

///////////////////////////////////////////////////////////////////
// Accessors
///////////////////////////////////////////////////////////////////
    /**
     * Use this getter function to get the path to the folder
     * @return the path to the folder
     */
    public String getFolderName() 
    {

        return getFullPathName();
    }
        
    /**
     * This returns the the full path from the parent
     */ 
    protected String getFullPathName() 
    {

        OperatorNode parent = (OperatorNode)getParent();
        return parent.getFullPathName() + File.separator + folderName;
    }

    
    /**
     * this tells the JTree to always render this like it
     * it has children
     * @return false
     */
    public boolean isLeaf() 
    {

        return false;
    }
    


///////////////////////////////////////////////////////////////////
// Modifiers
///////////////////////////////////////////////////////////////////
    /**
     * This is the function that gets called when you want to add a sub file
     * operator to this node
     * @param model the tree model for which this node is currently a member
     * @param newFileName the name of the new operator to add
     */
    public OperatorNode addFileOperator(OperatorWindow operatorWindow, SoarWorkingMemoryModel swmm, String newFileName) throws IOException 
    {

        File file = new File(getFullPathName() + File.separator + newFileName + ".soar");
        FileOperatorNode fon = null;

        if(!okayToCreateReplace(file)) 
        {

            return null;
        }
        //FileNode fn = operatorWindow.createFileNode(newFileName,file.getName());
        fon = operatorWindow.createFileOperatorNode(newFileName, file.getName());
        operatorWindow.addChild(this,fon);
        sourceChildren();
        return this;
    }

    /**
     * This is the function that gets called when you want to add a suboperator
     * to this node
     * @param model the tree model for which this node is currently a member
     * @param newOperatorName the name of the new operator to add
     */
    public OperatorNode addSuboperator(OperatorWindow operatorWindow,
                                       SoarWorkingMemoryModel swmm,
                                       String newOperatorName) throws IOException 
    {

        File rules = new File(getFullPathName() + File.separator + newOperatorName + ".soar");
        OperatorNode on = null;

        if (! okayToCreate(rules)) 
        {

            return this;
        }

        if (! rules.createNewFile()) 
        {

            throw new IOException();
        }

        on = operatorWindow.createSoarOperatorNode(newOperatorName,
                                                   rules.getName());
        operatorWindow.addChild(this,on);
        sourceChildren();
        return this;
    }

    /**
     * This is the function that gets called when you want to add a sub Impasse
     * Operator to this node
     * @param model the tree model for which this node is currently a member
     * @param newOperatorName the name of the new operator to add */
    public OperatorNode addImpasseOperator(OperatorWindow operatorWindow, SoarWorkingMemoryModel swmm,String newOperatorName) throws IOException 
    {

        File rules = new File(getFullPathName() + File.separator + newOperatorName + ".soar");
        SoarOperatorNode ion = null;

        if (! okayToCreate(rules)) 
        {

            return this;
        }

        if (! rules.createNewFile()) 
        {

            throw new IOException();
        }

        ion = operatorWindow.createImpasseOperatorNode(newOperatorName,
                                                       rules.getName());
        operatorWindow.addChild(this,ion);
        sourceChildren();

        //Automatically create an elaborations file.  Impasses do not have files
        //associated with them directly so we have to add an elaborations file
        //to the impasse (making it a high level operator) immediately.  I'm not
        //sure if this is the best place for this code design-wise.  But it does
        //work so I'm leaving it here for the time being.  -:AMN: 20 Oct 03
        try
        
        {

            ion.firstTimeAdd(operatorWindow, swmm);
        }
        catch(IOException ioe)
        
        {

            JOptionPane.showMessageDialog(
                MainFrame.getMainFrame(),
                "IOException adding elaborations file to impasse.",
                "IOException",
                JOptionPane.INFORMATION_MESSAGE);
        }

        return this;
    }

    
    public void notifyDeletionOfChild(OperatorWindow operatorWindow,
                                      OperatorNode child) 
    {

        try 
        {

            sourceChildren();
        }
        catch(IOException ioe) {}
    }


    /**
     * This adjusts the context menu so that only the valid commands 
     * are displayed
     * @param c the owner of the context menu, should be the OperatorWindow
     * @param x the horizontal position on the screen where the context menu
     * should be displayed
     * @param y the vertical position on the screen where the context menu
     * should be displayed
     */
    public void showContextMenu(Component c, int x, int y) 
    {

        if (name.equals("elaborations")) 
        {

            addSuboperatorItem.setEnabled(false);
            addFileItem.setEnabled(true);
            openRulesItem.setEnabled(false);
            openDataMapItem.setEnabled(false);
            deleteItem.setEnabled(false);
            renameItem.setEnabled(false);
            impasseSubMenu.setEnabled(true);
        }
        else if(name.equals("common")) 
        {

            addSuboperatorItem.setEnabled(true);
            addFileItem.setEnabled(true);
            openRulesItem.setEnabled(false);
            openDataMapItem.setEnabled(false);
            deleteItem.setEnabled(true);
            renameItem.setEnabled(false);
            impasseSubMenu.setEnabled(true);
        }
        else 
        {

            addSuboperatorItem.setEnabled(true);
            addFileItem.setEnabled(true);
            openRulesItem.setEnabled(false);
            openDataMapItem.setEnabled(false);
            deleteItem.setEnabled(false);
            renameItem.setEnabled(false);
            impasseSubMenu.setEnabled(true);
        }
        exportItem.setEnabled(false);
        importItem.setEnabled(true);
        checkChildrenAgainstDataMapItem.setEnabled(true);
        contextMenu.show(c,x,y);    
    }

    /**
     * Given a Writer this writes out a description of the folder node
     * that can be read back in later
     * @param w the writer 
     * @throws IOException if there is an error writing to the writer
     */
    public void write(Writer w) throws IOException 
    {

        w.write("FOLDER " + name + " " + folderName + " " + id);
    }   
    
    public void exportDesc(Writer w) throws IOException 
    {

        w.write("FOLDER " + name);
    }
    
    public void exportFile(Writer w,int id) throws IOException {}
    
    public void exportDataMap(Writer w) throws IOException 
    {

        w.write("DATAMAP\n");
        MainFrame.getMainFrame().getOperatorWindow().getDatamap().write(w);
    }
    
    public void exportType(Writer w) throws IOException 
    {

        w.write("Not expecting to export this\n");
    }
    
    public void delete(OperatorWindow operatorWindow) 
    {

        renameToDeleted(new File(getFolderName()));     
        OperatorNode parent = (OperatorNode)getParent();
        operatorWindow.removeNode(this);
        parent.notifyDeletionOfChild(operatorWindow,this);
    }

    
    public void importFunc(Reader r,
                           OperatorWindow operatorWindow,
                           SoarWorkingMemoryModel swmm) throws IOException, NumberFormatException 
    {

        if(name.equals("common") || name.equals("all"))
        {
            VSEImporter.read(r,
                             this,
                             operatorWindow,
                             swmm,
                             VSEImporter.HLOPERATOR | VSEImporter.OPERATOR);
        }
        else
        {
            VSEImporter.read(r,this,operatorWindow,swmm,VSEImporter.FILE);
        }
    }
    
    public void copyStructures(File folderToWriteTo) throws IOException 
    {

        File copyOfFolder = new File(folderToWriteTo.getPath() + File.separator + folderName);
        copyOfFolder.mkdir();
        for(int i = 0; i < getChildCount(); ++i) 
        {

            ((OperatorNode)getChildAt(i)).copyStructures(copyOfFolder);
        }       
    }
    
    public DataFlavor isDropOk(int action,DataFlavor[] dataFlavors) 
    {

        if(name.equals("elaborations") == false) 
        {

            if(action == java.awt.dnd.DnDConstants.ACTION_MOVE) 
            {

                List flavorList = Arrays.asList(dataFlavors);
                if(flavorList.contains(TransferableOperatorNodeLink.flavors[0])) 
                {

                    return TransferableOperatorNodeLink.flavors[0]; 
                }
                else 
                {

                    return null;
                }
            }
            else 
            {

                return null;
            }
        }
        else 
        {

            return null;
        }
    }
    
    public void source(Writer w) throws IOException 
    {

        String LINE = System.getProperty("line.separator");
        w.write("pushd "  + folderName + LINE + 
                "source " + folderName + "_source.soar" + LINE + 
                "popd" + LINE);
    }
    
    public void sourceChildren() throws IOException 
    {

        Writer w = new FileWriter(getFullPathName() + File.separator + folderName + "_source.soar");
        int childCount = getChildCount();
        for(int i = 0; i < childCount; ++i) 
        {

            OperatorNode child = (OperatorNode)getChildAt(i);
            child.source(w);
        }
        w.close();
    }
    
    public void sourceRecursive() throws IOException 
    {

        sourceChildren();
        int childCount = getChildCount();
        for(int i = 0; i < childCount; ++i) 
        {

            OperatorNode child = (OperatorNode)getChildAt(i);
            child.sourceRecursive();
        }
    }

    public void searchTestDataMap(SoarWorkingMemoryModel swmm,
                                  Vector errors) {}
    public void searchCreateDataMap(SoarWorkingMemoryModel swmm,
                                    Vector errors) {}
    public void searchTestNoCreateDataMap(SoarWorkingMemoryModel swmm,
                                          Vector errors) {}
    public void searchCreateNoTestDataMap(SoarWorkingMemoryModel swmm,
                                          Vector errors) {}
    public void searchNoTestNoCreateDataMap(SoarWorkingMemoryModel swmm,
                                            Vector errors) {}  
    
}
