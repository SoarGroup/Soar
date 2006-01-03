/* File: FlyingMissile.java
 * Aug 11, 2004
 */
package edu.umich.tanksoar;
import edu.umich.JavaBaseEnvironment.Location;

/**
 * A missile flying through the simulation, this class can take care of calculating
 * where it will fly, how fast it flies, who shot it, and running into obstacles
 * in the simulation.
 * @author John Duchi
 */
public class FlyingMissile{
	
	/** The distance that a missile flies per turn in a TankSoar simulation. This distance
	 * is rounded (for <code>Location</code> purposes) every turn, but its exact value
	 * is kept. */
	public static final double DistancePerTurn = 1.30;
	
	/**
	 * The health points a missile takes from a tank if it hits it while
	 * the Tank's shields are down.
	 */
	public static final int DamageDone = 400;
	
	/**
	 * The energy a missile takes from a tank if the missile hits the tank
	 * while the Tank's shields are up.
	 */
	public static final int EnergyTaken = 250;
	
	/** The <code>Tank</code> that shot this <code>FlyingMissile</code>. */
	private Tank myShooter;
	/** The <code>Location</code> whose x and y coordinates are closest to <code>myX
	 * </code> and <code>myY</code>. */
	private Location myLocation;
	/** The exact value of my x-coordinate. */
	double myX;
	/** The exact value of my y-coordinate. */
	double myY;
	/** The direction I am facing (as determined by <code>Tank</code>). */
	private int myDir;
	/** The <code>TankSoarJControl</code> managing the simulation. */
	private TankSoarJControl myTC;
	
	/**
	 * Constructs a new FlyingMissile with the requested parameters.
	 * @param shooter The Tank that shot this missile.
	 * @param direction The direction in which this missile was shot.
	 * @param loc The starting location of the missile.
	 * @param tc The TankSoarJControl that is running this simulation.
	 */
	public FlyingMissile(Tank shooter, int direction, Location loc, TankSoarJControl tc){
		myLocation = loc;
		myDir = direction;
		myShooter = shooter;
		myTC = tc;
		myX = myLocation.getX();
		myY = myLocation.getY();
	}
	
	/**
	 * Gives the caller the Tank that shot this missile.
	 * @return The Tank that shot this missile.
	 */
	public Tank getShooter(){
		return(myShooter);
	}
	/**
	 * Gives my <code>Location</code> in the simulation, which is rounded to be as
	 * close as possible to my exact x and y-coordinates.
	 * @return My <code>Location</code> in the TankSoar simulation.
	 */
	public Location getLocation(){
		return(myLocation);
	}
	/**
	 * Gives my exact x-coordinate.
	 * @return The exact (non-rounded) x-value for my position.
	 */
	public double getExactX(){
		return(myX);
	}
	/**
	 * Gives my exact y-coordinate.
	 * @return The exact (non-rounded) y-value for my position.
	 */
	public double getExactY(){
		return(myY);
	}
	
	/**
	 * Gives the caller the Locations this missile would pass through if flying unobstructed.
	 * Last Location in return array is Location, rounded to nearest x and y coordinates, at
	 * which this missile would stop its flight in one turn.
	 * @return Array of Location this missile would pass through on its next turn at if
	 * it flew unobstructed. Array contains [First location passed through, ... , Final location].
	 */
	public Location[] getTraversedLocations(){
		int len;
		if(myDir == Tank.NORTH || myDir == Tank.SOUTH){
			len = (int)Math.round(myY - (int)myY + DistancePerTurn);
		} else {
			len = (int)Math.round(myX - (int)myX + DistancePerTurn);
		}
		Location[] result = new Location[len];
		int x = myLocation.getX(), tempX = x;
		int y = myLocation.getY(), tempY = y;
		for(int i = 1; i <= len; i++){
			switch(myDir){
			case(Tank.NORTH):{ tempY = y - i; break; }
			case(Tank.EAST):{ tempX = x + i; break; }
			case(Tank.SOUTH):{ tempY = y + i; break; }
			case(Tank.WEST):{ tempX = x - i; break;}
			}
			result[i-1] = new Location(tempX, tempY);
		}
		return (result);
	}
	
