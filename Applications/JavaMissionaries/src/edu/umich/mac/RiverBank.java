//-----------------------------------------------------------------------------------
// edu.umich.mac.RiverBank
// Author: Trevor McCulloch
// Date Created: 25 March 2005
//-----------------------------------------------------------------------------------

package edu.umich.mac;

import sml.Agent;
import sml.Identifier;
import sml.IntElement;

/**
 * This class represents one of the two river banks in the MissionariesAndCannibals
 * problem. It keeps track of how many missionaries, cannibals, and boats are on its
 * shores, and udpates Soar with its current status.
 * 
 * @author Trevor McCulloch, University of Michigan
 * @version 1.0
 */
public class RiverBank {
    private Agent agent;
    
    private Identifier bank;
        private IntElement missionaries;
        private IntElement cannibals;
        private IntElement boats;
        private Identifier otherBank;
    
    private String side;
    private int missionaryCount;
    private int cannibalCount;
    private int boatCount;
    private RiverBank oppositeBank;
    
    /* strings for Soar */
    public static final String BANK = "-bank";
    public static final String MISSIONARIES = "missionaries";
    public static final String CANNIBALS = "cannibals";
    public static final String BOAT = "boat";
    public static final String OTHER_BANK = "other-bank";
    
    /**
     * Creates a new RiverBank that belongs to the given Agent, with assigned name
     * <code>side</code> and an initial number of missionaries, cannibals, and boats.
     * This initial representation is also created in Soar.
     * 
     * @param agent         - the Soar agent this river bank belongs to.
     * @param side          - side of the river, usually "left" or "right."
     * @param nmissionaries - initial number of missionaries on this bank.
     * @param ncannibals    - initial number of cannibals on this bank.
     * @param nboats        - initial number of boats on this bank.
     */
    public RiverBank(Agent agent, String side,
            int nmissionaries, int ncannibals, int nboats) {
        this.agent = agent;
        this.side = side;
        
        missionaryCount = nmissionaries;
        cannibalCount = ncannibals;
        boatCount = nboats;
        
        bank = agent.CreateIdWME(agent.GetInputLink(), side + BANK);
        missionaries = agent.CreateIntWME(bank, MISSIONARIES, missionaryCount);
        cannibals = agent.CreateIntWME(bank, CANNIBALS, cannibalCount);
        boats = agent.CreateIntWME(bank, BOAT, boatCount);
    }
    
    /**
     * Get the bank on the other side of the river.
     * 
     * @return the bank on the opposite side of the river, or <code>null</code> if no
     *         bank has been assigned.
     */
    public RiverBank getOppositeBank() {
        return oppositeBank;
    }
    /**
     * Sets the bank considered to be on the other side of the river. Setting this
     * changes the Soar representation.
     * 
     * @param aBank - the bank on the other side of the river.
     */
    public void setOppositeBank(RiverBank aBank) {
        oppositeBank = aBank;
        if (otherBank != null)
            agent.DestroyWME(otherBank);
        otherBank = agent.CreateSharedIdWME(bank, OTHER_BANK, oppositeBank.bank);
    }
    
    /**
     * Gets the number of missionaries on this bank.
     * 
     * @return the number of missionaries on this bank.
     */
    public int getMissionaryCount() {
        return missionaryCount;
    }
    /**
     * Gets the number of cannibals on this bank.
     * 
     * @return the number of cannibals on this bank.
     */
    public int getCannibalCount() {
        return cannibalCount;
    }
    /**
     * Gets the number of boats on this bank.
     * 
     * @return the number of boats on this bank.
     */
    public int getBoatCount() {
        return boatCount;
    }
    
    /**
     * Modifies the number of missionaries, cannibals and boats on this bank by the
     * passed amounts. Note that these number may be negative. Calling this method
     * will update Soar.
     * 
     * @param missionary - number of missionaries to add/subtract.
     * @param cannibal   - number of missionaries to add/subtract.
     * @param boat       - number of boats to add subtract.
     * @return <code>true</code> if the changes will not leave negative numbers of
     *         missionaries, cannibals and boats on this bank, if all of the changes
     *         are either negative or positive, and the number of people and number
     *         of boats moving is not equal to 0. Returns <code>false</code>
     *         otherwise.
     */
    public boolean modifyCounts(int missionary, int cannibal, int boat) {
        // make sure modifications put us in the appropriate range
        if ((missionary + missionaryCount < 0) || (cannibal + cannibalCount < 0) ||
                (boat + boatCount < 0)) {
            return false;
        }
        
        int movingPeople = missionary + cannibal;
        // both the people and boat should be changing
        if (movingPeople == 0 || boat == 0)
            return false;
        // likewise, the ought to be moving in the same direction
        if ((movingPeople > 0 && boat < 0) || (movingPeople < 0 && boat > 0))
            return false;
        
        missionaryCount += missionary;
        cannibalCount += cannibal;
        boatCount += boat;
        
        if (missionary != 0)
            agent.Update(missionaries, missionaryCount);
        if (cannibal != 0)
            agent.Update(cannibals, cannibalCount);
        agent.Update(boats, boatCount);
        
        return true;
    }
    
    /**
     * Sets the number of missionaries, cannibals and boats on this bank. These
     * numbers may not be negative. Calling this method will update Soar.
     * 
     * @param missionary - number of missionaries on this bank.
     * @param cannibal   - number of cannibals on this bank.
     * @param boat       - number of boats on this bank.
     * @return <code>true</code> if none of the arguments are negative,
     *         <code>false</code> otherwise.
     */
    public boolean setCounts(int missionary, int cannibal, int boat) {
        if (missionary < 0 || cannibal < 0 || boat < 0)
            return false;
        
        if (missionaryCount != missionary)
            agent.Update(missionaries, missionary);
        missionaryCount = missionary;
        
        if (cannibalCount != cannibal)
            agent.Update(cannibals, cannibal);
        cannibalCount = cannibal;
        
        if (boatCount != boat)
            agent.Update(boats, boat);
        boatCount = boat;
        
        return true;
    }
    
    // we're going to omit a finalizer
    // since presumably the only time this is going away is when the agent will
    // be destroyed anyway
}
