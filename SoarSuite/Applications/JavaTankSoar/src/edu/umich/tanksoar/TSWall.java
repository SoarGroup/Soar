package edu.umich.tanksoar;
/* File: TSWall.java
 * Jul 21, 2004
 */

/**
 * @author jduchi
 */
public class TSWall implements edu.umich.tanksoar.TankSoarJSquare {

    private int myWorth = 0;

	public int getWorth() {
		return (myWorth);
	}

    public String getName() {
        return("wall");
    }

    public boolean canEnter() {
        return(false);
    }

}
