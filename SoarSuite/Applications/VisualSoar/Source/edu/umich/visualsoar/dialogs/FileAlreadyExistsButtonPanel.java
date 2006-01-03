package edu.umich.visualsoar.dialogs;
 
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

/**
 * Panel that contains the buttons for the FileAlreadyExistsDialog
 * @author Brian Harleton
 * @see FileAlreadyExistsDialog
 */
class FileAlreadyExistsButtonPanel extends JPanel {

	JButton cancelButton = new JButton("Cancel");
	JButton useButton = new JButton("Use");
	JButton replaceButton = new JButton("Replace");

	public FileAlreadyExistsButtonPanel() {
		useButton.setMnemonic('u');
		cancelButton.setMnemonic('c');
		replaceButton.setMnemonic('r');
	
		setLayout(new FlowLayout());
		add(useButton);
		add(replaceButton);
		add(cancelButton);
	}
}