package edu.umich.visualsoar.dialogs;
import edu.umich.visualsoar.datamap.DataMapUtils; 
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.beans.*;

/**
 * Dialog which takes input for the creation of a comment field on an edge
 * in the datamap.
 * @author Brian Harleton
 * @see DataMapTree
 * @see NamedEdge
 */
public class CommentDialog extends JDialog {

	boolean 			approved = false;

	String 				commentText = null;
	
	/**
	 * panel which contains the comment input field
	 */
	NamePanel 			namePanel = new NamePanel("Comment");

	DefaultButtonPanel 	buttonPanel = new DefaultButtonPanel();
	
	/**
	 * @param owner Frame which owns the dialog
	 */
	public CommentDialog(final Frame owner, String oldComment) {
		super(owner, "Add/Edit Comment", true);

    namePanel.setText(oldComment);

		setResizable(false);
		Container contentPane = getContentPane();
		GridBagLayout gridbag = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();
		contentPane.setLayout(gridbag);
		
		// specifies component as last one on the row
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		          
		contentPane.add(namePanel, c);
		contentPane.add(buttonPanel, c);
		pack();
		getRootPane().setDefaultButton(buttonPanel.okButton);			
				
		addWindowListener(new WindowAdapter() {
			public void windowOpened(WindowEvent we) {
				setLocationRelativeTo(owner);
				owner.repaint();
			}
		});						
				
		buttonPanel.cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				approved = false;
				dispose();
			}
		});
				
		buttonPanel.okButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				commentText = namePanel.getText();
        approved = true;
					dispose();
			}
		});
		
		addWindowListener(new WindowAdapter() {
			public void windowActivated(WindowEvent e) {
				namePanel.requestFocus();
			}
		});
				
	}	
	
	public boolean wasApproved() {
		return approved;
	}

	public String getText() {
		return commentText;
	}
	
	/**
	 * @param s String to set the name entry field to 
	 * before making the dialog visible
	 */
	public void makeVisible(String s) {
		namePanel.setText(s);
		setVisible(true);
	}

}