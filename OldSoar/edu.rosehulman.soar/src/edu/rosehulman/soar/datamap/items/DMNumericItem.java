/*
 * Created on Dec 25, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items;

/**
 *
 * Abstract class representing a number on the datamap, 
 * to be subclassed by DMInteger and DMFloat.
 * 
 * @author Tim Jasko
 * @see DMInteger
 * @see DMFloat 
 * 
 */
public abstract class DMNumericItem extends DMItem {
	Number _lowerBound, _upperBound;
	
	/**
	 * Default constructor.
	 *
	 */
	public DMNumericItem() {
		super();
	}
	
	/**
	 * Creates a numeric item with the given name.
	 * 
	 * @param name The item's name.
	 */
	public DMNumericItem(String name) {
		super(name);
	}
	
	/**
	 * Displays the information contained in this item in a way suitable 
	 * for display on the datamap.
	 */
	public String toString() {
		String temp = getName() + ": " + getTypeName() 
			+ " [ "; // + getLowerBound() + " - " + getUpperBound() + " ]";
		
		Number lb = getLowerBound(), ub = getUpperBound();
		
		//Deal with that pesky infinity
		if (lb instanceof Integer && lb.intValue() == Integer.MIN_VALUE
			|| lb instanceof Double && lb.doubleValue() == Double.MIN_VALUE) {
			temp += "...";
		} else {
			temp += lb;
		} // else
		
		temp += " - ";
		
		// Deal with that pesky infinity
		if (ub instanceof Integer && ub.intValue() == Integer.MAX_VALUE
			|| ub instanceof Double && ub.doubleValue() == Double.MAX_VALUE) {
			temp += "...";
		} else {
			temp += ub;
		} // else
		
		temp += " ]";
		
		String comment = getComment(); 
		
		if (comment != null && !comment.equals("")) {
			temp += "       * " + comment + " *";
		} // if
		
		return temp;
	}
	
	/**
	 * Sets the lower bound for this number.
	 * 
	 * @return A Number representing the item's lower bound. <b>null</b> represents -infinity;
	 */
	public Number getLowerBound() {
		return _lowerBound;
	}
	
	/**
	 * Sets the lower bound for this number.
	 * 
	 * @param newLower The lower bound; <b>null</b> represents -infinity.
	 */
	public void setLowerBound( Number newLower) {
		_lowerBound = newLower;
	}
	
	/**
	 * Gets the upper bound for this number.
	 * 
	 * @return A Number representing the item's upper bound. <b>null</b> represents -infinity;
	 */
	public Number getUpperBound() {
		return _upperBound;
	}
	
	/**
	 * Sets the upper bound for this number.
	 * 
	 * @param newUpper The upper bound; <b>null</b> represents -infinity.
	 */
	public void setUpperBound( Number newUpper) {
		_upperBound = newUpper;
	}
}
