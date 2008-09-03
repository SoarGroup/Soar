package edu.umich.visualsoar.dialogs;
import edu.umich.visualsoar.ruleeditor.RuleEditor;
 
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import edu.umich.visualsoar.ruleeditor.RuleEditor;
import edu.umich.visualsoar.operatorwindow.OperatorWindow;
import edu.umich.visualsoar.operatorwindow.OperatorNode;


/**
 * Dialog which takes input for, and initiates a find operation over a set of
 * project files
 * @author Andrew Nuxoll
 * @see FindDialog
 */
public class FindInProjectDialog extends JDialog
{
    FindInProjectPanel  findPanel;
    FindButtonPanel     buttonPanel;
    OperatorWindow      opWin = null;
    OperatorNode        root;
        
    /**
     * The 'find in project' version
     * @param owner Frame which owns the dialog
     * @param operators a reference to the OperatorWindow
     * @param opNode operator tree to search (a null value indicates the entire
     *               project should be searched)
     */
    public FindInProjectDialog(final Frame owner,
                               OperatorWindow operators,
                               OperatorNode opNode)
    {
        super(owner, "Find", false);
        
        findPanel = new FindInProjectPanel();
        buttonPanel = new FindButtonPanel(true);
        opWin = operators;
        root = opNode;
        setResizable(false);
        Container contentPane = getContentPane();
        GridBagLayout gridbag = new GridBagLayout();
        GridBagConstraints c = new GridBagConstraints();
        contentPane.setLayout(gridbag);
        
        // specifies component as last one on the row
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.fill = GridBagConstraints.HORIZONTAL;
                  
        contentPane.add(findPanel, c);
        contentPane.add(buttonPanel, c);
        pack();
        getRootPane().setDefaultButton(buttonPanel.findButton);
        
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
        
        buttonPanel.findButton.addActionListener(
            new ActionListener() 
            {
                public void actionPerformed(ActionEvent e) 
                {
                    Object[] theData = findPanel.getData();
                    String toFind = (String)theData[0];
                    Boolean caseSensitive = (Boolean)theData[1];
                    
                    opWin.findInProject(root,
                                        toFind, 
                                        caseSensitive.booleanValue());
                    dispose();
                }
            });
    }//ctor   
    
}//class FindInProjectDialog
