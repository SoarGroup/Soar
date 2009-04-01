package edu.umich.visualsoar.dialogs;
import edu.umich.visualsoar.datamap.DataMapUtils;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;

/**
 * Dialog which comes up when a requested 'add file' command already
 * has a file that name at that location.  This dialog gives the user
 * the option of replacing or using that file.
 * in the data map.
 * @author Brian Harleton
 * @see OperatorWindow
 * @see OperatorNode
 */
public class FileAlreadyExistsDialog extends JDialog {


  private FileAlreadyExistsButtonPanel 	buttonPanel = new FileAlreadyExistsButtonPanel();
  private JLabel label1;
  private JLabel label2;
  private int approved = 0;


	public FileAlreadyExistsDialog(final Frame owner, String file) {
		super(owner, "File already exists", true);


    setResizable(false);
		Container contentPane = getContentPane();
		GridBagLayout gridbag = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();
		contentPane.setLayout(gridbag);

    label1 = new JLabel("A file named \"" + file + "\" already exists at this location." );
    label2 = new JLabel("Would you like to use this file or replace it with a new file?");
		// specifies component as last one on the row
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;

		contentPane.add(label1, c);
    contentPane.add(label2, c);
		contentPane.add(buttonPanel, c);
		pack();



		addWindowListener(new WindowAdapter() {
			public void windowOpened(WindowEvent we) {
				setLocationRelativeTo(owner);
				owner.repaint();
			}
		});		

		buttonPanel.cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				approved = 0;
				dispose();
			}
		});

    buttonPanel.useButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        approved = 1;
        dispose();
      }
    });

    buttonPanel.replaceButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        approved = 2;
        dispose();
      }
    });

  }

  public int wasApproved() {
		return approved;
	}




}