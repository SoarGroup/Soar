/* File: HealthSquare.java
 * Aug 11, 2004
 */
package edu.umich.tanksoar;

/**
 * Square in TankSoar simulation that is a health charger--a square on which a <code>Tank</code>
 * can recharge its health.
 * @author John Duchi
 */
public class HealthSquare extends EnterableSquare implements TankSoarJSquare{

    /**
     * The amount of health a <code>Tank</code> receives per turn it spends on this
     * <code>HealthSquare</code>.
     */
    public static final int HealthPerTurn = 150;
    
    public boolean canEnter(){
        return(true);
    }
    
}
