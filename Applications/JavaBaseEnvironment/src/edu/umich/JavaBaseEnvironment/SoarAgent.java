package edu.umich.JavaBaseEnvironment;
/* File: SoarAgent.java
 * Jul 16, 2004
 */

import java.io.File;


/**
 * Interface to be implemented by any agent interacting with a simulation.
 * Does not necessarily have to be a Soar agent, but it needs to be able to make
 * decisions and implement the rest of the methods in this interface.
 * @author John Duchi
 */
public interface SoarAgent {

	/**
	 * Method that, when queried, calculates the next decision that this instance
	 * of <code>SoarAgent</code> will make. Values of return are usually determined by called
	 * class.
	 * @return An <code>Object</code> associated with one decision made by this agent,
	 * defined usually by a class to contain the data.
	 */
	public Object makeDecision();
	
	/**
	 * Method that returns the name (usually a color, in our context) of this
	 * instance of <code>SoarAgent</code>.
	 * @return The <code>String</code> name of this <code>SoarAgent</code>.
	 */
	public String getName();
	
	/**
	 * Method to return the <code>Location</code> of this SoarAgent.
	 * @return The <code>Location</code> of this <code>SoarAgent</code> in the simulation.
	 * Contains an (x, y) pair.
	 */
	public Location getLocation();

	/**
	 * Returns the name of the color of this instance of SoarAgent.
	 * @return String name of this SoarAgent's color.
	 */
	public String getColorName();
	
	/**
	 * Resets this SoarAgent. Implementation determined by implementer,
	 * but expected to set score to zero and re-initialize any other
	 * variables (other than location).
	 */
	public void reset();
	
	/**
	 * <b>THIS CODE IS WHAT STILL NEEDS TO BE IMPLEMENTED!</b>
	 * <p>Method called to attach the specified .soar file to this <code>SoarAgent</code>.</p>
	 * @param toAttach The <code>File</code> to be added.
	 */
	public void attachSoarCode(File toAttach);
	
	/**
	 * Returns a path to a soar file for reload command
	 */
	public String getProductionPath();
	
}
