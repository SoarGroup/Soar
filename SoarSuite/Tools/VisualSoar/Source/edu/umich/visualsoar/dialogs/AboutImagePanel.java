package edu.umich.visualsoar.dialogs;
 
import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;

/**
 * Now simply includes text information about the authors
 * OLD - Panel that displays the AI logo for the about dialog
 * @author Jon Bauman
 * @see AboutDialog
 */
class AboutImagePanel extends JPanel {

  JLabel authorLabel =
				new JLabel("Visual Soar created by:");
  JLabel authorLabel1 =
				new JLabel("    Jon Bauman");
  JLabel authorLabel2 =
				new JLabel("    Brad Jones");
  JLabel authorLabel3 =
				new JLabel(" ");
  JLabel authorLabel4 =
				new JLabel("Further Visual Soar improvements by:");
  JLabel authorLabel5 =
				new JLabel("    Brian Harleton");
  JLabel authorLabel6 =
				new JLabel("    Andrew Nuxoll");
  JLabel authorLabel7 =
				new JLabel("    Douglas Pearson");

	public AboutImagePanel() {
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

		setBorder(new EmptyBorder(10,10,10,10));
		add(authorLabel);
        add(authorLabel1);
		add(authorLabel2);
		add(authorLabel3);
		add(authorLabel4);
        add(authorLabel5);
        add(authorLabel6);
	add(authorLabel7);
	}
}
