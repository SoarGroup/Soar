/* File: TankInputInfo.java
 * Aug 12, 2004
 */
package edu.umich.tanksoar;

import java.util.ArrayList;
import java.util.Iterator;


/**
 * A simple struct-like class to contain the information that a simulation can pass through
 * the input-link of a <code>Tank</code> into Soar so that an agent can make a decision.
 * It is structured, with the exception of the radar input, almost exactly as the Soar productions
 * interpret the wme structure.
 * @author John Duchi
 */
public class TankInputInfo {
	
	/** <code>boolean</code> to indicate an 'on' value for a switch on the input-link. */
	public static final boolean on = true;
	/** <code>boolean</code> to indicate an 'off' value for a switch on the input-link. */
	public static final boolean off = false;
	/** <code>boolean</code> to indicate a 'yes' value for an attribute on the input-link. */
	public static final boolean yes = true;
	/** <code>boolean</code> to indicate a 'no' value for an attribute on the input-link. */
	public static final boolean no = false;
	/**
	 * A global counter for all <code>Tank</code>s, beginning at 0, and incremented at each decision.
	 * <p><b>IO Usage:</b> ^clock 1-N
	 */
	public int clock = 0;
	/**
	 * The direction the <code>Tank</code> is facing, defined by the integer value in class Tank.
	 * <p><b>IO Usage:</b> ^direction north/east/south/west
	 */
	public int direction = Tank.NORTH;
	/**
	 * The amount of energy <code>Tank</code> has.
	 * <p><b>IO Usage:</b> ^energy 0-1000
	 */
	public int energy = 0;
	/** 
	 * <code>true</code> if <code>Tank</code> is sitting on an energy recharger.
	 * <p><b>IO Usage:</b> ^energyrecharger no/yes
	 */
	public boolean energyrecharger = false;
	/**
	 * The amount of health this <code>Tank</code> has.
	 * <p><b>IO Usage:</b> ^health 0-1000
	 */
	public int health = 0;
	/**
	 * <code>true</code> if this <code>Tank</code> is sitting on a health recharger.
	 * <p><b>IO Usage:</b> ^healthrecharger no/yes
	 */
	public boolean healthrecharger = false;

  //Returns a container of all of the energy rechargers currently on the radar, if any
  public RadarSightsQueryResult getEnergySquaresInSight()
  {
    RadarSightsQueryResult result = new RadarSightsQueryResult();
    if(radarStatus == true)
    {
      for(int pos = 0; pos < Tank.RadarWidth; ++pos)//0->left, 1->center, 2->right
      {
        for(int dist = 0; dist <= radarDistance + 1; ++dist)
        {
          if(radarSights[pos][dist] instanceof EnergySquare)
            result.addToResults(radarSights[pos][dist], new RadarSightsIndex(pos, dist));
        }//inner for        
      }//outer for
    }
    return result;
  }

  //Returns a container of all of the health rechargers currently on the radar, if any
  public RadarSightsQueryResult getHealthSquaresInSight()
  {
    RadarSightsQueryResult result = new RadarSightsQueryResult();
    if(radarStatus == true)
    {
      for(int pos = 0; pos < Tank.RadarWidth; ++pos)//0->left, 1->center, 2->right
      {
        for(int dist = 0; dist <= radarDistance + 1; ++dist)
        {
          if(radarSights[pos][dist] instanceof HealthSquare)
            result.addToResults(radarSights[pos][dist], new RadarSightsIndex(pos, dist));
        }//inner for
      }//outer for
    }
    return result;
  }

  //Returns a container of all of the missile powerups currently on the radar, if any
  public RadarSightsQueryResult getMissilePowerupsInSight()
  {
    RadarSightsQueryResult result = new RadarSightsQueryResult();
    if(radarStatus == true)
    {
      for(int pos = 0; pos < Tank.RadarWidth; ++pos)//0->left, 1->center, 2->right
      {
        for(int dist = 0; dist <= radarDistance + 1; ++dist)
        {            
          if(radarSights[pos][dist] instanceof EnterableSquare)
          {
            if(((EnterableSquare)radarSights[pos][dist]).containsMissiles())
              result.addToResults(((EnterableSquare)radarSights[pos][dist]).getMissileContainer(),
                                                                     new RadarSightsIndex(pos, dist));
          }
        }//inner for
      }//outer for
    }
    return result;
  }

