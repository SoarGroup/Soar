package edu.umich.visualsoar.dialogs;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/**
 * Dialog which takes input for the editing of one of four different
 * types of numerical vertices in the data map
 * @author Jon Bauman
 * @see FloatRangeVertex
 * @see FloatVertex
 * @see IntegerRangeVertex
 * @see IntegerVertex
 * @see DataMapTree 
 */
public class EditNumberDialog extends JDialog {

	static int 			LOW = 1;
	static int 			HIGH = 2;
	static int 			INTEGER = 3;
	static int 			FLOAT = 4;
	
	/**
	 * which entry field will recieve focus. Valid values are
	 * EditNumberDialog.LOW and EditNumberDialog.HIGH
	 */
	int 				focusTarget = LOW;
	
	/**
	 * describes what type of vertex this dialog is being used
	 * to edit. Valid values are EditNumberDialog.INTEGER and
	 * EditNumberDialog.FLOAT
	 */
	int 				dialogType = INTEGER;
	
	boolean 			approved = false;
	
	Number 				low = null;
	Number 				high = null;

	/**
	 * Panel which contians the range entry fields
	 */
	RangePanel 			rangePanel;
	
	DefaultButtonPanel 	buttonPanel = new DefaultButtonPanel();
	  	 
	/**
	 * @param owner Frame which owns the dialog
	 * @param type a String which represents what type of 
	 * input is expected. "Integer" or "Float" is expected
	 */
	public EditNumberDialog(final Frame owner, String type) {
		super(owner, "Enter " + type, true);
		rangePanel = new RangePanel(type);
		
		if (type.equals("Integer")) {
			dialogType = INTEGER;
		}	
		else { // float
			dialogType = FLOAT;
		}
		
		setResizable(false);
		Container contentPane = getContentPane();
		GridBagLayout gridbag = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();
		contentPane.setLayout(gridbag);
		
		// specifies component as last one on the row
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		
		contentPane.add(rangePanel, c);
		contentPane.add(buttonPanel, c);
		pack();		
		getRootPane().setDefaultButton(buttonPanel.okButton);			

		addWindowListener(new WindowAdapter() {
			public void windowActivated(WindowEvent we) {
				setLocationRelativeTo(owner);
				owner.repaint();
			}
		});

		buttonPanel.cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				dispose();
				approved = false;
			}
		});
		
		buttonPanel.okButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				low = rangePanel.getLow();
				high = rangePanel.getHigh();
				boolean rangeIsValid = true;
				
				try {
				    if (dialogType == INTEGER) {
						rangePanel.getHigh().intValue();
					}
					else { // Float
						rangePanel.getHigh().floatValue();
					}
				} catch (NullPointerException npe) {
					rangeIsValid = false;
					focusTarget = HIGH;
				}
				try {
				    if (dialogType == INTEGER) {
						rangePanel.getLow().intValue();
					}
					else { // Float
						rangePanel.getLow().floatValue();
					}
				} catch (NullPointerException npe) {
					rangeIsValid = false;
					focusTarget = LOW;
				}

				
				if (! rangeIsValid) {
					JOptionPane.showMessageDialog(EditNumberDialog.this, 
						"Only valid numbers may be entered in the range", 
						"Invalid Range", JOptionPane.ERROR_MESSAGE);
				}
				else if (low.floatValue() > high.floatValue()) {
					JOptionPane.showMessageDialog(EditNumberDialog.this, 
						"The left field must be less than the right field", 
						"Invalid Range", JOptionPane.ERROR_MESSAGE);
					rangeIsValid = false;
					focusTarget = LOW;
				}
				else {
					dispose();
					approved = true;
				}
			}
		});
		
		addWindowListener(new WindowAdapter() {
			public void windowActivated(WindowEvent e) {
				if (focusTarget == LOW) {
					rangePanel.requestFocus(rangePanel.LOW);
				}
				else { // focusTarget == HIGH
					rangePanel.requestFocus(rangePanel.HIGH);
				}
			}
		});		

	}

	public boolean wasApproved() {
		return approved;
	}
	
	public Number getLow() {
		return low;
	}
	
	public Number getHigh() {
		return high;
	}
	
	public void setLow(Number l) {
		rangePanel.setLow(l);
	}
	
	public void setHigh(Number h) {
		rangePanel.setHigh(h);
	}
	
}
