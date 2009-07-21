package edu.umich.visualsoar.dialogs;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;

/**
 * Panel that contains the input field for the find string and the
 * option panel for the find dialogs
 * @author Andrew Nuxoll
 * @see FindInProjectDialog
 * @see FindReplaceInProjectDialog
 */
class FindInProjectPanel extends JPanel
{
    JTextField          findField = new JTextField(20);
    FindInProjectOptionsPanel   optionsPanel;

    public FindInProjectPanel()
    {
        optionsPanel = new FindInProjectOptionsPanel();
        
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        add(findField);
        add(optionsPanel);

        setBorder(new CompoundBorder(
            BorderFactory.createTitledBorder("Find"),
            BorderFactory.createEmptyBorder(10,10,10,10)));
            
        // So that enter can affirmatively dismiss the dialog   
        findField.getKeymap().removeKeyStrokeBinding(
            KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0));
    }

    /**
     * gets all the data input into the panel by the user
     * @return an array of objects representing the data
     */
    public Object[] getData() 
    {
        Object[] findData = new Object[2];
        
        findData[0] = findField.getText();
        findData[1] = optionsPanel.getMatchCase();
        
        return findData;
    }
    
    public void requestFocus() 
    {
        findField.requestFocus(); 
    }
}
