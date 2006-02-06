package edu.umich.eaters;
/* File: BonusFood.java
 * Jul 21, 2004
 */

/**
 * @author jduchi
 */
public class BonusFood implements EatersSquare {

    private int myWorth;
    private boolean canEnter = true;
    
    public BonusFood(int worth){
        myWorth = worth;
    }
    
    public int getWorth() {
        return (myWorth);
    }

    public String getName() {
        return("bonusfood");
    }

    public boolean canEnter() {
        return (canEnter);
    }

}
