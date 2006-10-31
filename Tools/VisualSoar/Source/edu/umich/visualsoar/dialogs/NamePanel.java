package edu.umich.visualsoar.dialogs;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;

/**
 * Panel containing an input field for a name. For use in various
 * dialogs
 * @author Jon Bauman
 */
class NamePanel extends JPanel {

	JTextField nameField = new JTextField(20);
	
	/**
	 * Creates a titled border around the input field
	 * @param theType the name to give to the titled border
	 */
	public NamePanel(String theType) {
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		add(nameField);
		setBorder(new CompoundBorder(
			BorderFactory.createTitledBorder(theType),
			BorderFactory.createEmptyBorder(10,10,10,10)));
						
		// So that enter can affirmatively dismiss the dialog	
		nameField.getKeymap().removeKeyStrokeBinding(
			KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0));
	}
	
	/**
	 * @return the text inputted by the user
	 */
	public String getText() { 
		return nameField.getText();
	}
	
	/**
	 * @param s the text to set the input field to
	 */
	public void setText(String s) {
		nameField.setText(s);
		nameField.selectAll();
	}
	
	public void requestFocus() { 
		nameField.selectAll();
		nameField.requestFocus();
	}
}
