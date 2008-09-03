package edu.umich.visualsoar.dialogs;

import edu.umich.visualsoar.operatorwindow.OperatorWindow;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/**
 * Dialog which takes input for saving an agent as a new agent
 * @author Brian Harleton
 */

public class SaveProjectAsDialog extends JDialog {
  String  newAgentName = null;
  String  newAgentPath = null;

  boolean approved = false;

  /**
   *  Name of Agent Input field
   */
  AgentNamePanel   namePanel = new AgentNamePanel();

  /**
   *  Path of where the new agent is to be located
   */
  AgentPathPanel  pathPanel = new AgentPathPanel();

  SaveAsButtonPanel buttonPanel = new SaveAsButtonPanel();



  public SaveProjectAsDialog(final Frame owner) {
    super(owner, "Save Project As . . .", true);

    setResizable(false);
    Container contentPane = getContentPane();
    GridBagLayout gridbag = new GridBagLayout();
    GridBagConstraints c = new GridBagConstraints();
    contentPane.setLayout(gridbag);

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
					JOptionPane.showMessageDialog(SaveProjectAsDialog.this,
						"Project names cannot have length zero", 
						"Invalid Name", JOptionPane.ERROR_MESSAGE);
				}
				else if (! OperatorWindow.isProjectNameValid(nameText)) {
					JOptionPane.showMessageDialog(SaveProjectAsDialog.this, 
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
  }   // end of SaveProjectAsDialog constructor

  public String getNewAgentName() {
    return newAgentName;
  }

  public String getNewAgentPath() {
    return newAgentPath;
  }

  public boolean wasApproved() {
    return approved;
  }
}   // end of SaveProjectAsDialog class1
