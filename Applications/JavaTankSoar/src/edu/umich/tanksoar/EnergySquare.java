/* File: EnergySquare.java
 * Aug 11, 2004
 */
package edu.umich.tanksoar;

/**
 * Square in TankSoar simulation that is an Energy recharger, on which a <code>Tank</code>
 * can recharge its energies.
 * @author John Duchi
 */
public class EnergySquare extends EnterableSquare implements TankSoarJSquare{
	
	/**
	 * The amount of Energy a tank recovers every turn it spends on the <code>EnergySquare</code>.
	 */
//%%%	public static final int EnergyPerTurn = 250;

    //EPISODIC_MEMORY:  Reduced this value to zero to remove the
    //importance of charger placement in regards to experiments
	public static final int EnergyPerTurn = 0; 

	public boolean canEnter(){
		return(true);
	}
	
}
