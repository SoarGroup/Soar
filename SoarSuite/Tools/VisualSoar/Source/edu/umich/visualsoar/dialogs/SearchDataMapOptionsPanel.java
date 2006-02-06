package edu.umich.visualsoar.dialogs;

import java.awt.*;
import javax.swing.*;
import javax.swing.border.*;

/**
 * Panel that contains the buttons for different find options for
 * the SearchDataMapDialog.  Allows user to decide to ignore certain
 * types of wme's within the datamap
 * @author Brian Harleton
 * @see SearchDataMapDialog
 */
class SearchDataMapOptionsPanel extends JPanel {

	JCheckBox 		identifierCase = new JCheckBox("Identifiers", true);
	JCheckBox 		enumerationCase = new JCheckBox("Enumerations", true);
	JCheckBox 		stringCase = new JCheckBox("Strings", true);
	JCheckBox 		integerCase = new JCheckBox("Integers", true);
	JCheckBox 		floatCase = new JCheckBox("Floats", true);

	public SearchDataMapOptionsPanel() {
	
		setLayout(new FlowLayout(FlowLayout.LEFT));
		add(identifierCase);
		add(enumerationCase);
		add(stringCase);
		add(integerCase);
		add(floatCase);

    setBorder(new CompoundBorder(
      BorderFactory.createTitledBorder("Search Includes"),
      BorderFactory.createEmptyBorder(10,10,10,10)));
	}
	
	/**
	 * @return true if a case identifier search is specified
	 */
	public Boolean getidentifierCase() {
		return new Boolean(identifierCase.isSelected());
	}

	/**
	 * @return true if a case enumeration search is specified
	 */
	public Boolean getEnumerationCase() {
		return new Boolean(enumerationCase.isSelected());
	}

	/**
	 * @return true if a case string search is specified
	 */
	public Boolean getStringCase() {
		return new Boolean(stringCase.isSelected());
	}

	/**
	 * @return true if a case integer search is specified
	 */
	public Boolean getIntegerCase() {
		return new Boolean(integerCase.isSelected());
	}

	/**
	 * @return true if a case float search is specified
	 */
	public Boolean getFloatCase() {
		return new Boolean(floatCase.isSelected());
	}

	/**
	 * gets all the data input into the panel by the user
	 * @return an array of objects representing the data
	 */
	public Boolean[] getData() {
		Boolean[] optionsData = new Boolean[5];

    optionsData[0] = new Boolean(identifierCase.isSelected());
    optionsData[1] = new Boolean(enumerationCase.isSelected());
    optionsData[2] = new Boolean(stringCase.isSelected());
    optionsData[3] = new Boolean(integerCase.isSelected());
    optionsData[4] = new Boolean(floatCase.isSelected());
		
		return optionsData;
	}


	
}