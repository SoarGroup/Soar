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

class SoarOperatorNode extends FileNode 
{

    /////////////////////////////////////////
    // DataMembers
    /////////////////////////////////////////

    // this member tells us whether or not this Operator is highlevel or not
    protected boolean isHighLevel = false;

    // if this SoarOperatorNode is highlevel then this is a string representing
    // just the folder name, else it is null
    protected String folderName;

    // if this SoarOperatorNode is highlevel then this is the Associated state
    // in the datamap, else it is null
    protected SoarIdentifierVertex dataMapId;

    // this is the number associated with the dataMapId, if the dataMapId is
    // null, then this is left unintialized, or 0
    protected int dataMapIdNumber;

    private List linkNodes = new LinkedList();

    /////////////////////////////////////////
    // Constructors
    /////////////////////////////////////////
    /**
     * this creates a low-level operator with the given name and file
     */
    public SoarOperatorNode(String inName,int inId,String inFileName) 
    {

        super(inName,inId,inFileName);
    }

    /**
     * this creates a highlevel operator with the given name, file, folder and
     * dataMapId
     */
    public SoarOperatorNode(String inName,int inId,String inFileName,String inFolderName,SoarIdentifierVertex inDataMapId) 
    {

        this(inName,inId,inFileName);
        folderName = inFolderName;
        dataMapId = inDataMapId;
        dataMapIdNumber = inDataMapId.getValue();
        isHighLevel = true;
    }

