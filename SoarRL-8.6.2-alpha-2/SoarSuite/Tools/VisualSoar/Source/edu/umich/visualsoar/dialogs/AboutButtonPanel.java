package edu.umich.visualsoar.dialogs;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

/**
 * Panel that contains the button for the about dialog
 * @author Jon Bauman
 * @see AboutDialog
 */
class AboutButtonPanel extends JPanel {

	JButton greatButton = new JButton("Great");
	 
	public AboutButtonPanel() {
		greatButton.setMnemonic('g');
	
		setLayout(new FlowLayout());
		add(greatButton);
	}
}
