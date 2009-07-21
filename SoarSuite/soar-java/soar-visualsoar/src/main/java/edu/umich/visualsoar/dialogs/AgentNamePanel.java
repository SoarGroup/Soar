package edu.umich.visualsoar.dialogs;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;

/**
 * Panel that contians the input field for the name of the agent
 * in the new agent dialog
 * @author Jon Bauman
 * @see NewAgentDialog
 */
class AgentNamePanel extends JPanel {

	JTextField nameField = new JTextField("untitled-agent", 20);

	public AgentNamePanel() {
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		add(nameField);

		setBorder(new CompoundBorder(
			BorderFactory.createTitledBorder("Agent Name"),
			BorderFactory.createEmptyBorder(10,10,10,10)));
			
		nameField.select(0, 14);
		
		// So that enter can affirmatively dismiss the dialog
		nameField.getKeymap().removeKeyStrokeBinding(
			KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0));	
	}
	
	/**
	 * @return the inputted name for the new agent
	 */
	public String getName() { 
		return nameField.getText(); 
	}
	
	public void requestFocus() {
		nameField.requestFocus(); 
	}
}
