package edu.umich.eaters;
/* File: TSWall.java
 * Jul 21, 2004
 */

/**
 * @author jduchi
 */
public class EatersWall implements edu.umich.eaters.EatersSquare {

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
