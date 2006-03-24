//-----------------------------------------------------------------------------------
// edu.umich.mac.MacEnvironment
// Author: Trevor McCulloch
// Date Created: 25 March 2005
//-----------------------------------------------------------------------------------

package edu.umich.mac;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import sml.Agent;
import sml.Identifier;
import sml.IntElement;
import sml.Kernel;
import sml.WMElement;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;

/**
 * Creates a representation of the problem space for the Missionaries &
 * Cannibals problem, and interfaces it to Soar, allowing Soar to solve the
 * problem. The representation includes a "left" river bank and a "right" river
 * bank, with 3 missionaries, 3 cannibals, and a boat on the left bank. The goal
 * is for all of the missionaries and cannibals to make it to the right bank.
 * 
 * @author Trevor McCulloch, University of Michigan
 * @version 1.0
 */
public class MacEnvironment implements Runnable, Kernel.SystemEventInterface, Kernel.UpdateEventInterface {
    private Kernel kernel;
    private Agent agent;
    
    private RiverBank leftBank;
    private RiverBank rightBank;
    
    private Thread runThread;
    private boolean stopSoar = false;
    
    private List environmentListeners = new LinkedList();
    
    /* strings for Soar */
    public static final String MOVE_BOAT = "move-boat";
    public static final String MISSIONARIES = RiverBank.MISSIONARIES;
    public static final String CANNIBALS = RiverBank.CANNIBALS;
    public static final String BOAT = RiverBank.BOAT;
    public static final String FROM_BANK = "from-bank";
    
