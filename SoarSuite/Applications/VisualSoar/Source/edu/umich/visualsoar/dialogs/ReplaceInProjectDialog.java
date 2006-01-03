package edu.umich.visualsoar.dialogs;

import edu.umich.visualsoar.ruleeditor.RuleEditor;
import edu.umich.visualsoar.operatorwindow.OperatorWindow;
import edu.umich.visualsoar.operatorwindow.OperatorNode;
import edu.umich.visualsoar.operatorwindow.TreeNode;

import edu.umich.visualsoar.parser.*;
import edu.umich.visualsoar.datamap.*;
import edu.umich.visualsoar.dialogs.*;
import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.MainFrame;
import edu.umich.visualsoar.util.*;
import edu.umich.visualsoar.misc.FeedbackListObject;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.filechooser.FileFilter;
import java.awt.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;

/**
 * Dialog which takes input for, and initiates a find or replace
 * operation over multiple project files.
 * @author Brian Harleton
 * @see FindInProjectDialog
 */
public class ReplaceInProjectDialog extends JDialog 
{
    /**
     * These keep track of place in directory tree that search is currently being
     * performed.
     */
    OperatorNode root;
    Enumeration bfe;
    boolean searchingRuleEditor;
    boolean stringFound;
    boolean stringSelected;
    OperatorNode current;
    String fn;
    String lastToFind;

    /**
     * panel which contains the find input field and match case option
     */
    FindInProjectPanel findPanel = new FindInProjectPanel();


    /**
     * Holds the current rule editor that find/replace is currently in
     *
     */
    RuleEditor        d_ruleEditor = null;
    OperatorWindow    opWin = null;

    /**
     * panel which contains all the replace input field
     */
    ReplacePanel           replacePanel = new ReplacePanel();
    FindReplaceButtonPanel buttonPanel  = new FindReplaceButtonPanel();
    Vector                 v            = new Vector();

