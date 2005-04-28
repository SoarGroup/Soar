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
import sml.Kernel;
import sml.WMElement;
import sml.smlPrintEventId;

/**
 * Creates a representation of the problem space for the Missionaries & Cannibals
 * problem, and interfaces it to Soar, allowing Soar to solve the problem. The
 * representation includes a "left" river bank and a "right" river bank, with 
 * 3 missionaries, 3 cannibals, and a boat on the left bank. The goal is for all of
 * the missionaries and cannibals to make it to the right bank.
 * 
 * @author Trevor McCulloch, University of Michigan
 * @version 1.0
 */
public class MacEnvironment implements Runnable {
    private Kernel kernel;
    private Agent agent;
    
    private RiverBank leftBank;
    private RiverBank rightBank;
    
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
        try {
            kernel = Kernel.CreateKernelInNewThread("SoarKernelSML");
        } catch (Exception e) {
            System.out.println("Error creating kernel: " + e.getMessage());
            System.exit(1);
        }
        
        agent = kernel.CreateAgent("MAC");
        boolean load = agent.LoadProductions("mac/mac.soar");
        if (!load || agent.HadError()) {
            throw new IllegalStateException("Error loading productions: "
                    + agent.GetLastErrorDescription());
        }
        
        leftBank = new RiverBank(agent, "left", 3, 3, 1);
        rightBank = new RiverBank(agent, "right", 0, 0, 0);
        
        leftBank.setOppositeBank(rightBank);
        rightBank.setOppositeBank(leftBank);
        
        agent.Commit();
        
        // TODO make a debug flag turn this on
        agent.RegisterForPrintEvent(smlPrintEventId.smlEVENT_PRINT, this,
                "printEventHandler", null);
    }
    
    private void printEventHandler(int eventID, Object data,
            Agent agent, String message) {
        System.out.print(message);
    }
    
    /**
     * Runs one Soar decision cycle and performs any commands that Soar may issue to
     * modify the environment.
     */
    public void run() {
        if (!isAtGoalState())
            agent.RunSelfTilOutput(15);
        else
            return;
        
        // perform the command on the output link
        if (!agent.Commands())
            throw new IllegalStateException("No Command provided on OutputLink");
        
        Identifier command = agent.GetCommand(0);
        if (!command.GetCommandName().equals(MOVE_BOAT)) {
            throw new IllegalStateException("Unknown Command: "
                    + command.GetCommandName());
        }
        
        WMElement wme = command.FindByAttribute(MISSIONARIES, 0);
        int missionaries = (wme != null ? wme.ConvertToIntElement().GetValue() : 0);
        wme = command.FindByAttribute(CANNIBALS, 0);
        int cannibals = (wme != null ? wme.ConvertToIntElement().GetValue() : 0);
        wme = command.FindByAttribute(BOAT, 0);
        int boats = (wme != null ? wme.ConvertToIntElement().GetValue() : 0);
        
        // whichever bank has the boat is the from bank
        RiverBank fromBank = (leftBank.getBoatCount() > 0 ? leftBank : rightBank);
        RiverBank toBank = fromBank.getOppositeBank();
        
        // make the move
        fromBank.modifyCounts(-missionaries, -cannibals, -boats);
        toBank.modifyCounts(missionaries, cannibals, boats);
        command.AddStatusComplete();
        agent.Commit();
        fireBoatMoved(fromBank, missionaries, cannibals, boats);
        
        if (isAtGoalState())
            fireAtGoalState();
    }
    
    /**
     * Resets the environment to its initial state, with 3 missionaries, 3 cannibals,
     * and a boat on the left bank. Also re-initializes Soar.
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
     * Checks to see if 3 missionaries, 3 cannibals, and the boat are all on the
     * right bank.
     * 
     * @return <code>true</code> if all of the missionaries and cannibals are on the
     *         right bank, <code>false</code> otherwise.
     */
    public boolean isAtGoalState() {
        return (rightBank.getMissionaryCount() == 3 &&
                rightBank.getCannibalCount() == 3 && rightBank.getBoatCount() == 1);
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
     * Adds the listener to the set of listeners that will be notified whenever the
     * the boat is moved or when this environment reaches its goal state by calling
     * methods from the <code>MacEnvironmentListener</code> interface.
     * 
     * @param l - the listener to add to the set.
     */
    public void addEnvironmentListener(MacEnvironmentListener l) {
        environmentListeners.add(l);
    }
    /**
     * Removes the listener from the set of listeners that will be notified whenever
     * the boat is moved or when the environment reaches its goal state.
     * 
     * @param l - the listener to remove from the set.
     */
    public void removeEnvironmentListener(MacEnvironmentListener l) {
        environmentListeners.remove(l);
    }
    
    /**
     * Calls the <code>boatMoved</code> method for each listener with the arguments
     * <code>this, fromBank, m, c, b</code>.
     * 
     * @param fromBank - bank the boats and people moved from.
     * @param m        - number of missionaries moving from <code>fromBank</code>. 
     * @param c        - number of cannibals moving from <code>fromBank</code>.
     * @param b        - number of boats moving from <code>fromBank</code>.
     */
    private void fireBoatMoved(RiverBank fromBank, int m, int c, int b) {
        Iterator i = environmentListeners.iterator();
        while (i.hasNext())
            ((MacEnvironmentListener)i.next()).boatMoved(this, fromBank, m, c, b);
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
