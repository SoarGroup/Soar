package edu.umich.visualsoar.dialogs;

import edu.umich.visualsoar.operatorwindow.OperatorWindow;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/**
 * Dialog which takes input for the creation of a new VisualSoar agent
 * @author Jon Bauman
 */
public class NewAgentDialog extends JDialog {

	String 				newAgentName = null;
	String 				newAgentPath = null;
	
	boolean 			approved = false;

	/**
	 * panel which contains the name input field
	 */
	AgentNamePanel 		namePanel = new AgentNamePanel();
	
	/**
	 * panel which contians the path input components
	 */ 
	AgentPathPanel 		pathPanel = new AgentPathPanel();
	 
	AgentButtonPanel 	buttonPanel = new AgentButtonPanel();
	
	/**
	 * @param owner Frame which owns the dialog
	 */
	
	public NewAgentDialog(final Frame owner) {
		super(owner, "New Agent", true);
		
		setResizable(false);
		Container contentPane = getContentPane();
		GridBagLayout gridbag = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();
		contentPane.setLayout(gridbag);
		
		// specifies component as last one on the row
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		
		contentPane.add(namePanel, c);
		contentPane.add(pathPanel, c);
		contentPane.add(buttonPanel, c);
		pack();
		getRootPane().setDefaultButton(buttonPanel.newButton);			

		addWindowListener(new WindowAdapter() {
			public void windowActivated(WindowEvent we) {
				setLocationRelativeTo(owner);
				namePanel.requestFocus();
				owner.repaint();
			}
		});

		buttonPanel.cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				dispose();
				approved = false;
			}
		});
		
		buttonPanel.newButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				String nameText = namePanel.getName();
				if (nameText.length() == 0) {
					JOptionPane.showMessageDialog(NewAgentDialog.this, 
						"Project names cannot have length zero", 
						"Invalid Name", JOptionPane.ERROR_MESSAGE);
				}
				else if (! OperatorWindow.isProjectNameValid(nameText)) {
					JOptionPane.showMessageDialog(NewAgentDialog.this, 
						"Project names may only contain letter, numbers, hyphens and underscores", 
						"Invalid Name", JOptionPane.ERROR_MESSAGE);
				}
				else {
					newAgentName = namePanel.getName();
					newAgentPath = pathPanel.getPath();
					approved = true;
					dispose();
				}
			}
		});
	}

	public String getNewAgentName() {
		return newAgentName;
	}

	public String getNewAgentPath() {
		return newAgentPath;
	}

	public boolean wasApproved() {
		return approved;
	}
	
}
