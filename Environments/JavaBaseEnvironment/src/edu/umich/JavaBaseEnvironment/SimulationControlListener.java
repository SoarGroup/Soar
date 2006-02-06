package edu.umich.JavaBaseEnvironment;
/* File: SimulationControlListener.java
 * Jul 12, 2004
 */

/**
 * Specification for the SimulationControlListener interface. Classes implementing this
 * interface are usually part of implementing an Eaters simulation.
 * @author John Duchi
 */
public interface SimulationControlListener {

	/**
	 * Notification sent when the location identified by loc has changed
	 * (it may have lost an occupant, etc.).
	 * @param loc The location that has changed.
	 */
	public void locationChanged(Location loc);
	
	/**
	 * Notification sent a SimulationControlListener when the simulation has begun.
	 */
	public void simStarted();
	
	/**
	 * Notification sent a SimulationControl when the simulation ends.
	 * @param message The message giving the reason the simulation ended.
	 */
	public void simEnded(String message);
	
	/**
	 * Notification sent a SimulationControlListener when a SoarAgent is created.
	 * @param created As convenience, the eater created is passed with this method.
	 */
	public void agentCreated(SoarAgent created);
	
	/**
	 * Notification sent a SimulationControlListener when a SoarAgent is destroyed.
	 * @param destroyed As a convenience, the SoarAgent that has been destroyed is passed
	 * with this method.
	 */
	public void agentDestroyed(SoarAgent destroyed);
	
	/**
	 * Notification sent an SimulationControlListener when the world count of
	 * the simulation has changed.
	 * @param worldCount The new world count
	 */
	public void worldCountChanged(int worldCount);
	
	/**
	 * Notification sent an SimulationControlListener when the simulation has quit.
	 */
	public void simQuit();
	
	/**
	 * Notification sent when a new map is set to be the internal map for the Simulation.
	 * @param message An optional message giving information a user may want about a map.
	 */
	public void newMap(String message);
	
}
