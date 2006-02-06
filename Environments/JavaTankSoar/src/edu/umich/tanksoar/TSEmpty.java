package edu.umich.tanksoar;
/* File: TSEmpty.java
 * Jul 21, 2004
 */

/**
 * Represents an empty square in either a TankSoar or Eaters simulation.
 * @author John Duchi
 */

public class TSEmpty extends edu.umich.tanksoar.EnterableSquare implements edu.umich.tanksoar.TankSoarJSquare
{

  private int myWorth;
	private boolean canEnter = true;

	public TSEmpty()
  {
	  myWorth = 0;
	}

	public TSEmpty(int worth)
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
