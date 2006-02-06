package edu.umich.visualsoar.dialogs;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;

/**
 * Panel that contains the input field for the find string and the
 * option panel for the find dialogs
 * @author Jon Bauman
 * @see FindDialog
 * @see FindReplaceDialog
 */
class FindPanel extends JPanel {

	JTextField 			findField = new JTextField(20);
	FindOptionsPanel 	optionsPanel;

	public FindPanel() {
		optionsPanel = new FindOptionsPanel();
	
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
	public Object[] getData() {
		Object[] findData = new Object[4];
		
		findData[0] = findField.getText();
		findData[1] = optionsPanel.getDirection();
		findData[2] = optionsPanel.getMatchCase();
		findData[3] = optionsPanel.getWrap();
		
		return findData;
	}
	
	public void requestFocus() {
		findField.requestFocus(); 
	}
}
