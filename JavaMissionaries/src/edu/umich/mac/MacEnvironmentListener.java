//-----------------------------------------------------------------------------------
// edu.umich.mac.MacEnvironmentListener
// Author: Trevor McCulloch
// Date Created: 25 March 2005
//-----------------------------------------------------------------------------------

package edu.umich.mac;

/**
 * Class that implement this interface provide methods to deal with events when the
 * state of a <code>MacEnvironment</code> is changed by Soar.
 * 
 * @author Trevor McCulloch, University of Michigan
 * @version 1.1
 */
public interface MacEnvironmentListener {
    /**
     * Sent when the Soar kernel sends the <code>SYSTEM_STARTED</code> event.
     * 
     * @param e - environment that started.
     */
    public void systemStarted(MacEnvironment e);
    /**
     * Sent when the Soar kernel sends the <code>SYSTEM_STOPPED</code> event.
     * 
     * @param e - the environment that stopped.
     */
    public void systemStopped(MacEnvironment e);
    /**
     * Sent when a boat is moved from one bank to another.
     * 
     * @param e            - environment in which the boat moved.
     * @param fromBank     - bank the boat moved from.
     * @param missionaries - number of missionaries in the boat.
     * @param cannibals    - number of cannibals in the boat.
     * @param boats        - number of boats moved.
     */
    public void boatMoved(MacEnvironment e, RiverBank fromBank,
            int missionaries, int cannibals, int boats);
    /**
     * Sent when the <code>MacEnvironment</code> reaches its goal state. If this
     * message is sent as a result of the boat being moved, <code>boatMoved</code>
     * will be called first.
     * 
     * @param e - <code>MacEnvironment</code> that reached the goal state.
     */
    public void atGoalState(MacEnvironment e);
}
