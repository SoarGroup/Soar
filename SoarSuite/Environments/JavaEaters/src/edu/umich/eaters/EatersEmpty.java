package edu.umich.eaters;
/* File: EatersEmpty.java
 * Jul 21, 2004
 */

/**
 * Represents an empty square in either a TankSoar or Eaters simulation.
 * @author John Duchi
 */

public class EatersEmpty implements edu.umich.eaters.EatersSquare
{

  private int myWorth;
	private boolean canEnter = true;

	public EatersEmpty()
  {
	  myWorth = 0;
	}

	public EatersEmpty(int worth)
  {
	  myWorth = worth;
	}
 
  public int getWorth()
  {
    return(myWorth);
  }

  public String getName()
  {
    return ("empty");
  }

  public boolean canEnter()
  {
    return(canEnter);
  }  
}
