package edu.umich.visualsoar.operatorwindow;
import edu.umich.visualsoar.parser.*;
import edu.umich.visualsoar.ruleeditor.RuleEditor;
import edu.umich.visualsoar.MainFrame;
import edu.umich.visualsoar.datamap.*;
import edu.umich.visualsoar.util.*;
import edu.umich.visualsoar.misc.*;

import java.io.*;
import java.awt.Component;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.JOptionPane;
import javax.swing.tree.TreePath;
import java.util.*;
import java.awt.*;

/**
 * This is the  file node for the operator window
 * @author Brad Jones
 * @version 0.5a 5 Aug 1999
 */
public class FileNode extends OperatorNode implements java.io.Serializable 
{
///////////////////////////////////////////////////////////////////
// Data Members
///////////////////////////////////////////////////////////////////
    /**
     * a string that is the path to the file which is associated with this file
     */
    protected String fileAssociation = null;

    /**
     * a reference to the rule editor, null if there isn't one
     */
    protected RuleEditor ruleEditor = null;
    
    protected String fullTransferFileName = null;
    protected TreePath transferTreePath = null;
    
///////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////
    public FileNode(String inName,int inId,String inFile) 
    {
        super(inName,inId);
        fileAssociation = inFile;
    }
    
///////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////
    /**
     * Use this getter function to get the path to the rule file
     * @return the path to the datamap
     */
    public String getFileName() 
    {
        OperatorNode parent = (OperatorNode)getParent();
        return parent.getFullPathName() + File.separator + fileAssociation;
    }
    
    /**
     * This is the function that gets called when you want to add a file to this
     * node
     * @param model the tree model for which this node is currently a member
     * @param newFileName the name of the new operator to add
     */
    public void addFile(OperatorWindow operatorWindow, String newFileName) throws IOException 
    {
        File file = new File(getFullPathName() + File.separator + newFileName + ".soar");

        // Check to make sure file does not exist
        if (! okayToCreateReplace(file)) 
        {
            return;
        }
    
        //FileNode fn = operatorWindow.createFileNode(newFileName,file.getName());
        FileOperatorNode fon = operatorWindow.createFileOperatorNode(newFileName,file.getName());
        operatorWindow.addChild(this,fon);
        sourceChildren();
    }



    public void setTransferFullPath() 
    {
        fullTransferFileName = getFileName();
        transferTreePath = new TreePath(getPath());
    }
    
    public javax.swing.tree.TreePath getTransferTreePath() 
    {
        return transferTreePath;
    }
    
    public void moveAssociations() 
    {
        File file = new File(fullTransferFileName);
        file.renameTo(new File(getFileName()));
    }
    
    /**
     * This is a helper function that renames the file for which this node is
     * associated to it's new file name, and also notifies the rule editor if
     * there is one of the change
     * @param newFileName a String that represents the new file path
     */
    protected void renameFile(String newFileName) 
    {
        File file = new File(fileAssociation);
        if (file.renameTo(new File(newFileName))) 
        {
            fileAssociation = newFileName; 
            if (ruleEditor != null)
            ruleEditor.fileRenamed(newFileName);
        }
    }
    
    /**
     * This resets the rule editor to null for this node
     */
    public void clearRuleEditor() 
    {
        ruleEditor = null;
    }
    
    public void setRuleEditor(RuleEditor re) 
    {
        ruleEditor = re;
    }

    
    /**
     * An ancestor has been renamed so we must update our file association
     * @param oldFilePath the ancestor's old File path
     * @param newFilePath the ancestor's new File path
     */
    public void notifyChildrenOfRename(String oldFilePath, String newFilePath) 
    {
        int uniqueToThisNode = oldFilePath.length();
        String newFileName = newFilePath + fileAssociation.substring(uniqueToThisNode,fileAssociation.length());
        fileAssociation = newFileName;
        if (ruleEditor != null)
        ruleEditor.fileRenamed(newFileName);
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
        File oldFile = new File(getFileName());
        File newFile = new File(oldFile.getParent() + File.separator + newName + ".soar");

        if (! okayToCreate(newFile)) 
        {
            throw new IOException("Bad file name");
        }

        if (!oldFile.renameTo(newFile)) 
        {
            throw new IOException("Unable to rename operator file.");
        }
        else 
        {
            name = newName;
            fileAssociation = newFile.getName();
            if (ruleEditor != null)
            ruleEditor.fileRenamed(newFile.getPath());
        }   
        model.nodeChanged(this);  
    }
        