    /**
     * Dialog that searches through all the files within a project for a string
     * and replaces that string with another string.
     * Replace button replaces currently selected string with replacer string and
     * then moves to the next matching string in the project.
     * Find Next button highlights finds the next matching string in the project'
     * and highlights that string.
     *
     * @param owner Frame which owns the dialog
     * @param operators a reference to the OperatorWindow
     * @param opNode operator tree to search (a null value indicates the entire
     *               project should be searched)
     */
    public ReplaceInProjectDialog(final Frame owner,
                                  OperatorWindow operators,
                                  OperatorNode opNode) 
    {
        super(owner, "Find and Replace In Project", false);

        opWin = operators;
        setResizable(false);
        Container contentPane = getContentPane();
        GridBagLayout gridbag = new GridBagLayout();
        GridBagConstraints c = new GridBagConstraints();
        contentPane.setLayout(gridbag);

        root = opNode;
        bfe = root.breadthFirstEnumeration();
        searchingRuleEditor = false;
        stringFound = false;
        current = null;
        lastToFind = null;
        stringSelected = false;

        // specifies component as last one on the row
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.fill = GridBagConstraints.HORIZONTAL;

        contentPane.add(findPanel, c);
        contentPane.add(replacePanel, c);
        contentPane.add(buttonPanel, c);
        pack();
        getRootPane().setDefaultButton(buttonPanel.findButton);


        //Set the match case as unfocusable so user can
        //quickly tab between the find & replace fields
        findPanel.optionsPanel.matchCase.setFocusable(false);

        addWindowListener(
            new WindowAdapter() 
            {
                public void windowOpened(WindowEvent we) 
                    {
                        setLocationRelativeTo(owner);
                        findPanel.requestFocus();
                    }
            });

        buttonPanel.cancelButton.addActionListener(
            new ActionListener() 
            {
                public void actionPerformed(ActionEvent e) 
                    {
                        dispose();
                    }
            });


        /**
         *   Replace all replaces all requested strings with the replacement string.
         *   All instances of replacement are sent to the feedback list.
         */
        buttonPanel.replaceAllButton.addActionListener(
            new ActionListener()
            {
                public void actionPerformed(ActionEvent e)
                    {
                        Object[] theData = findPanel.getData();
                        String toFind = (String)theData[0];
                        String toReplace = replacePanel.getText();
                        Boolean caseSensitive = (Boolean)theData[1];

                        // If user changed the 'To Find' string, reset search to beginning of tree
                        if( !toFind.equals(lastToFind) )
                        {
                            lastToFind = toFind;
                            // Reset tree to beginning of project
                            bfe = root.breadthFirstEnumeration();

                            stringSelected = false;
                            searchingRuleEditor = false;
                        }


                        //  Do initial search to get all of the line numbers of all the replaced components
                        if (! caseSensitive.booleanValue() ) 
                        {
                            toFind = toFind.toLowerCase();
                        }

                        while(bfe.hasMoreElements()) 
                        {
                            OperatorNode current = (OperatorNode)bfe.nextElement();
                            String fn = current.getFileName();

                            if (fn != null) 
                            {
                                try 
                                {
                                    LineNumberReader lnr = new LineNumberReader(new FileReader(fn));
                                    String line = lnr.readLine();
                                    while (line != null) 
                                    {
                                        if (!caseSensitive.booleanValue() ) 
                                        {
                                            line = line.toLowerCase();
                                        }
                                        if (line.indexOf(toFind) != -1) 
                                        {
                                            v.add(new FeedbackListObject(current,
                                                                         lnr.getLineNumber(),
                                                                         "Replaced " + toFind + " with " + toReplace + ".",
                                                                         toReplace) );
                                        }
                                        line = lnr.readLine();
                                    }
                                    lnr.close();
                                }
                                catch(FileNotFoundException fnfe) 
                                {
                                    System.err.println("Couldn't find: " + fn);
                                }
                                catch(IOException ioe) 
                                {
                                    System.err.println("Error reading from file " + fn);
                                }
                            }
                        }

                        if (v.isEmpty()) 
                        {
                            v.add(toFind + " not found in project");
                        }



                        // Now do the actual replacement of the strings
                        bfe = root.breadthFirstEnumeration();

                        while( bfe.hasMoreElements() || searchingRuleEditor )
                        {
                            if(stringSelected) 
                            {
                                // replace
                                d_ruleEditor.replace();
                            }

                            findInProject(toFind, toReplace, caseSensitive, true);
                        }

                        MainFrame.getMainFrame().setFeedbackListData(v);

                        if (! buttonPanel.keepDialog.isSelected() ) 
                        {
                            dispose();
                        }

                    }

            });

        buttonPanel.replaceButton.addActionListener(
            new ActionListener()
            {
                public void actionPerformed(ActionEvent e)
                    {
                        Object[] theData = findPanel.getData();
                        String toFind = (String)theData[0];
                        String toReplace = replacePanel.getText();
                        Boolean caseSensitive = (Boolean)theData[1];

                        // If user changed the 'To Find' string
                        if( !toFind.equals(lastToFind) )
                        {
                            lastToFind = toFind;
                            // Reset tree to beginning of project
                            bfe = root.breadthFirstEnumeration();

                            stringSelected = false;
                            searchingRuleEditor = false;
                        }


                        if(stringSelected) 
                        {
                            // replace
                            d_ruleEditor.replace();
                        }

                        findInProject(toFind, toReplace, caseSensitive, false);

                        if (! buttonPanel.keepDialog.isSelected() ) 
                        {
                            dispose();
                        }
                    }
            });

        buttonPanel.findButton.addActionListener(
            new ActionListener()
            {
                public void actionPerformed(ActionEvent e)
                    {
                        Object[] theData = findPanel.getData();
                        String toFind = (String)theData[0];
                        String toReplace = replacePanel.getText();
                        Boolean caseSensitive = (Boolean)theData[1];

                        // If user changed the 'To Find' string
                        if( !toFind.equals(lastToFind) )
                        {
                            lastToFind = toFind;
                            // Reset tree to beginning of project
                            bfe = root.breadthFirstEnumeration();

                            searchingRuleEditor = false;
                        }


                        findInProject(toFind, toReplace, caseSensitive, false);

                        if (! buttonPanel.keepDialog.isSelected() ) 
                        {
                            dispose();
                        }

                    }
            });
    }//ReplaceInProjectDialog ctor

