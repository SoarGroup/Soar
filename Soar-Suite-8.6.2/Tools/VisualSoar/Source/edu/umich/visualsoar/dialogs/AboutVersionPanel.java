package edu.umich.visualsoar.dialogs;
 
import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;

/**
 * Panel that displays the Version number of Visual Soar for the about dialog
 * @author Brian Harleton
 * @see AboutDialog
 */
class AboutVersionPanel extends JPanel {

	JLabel versionLabel =
				new JLabel("Visual Soar");
	JLabel versionLabel2 =
				new JLabel("    Version 4.6.1");

	public AboutVersionPanel() {
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		setBorder(new EmptyBorder(10,10,10,10));			
		add(versionLabel);
		add(versionLabel2);
	}
}
