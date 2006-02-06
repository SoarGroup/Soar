package edu.umich.eaters;
/* File: NormalFood.java
 * Jul 21, 2004
 */

/**
 * @author jduchi
 */
public class NormalFood implements EatersSquare{

    private int myWorth;
    private boolean canEnter = true;
    
    public NormalFood(int worth){
        myWorth = worth;
    }
    
    public int getWorth(){
        return(myWorth);
    }
    
    public String getName(){
        return("normalfood");
    }
    
    public boolean canEnter(){
        return(canEnter);
    }
    
}