    private void findInProject(String toFind, String toReplace, Boolean caseSensitive, boolean outputToFeedbackList)
    {
        boolean matchCase = caseSensitive.booleanValue();
        boolean foundInFile = false;
        stringFound = false;
        String reFileName = null;

        while( !stringFound && (bfe.hasMoreElements() || searchingRuleEditor) )
        {
            if( !searchingRuleEditor)
            {
                current = (OperatorNode)bfe.nextElement();
                fn = current.getFileName();

                // See if Rule Editor is already open for that file.  If it is, then start searching Rule Editor
                JInternalFrame[] bif = MainFrame.getMainFrame().getDesktopPane().getAllFrames();   // Get all open Rule Editors
                for(int i = 0; i < bif.length; ++i)
                {
                    if(bif[i] instanceof RuleEditor)
                    {
                        RuleEditor be = (RuleEditor)bif[i];
                        reFileName = be.getFile();

                        if( reFileName.equals(fn) )
                        {
                            d_ruleEditor = be;
                            d_ruleEditor.resetCaret();
                            searchingRuleEditor = true;
                            fn = null;
                        }    // found rule editor that matches filename where bfe is currently at
                    }
                }  // end of for (going through open RE's looking for correct one)

                if(fn != null)
                {
                    try 
                    {
                        LineNumberReader lnr = new LineNumberReader(new FileReader(fn));
                        String line = lnr.readLine();

                        while( (line != null) && !foundInFile)
                        {
                            if (! matchCase) 
                            {
                                line = line.toLowerCase();
                            }
                            if (line.indexOf(toFind) != -1)
                            {
                                // Found a matching string in this line
                                foundInFile = true;
                                current.openRules(MainFrame.getMainFrame());
                                // set correct rule editor

                                JInternalFrame[] jif = MainFrame.getMainFrame().getDesktopPane().getAllFrames();   // Get all open Rule Editors
                                for(int i = 0; i < jif.length; ++i)
                                {
                                    if(jif[i] instanceof RuleEditor)
                                    {
                                        RuleEditor re = (RuleEditor)jif[i];
                                        reFileName = re.getFile();

                                        if( reFileName.equals(fn) )
                                        {
                                            d_ruleEditor = re;
                                            d_ruleEditor.resetCaret();
                                            searchingRuleEditor = true;
                                        }    // found rule editor that matches filename where bfe is currently at
                                    }
                                }  // end of for (going through open RE's looking for correct one)
                            }    // end of If(found the string on current line
                            line = lnr.readLine();  // get next line

                        }   // end of while searching for a match in current file
                        lnr.close();
                    }  // end of try reading a line in a file
                    catch(FileNotFoundException fnfe) 
                    {
                        System.err.println("Couldn't find: " + fn);
                    }
                    catch(IOException ioe) 
                    {
                        System.err.println("Error reading from file " + fn);
                    }
                }   // if fn is a valid file
            }   // end of not searching in a rule editor
            if( searchingRuleEditor)
            {
                // Do rule editor stuff
                d_ruleEditor.setFindReplaceData(toFind,
                                                toReplace,
                                                new Boolean(true),
                                                caseSensitive,
                                                new Boolean(true));
                if( d_ruleEditor.findResult() == true)
                {
                    stringFound = true;
                    stringSelected = true;
                }
                else
                {
                    searchingRuleEditor = false;
                }

            }   // end of searching within a rule editor

        }   // end of while, either found string or no more strings in project

        if( !bfe.hasMoreElements() && !searchingRuleEditor && !outputToFeedbackList) 
        {
            JOptionPane.showMessageDialog(null, "No more instances of " + toFind + " found in project", "End of Search", JOptionPane.INFORMATION_MESSAGE);
        }

    }                // end of findInProject()

}// end of ReplaceInProjectDialog class

