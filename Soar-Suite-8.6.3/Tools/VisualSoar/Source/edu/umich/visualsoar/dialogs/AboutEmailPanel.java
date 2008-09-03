package edu.umich.visualsoar.dialogs;
 
import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;

/**
 * Panel that displays the contact info for the about dialog
 * @author Jon Bauman
 * @see AboutDialog
 */
class AboutEmailPanel extends JPanel {

	JLabel emailLabel = 
				new JLabel("If you have any questions or comments,");	 
	JLabel emailLabel2 = 
				new JLabel("contact us at visualSoar@eecs.umich.edu");

	public AboutEmailPanel() {
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		setBorder(new EmptyBorder(10,10,10,10));			
		add(emailLabel);
		add(emailLabel2);
	}
}