  //Returns a container of all of the energy rechargers currently on the radar, if any
  public RadarSightsQueryResult getObstaclesInSight()
  {
    RadarSightsQueryResult result = new RadarSightsQueryResult();
    if(radarStatus == true)
    {
      for(int pos = 0; pos < Tank.RadarWidth; ++pos)//0->left, 1->center, 2->right
      {
        for(int dist = 0; dist <= radarDistance + 1; ++dist)
        {
          Object subject = radarSights[pos][dist];
          //we're classifying any square that can't be entered as an obstacle
          if(subject instanceof TankSoarJSquare && !((TankSoarJSquare) subject).canEnter())
            result.addToResults(radarSights[pos][dist], new RadarSightsIndex(pos, dist));
        }//inner for
      }//outer for
    }
    return result;
  }
  //Returns a container of all of the open spaces currently on the radar, if any
  //Note that even squares that contain other Tanks are considered open
  public RadarSightsQueryResult getOpenSpacesInSight()
  {
    RadarSightsQueryResult result = new RadarSightsQueryResult();
    if(radarStatus == true)
    {
      for(int pos = 0; pos < Tank.RadarWidth; ++pos)//0->left, 1->center, 2->right
      {
        for(int dist = 0; dist <= radarDistance + 1; ++dist)
        {
          Object subject = radarSights[pos][dist];
          if(subject instanceof EnterableSquare && subject instanceof TankSoarJSquare)
            result.addToResults(subject, new RadarSightsIndex(pos, dist));
        }//inner for
      }//outer for
    }
    return result;
  }

  //Returns a container of all of the Tanks currently on the radar, if any
  public RadarSightsQueryResult getTanksInSight(Tank t)
  {
    RadarSightsQueryResult result = new RadarSightsQueryResult();
    if(radarStatus == true)
    {
      for(int pos = 0; pos < Tank.RadarWidth; ++pos)//0->left, 1->center, 2->right
      {
        for(int dist = 0; dist <= radarDistance; ++dist)
        {
          if(radarSights[pos][dist] instanceof EnterableSquare)
          {
            Object[] squareContents = ((EnterableSquare)radarSights[pos][dist]).getOccupants();
            for(int index = 0; index < squareContents.length; ++index)
            {
              if(squareContents[index] instanceof Tank && squareContents[index] != t)
                result.addToResults(squareContents[index], new RadarSightsIndex(pos, dist));
            }
          }
        }//inner for
      }//outer for
    }
    return result;
  }

	/**
	 * The number of missiles this <code>Tank</code> has.
	 * <p><b>IO Usage:</b> ^missiles 0-N
	 */
	public int missiles = 0;

	/**
	 * The effective distance of the radar the last time it was used, that is,
	 * the distance it reached before it was blocked. If there were no obstacles,
	 * this is the same as the radar setting.
	 * <p><b>IO Usage:</b> ^radar-distance 1-14
	 */
	public int radarDistance = 1;
	/**
	 * The distance that the radar has been set to using the radar-power output command.
	 * <p><b>IO Usage:</b> ^radar-setting 1-14
	 */
	public int radarSetting = 1;
	/**
	 * <code>true</code> if the <code>Tank</code>'s radar is set to on.
	 * <p><b>IO Usage:</b> ^radar-status on/off
	 */
	public boolean radarStatus = on;
	/**
	 * Whenever a <code>Tank</code> dies and is resurrected, this has value yes.
	 * <p><b>IO Usage:</b> ^resurrected no/yes
	 */
	public boolean resurrected = no;
	/**
	 * Tells whether the shields are turned on or not.
	 * <p><b>IO Usage:</b> ^shield-status on/off
	 */
	public boolean shieldStatus = off;
	/**
	 * The x-coordinate of this <code>Tank</code>. X-coordinates range from 1 (left side) to 14 (right side).
	 * <p><b>IO Usage:</b> ^x 1-14
	 */
	public int X = 0;
	/**
	 * The y-coordinate of this <code>Tank</code>. Y-coordinates range from 1 (top) to 14 (bottom).
	 * <p><b>IO Usage:</b> ^y 1-14
	 */
	public int Y = 0;
	/**
	 * The score of this <code>Tank</code>.
	 * <p><b>IO Usage:</b> ^my-score N
	 */
	public int myScore = 0;
	/**
	 * The amount the score of this <code>Tank</code> has changed since the previous turn.
	 * <p><b>IO Usage:</b> ^score-change N
	 */
	public int scoreChange = 0;
	/** 
	 * A random real number between 0 and 1 that changes every decision.
	 * <p><b>IO Usage:</b> ^random 0.0-1.0
	 */
	public double random = 0.0;

