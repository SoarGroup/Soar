package edu.umich.visualsoar.dialogs;

import java.awt.*;
import javax.swing.*;
import javax.swing.border.*;

/**
 * Panel that contains the buttons for different find options for
 * the find dialogs. This includes direction, case matching, and 
 * search wrapping.
 * @author Andrew Nuxoll
 * @see FindInProjectDialog
 * @see FindReplaceInProjectDialog
 */
class FindInProjectOptionsPanel extends JPanel
{
    JCheckBox matchCase = new JCheckBox("Match Case", false);

    public FindInProjectOptionsPanel()
    {
        matchCase.setMnemonic('m');
        setLayout(new FlowLayout(FlowLayout.LEFT));
        add(matchCase);
    }   

    /**
     * @return true if a case specific search is specified
     */     
    public Boolean getMatchCase()
    {
        return new Boolean(matchCase.isSelected());
    }
    
}//class FindInProjectOptionsPanel