	/**
	 * Gives the caller the location (rounded to nearest x and y coordinates)
	 * this missile would be at if flying unobstructed.
	 * @return The Location this missile would end its next turn at if there
	 * are no Walls or Tanks in the way.
	 */
	public Location getNextLocation(){
		double x = myX, y = myY;
		switch(myDir){
		case(Tank.NORTH):{ y = myY - DistancePerTurn; break; }
		case(Tank.EAST):{ x = myX + DistancePerTurn; break; }
		case(Tank.SOUTH):{ y = myY + DistancePerTurn; break; }
		case(Tank.WEST):{ x = myX - DistancePerTurn; break; }
		}
		return(new Location((int)Math.round(x), (int)Math.round(y)));
	}
	
	/**
	 * Moves the missile to its next Location, checking along the way for obstacles.
	 * Will add the missile to its final <code>Location</code>, if there were no obstacles
	 * encountered along the way.
	 * @return <code>true</code> if the missile did not crash into any obstacles, <code>false</code> otherwise.
	 */
	public boolean fly(){
		Location currLocation = null;
		TankSoarJSquare ts;
		switch(myDir){
		case(Tank.NORTH):{
			for(double y = myY; y > myY - DistancePerTurn; --y){
				currLocation = new Location((int)Math.round(myX), (int)Math.round(y));
				ts = (TankSoarJSquare)myTC.getLocationContents(currLocation);
				if(!squareOK(ts)) return false;
			}
			myY -= DistancePerTurn;
			break;
		}
		case(Tank.EAST):{
			for(double x = myX; x < myX + DistancePerTurn; ++x){
				currLocation = new Location((int)Math.round(x), (int)Math.round(myY));
				ts = (TankSoarJSquare)myTC.getLocationContents(currLocation);
				if(!squareOK(ts)) return false;
			}
			myX += DistancePerTurn;
			break;
		}
		case(Tank.SOUTH):{
			for(double y = myY; y < myY + DistancePerTurn; ++y){
				currLocation = new Location((int)Math.round(myX), (int)Math.round(y));
				ts = (TankSoarJSquare)myTC.getLocationContents(currLocation);
				if(!squareOK(ts)) return false;
			}
			myY += DistancePerTurn;
			break;
		}
		case(Tank.WEST):{
			for(double x = myX; x > myX - DistancePerTurn; --x){
				currLocation = new Location((int)Math.round(x), (int)Math.round(myY));
				ts = (TankSoarJSquare)myTC.getLocationContents(currLocation);
				if(!squareOK(ts)) return false;
			}
			myX -= DistancePerTurn;
			break;
		}
		}
		myLocation = new Location((int)Math.round(myX), (int)Math.round(myY));
		if(myLocation.equals(currLocation)) return true;
		if(squareOK((TankSoarJSquare)myTC.getLocationContents(myLocation))){
			((EnterableSquare)myTC.getLocationContents(myLocation)).addOccupant(this);
			return(true);
		}
		return(false);
	}
	
	/**
	 * Tells if the specified <code>TankSoarJSquare</code> is OK to be entered by a missile.
	 * If it is, returns <code>true</code>. If there is a <code>Tank</code> in the square
	 * or a <code>TSWall</code> the missile will crash and the <code>Tank</code> will be hit
	 * with the missile.
	 * @param ts The <code>TankSoarJSquare</code> being checked.
	 * @return <code>true</code> if the specified square can be entered and flown through,
	 * <code>false</code> otherwise.
	 */
	private boolean squareOK(TankSoarJSquare ts){
		if(ts instanceof EnterableSquare && ((EnterableSquare)ts).containsAgent()){
			((Tank)((EnterableSquare)ts).getAgent()).hitWithMissile(this);
			return false;
		} else if(ts instanceof TSWall){
			return false;
		}
		return true;
	}
	
	/**
	 * Gives caller the integer value of the direction that this <code>FlyingMissile</code>
	 * is travelling, as defined in <code>Tank</code>.
	 * @return The direction this <code>FlyingMissile</code> is going.
	 */
	public int getDirection(){
		return(myDir);
	}

}
