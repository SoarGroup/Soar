package edu.umich.eaters;
/* File: EatersSquare.java
 */

/**
 * @author jduchi
 *
 */
public interface EatersSquare {
	/**
	 * Used to find the point value of entering a square.
	 * @return The point value of entering the square implementing EatersSquare
	 */
	public int getWorth();
	
	/**
	 * Gives the eaters/soar name of the contents of the square.
	 * @return A string that is the name of the contents of the square.
	 */
	public String getName();
	
	/**
	 * Tests whether an eater can enter the given square.
	 * @return True if the eater can enter the square; false otherwise.
	 */
	public boolean canEnter();
}