    /**
     * Given a Writer this writes out a description of the operator node
     * that can be read back in later
     * @param w the writer 
     * @throws IOException if there is an error writing to the writer
     */
    public void write(Writer w) throws IOException 
    {
        w.write("FILE " + name + " " + fileAssociation + " " + id);     
    }
    
    /**
     * Given a Writer this writes out a description of the operator node
     * that can be read back later
     * @param w where the description should be written to
     * @throws IOException if there is an error writing to the writer
     */ 
    public void exportDesc(Writer w) throws IOException 
    {
        w.write("FILE " + name);
    }
    
    public void exportType(Writer w) throws IOException 
    {
        w.write("IMPORT_TYPE " + VSEImporter.FILE + "\n");
    }
    
    /**
     * Given a Writer this writes out the rules as it is either in
     * the file or the rule editor
     * @param w where the file should be written to
     * @throws IOException if there is an error writing to the writer
     */
    public void exportFile(Writer w,int id) throws IOException 
    {
        w.write("RULE_FILE " + id + " ");
        if(ruleEditor == null) 
        {
            StringWriter sw = new StringWriter();
            LineNumberReader lnr =
                new LineNumberReader(new FileReader(getFileName()));
            int lines = 0;
            String s = lnr.readLine();
            while(s != null) 
            {
                ++lines;
                sw.write(s + "\n");
                s = lnr.readLine();
            }           
            w.write("" + lines + "\n");
            w.write(sw.toString() + "\n");
        }
        else 
        {
            w.write("" + ruleEditor.getNumberOfLines() + "\n");
            w.write(ruleEditor.getAllText() + "\n");
        }
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
    public void showContextMenu(Component c,int x, int y) 
    {
        if (name.equals("elaborations") ) 
        {
            addSuboperatorItem.setEnabled(false);
            addFileItem.setEnabled(false);
            openRulesItem.setEnabled(true);
            openDataMapItem.setEnabled(false);
            deleteItem.setEnabled(getParent().getChildCount() == 1);
            renameItem.setEnabled(false);
            exportItem.setEnabled(true);
            impasseSubMenu.setEnabled(false);
            checkChildrenAgainstDataMapItem.setEnabled(false);
        }
        else 
        {
            addSuboperatorItem.setEnabled(false);
            addFileItem.setEnabled(false);
            openRulesItem.setEnabled(true);
            openDataMapItem.setEnabled(false);
            deleteItem.setEnabled(true);
            renameItem.setEnabled(true);
            exportItem.setEnabled(true);
            impasseSubMenu.setEnabled(false);
            checkChildrenAgainstDataMapItem.setEnabled(false);
        }
        contextMenu.show(c,x,y);
    }

    /**
     * Removes the selected file from the tree if it is allowed
     */ 
    public void delete(OperatorWindow operatorWindow) 
    {
        OperatorNode parent = (OperatorNode)getParent();

        if (name.equals("elaborations") ) 
        {
            JOptionPane.showMessageDialog(MainFrame.getMainFrame(),
                                          "The elaborations file may not be deleted", 
                                          "Delete Error",
                                          JOptionPane.ERROR_MESSAGE); 
            return;
        }
    
        renameToDeleted(new File(getFileName()));
        
        operatorWindow.removeNode(this);
        parent.notifyDeletionOfChild(operatorWindow,this);  
    }
    
    /**
     * This will parse the productions for a given file node
     * if a rule editor is open for the file it just forwards the call to the 
     * open rule editor, else it opens the file and attempts to parse the
     * productions 
     */
    
    public Vector parseProductions() throws ParseException, java.io.IOException 
    {
        if(name.startsWith("_")) return null;
        
        if(ruleEditor == null) 
        {
            java.io.Reader r = new java.io.FileReader(getFileName());
            SoarParser aParser = new SoarParser(r);
            Vector v = aParser.VisualSoarFile();
            r.close();
            return v;
        }
        return ruleEditor.parseProductions();
    }

    /**
     * This will check the productions in this file node for datamap errors.
     * if a rule editor is open for the file it just forwards the call to the 
     * open rule editor, else it opens the file and attempts to parse the
     * productions 
     */
    public boolean CheckAgainstDatamap(Vector vecErrors) throws IOException
    {
        Vector parsedProds = new Vector();
        boolean anyErrors = false;
        java.util.List errors = new LinkedList();
        
        try
        {
            parsedProds = parseProductions();
        }
        catch(ParseException pe)
        {
            anyErrors = true;
            String parseError = pe.toString();
            int i = parseError.lastIndexOf("line ");
            String lineNum = parseError.substring(i + 5);
            i = lineNum.indexOf(',');
            lineNum = "(" + lineNum.substring(0, i) + "): ";
            String errString = getFileName() + lineNum + "Unable to check productions due to parse error";
            errors.add(errString);
        }
        catch(TokenMgrError tme) 
        {
            tme.printStackTrace();
        }

        if (parsedProds!= null)
        {
            OperatorWindow ow = MainFrame.getMainFrame().getOperatorWindow();
            ow.checkProductions((OperatorNode)getParent(), parsedProds, errors);
        }
        
        if (!errors.isEmpty())
        {
            anyErrors = true;
            Enumeration e = new EnumerationIteratorWrapper(errors.iterator());
            while(e.hasMoreElements()) 
            {
                try 
                {
                    String errorString = e.nextElement().toString();
                    String numberString = errorString.substring(errorString.indexOf("(")+1,errorString.indexOf(")"));
                    if(errorString.endsWith("Unable to check productions due to parse error"))
                    {
                        vecErrors.add(
                            new FeedbackListObject(this,Integer.parseInt(numberString),errorString,true,true));
                    }
                    else
                    {
                        vecErrors.add(
                            new FeedbackListObject(this,Integer.parseInt(numberString),errorString,true));
                    }
                }
                catch(NumberFormatException nfe)
                { /* should never happen*/ }
            }//while
        }//if

        return anyErrors;

        
    }//CheckAgainstDatamap
    
    /**
     * This opens/shows a rule editor with this nodes associated file
     * @param pw the MainFrame 
     */
    public void openRules(MainFrame pw) 
    {
        if (ruleEditor == null || ruleEditor.isClosed()) 
        {
            try 
            {
                ruleEditor = new RuleEditor(new java.io.File(getFileName()),
                                            this);
                ruleEditor.setVisible(true);
                pw.addRuleEditor(ruleEditor);
                ruleEditor.setSelected(true);
            }
            catch(IOException ioe) 
            {
                JOptionPane.showMessageDialog(pw,
                                              "There was an error reading file: " +  fileAssociation,
                                              "I/O Error",
                                              JOptionPane.ERROR_MESSAGE); 
            }
            catch(java.beans.PropertyVetoException pve)
            {
                //No sweat. This just means the new window failed to get focus.
            }
        } 
        else 
        {
            pw.showRuleEditor(ruleEditor);
        }
    }
    
    /**
     * This opens/shows a rule editor with this nodes associated file
     * and places the caret on the given line number
     * @param pw the Project window
     * @param line the line number to place the caret on
     */
    public void openRules(MainFrame pw, int line) 
    {
        openRules(pw);
        ruleEditor.setLine(line);
    }
    
    /**
     * This opens/shows a rule editor with this nodes associated file
     * and displays a substring of the file starting on a given line
     * @param pw the Project window
     * @param line the line number to place the caret on
     * @param assocString the substring to place the caret on
    */
    public void openRulesToString(MainFrame pw, int line, String assocString) 
    {
        openRules(pw);
        ruleEditor.highlightString(line, assocString);
    }
    
    protected String getFullPathName() 
    {
        return null;
    }   
    
    public void exportDataMap(Writer w) throws IOException 
    {
        w.write("NODATAMAP\n");
    }
    
    public void copyStructures(File folderToWriteTo) throws IOException 
    {
        File copyOfFile = new File(folderToWriteTo.getPath() + File.separator + fileAssociation);
        Writer w = new FileWriter(copyOfFile);
        Reader r = new FileReader(getFileName());
        edu.umich.visualsoar.util.ReaderUtils.copy(w,r);
        w.close();
        r.close();
    }
    
    public void source(Writer w) throws IOException 
    {
        String LINE = System.getProperty("line.separator");
        w.write("source " + fileAssociation + LINE);
    }
    
    public boolean needsToSourceChildren() 
    {
        return false;
    }
    
    public void sourceChildren() throws IOException {}
    public void sourceRecursive() throws IOException {}

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
