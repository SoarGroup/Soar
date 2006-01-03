/* File: TankSoarJSquare.java
 * Aug 11, 2004
 */
package edu.umich.tanksoar;

/**
 * Interface implemented by elements of TankSoar simulation's map.
 * @author John Duchi
 */
public interface TankSoarJSquare {

	/**
	 * Method to tell caller whether this square is normally enterable by a <code>Tank</code>.
	 * @return <code>true</code> if the <code>Tank</code> can normally enter, <code>false</code> otherwise.
	 */
	public boolean canEnter();

}