	/**
	 * Detects whether the squares immediately adjacent to a <code>Tank</code> are
	 * blocked or open (yes = blocked, no = open). Can be blocked
	 * by obstacles or <code>Tank</code>s.
	 */
	public static class BlockedSensor{
		/**
		 * <code>true</code> if blocked from the back.
		 * <p><b>IO Usage</b>: ^blocked.backward yes/no
		 */
		public boolean backward = no;
		/**
		 * <code>true</code> if blocked from the front.
		 * <p><b>IO Usage</b>: ^blocked.forward yes/no
		 */
		public boolean forward = no;
		/**
		 * <code>true</code> if blocked from the left.
		 * <p><b>IO Usage</b>: ^blocked.left yes/no
		 */
		public boolean left = no;
		/**
		 * <code>true</code> if blocked from the right.
		 * <p><b>IO Usage</b>: ^blocked.right yes/no
		 */
		public boolean right = no;
	}
	/**
	 * This TankInputInfo's blocked sensor.
	 */
	public BlockedSensor blocked = new BlockedSensor();

	/**
	 * Detects whether there is a missile approaching a <code>Tank</code> at any distance, even
	 * if the missile is on the other side of an obstacle and would not actually hit
	 * the <code>Tank</code>. Does not detect the <code>Tank</code>'s own missiles.
	 */
	public static class IncomingSensor{
		/**
		 * <code>true</code> if there is a missile incoming from behind.
		 * <p><b>IO Usage:</b> ^incoming.backward yes/no
		 */
		public boolean backward = no;
		/**
		 * <code>true</code> if there is a missile incoming from in front.
		 * <p><b>IO Usage:</b> ^incoming.forward yes/no
		 */
		public boolean forward = no;
		/**
		 * <code>true</code> if there is a missile incoming from the right of this <code>Tank</code>.
		 * <p><b>IO Usage:</b> ^incoming.right yes/no
		 */
		public boolean right = no;
		/**
		 * <code>true</code> if there is a missile incoming from the left of this <code>Tank</code>.
		 * <p><b>IO Usage:</b> ^incoming.left yes/no
		 */
		public boolean left = no;
	}
	/**
	 * This TankInputInfo's incoming sensor.
	 */
	public IncomingSensor incoming = new IncomingSensor();

	/**
	 * Shows what it is the <code>Tank</code> can see in a <code>Tank.RadarWidth*(Tank.MaxRadar+1)</code> array of objects.
	 * <code>null</code>s or simple <code>Object</code>s are squares not visible to the Tank.
   *
   * (stokesd edit)  Why is this text here?
	 * On the output link should be the score and direction any enemy <code>Tank</code>s are
	 * facing, too.
	 */
	public Object[][] radarSights = new Object[Tank.RadarWidth][Tank.MaxRadar];

  /**
   * @author stokesd
   *
   * The 2D radarSights array contains object that a Soar agent needs to know about
   * if the radar is on for the current update. Update code can query for certain 
   * objects, and this class will contain the results.  The class can answer certain
   * questions about that collection.
   */
  public class RadarSightsQueryResult
  {  
    private int numResults = 0;    
    
    public int getNumResults(){return numResults;}

    public void addToResults(Object o, RadarSightsIndex sIndex)
    {
      sightings.add(o);
      ++numResults;
      relativePositions.add(sIndex);
    }
    
    private ArrayList sightings = new ArrayList();

    private ArrayList relativePositions = new ArrayList();
    
    public Iterator getResultItr() {return sightings.listIterator();}    
    
    private final int BOGUS_INDEX = -1;

    protected int matchIndex = BOGUS_INDEX;//index into sightings/locations array of last matched item

    public RadarSightsIndex getRelativePositionForElementAt(int index)
    {
      return (RadarSightsIndex) relativePositions.get(index);
    }
 
    //TODO comment
    public String getColorForElementAt(int index)
    {
      String colorReturn;
      try
      {
          colorReturn = ((Tank) sightings.get(index)).getColorName();
          return colorReturn;
      }
      catch (ClassCastException e){}
      return null;
    }
    //brittle //TODO make this less nasty
    public int getLastMatchIndex()
    {
      return matchIndex;
    }

