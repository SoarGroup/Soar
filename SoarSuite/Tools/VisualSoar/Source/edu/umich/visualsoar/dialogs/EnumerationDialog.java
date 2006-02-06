package edu.umich.visualsoar.dialogs;
import edu.umich.visualsoar.datamap.DataMapUtils;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/**
 * Dialog which takes input for the creation of an EnumerationVertex 
 * in the data map.
 * @author Jon Bauman
 * @see EnumerationVertex
 * @see DataMapTree
 */
public class EnumerationDialog extends JDialog 
{

    static int NAME = 0;
    static int ENUMERATION = 1;
    boolean    approved = false;
    Vector     theStrings = null;
    String     nameText = null;
    Action     enterAction = new EnterAction();
     
    /**
     * which entry field will recieve focus. Valid values are
     * EnumerationDialog.NAME and EnumerationDialog.ENUMERATION
     */
    int                 focusTarget = NAME;
     
    /**
     * panel which contains the name imput field
     */
    NamePanel           namePanel = new NamePanel("Attribute Name");

    /**
     * panel which facilitates entry of enumerations
     */
    EnumPanel           enumPanel = new EnumPanel();
    
    EnumButtonPanel     buttonPanel = new EnumButtonPanel();
     
    /**
     * @param owner Frame which owns the dialog
     */
    public EnumerationDialog(final Frame owner) 
    {
        super(owner, "Enter Enumeration", true);
        
        setResizable(false);
        Container contentPane = getContentPane();
        GridBagLayout gridbag = new GridBagLayout();
        GridBagConstraints c = new GridBagConstraints();
        contentPane.setLayout(gridbag);
        
        // specifies component as last one on the row
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.fill = GridBagConstraints.HORIZONTAL;
        
        contentPane.add(namePanel, c);
        contentPane.add(enumPanel, c);
        contentPane.add(buttonPanel, c);
        pack();
        
        KeyStroke enter = KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0);

        namePanel.nameField.getKeymap().addActionForKeyStroke(enter,enterAction); 
          
        
        addWindowListener(new WindowAdapter() 
                          {
                              public void windowOpened(WindowEvent we) 
                                  {
                                      setLocationRelativeTo(owner);
                                      owner.repaint();
                                  }
                          });       

        buttonPanel.cancelButton.addActionListener(
            new ActionListener() 
            {
                public void actionPerformed(ActionEvent e) 
                    {
                        approved = false;
                        dispose();
                    }
            });
        
        buttonPanel.addButton.addActionListener(
            new ActionListener() 
            {
                public void actionPerformed(ActionEvent e) 
                    {
                        if (enumPanel.addString()) 
                        {
                            enumPanel.clearText();
                        }
                        focusTarget = ENUMERATION;
                    }
            });
        
        buttonPanel.removeButton.addActionListener(
            new ActionListener() 
            {
                public void actionPerformed(ActionEvent e) 
                    {
                        enumPanel.removeString();
                    }
            });
        
        buttonPanel.okButton.addActionListener(
            new ActionListener() 
            {
                public void actionPerformed(ActionEvent e) 
                    {
                        nameText = namePanel.getText().trim();
                        theStrings = enumPanel.getVector();
                
                        if (nameText.length() == 0) 
                        {
                            JOptionPane.showMessageDialog(EnumerationDialog.this, 
                                                          "Attribute names cannot have length zero", 
                                                          "Invalid Name", JOptionPane.ERROR_MESSAGE);
                            focusTarget = NAME;
                        }
                        else if (! DataMapUtils.attributeNameIsValid(nameText))
                        {
                            JOptionPane.showMessageDialog(EnumerationDialog.this, 
                                                          "Attribute names may only contain letter, numbers, and hyphens", 
                                                          "Invalid Name", JOptionPane.ERROR_MESSAGE);
                            focusTarget = NAME;
                        }
                        else if (theStrings.isEmpty())
                        {
                            JOptionPane.showMessageDialog(EnumerationDialog.this, 
                                                          "Enumeration may not have zero elements", 
                                                          "Invalid Enumeration", JOptionPane.ERROR_MESSAGE);
                            focusTarget = ENUMERATION;
                        }
                        else { // valid entry
                            approved = true;
                            dispose();
                        }
                    }
            });

        addWindowListener(new WindowAdapter() 
                          {
                              public void windowActivated(WindowEvent e) 
                                  {
                                      if (focusTarget == NAME) 
                                      {
                                          namePanel.requestFocus();
                                      }
                                      else { // focusTarget == ENUMERATION
                                          enumPanel.requestFocus();
                                      }
                                  }
                          });
        
    }

    public Vector getVector() 
    {
        return theStrings;
    }
    
    public String getText() 
    {
        return nameText;
    }
    
    public boolean wasApproved() 
    {
        return approved;
    }
    
    class EnterAction extends AbstractAction 
    {
        public EnterAction() 
        {
            super("Enter Action");
        }
        
        public void actionPerformed(ActionEvent e)  
        {
            Object o = e.getSource();
            if(o == namePanel.nameField) 
            {
                enumPanel.newString.requestFocus();
            }
            else if(o == enumPanel.newString) 
            {
                if(enumPanel.newString.getText().length() == 0) 
                {
                    buttonPanel.okButton.doClick();
                }
                else 
                {
                    buttonPanel.addButton.doClick();
                }
            }
        }
    }//class EnterAction
    
}//class EnumerationDialog

