package edu.umich.visualsoar.dialogs;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;

/**
 * Panel that facilitates the entry of a list of strings, 
 * or enumeration, for the enumeration dialogs. 
 *
 * @author Jon Bauman
 * @see EnumerationDialog
 * @see EditEnumerationDialog
 */
class EnumPanel extends JPanel {

	JTextField		newString = new JTextField(20);
	Vector 			theStrings = new Vector();
	JList 			theList = new JList(theStrings);
	JScrollPane		sp = new JScrollPane(theList);
	
	public EnumPanel() {
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		add(newString);
		add(Box.createVerticalStrut(15));
		add(sp);

		setBorder(new CompoundBorder(
			BorderFactory.createTitledBorder("Enumeration"),
			BorderFactory.createEmptyBorder(10,10,10,10)));
								
		// So that enter can affirmatively dismiss the dialog
		newString.getKeymap().removeKeyStrokeBinding(
			KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0));	
	}
	
	/**
	 * removes all the strings
	 */
	public void clear() { 
		theStrings.removeAllElements();
		theList.setListData(theStrings);
		clearText();
	}
	
	/**
	 * clears the string entry field
	 */
	public void clearText() {
		newString.setText(""); 
	}
	
	/**
	 * @return vector of strings entered
	 */
	public Vector getVector() { 
		return (Vector)theStrings.clone();
	}
	
	/**
	 * @param v vector to set the list data to
	 */ 
	public void setVector(Vector v) {
		theStrings = v;
		theList.setListData(theStrings);
		clearText();
	}
	
	boolean addString() {
		String s = newString.getText().trim();
		if (s.length() == 0) {
			JOptionPane.showMessageDialog(this,
				"Zero length strings are not allowed in enumerations",
				"Invalid Enumeration Data", JOptionPane.ERROR_MESSAGE);
			return false;
		}
		if (s.indexOf(' ') != -1) {
			JOptionPane.showMessageDialog(this,
				"Spaces are not allowed in enumeration values",
				"Invalid Enumeration Data", JOptionPane.ERROR_MESSAGE);
			return false;
		}
		theStrings.add(s);
		theList.setListData(theStrings);
		return true;
	}
	
	void removeString() {
		Object val = theList.getSelectedValue();
		theStrings.remove(val);
		theList.setListData(theStrings);
	}
	
	public void requestFocus() { 
		newString.selectAll();
		newString.requestFocus(); 
	}

}