    /**
     * @param element
     * @return true if there is an object within results that matches WME value
     */
    public boolean positionAndDistanceMatch(RadarSightsIndex relativePos)
    {
      boolean found = false;
      //matchIndex = locations.lastIndexOf(inLoc);
      matchIndex = relativePositions.lastIndexOf(relativePos);
      if(matchIndex != BOGUS_INDEX)
        found = true;
      return found;
    }

    public boolean positionDistanceAndColorMatch(RadarSightsIndex inPos, String inColor)
    {
      boolean positionAndDistMatch = positionAndDistanceMatch(inPos);
      if(positionAndDistMatch == false)
      {
        matchIndex = BOGUS_INDEX;
        return false;
      }

      RadarSightsIndex candidateRadarMatch = (RadarSightsIndex) relativePositions.get(getLastMatchIndex());
      TankSoarRadarElement element = (TankSoarRadarElement)
                         (radarSights[candidateRadarMatch.position][candidateRadarMatch.distance]);
      if(element.getColor().equals(inColor))
        return true;//match index is unchanged

      else
      {
        matchIndex = BOGUS_INDEX;
        return false;           
      }
    }
  }
  /**
	 * Sensor detects if the radar of another <code>Tank</code> is detecting the <code>Tank</code> from one
	 * of the four directions.
	 */
	public static class RWavesSensor{
		/**
		 * Tells if the radar of another <code>Tank</code> is detecting the <code>Tank</code> from back.
		 * <p><b>IO Usage:</b> ^rwaves.backward yes/no
		 */
		public boolean backward = no;
		/**
		 * Tells if the radar of another <code>Tank</code> is detecting the <code>Tank</code> from front.
		 * <p><b>IO Usage:</b> ^rwaves.forward yes/no
		 */
		public boolean forward = no;
		/**
		 * Tells if the radar of another <code>Tank</code> is detecting the <code>Tank</code> from right.
		 * <p><b>IO Usage:</b> ^rwaves.right yes/no
		 */
		public boolean right = no;
		/**
		 * Tells if the radar of another <code>Tank</code> is detecting the <code>Tank</code> from left.
		 * <p><b>IO Usage:</b> ^rwaves.left yes/no
		 */
		public boolean left = no;
	}
	/**
	 * This TankInputInfor's rwaves sensor.
	 */
	public RWavesSensor rwaves = new RWavesSensor();

	/**
	 * Detects the closest <code>Tank</code> that moved during the last decision, as long as that
	 * <code>Tank</code> was <code>Tank.MaxHear</code> (currently 7) or less squares away.
	 * If two or more <code>Tank</code>s moved and are equally close (Manhattan distance),
	 * chooses one randomly.
	 * <p><b>IO Usage:</b> ^sound silent/left/right/forward/backward
	 */
	public String sound = "silent";

	/**
	 * The smell sensor detects the closest <code>Tank</code> and provides information on how close that
	 * <code>Tank</code> is and what its color is.
	 */
	public static class SmellSensor{
		/**
		 * Color of the closest <code>Tank</code> that this <code>Tank</code> can smell.
		 * <p><b>IO Usage:</b> ^smell.color none/red/blue/purple...
		 */
		public String color = "none";
		/**
		 * The distance (Manhattan distance--sum of x and y distances) between the two
		 * <code>Tank</code>s.
		 * <p><b>IO Usage:</b> ^smell.distance none/0-28
		 */
		public int distance = 0;
	}
	/**
	 * This <code>TankInputInfo</code>'s smell sensor.
	 */
	public SmellSensor smell = new SmellSensor();

	/**
	 * Initializes the rwaves, incoming, blocked, smell, and sound sensors
	 * to their default off, none, and silent values.
	 */
	public void initializeNonradarSensors(){
		rwaves.backward = no;
		rwaves.forward = no;
		rwaves.left = no;
		rwaves.right = no;
		incoming.backward = no;
		incoming.right = no;
		incoming.left = no;
		incoming.forward = no;
		blocked.backward = no;
		blocked.forward = no;
		blocked.right = no;
		blocked.left = no;
		smell.color = "none";
		smell.distance = -1;
		sound = "silent";
    radarSetting = 1;//stokesd added these two lines to attempt to get the radar to start on
    radarStatus = true;
	}
}

