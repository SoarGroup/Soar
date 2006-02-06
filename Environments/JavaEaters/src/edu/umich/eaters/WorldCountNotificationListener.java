/* File: WorldCountNotificationListener.java
 * Sep 16, 2004
 */
package edu.umich.eaters;

/**
 * An interface to implement to be notified when a <code>SimulationControl</code> has finished
 * notifying its listeners that its world count has changed.
 * @author John Duchi
 */
public interface WorldCountNotificationListener {
	
	/**
	 * Notification fired when a <code>SimulationControl</code> finishes
	 * notifying <code>SimulationControlListener</code>s that the world
	 * count has changed.
	 */
	public void finishedWorldCountUpdating();

}
