package edu.umich.visualsoar.dialogs;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;

/**
 * Panel containing input fields for entering a numeric range for
 * the number panels.
 * @author Jon Bauman
 * @see NumberDialog
 * @see EditNumberDialog
 */
class RangePanel extends JPanel {

	static int INTEGER = 0;
	static int FLOAT = 1;
	static int LOW = 2;
	static int HIGH = 3;
	
	int 		type;
	JLabel 		infoLabel = 
					new JLabel("blank field indicates (+/-) infinity");
	JTextField 	lowField = new JTextField(5);
	JTextField 	highField = new JTextField(5);
	JLabel 		to = new JLabel("to");

	public RangePanel(String s) {
	
		if (s.equals("Integer")) {
			type = INTEGER;
		}
		else {
			type = FLOAT;
		}
		
		GridBagLayout gridbag = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();
		setLayout(gridbag);
		
		// specifies component as last one on the row
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1.0;

		add(lowField, c);
		
		c.fill = GridBagConstraints.NONE;
		add(to, c);

		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridwidth = GridBagConstraints.REMAINDER;
		add(highField, c);
		
		c.anchor = GridBagConstraints.WEST;
		c.ipady = 10;
		add(infoLabel, c);

		setBorder(new CompoundBorder(
			BorderFactory.createTitledBorder("Range"),
			BorderFactory.createEmptyBorder(10,10,10,10)));
								
		// So that enter can affirmatively dismiss the dialog
		lowField.getKeymap().removeKeyStrokeBinding(
			KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0));	
		highField.getKeymap().removeKeyStrokeBinding(
			KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0));	
	}
	
	/**
	 * @return the number input in the low field
	 */	
	public Number getLow() {
		String lowText = lowField.getText().trim();
		Number lowVal;
		
		if (type == INTEGER) {
			if (lowText.length() == 0) {
				return (new Integer(Integer.MIN_VALUE));
			}
			else {
				try {
					lowVal = new Integer(lowText);
					return lowVal;
				} catch (NumberFormatException nfe) {
					return null;
				}
			}
		}
		else { // type == FLOAT
			if (lowText.length() == 0) {
				return (new Float(Float.NEGATIVE_INFINITY));
			}
			else {
				try {
					lowVal = new Float(lowText);
					return lowVal;
				} catch (NumberFormatException nfe) {
					return null;
				}
			}
		}
	}
	
	/**
	 * @return the number input in the high field
	 */	
	public Number getHigh() {
		String highText = highField.getText().trim();
		Number highVal;
		
		if (type == INTEGER) {
			if (highText.length() == 0) {
				return (new Integer(Integer.MAX_VALUE));
			}
			else {
				try {
					highVal = new Integer(highText);
					return highVal;
				} catch (NumberFormatException nfe) {
					return null;
				}
			}
		}
		else { // type == FLOAT
			if (highText.length() == 0) {
				return (new Float(Float.POSITIVE_INFINITY));
			}
			else {
				try {
					highVal = new Float(highText);
					return highVal;
				} catch (NumberFormatException nfe) {
					return null;
				}
			}
		}
	}

	/**
	 * @param l the number to set the low field to
	 */
	public void setLow(Number l) { 
		if(l.floatValue() == Float.NEGATIVE_INFINITY)
			lowField.setText("");			
		else
			lowField.setText(l.toString());
	}
	
	/**
	 * @param h the number to set the high field to
	 */
	public void setHigh(Number h) {
		if(h.floatValue() == Float.POSITIVE_INFINITY)
			highField.setText("");			
		else
			highField.setText(h.toString());

	}
	
	/**
	 * @param field the field to give the focus to.
	 * Either RangePanel.LOW or RangePanel.HIGH are accepted
	 */	
	public void requestFocus(int field) {
		if (field == LOW) { 
			lowField.selectAll();
			lowField.requestFocus();
		} 
		else { // field == HIGH
			highField.selectAll();
			highField.requestFocus();
		}
	}
}
