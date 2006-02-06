package edu.umich.visualsoar.dialogs;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

/**
 * Panel that contains the buttons for the Save As Dialog
 *
 * @author Brian Harleton
 * @see SaveProjectAsDialog
 */
class SaveAsButtonPanel extends JPanel {

	JButton cancelButton = new JButton("Cancel");
	JButton newButton = new JButton("Save As");

	public SaveAsButtonPanel() {
		newButton.setMnemonic('n');
		cancelButton.setMnemonic('c');
	
		setLayout(new FlowLayout());
		add(newButton);
		add(cancelButton);
	}
}