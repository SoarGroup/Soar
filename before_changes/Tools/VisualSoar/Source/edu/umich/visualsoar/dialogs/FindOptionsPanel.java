package edu.umich.visualsoar.dialogs;

import java.awt.*;
import javax.swing.*;
import javax.swing.border.*;

/**
 * Panel that contains the buttons for different find options for
 * the find dialogs. This includes direction, case matching, and 
 * search wrapping.
 * @author Jon Bauman
 * @see FindDialog
 * @see FindReplaceDialog
 */
class FindOptionsPanel extends JPanel {

	JLabel 			upDownLabel = new JLabel("Find Direction");
	JRadioButton 	upButton = new JRadioButton("Up", false); 
	JRadioButton 	downButton = new JRadioButton("Down", true); 
	ButtonGroup 	upDownGroup = new ButtonGroup();
	JCheckBox 		matchCase = new JCheckBox("Match Case", false);
	JCheckBox 		wrap = new JCheckBox("Wrap",false);

	public FindOptionsPanel() {
		upButton.setMnemonic('u');
		downButton.setMnemonic('d');
		matchCase.setMnemonic('m');
		wrap.setMnemonic('w');
		upDownGroup.add(upButton);
		upDownGroup.add(downButton);
	
		setLayout(new FlowLayout(FlowLayout.LEFT));
		add(upDownLabel);
		add(upButton);
		add(downButton);
		add(matchCase);	
		add(wrap);
	}
	
	/**
	 * @return true if a downward search is specified
	 */
	public Boolean getDirection() { 
		return new Boolean(downButton.isSelected());
	}
	
	/**
	 * @return true if a case specific search is specified
	 */		
	public Boolean getMatchCase() {
		return new Boolean(matchCase.isSelected());
	}
	
	/**
	 * @return true if a wrapped search is specified
	 */		
	public Boolean getWrap() {
		return new Boolean(wrap.isSelected());
	}
	
}