    /**
     * Creates the representation of the environment and links it to Soar.
     */
    public MacEnvironment() {
        kernel = Kernel.CreateKernelInNewThread("SoarKernelSML");
        if (kernel.HadError()) {
            System.out.println("Error creating kernel: "
                    + kernel.GetLastErrorDescription());
            System.exit(1);
        }
        
        agent = kernel.CreateAgent("MAC");
        boolean load = agent.LoadProductions("mac/mac.soar");
        if (!load || agent.HadError()) {
            throw new IllegalStateException("Error loading productions: "
                    + agent.GetLastErrorDescription());
        }
        
        kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START,
                this, null);
        kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP,
                this, null);
        kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES,
                this, null);
        
        leftBank = new RiverBank(agent, "left", 3, 3, 1);
        rightBank = new RiverBank(agent, "right", 0, 0, 0);
        
        leftBank.setOppositeBank(rightBank);
        rightBank.setOppositeBank(leftBank);
        
        agent.Commit();
    }
    
    /**
     * Starts a thread that runs Soar continuously until the goal state has been
     * reached.
     * 
     * @return <code>true</code> if run thread started successfully.
     */
    public boolean runSystem() {
        runThread = new Thread(this);
        if (runThread == null)
            return false;
        
        stopSoar = false;
        runThread.start();
        return true;
    }
    
    /**
     * If there is a thread running Soar continuously created by
     * <code>startSystem</code>, this method stops it.
     * 
     * @return <code>true</code> if thread existed and was stopped successfully.
     */
    public boolean stopSystem() {
        runThread = null;
        stopSoar = true;
        return true;
    }
    
    /**
     * @return <code>true</code> if there is a thread continuously running Soar
     * as far as MacEnvironment knows.
     */
    public boolean isRunning() {
        return (runThread != null);
    }
    
    /**
     * Runs one Soar decision cycle, and performs any commands that are put on
     * the output link.
     */
    public void step() {
        if (isAtGoalState())
            return;
        
        kernel.RunAllAgents(1);
        updateWorld();
    }
    
    /**
     * Resets the environment to its initial state, with 3 missionaries,
     * 3 cannibals, and a boat on the left bank. Also re-initializes Soar.
     */
    public void reset() {
        int missionaries = rightBank.getMissionaryCount();
        int cannibals = rightBank.getCannibalCount();
        int boat = rightBank.getBoatCount();
        
        leftBank.setCounts(3, 3, 1);
        rightBank.setCounts(0, 0, 0);
        fireBoatMoved(rightBank, missionaries, cannibals, boat);
	
        agent.InitSoar();
    }
    
    /**
     * Main loop for the run thread.
     */
    public void run() {
        kernel.RunAllAgentsForever();
    }

    /**
     * Update the state of the environment.
     * 
     * This involves:
     * a) Collecting any output the agent has sent since the last update
     * b) Changing the world state
     * c) Sending new input to the agent
     */
    public void updateWorld() {
        if (agent.Commands()) {
            for (int i = 0; i < agent.GetNumberCommands(); ++i) {
                Identifier cmd = agent.GetCommand(i);
                
                if (!cmd.GetCommandName().equals(MOVE_BOAT)) {
                    throw new IllegalStateException("Unknown Command: "
                            + cmd.GetCommandName());
                }
                
                WMElement wme;
                IntElement ie;
                
                wme = cmd.FindByAttribute(MISSIONARIES, 0);
                ie  = (wme != null ? wme.ConvertToIntElement() : null);
                int missionaries = (ie != null ? ie.GetValue() : 0);
                wme = cmd.FindByAttribute(CANNIBALS, 0);
                ie  = (wme != null ? wme.ConvertToIntElement() : null);
                int cannibals = (ie != null ? ie.GetValue() : 0);
                wme = cmd.FindByAttribute(BOAT, 0);
                ie  = (wme != null ? wme.ConvertToIntElement() : null);
                int boats = (ie != null ? ie.GetValue() : 0);
                
                // whichever bank has the boat is the from bank
                RiverBank fromBank = (leftBank.getBoatCount() > 0 ?
                        leftBank : rightBank);
                RiverBank toBank = fromBank.getOppositeBank();
                
                // make the move
                fromBank.modifyCounts(-missionaries, -cannibals, -boats);
                toBank.modifyCounts(missionaries, cannibals, boats);
                cmd.AddStatusComplete();
                fireBoatMoved(fromBank, missionaries, cannibals, boats);
                
                if (isAtGoalState())
                    fireAtGoalState();
            }
        }
        
        //NOTE: Don't have to explicitly commit changes because autocommit is on by default (and we didn't turn it off)
        agent.ClearOutputLinkChanges();
    }
    
    /**
     * This method is called when the "after_all_output_phases" event fires, at
     * which point we update the world
     */
    public void updateEventHandler(int eventID, Object data, Kernel kernel,
            int runFlags)
    {
        // We have a problem at the moment with calling Stop() from arbitrary
        // threads so for now we'll make sure to call it within an event
        // callback.
        if (stopSoar) {
            stopSoar = false;
            kernel.StopAllAgents();
        }
        
        updateWorld() ;
    }
    
    /**
     * Checks to see if 3 missionaries, 3 cannibals, and the boat are all on the
     * right bank.
     * 
     * @return <code>true</code> if all of the missionaries and cannibals are on
     *         the right bank, <code>false</code> otherwise.
     */
    public boolean isAtGoalState() {
        return (rightBank.getMissionaryCount() == 3 &&
                rightBank.getCannibalCount() == 3 &&
                rightBank.getBoatCount() == 1);
    }
    
    /**
     * Gets the bank representing the "left" bank.
     * 
     * @return the left <code>RiverBank</code>.
     */
    public RiverBank getLeftBank() {
        return leftBank;
    }
    /**
     * Gets the bank representing the "right" bank.
     * 
     * @return the right <code>RiverBank</code>.
     */
    public RiverBank getRightBank() {
        return rightBank;
    }
    
    /**
     * Adds the listener to the set of listeners that will be notified whenever
     * the boat is moved or when this environment reaches its goal state by
     * calling methods from the <code>MacEnvironmentListener</code> interface.
     * 
     * @param l - the listener to add to the set.
     */
    public void addEnvironmentListener(MacEnvironmentListener l) {
        environmentListeners.add(l);
    }
    /**
     * Removes the listener from the set of listeners that will be notified
     * whenever the boat is moved or when the environment reaches its goal state.
     * 
     * @param l - the listener to remove from the set.
     */
    public void removeEnvironmentListener(MacEnvironmentListener l) {
        environmentListeners.remove(l);
    }
    
    /**
     * Destroys the Soar kernel and agent associated with this environment.
     */
    public void detachSoar() {
	kernel.Shutdown();
    }
    
    public void systemEventHandler(int eventID, Object data, Kernel kernel)
    {
    	if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue())
    		fireSystemStarted(eventID, data, kernel) ;
    	else if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue())
    		fireSystemStopped(eventID, data, kernel) ;
    }
    
    /**
     * Calls the <code>systemStarted</code> method for each listener with the
     * argument <code>this</code>.
     * 
     * @param eventID - superfluous data from kernel callback
     * @param data    - superfluous data from kernel callback
     * @param kernel  - superfluous data from kernel callback
     */
    private void fireSystemStarted(int eventID, Object data, Kernel kernel) {
        Iterator i = environmentListeners.iterator();
        while (i.hasNext())
            ((MacEnvironmentListener)i.next()).systemStarted(this);
    }
    /**
     * Calls the <code>systemStopped</code> method for each listener with the
     * argument <code>this</code>.
     * 
     * @param eventID - superfluous data from kernel callback
     * @param data    - superfluous data from kernel callback
     * @param kernel  - superfluous data from kernel callback
     */
    private void fireSystemStopped(int eventID, Object data, Kernel kernel) {
        Iterator i = environmentListeners.iterator();
        while (i.hasNext())
            ((MacEnvironmentListener)i.next()).systemStopped(this);
    }
    /**
     * Calls the <code>boatMoved</code> method for each listener with the
     * arguments <code>this, fromBank, m, c, b</code>.
     * 
     * @param fromBank - bank the boats and people moved from.
     * @param m        - number of missionaries moving from <code>fromBank</code>. 
     * @param c        - number of cannibals moving from <code>fromBank</code>.
     * @param b        - number of boats moving from <code>fromBank</code>.
     */
    private void fireBoatMoved(RiverBank fromBank, int m, int c, int b) {
        Iterator i = environmentListeners.iterator();
        while (i.hasNext())
            ((MacEnvironmentListener)i.next()).boatMoved(this,fromBank,m,c,b);
    }
    /**
     * Calls the <code>atGoalState</code> method for each listener with
     * <code>this</code> as the argument.
     */
    private void fireAtGoalState() {
        Iterator i = environmentListeners.iterator();
        while (i.hasNext())
            ((MacEnvironmentListener)i.next()).atGoalState(this);
    }
}
