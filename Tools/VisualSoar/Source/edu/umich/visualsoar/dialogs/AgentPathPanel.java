package edu.umich.visualsoar.dialogs;

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;
import edu.umich.visualsoar.misc.Preferences;

/**
 * Panel that contains the input field for the path of the agent
 * in the new agent dialog
 * @author Jon Bauman
 * @see NewAgentDialog
 */
class AgentPathPanel extends JPanel {

	String 						workingDir = Preferences.getInstance().getOpenFolder().getPath();
	JTextField 					pathField = new JTextField(workingDir, 20);
	JButton 					browse = new JButton("Browse...");

	public AgentPathPanel() {
		browse.setMnemonic('b');
	
		setLayout(new FlowLayout());
		add(pathField);
		add(browse);

		setBorder(new CompoundBorder(
			BorderFactory.createTitledBorder("Agent Path"),
			BorderFactory.createEmptyBorder(10,10,10,10)));
			
		// So that enter can affirmatively dismiss the dialog
		// pathField.getKeymap().removeKeyStrokeBinding(
		// KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0));
								
		browse.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				DirectorySelectionDialog dsd = new DirectorySelectionDialog(edu.umich.visualsoar.MainFrame.getMainFrame()); 
				dsd.setPath(new File(workingDir));
				dsd.setVisible(true);
				if (dsd.wasApproved()) {
					File file = dsd.getSelectedDirectory();
					pathField.setText(file.getPath());
				}
			}
		});
	}
	
	/**
	 * @return the inputted path for the new agent
	 */
	public String getPath() {
		return pathField.getText();
	}
}
