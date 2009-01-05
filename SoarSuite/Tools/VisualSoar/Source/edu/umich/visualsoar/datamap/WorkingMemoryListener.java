package edu.umich.visualsoar.datamap;

/**
 * This is the interface one must support inorder to
 * listen for Working Memory Events
 * @author Brad Jones
 * @version 0.5a Oct 1999
 */

public interface WorkingMemoryListener {
	/**
	 * Gets called when a wme is added 
	 */
	void WMEAdded(WorkingMemoryEvent wme);
	
	/**
	 * Gets called when a wme is removied
	 */
	void WMERemoved(WorkingMemoryEvent wme);
}
