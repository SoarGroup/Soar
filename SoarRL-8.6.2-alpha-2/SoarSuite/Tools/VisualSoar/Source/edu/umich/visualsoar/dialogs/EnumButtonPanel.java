package edu.umich.visualsoar.dialogs;
 
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

/**
 * Panel that contians the buttons for the enumeration dialogs
 * @author Jon Bauman
 * @see EnumerationDialog
 * @see EditEnumerationDialog
 */
class EnumButtonPanel extends JPanel {

	JButton cancelButton = new JButton("Cancel");
	JButton okButton = new JButton("OK");
	JButton addButton = new JButton("Add");
	JButton removeButton = new JButton("Remove");

	public EnumButtonPanel() {
		okButton.setMnemonic('o');
		cancelButton.setMnemonic('c');
		addButton.setMnemonic('a');
		removeButton.setMnemonic('r');
	
		setLayout(new FlowLayout());
		add(okButton);
		add(addButton);
		add(removeButton);
		add(cancelButton);
	}
}