    /**
     * This will construct a high-level operator node, this one supports
     * serialization, restoreId must be called to get this object into a good
     * state
     */
    public SoarOperatorNode(String inName,int inId,String inFileName,String inFolderName,int inDataMapIdNumber) 
    {

        this(inName,inId,inFileName);
        folderName = inFolderName;
        dataMapIdNumber = inDataMapIdNumber;
        isHighLevel = true;
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

        if(isHighLevel) 
        {

            addSuboperatorItem.setEnabled(true);
            addFileItem.setEnabled(true);
            openRulesItem.setEnabled(true);
            openDataMapItem.setEnabled(true);
            deleteItem.setEnabled(true);
            renameItem.setEnabled(true);
            exportItem.setEnabled(true);
            impasseSubMenu.setEnabled(true);
            checkChildrenAgainstDataMapItem.setEnabled(true);
        }
        else 
        {

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


    /**
     * Given a Writer this writes out a description of the soar operator node
     * that can be read back in later
     * @param w the writer
     * @throws IOException if there is an error writing to the writer
     */

    public void exportDesc(Writer w) throws IOException 
    {

        if(isHighLevel)
        w.write("HLOPERATOR " + name + " " + dataMapIdNumber);
        else
        w.write("OPERATOR " + name);
    }

    /*
     * This represents the set of actions that should be preformed when an
     * operator becomes a high-level operator.  This function is overridden
     * in FileOperatorNode since it does not represent a Soar state.
     */
    public void firstTimeAdd(OperatorWindow operatorWindow,
                             SoarWorkingMemoryModel swmm) throws IOException 
    {

        // Create the Folder
        File folder = new File(((OperatorNode)getParent()).getFullPathName() + File.separator + name);

        if (!okayToCreate(folder, true))
        return;

        if (!folder.mkdir())
        throw new IOException();


        // Create the elaborations file
        File elaborationsFile = new File(folder.getPath() + File.separator + "elaborations.soar");
        elaborationsFile.createNewFile();

        // Create the elaborations node
        OperatorNode elaborationNode = operatorWindow.createFileOperatorNode("elaborations",elaborationsFile.getName());

        // Create the datamap id
        dataMapId = swmm.createNewStateId(((OperatorNode)getParent()).getStateIdVertex(),name);
        dataMapIdNumber = dataMapId.getValue();

        // Make this node highlevel
        isHighLevel = true;

        //Add the elaborations node and folder
        operatorWindow.addChild(this,elaborationNode); 
        folderName = folder.getName();
    }
    
    public void importFunc(Reader r,
                           OperatorWindow operatorWindow,
                           SoarWorkingMemoryModel swmm) throws IOException, NumberFormatException 
    {

        if(!isHighLevel)
        firstTimeAdd(operatorWindow,swmm);
        VSEImporter.read(r,this,operatorWindow,swmm,VSEImporter.HLOPERATOR | VSEImporter.OPERATOR);
    }
    
    public boolean isDragOk(int action) 
    {

        if(isHighLevel && action == java.awt.dnd.DnDConstants.ACTION_LINK)
        return true;
        else
        return false;
    }
    
    public DataFlavor isDropOk(int action,DataFlavor[] dataFlavors) 
    {

        if(    (action == java.awt.dnd.DnDConstants.ACTION_LINK)
            || (action == java.awt.dnd.DnDConstants.ACTION_MOVE) )
        {

            List flavorList = Arrays.asList(dataFlavors);
            if(flavorList.contains(TransferableOperatorNodeLink.flavors[0])) 
            {

                return TransferableOperatorNodeLink.flavors[0]; 
            }
        }
        return null;
    }
    
    public void exportType(Writer w) throws IOException 
    {

        if(isHighLevel) 
        {

            w.write("IMPORT_TYPE " + VSEImporter.HLOPERATOR + "\n");
        }
        else 
        {

            w.write("IMPORT_TYPE " + VSEImporter.OPERATOR + "\n");
        }
    }
    
    /**
     * Use this getter function to get the path to the folder
     * @return the path to the folder
     */
    public String getFolderName() 
    {

        return getFullPathName();
    }
    
    /**
     * Use this getter function to get the path to the rule file
     * @return the path to the rule file
     */
    public String getFileName() 
    {

        OperatorNode parent = (OperatorNode)getParent();
        return parent.getFullPathName() + File.separator + fileAssociation;
    }

    protected String getFullPathName() 
    {

        if(isHighLevel)
        return ((OperatorNode)getParent()).getFullPathName() + File.separator + folderName;
        else
        return ((OperatorNode)getParent()).getFullPathName();
    }

    public SoarIdentifierVertex getStateIdVertex() 
    {

        return dataMapId;
    }

    /**
   * @return whether an openDataMap() call on this node will work
   */
	public boolean hasDataMap()
    {
		return isHighLevel;
	}


    
    /**
     * This is a helper function that renames the folder for which this node is
     * associated to it's new folder name, and also notifies the children of
     * this node that there has been a change
     * @param newFolderName a String that represents the new folder path
     */
    protected void renameFolder(String newFolderName) 
    {

        File folder = new File(folderName);
        if (folder.renameTo(new File(newFolderName))) 
        {

            folderName = newFolderName;
            System.out.println("Folder renamed to: " + newFolderName + " from " + folder.getPath());
            // Notify the children of the change
            java.util.Enumeration e = breadthFirstEnumeration();
            // skip the first one
            if (e.hasMoreElements())
            e.nextElement();
            while(e.hasMoreElements()) 
            {

                OperatorNode childNode = (OperatorNode)e.nextElement();
            }
        }
    }
    
    public void restoreId(SoarWorkingMemoryModel swmm) 
    {

        dataMapId = (SoarIdentifierVertex)swmm.getVertexForId(dataMapIdNumber);
    }
    
    /**
     * The user wants to rename this node
     * @param model the model for which this node is contained within
     * @param newName the new name that the user wants this node to be called
     */
        
    public void rename(OperatorWindow operatorWindow,
                       String newName) throws IOException 
    {

        DefaultTreeModel model = (DefaultTreeModel)operatorWindow.getModel();
        if(isHighLevel) 
        {

            boolean success = true;
            
            // Rename File
            File oldFile = new File(getFileName());
            File oldFolder = new File(getFolderName());     
            
            File newFile = new File(oldFile.getParent() + File.separator + newName + ".soar");
            File newFolder = new File(oldFolder.getParent() + File.separator + newName);
            
            
            if ((! okayToCreate(newFolder)) || (! okayToCreate(newFile))) 
            {

                return;
            }
            
            if (oldFile.renameTo(newFile)) 
            {

                fileAssociation = newFile.getName();
                if (ruleEditor != null)
                ruleEditor.fileRenamed(newFile.getPath());
            }   
            else { 
                throw new IOException();
            }
                    
            if (success) 
            {

                // Rename Folder
                //%%%Remove the println calls from this
                if (!oldFolder.renameTo(newFolder)) 
                {

                    success = false;
                    System.out.println("Folder Rename Failed!, HighLevelOperatorNode.rename");
                    if (!newFile.renameTo(oldFile)) 
                    {

                        System.out.println("Foobar, the File was phreaking renamed, folder rename wasn't successful, tried to rename the file back, but wouldn't let me");
                        System.exit(-1);
                    }
                    else 
                    {

                        fileAssociation = newFile.getName();
                        if (ruleEditor != null)
                        ruleEditor.fileRenamed(newFile.getPath());
                    }
                }
                else 
                folderName = newFolder.getName(); 
            }
            if (success)
            name = newName;
            // Notify the tree model
            model.nodeChanged(this);    
        }
        else 
        {

            File oldFile = new File(getFileName());
            File newFile = new File(oldFile.getParent() + File.separator + newName + ".soar");
            
            if (! okayToCreate(newFile)) 
            {

                return;
            }
            String oldName = name;
            if (!oldFile.renameTo(newFile)) 
            throw new IOException();
            else 
            {

                name = newName;
                fileAssociation = newFile.getName();
                if (ruleEditor != null)
                ruleEditor.fileRenamed(newFile.getPath());
            }   
            model.nodeChanged(this);
        }
        
    }

    /**
     * Given a Writer this writes out a description of the high level operator
     * node that can be read back in later
     * @param w the writer 
     * @throws IOException if there is an error writing to the writer
     */
/*
  public void write(Writer w) throws IOException 
  {

  if(isHighLevel) 
  {

  w.write("HLOPERATOR " + name + " " + fileAssociation + " " + folderName + " " + dataMapId.getValue() + " " + id);
  }
  else 
  {

  w.write("OPERATOR " + name  + " " + fileAssociation + " " + id);
  }
  }
*/



    /**
     * This is the function that gets called when you want to add a suboperator to this node
     * @param model the tree model for which this node is currently a member
     * @param newOperatorName the name of the new operator to add
     */
    public OperatorNode addSuboperator(OperatorWindow operatorWindow,
                                       SoarWorkingMemoryModel swmm,
                                       String newOperatorName) throws IOException 
    {

        if(!isHighLevel)
        firstTimeAdd(operatorWindow,swmm);
        
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
        
        SoarVertex opName = swmm.createNewEnumeration(newOperatorName);
        SoarVertex operId = swmm.createNewSoarId();
        swmm.addTriple(dataMapId, "operator", operId);
        swmm.addTriple(operId,"name",opName);

        notifyLinksOfUpdate(operatorWindow);
        sourceChildren();
        return this;
    }

    /**
     * This is the function that gets called when you want to add a sub Impasse Operator to this node
     * @param model the tree model for which this node is currently a member
     * @param newOperatorName the name of the new operator to add
     */
    public OperatorNode addImpasseOperator(OperatorWindow operatorWindow,
                                           SoarWorkingMemoryModel swmm,
                                           String newOperatorName) throws IOException 
    {

        if(!isHighLevel)
        firstTimeAdd(operatorWindow,swmm);

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

    /**
     * This is the function that gets called when you want to add a sub file
     * operator to this node
     * @param model the tree model for which this node is currently a member
     * @param newFileName the name of the new operator to add
     */
    public OperatorNode addFileOperator(OperatorWindow operatorWindow, SoarWorkingMemoryModel swmm, String newFileName) throws IOException 
    {

        if(!isHighLevel)
        firstTimeAdd(operatorWindow,swmm);

        File file = new File(getFullPathName() + File.separator + newFileName + ".soar");
        FileOperatorNode fon = null;

        if(!okayToCreateReplace(file)) 
        {

            return null;
        }
        fon = operatorWindow.createFileOperatorNode(newFileName, file.getName());
        operatorWindow.addChild(this,fon);
        sourceChildren();
        return this;
    }

    
    public void addLink(OperatorWindow operatorWindow,LinkNode linkNode) 
    {

        try 
        {

            if(!isHighLevel)
            firstTimeAdd(operatorWindow,operatorWindow.getDatamap());
            operatorWindow.addChild(this,linkNode);
            File rules = new File(linkNode.getFileName());
            rules.createNewFile();
            sourceChildren();
        }
        catch(IOException ioe) {}
    }
    
    /**
     * Removes the selected operator from the tree if it is allowed
     * @param model the model for which this node is a member of
     */ 
    public void delete(OperatorWindow operatorWindow) 
    {

        if(isHighLevel) 
        {

            if(!linkNodes.isEmpty()) 
            {

                int selOption =
                    JOptionPane.showConfirmDialog(MainFrame.getMainFrame(),
                                                  "This Operator has links, which will be deleted do you want to continue?",
                                                  "Link Deletion Confirmation",
                                                  JOptionPane.YES_NO_OPTION);
                if(selOption == JOptionPane.NO_OPTION) return;
                List linkNodesCopy = (List)((LinkedList)linkNodes).clone();
                Iterator i = linkNodesCopy.iterator();
                while(i.hasNext()) 
                {

                    LinkNode linkNodeToDelete = (LinkNode)i.next();
                    linkNodeToDelete.delete(operatorWindow);
                }
            }
            renameToDeleted(new File(getFileName()));
            renameToDeleted(new File(getFolderName()));     
            OperatorNode parent = (OperatorNode)getParent();
            operatorWindow.removeNode(this);
            parent.notifyDeletionOfChild(operatorWindow,this);
            
        }
        else 
        {

            renameToDeleted(new File(getFileName()));
            OperatorNode parent = (OperatorNode)getParent();
            operatorWindow.removeNode(this);
            parent.notifyDeletionOfChild(operatorWindow,this);
        }
    }   
    
    
    
    /**
     * A child has been deleted from this node, so check if this node has
     * become a low-level operator now
     * @param model the model for which this node is associated
     */
    public void notifyDeletionOfChild(OperatorWindow operatorWindow,
                                      OperatorNode child) 
    {

        if (getChildCount() == 0) 
        {

            DefaultTreeModel model = (DefaultTreeModel)operatorWindow.getModel();
            renameToDeleted(new File(getFolderName()));
            OperatorNode parent = (OperatorNode)getParent();
            int index = model.getIndexOfChild(parent,this); 
            isHighLevel = false;
            folderName = null;
            dataMapId = null;
            
            if(!linkNodes.isEmpty()) 
            {

                JOptionPane.showMessageDialog(MainFrame.getMainFrame(),
                                              "This node is no longer high-level, links will be deleted.",
                                              "Link deletions",
                                              JOptionPane.INFORMATION_MESSAGE);
                List linkNodesCopy = (List)((LinkedList)linkNodes).clone();
                Iterator i = linkNodesCopy.iterator();
                while(i.hasNext()) 
                {

                    LinkNode linkNodeToDelete = (LinkNode)i.next();
                    linkNodeToDelete.delete(operatorWindow);
                }
            }
        }
        notifyLinksOfUpdate(operatorWindow);
        try 
        {

            sourceChildren();
        }
        catch(IOException ioe) {}
    }

    /**
     * This opens/shows a dataMap with this nodes associated Data Map File
     * @param pw the MainFrame 
     */
    public void openDataMap(SoarWorkingMemoryModel swmm,MainFrame pw) 
    {

        DataMap dataMap;
        SoarIdentifierVertex dataMapParent =
            new SoarIdentifierVertex(dataMapId.getValue());

        // Check to see if Operator uses topstate datamap.
        if(dataMapIdNumber != 0) 
        {

            // If File Operator, get the datamap of the first OperatorOperatornode above it
            if(this instanceof FileOperatorNode) 
            {

                OperatorNode parent = (OperatorNode)this.getParent();
                dataMapParent = parent.getStateIdVertex();
                while(    (( parent.getStateIdVertex()).getValue() != 0)
                       && (!(parent instanceof OperatorOperatorNode)) ) 
                {

                    parent = (OperatorNode)parent.getParent();
                    dataMapParent = parent.getStateIdVertex();
                }   
                dataMap = new DataMap(swmm,dataMapParent, parent.toString());
                pw.getDesktopPane().dmAddDataMap(dataMapParent.getValue(), dataMap);
            }
            else 
            {

                dataMap = new DataMap(swmm,dataMapId,toString());
                pw.getDesktopPane().dmAddDataMap(dataMapId.getValue(), dataMap);
            }
        }
        else 
        {

            dataMap = new DataMap(swmm, swmm.getTopstate(),toString());
            pw.getDesktopPane().dmAddDataMap(swmm.getTopstate().getValue(),
                                             dataMap);
        }
      
        dataMap.setVisible(true);
        pw.addDataMap(dataMap);
    }

    public SoarIdentifierVertex getState() 
    {

        return dataMapId;
    }
    
    public void exportDataMap(Writer w) throws IOException 
    {

        w.write("DATAMAP\n");
        MainFrame.getMainFrame().getOperatorWindow().getDatamap().write(w);
    }
    
    public boolean isHighLevel() 
    {

        return isHighLevel;
    }
    
    public void registerLink(LinkNode inLinkNode) 
    {

        linkNodes.add(inLinkNode);
    }
    
    public void removeLink(LinkNode inLinkNode) 
    {

        linkNodes.remove(inLinkNode);
    }
    
    void notifyLinksOfUpdate(OperatorWindow operatorWindow) 
    {

        DefaultTreeModel model = (DefaultTreeModel)operatorWindow.getModel();
        Iterator i = linkNodes.iterator();
        while(i.hasNext()) 
        {

            LinkNode nodeToUpdate = (LinkNode)i.next();
            model.nodeStructureChanged(nodeToUpdate);
        }
    }
    
    
    public void copyStructures(File folderToWriteTo) throws IOException 
    {

        super.copyStructures(folderToWriteTo);
        
        if(isHighLevel) 
        {

            File copyOfFolder = new File(folderToWriteTo.getPath() + File.separator + folderName);
            copyOfFolder.mkdir();
            for(int i = 0; i < getChildCount(); ++i) 
            {

                ((OperatorNode)getChildAt(i)).copyStructures(copyOfFolder);
            }       
        }
    }
    
    public boolean move(OperatorWindow operatorWindow,OperatorNode newParent) 
    {

        if(    (newParent instanceof SoarOperatorNode)
           && !((SoarOperatorNode)newParent).isHighLevel() ) 
        {

            try 
            {

                ((SoarOperatorNode)newParent).firstTimeAdd(operatorWindow,
                                                           operatorWindow.getDatamap());
            }
            catch(IOException ioe) 
            {

                System.out.println("Move failed, because firstTimeAdd on parent failed");
                return true;
            }
        }   
        if(!newParent.okayToCreate(new File(newParent.getFolderName() + File.separator + fileAssociation))) 
        {

            return true;
        }

        if(isHighLevel) 
        {

            // Rename Folder
            File oldFolder = new File(getFolderName());
            File newFolder = new File(newParent.getFolderName() + File.separator + folderName);
            oldFolder.renameTo(newFolder);
            
            // Adjust Superstate Link
            SoarWorkingMemoryModel swmm = operatorWindow.getDatamap();
            java.util.List list = new LinkedList();
            Enumeration e = swmm.emanatingEdges(dataMapId);
            while(e.hasMoreElements()) 
            {

                NamedEdge ne = (NamedEdge)e.nextElement();
                list.add(ne);
            }
            Iterator i = list.iterator();
            while(i.hasNext()) 
            {

                NamedEdge ne = (NamedEdge)i.next();
                if(ne.getName().equals("superstate")) 
                {

                    swmm.removeTriple((SoarVertex)ne.V0(),
                                      ne.getName(),
                                      (SoarVertex)ne.V1());
                }
            }
            
            // Add new superstate link
            SoarVertex soarVertex = newParent.getStateIdVertex();
            if(soarVertex == null)
            soarVertex = swmm.getTopstate();
            
            swmm.addTriple(dataMapId,"superstate",soarVertex);
        }
        
        // Rename File
        File oldFile = new File(getFileName());
        File newFile = new File(newParent.getFolderName() + File.separator + fileAssociation);
        oldFile.renameTo(newFile);
        
        DefaultTreeModel model = (DefaultTreeModel)operatorWindow.getModel();
        OperatorNode oldParent = (OperatorNode)getParent();
        model.removeNodeFromParent(this);
        oldParent.notifyDeletionOfChild(operatorWindow,this);
        
        operatorWindow.addChild(newParent,this);
        // Adjust rule editor if one is open
        if (ruleEditor != null)
        ruleEditor.fileRenamed(newFile.getPath());

        return true;
    }   
    
    public void source(Writer w) throws IOException 
    {

        super.source(w);
        if(isHighLevel) 
        {

            String LINE = System.getProperty("line.separator");
            w.write("pushd "  + folderName + LINE + 
                    "source " + folderName + "_source.soar" + LINE + 
                    "popd" + LINE);
        }
    }
    
    public void sourceChildren() throws IOException 
    {

        if(isHighLevel) 
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
    }
    
    public void sourceRecursive() throws IOException 
    {

        if(isHighLevel) 
        {

            sourceChildren();
            int childCount = getChildCount();
            for(int i = 0; i < childCount; ++i) 
            {

                OperatorNode child = (OperatorNode)getChildAt(i);
                child.sourceRecursive();
            }
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
