/* File: EnterableSquare.java
 * Aug 13, 2004
 */
package edu.umich.tanksoar;

import edu.umich.JavaBaseEnvironment.SoarAgent;

import java.util.ArrayList;
import java.util.Iterator;

/**
 * A class that any square in a TankSoar simulation must subclass off of if it can
 * be entered or travelled through.
 * @author John Duchi
 */
public class EnterableSquare {

	/** The number of <code>SoarAgent</code> occupants of this square. */
	protected int agentContents = 0;
	/** The last <code>SoarAgent</code> to enter this square, <code>null</code> if none has entered. */
	protected SoarAgent lastAgentIn = null;
	/** An <code>ArrayList</code> of the occupants of this <code>EnterableSquare</code>. */
	protected ArrayList myOccupants = null;
	/** A <code>MissileBucket</code> that is <code>null</code> if there are no missiles contained
	 * in this <code>EnterableSquare</code>, or it is the <code>MissileBucket</code> that is contained
	 * if there are. */
	protected MissileBucket missilesContained = null;
	
	/**
	 * Gives caller an array of the square's occupants.
	 * @return An array of <code>Object</code>s who are the occupants of the square.
	 * Specified to be non-<code>null</code>.
	 */
	public Object[] getOccupants(){
		if(myOccupants == null) return (new Object[0]);
		return(myOccupants.toArray());
	}
	
	/**
	 * Gives the caller the number of agents currently on this square.
	 * @return The number of <code>SoarAgent</code>s currently on this <code>EnterableSquare</code>.
	 */
	public int getNumAgents(){
		return(agentContents);
	}
	
	/**
	 * Adds the specified occupant to the square's occupant list.
	 * <code>addOccupant</code> checks to see if a <code>SoarAgent</code> has been added,
	 * and if so, increments the number of agents on the square. If a <code>Tank</code>
	 * has been added and there are already agents, notifies the <code>Tank</code>'s 
	 * <code>TankSoarJControl</code> that there has been a collision in the 
	 * <code>Tank</code>'s <code>Location</code>. Additionally, if toAdd is 
	 * a <code>MissileBucket</code>, this <code>EnterableSquare</code> sets its
	 * <code>missilesContained</code> to be toAdd.
	 * @param toAdd The <code>Object</code> to add as an occupant to the square.
	 */
	public void addOccupant(Object toAdd){
		if(myOccupants == null) myOccupants = new ArrayList();
		myOccupants.add(toAdd);
		if(toAdd instanceof SoarAgent){
			agentContents++;
			if(toAdd instanceof Tank && agentContents > 1){
				((Tank)toAdd).getTankSimulationControl().addCollisionLocation(((Tank)toAdd).getLocation());
			}
			lastAgentIn = (SoarAgent)toAdd;
		} else if(toAdd instanceof MissileBucket){
			missilesContained = (MissileBucket)toAdd;
		}
	}
	
	/**
	 * Removes the specified occupant from the square's occupant list.
	 * @param toRemove The <code>Object</code> to remove from the square.
	 */
	public void removeOccupant(Object toRemove){
		if(myOccupants == null) return;
		myOccupants.remove(toRemove);
		if(toRemove instanceof SoarAgent){
			agentContents--;
			if(agentContents > 0){
				Iterator iter = myOccupants.iterator();
				while(iter.hasNext()){
					Object o = iter.next();
					if(o instanceof SoarAgent){
						lastAgentIn = (SoarAgent)o;
					}
				}
			} else {
				lastAgentIn = null;
			}
			if(toRemove instanceof Tank && agentContents == 1){
				((Tank)toRemove).getTankSimulationControl().addExitedLocation(((Tank)toRemove).getLocation());
			}
			if(agentContents < 0) agentContents = 0;
		} else if(toRemove instanceof MissileBucket){
			missilesContained = null;
		}
	}
	
	/**
	 * Tells the caller whether this <code>EnterableSquare</code> contains missiles
	 * that can be picked up.
	 * @return <code>true</code> if this <code>EnterableSquare</code> contains a
	 * <code>MissileBucket</code> among its occupants, <code>false</code> otherwise.
	 */
	public boolean containsMissiles(){
		return(missilesContained != null);
	}
	
	/**
	 * Gives to the caller the <code>MissileBucket</code> that this <code>EnterableSquare</code>
	 * contains, assuming it contains one, which can then be picked up by the caller.
	 * @return <code>null</code> if there is no <code>MissileBucket</code> contained by
	 * this <code>EnterableSquare</code>, or the <code>MissileBucket</code> contained if
	 * there is.
	 */
	public MissileBucket getMissileContainer(){
		return(missilesContained);
	}
	
	/**
	 * Tells whether this <code>EnterableSquare</code> contains a <code>SoarAgent</code>.
	 * @return <code>true</code> if this square has a <code>SoarAgent</code> as an occupant,
	 * <code>false</code> otherwise.
	 */
	public boolean containsAgent(){
		return(agentContents > 0);
	}
	
	/**
	 * Gives to the caller the last <code>SoarAgent</code> that entered this square in the simulation.
	 * If no <code>SoarAgent</code>s are currently on the square, will return <code>null</code>.
	 * @return The last <code>SoarAgent</code> that entered, or <code>null</code> if there are none.
	 */
	public SoarAgent getAgent(){
		return(lastAgentIn);
	}
	
}
