package edu.umich.visualsoar.dialogs;
 
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

/**
 * Panel that contains the buttons for the average dialog
 * @author Jon Bauman
 */
class DefaultButtonPanel extends JPanel {

	JButton cancelButton = new JButton("Cancel");
	JButton okButton = new JButton("OK");

	public DefaultButtonPanel() {
		okButton.setMnemonic('o');
		cancelButton.setMnemonic('c');
	
		setLayout(new FlowLayout());
		add(okButton);
		add(cancelButton);
	}
}
