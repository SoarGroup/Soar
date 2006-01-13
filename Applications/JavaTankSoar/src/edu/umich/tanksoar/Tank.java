/* File: Tank.java
 * Aug 11, 2004
 */
package edu.umich.tanksoar;

import edu.umich.JavaBaseEnvironment.Location;
import edu.umich.JavaBaseEnvironment.SoarAgent;

import sml.Kernel;
import sml.Agent;
import sml.Identifier;

import java.util.*;
import java.io.File;

/**
 * This class represents a <code>Tank</code> in a TankSoar simulation.
 * <p>The <code>Tank</code> communicates with Soar and returns to the <code>TankSoarJControl</code>
 * the decisions it has made regarding movement, firing missiles, turning shields on or off,
 * turning radar on or off, and rotating.
 * <p>The <code>Tank</code> does many of its own calculations, including calculating the radar
 * and an ability to undo its most recent moves in case of crashes (into other <code>Tank</code>s)
 * or other events.
 * @author John Duchi
 */
public class Tank implements SoarAgent
{
  protected Kernel kernel = null;
  protected Agent agent = null;
  protected TankSoarInputLink iLink = null;

	/** <code>int</code> to represent a move forward. */
	public static final int FORWARD = 0;
	/** <code>int</code> to represent a move to the right. */
	public static final int RIGHT = 1;
	/** <code>int</code> to represent a move backward. */
	public static final int BACKWARD = 2;
	/** <code>int</code> to represent a move to the left. */
	public static final int LEFT = 3;
	/** <code>int</code> to represent a turn to the right. */
	public static final int TURNRIGHT = 4;
	/** <code>int</code> to represent a turn to the left. */
	public static final int TURNLEFT = 5;
	
	/** Clock variable incremented each time a round of decision making is done. */
	private int TotalTankDecisionCount = 0;
  
  /** <code>int</code> to be concatenated onto the end of the production name, 
   to ensure that all agents have unique names.  Is incremented during each use */
  private static int agentNumber = 0;
  
  /** This indicates whether a Soar agent has made at least one decision. */
  private boolean hasBeenRun = false;
  
  public void flagAsRun(){ hasBeenRun = true; } 

	/** <code>int</code> to represent the direction north.
	 * Same as <code>Location.North</code> */
	public static final int NORTH = Location.North;
	/** <code>int</code> to represent the direction east.
	 * Same as <code>Location.East</code> */
	public static final int EAST = Location.East;
	/** <code>int</code> to represent the direction south.
	 * Same as <code>Location.South</code> */
	public static final int SOUTH = Location.South;
	/** <code>int</code> to represent the direction west.
	 * Same as <code>Location.West</code> */
	public static final int WEST = Location.West;
	/** The random number generator for all <code>Tank</code>s. May be seeded through
	 * the method <code>seedRandomNumber</code>. */
	private static Random tankRandom = null;
	
	/** The <code>ArrayList</code> containing all the <code>TankListener</code>s listening
	 * to this <code>Tank</code>. */
	private ArrayList myListeners = null;
	/** The <code>TankSoarJControl</code> running the simulation in which this
	 * <code>Tank</code> exists. */
	protected TankSoarJControl myTC = null;
	
	/** The <code>String</code> name of the color of this <code>Tank</code>. */
	protected String myColorName = null;
	/** The <code>Location</code> of this <code>Tank</code>.*/
	protected Location myLocation = null;
	/** The direction the <code>Tank</code> is facing. Defaults to <code>Tank.NORTH</code>.*/
	protected int myDirection = NORTH;
	
	/** Defines the health points the <code>Tank</code> loses every turn. */
	public static final int HealthLostPerTurn = 0;
	/** Defines the energy points the <code>Tank</code> loses every turn its shields are on. */
	public static final int ShieldEnergyPerTurn = 20;
	/** Defines the energy points the <code>Tank</code> loses every turn for every distance
	 * increment its radar covers. */
	public static final int RadarEnergyPerTurn = 1;
	/** The initial number of missiles a <code>Tank</code> starts with. */
	public static final int InitMissileCount = 15;
	/** The initial and maximum health a <code>Tank</code> can have. */
	public static final int MaxHealth = 1000;
	/** The initial and maximum energy a <code>Tank</code> can have. */
	public static final int MaxEnergy = 1000;
	/** The initial radar-setting, that is, the distance to which the <code>Tank</code>'s
	 * radar sees. */
	public static final int InitRadarSet = 1;
  /** The width of the radar "cone" */
  public static final int RadarWidth = 3;
	/** The maximum setting of the radar. */
	public static final int MaxRadar = 14;
	/** The damage a <code>Tank</code> takes when it is crashed into or it crashes into something. */
	public static final int CrashHealthDamage = 100;
	/** The maximum distance to which a <code>Tank</code> can smell. */
	public static final int MaxSmell = 28;
	/** The maximum distance to which a <code>Tank</code> can hear other <code>Tank</code>s moving. */
	public static final int MaxHear = 7;
	
	/** The number of missiles currently carried by this <code>Tank</code>. */
	private int myMissiles = InitMissileCount;
	/** The amount of energy this <code>Tank</code> currenlty has. */
	private int myEnergy = MaxEnergy;
	/** The amount of health this <code>Tank</code> currenlty has. */
	private int myHealth = MaxHealth;
	/** The number of moves this <code>Tank</code> has taken, incremented every time the
	 * <code>Tank</code> moves. */
	private int myMoveCount = 0;
	/** The score of this <code>Tank</code>. */
	private int myScore = 0;
	/** The amount the score of this <code>Tank</code> changed during its last turn. */
	private int myScoreChange = 0;
	/** The power to which our radar is set. */
	protected int radarSetting = InitRadarSet;
	/** The distance the radar last reached before hitting an obstacle. */
	private int radarDistance = InitRadarSet;
	/** Whether the radar is on or off. If on, equal to <code>true</code>. */
	protected boolean radarOn = true;
	/** Whether the shields are on or off. If on, equal to <code>true</code>. */
	protected boolean shieldsOn = false;
	/** Parameter indicating whether this <code>Tank</code> has been resurrected.
	 * If <code>true</code>, this <code>Tank</code> has been resurrected at least
	 * once. <---though Duchi says "at least once", it's unclear if this applices
   * only to whether this was resurrected _last turn_*/
	private boolean resurrected = false;
	/** Whether this <code>Tank</code> successfully completed actual movement during
	 * its last decision. */
	private boolean movedLastTurn = false;
  /** Whether this <code>Tank</code> rotated during its last decision. */    
  private boolean rotatedLastTurn = false;
	/** The world count of the turn when this <code>Tank</code> was last hit by a missile. */
	private int hitWithMissile = -1;
	/** The <code>Tank</code> that most recently shot this <code>Tank</code> with a missile. */
	private Tank lastEnemy = null;
	/** A 3 * <code>MaxRadar</code>+1 array containing all the squares this <code>Tank</code>
	 * can see with its radar. */
	private Object[][] myRadarSights = new Object[3][MaxRadar + 1];
	/** A <code>MaxRadar</code> length array containing the locations that this <code>Tank</code>'s
	 * radar traversed, with <code>null</code>s for unseen indices in the array. That is, it
	 * will contain <code>null</code>s for every <code>radarLocations[i]</code>, where
	 * i > <code>radarSetting</code>. */
	private Location[] radarLocations = new Location[MaxRadar];
	/** The world count of the turn when the radar was last calculated. Allows <code>Tank</code>
	 * to avoid recalculating radar unnecessarily. */
	private int lastRadarCalculationCount = -1;
	/** The <code>Location</code> of this <code>Tank</code> when the radar was last calculated.
	 * Allows <code>Tank</code> to avoid recalculating radar unnecessarily. */
	private Location lastRadarCalculationLocation = new Location(-1, -1);
	/** The <code>TankOutputInfo</code> that is this <code>Tank</code>'s most recently made
	 * decision; used to inform listeners of the <code>Tank</code>'s decisions. */
	protected TankOutputInfo lastDecision = null;
	/** The last <code>Location</code> at which this <code>Tank</code> existed--that is,
	 * its previous turn's <code>Location</code>. */
	protected Location lastLocation = null;
	/** <code>true</code> if the <code>Tank</code> has finished its moves for the turn. */
	private boolean finishedMoves = false;

	private String productionPath;

	/**
	 * Creates a new Tank at specified location, of specified color name, and
	 * facing north.
	 * @param colorName The String name of the color of tank to create.
	 * @param loc The Location at which this Tank has been created.
	 * @param tc The TankSoarJControl running the simulation.
	 */
	public Tank(String colorName, Location loc, TankSoarJControl tc){
		myColorName = colorName;
		myLocation = loc;
		myTC = tc;
    //In the Tcl implementation of TankSoar, the Tank starts with its radar on, setting 1
    this.radarOn = true;
    this.radarSetting = 1;
	}

  public void cleanUp()
  {
TankSoarLogger.log("Cleaning up the " + agent.GetAgentName() + " agent now");
    agent.GetKernel().DestroyAgent(agent);
TankSoarLogger.log("Kernel should have cleaned the real agent here...");
    agent = null;
  }
	/**
	 * Gives to the caller the TankSoarJControl that is running the simulation
	 * of which this tank is a part.
	 * @return The TankSoarJControl running the simulation.
	 */
	public TankSoarJControl getTankSimulationControl(){
		return(myTC);
	}

	public void setKernel(Kernel newKernel)
	{
	  kernel = newKernel;
	  if(kernel == null)
	    TankSoarLogger.log("\t\t<><><>Null kernel was passed in to Tank class<><><>");
	  //else
	  //  TankSoarLogger.log("\t\tgood kernel passed in...");
	}

  /**
   * Loads the specified productions into Soar
   *
   * */
	public void attachSoarCode(File toAttach)
	{   
		//TankSoarLogger.log("Tank::attachSoarCode() called....");
    if(kernel == null)
    {
   	  TankSoarLogger.log("\t<><><>Kernel was null in Tank class, couldn't create agent<><><>");
   	  return;
    }
    String agentName = toAttach.getName();
    //use the production name, stopping before '.'
    agentName = getColorName() + "-" + agentName.substring(0, agentName.indexOf('.'));
    agentName += String.valueOf(agentNumber++);//ensure that tank name is unique
    //create the new agent.
    agent = kernel.CreateAgent(agentName);
    if(agent == null)
    {
      TankSoarLogger.log("\t\tthe creation of the agent FAILED!!!");
      TankSoarLogger.log("\t\tand the reason was: " + kernel.GetLastErrorDescription());
    }

    iLink = new TankSoarInputLink(agent, this);
    if(iLink == null)
      TankSoarLogger.log("\t\tthe creation of the input link FAILED!!!");
    String[] pathAndFilename = toAttach.getPath().split(agentName);
    String pathOnly = pathAndFilename[0];

    productionPath = toAttach.getAbsolutePath();

    if(agent.LoadProductions(productionPath))
      TankSoarLogger.log("Productions for agent: " + agentName + " loaded successfully.");
    else
    {
      TankSoarLogger.log("\tProductions for agent: " + agentName + " did not load correctly.");
      TankSoarLogger.log("\tReason: " + agent.GetLastErrorDescription());
      TankSoarLogger.log("\tand with a commandline result of: " + kernel.GetLastCommandLineResult());
    }
	}

  public String getProductionPath() 
  {
  	return productionPath;	
  }

  public void debugPrintInputLink()
  {
    TankSoarLogger.log("debug printing input link...");
    TankSoarLogger.log(agent.ExecuteCommandLine("print --depth 10 i2"));
  }
  
  public void debugPrintOutputLink()
  {
    TankSoarLogger.log("debug printing output link...");
    TankSoarLogger.log(agent.ExecuteCommandLine("print --depth 10 i3"));
  }
	/**
	 * Gives caller a <code>3*(MaxRadar+1)</code> array of containing what this <code>Tank</code> can see on
	 * its radar. Locations that this Tank cannot see are returned as <code>null</code>s,
	 * otherwise they are whatever is in the simulation. Note that the radar will travel
	 * through other <code>Tank</code>s, revealing what is behind them.
	 * <p>Also calculates the distance that the radar has travelled unobstructed during the
	 * turn. If client is trying to set the radar-distance, make sure to call this
	 * method first.</p>
	 * @return An array containing all the things this <code>Tank</code> can see. It is an 
	 * <code>Object[3][MaxRadar+1]</code> array.
	 */
	public Object[][]getRadarSights()
  {
//TankSoarLogger.log("\tTank::getRadarSights called...");
		if(lastRadarCalculationCount == myTC.getWorldCount() && lastRadarCalculationLocation.equals(myLocation))
      return(myRadarSights);
		int nullStartIndex = 0;
		for(int i = 0; i < radarLocations.length; ++i)
      radarLocations[i] = null;
		Location center = myLocation;
		if(radarOn)
    {
			for(int y = 0; y < radarSetting+1; ++y, center = center.getAdjacent(myDirection))
      {
				nullStartIndex = y;
				if(y >= 1)
        {
					//Keeps a record of the locations that this Tank's radar has travelled through.
					radarLocations[y-1] = center;
				}
				myRadarSights[1][y] = myTC.getLocationContents(center);
				if(myRadarSights[1][y] instanceof TSWall)
        {
					myRadarSights[0][y] = null;
					myRadarSights[2][y] = null;
					if(y < MaxRadar) radarLocations[y] = null;
					break;
				}
				myRadarSights[0][y] = myTC.getLocationContents(
						center.getAdjacent(getTurnDirection(myDirection, TURNLEFT)));
				myRadarSights[2][y] = myTC.getLocationContents(
						center.getAdjacent(getTurnDirection(myDirection, TURNRIGHT)));
			}
		} else {
			myRadarSights[0][0] = null;
			myRadarSights[1][0] = null;
			myRadarSights[2][0] = null;
			radarLocations[0] = null;
		}
		if(myRadarSights[1][nullStartIndex] instanceof TSWall){
			radarDistance = (0 > (nullStartIndex - 1) ? 0:(nullStartIndex - 1));
		} else {
			radarDistance = radarSetting;
		}
		for(int i = nullStartIndex + 1; i < MaxRadar + 1; ++i){
			for(int j = 0; j < 3; ++j){
				myRadarSights[j][i] = null;
				if(i < MaxRadar) radarLocations[i] = null;
			}
		}
		lastRadarCalculationCount = myTC.getWorldCount();
		lastRadarCalculationLocation = myLocation;
//TankSoarLogger.log("\tTank::getRadarSights ended.");
		return(myRadarSights);
	}
	
	/**
	 * Gives to the caller an array containing all the locations directly forward
	 * of this tank that the radar of the tank is hitting. Note that this array is
	 * rarely reallocated, so its address in memory does not change.
	 * @return A <code>MaxRadar</code> length array whose elements are Locations that the
	 * radar hits, covering [Location in front of tank, ... , Last enterable location radar hits,
	 * <code>null</code>, ... , <code>null</code>].
	 * That is, walls are not included, but the radar does travel through <code>Tank</code>s.
	 * Elements of the Simulation that cannot be entered will have a null value in the array,
	 * thus only elements before the first null matter.
	 */
	public Location[]getRadarLocations(){
		if(lastRadarCalculationCount == myTC.getWorldCount() && lastRadarCalculationLocation.equals(myLocation)) return (radarLocations);
		getRadarSights();
		return(radarLocations);
	}
	
	/**
	 * Gives to caller the distance the radar travelled unobstructed in the Tank's last evaluation
	 * of its radar. Before calling this method, to ensure accurate result, call getRadarSights().
	 * @return The effective distance of the radar the last time it was used, that is,
	 * the distance it reached before it was blocked. If there were no obstacles,
	 * this is the same as the radar setting.
	 */
	public int getRadarDistance(){
		return(radarDistance);
	}
	
	/**
	 * Gives to caller the power setting of the radar.
	 * @return The distance that the radar has been set to using the radar-power output command.
	 */
	public int getRadarSetting(){
		return(radarSetting);
	}

	/**
	 * Sets the radar power setting to be that specified in the call.
	 * @param newSet The new setting for the radar output power.
	 */
	public void setRadar(int newSet){
		if(newSet < 0 || newSet > MaxRadar) return;
		radarSetting = newSet;
	}
	
	/**
	 * Gives to the caller the current health points of this <code>Tank</code>.
	 * @return The <code>int</code> current health of this <code>Tank</code>.
	 */
	public int getHealth(){
		return(myHealth);
	}
	
	/**
	 * Gives to the caller the current energy points of this <code>Tank</code>.
	 * @return The <code>int</code> current energy of this <code>Tank</code>.
	 */
	public int getEnergy(){
		return(myEnergy);
	}
	
	/**
	 * Gives to the caller the current score of this <code>Tank</code>.
	 * @return The <code>int</code> current score of this <code>Tank</code>.
	 */
	public int getScore(){
		return(myScore);
	}
	
	/**
	 * Restarts the tank at the given Location, giving it full health, energy, and initializing
	 * missiles and radar. Also notifies the locations from which the tank is being removed and
	 * to which it is being added in the simulation that the Tank will be in (or out) of those
	 * locations.
	 * @param loc The Location at which this tank has been resurrected. Expected to be
	 * an enterable square in the Simulation.
	 */
	public void resurrect(Location loc){
		EnterableSquare es = (EnterableSquare)myTC.getLocationContents(myLocation);
		es.removeOccupant(this);
		myLocation = loc;
		try{
			((EnterableSquare)myTC.getLocationContents(myLocation)).addOccupant(this);
		} catch(ClassCastException e){
			TankSoarLogger.log("Tank::resurrect: Not passed enterable location!");
		}
		reset();
		resurrected = true;
	}
	
	/**
	 * In the <code>Tank</code> case, returns a <code>String</code> containing information
	 * about the <code>Tank</code>'s score, color, location, mssile count, health, and energy.
	 *  {@inheritDoc}
	 */
	public String toString(){
		return("Tank " + myColorName + " @ location " + myLocation + "\n\tScore: " + myScore
				+"\tMissiles: " + myMissiles + "\tHealth: " + myHealth+ "\tEnergy: " + myEnergy);
	}
	
	/**
	 * Method called to notify this <code>Tank</code> that it has picked up the specified
	 * number of missiles.
	 * @param numMissiles The number of missiles the <code>Tank</code> has picked up.
	 */
	public void gotMissiles(int numMissiles){
		if(numMissiles < 0) return;
		myMissiles += numMissiles;
		fireMissileCountChangeNotification();
	}
	
	/**
	 * Gives caller the number of missiles this <code>Tank</code> is currently carrying.
	 * @return The number of missiles this <code>Tank</code> has.
	 */
	public int getMissileCount(){
		return(myMissiles);
	}
	
	/**
	 * Gives caller the direction this <code>Tank</code> is facing.
	 * @return The direction this <code>Tank</code> is facing, either
	 * <code>Tank.NORTH</code>, <code>Tank.EAST</code>, <code>Tank.SOUTH</code>,
	 * or <code>Tank.WEST</code>.
	 */
	public int getDirection(){
		return(myDirection);
	}
	
	/**
	 * Sets the direction of this <code>Tank</code>.
	 * @param directionToSet The direction desired for the <code>Tank</code> to be facing.
	 * <p>Possible values are <code>Tank.NORTH</code>, <code>Tank.EAST</code>,
	 * <code>Tank.SOUTH</code>, or <code>Tank.WEST</code>.
	 */
	public void setDirection(int directionToSet){
		myDirection = (directionToSet < 0 ? myDirection:directionToSet%4);
		fireRotationChangedNotification();
	}
	
	/**
	 * Tells caller whether this <code>Tank</code> has its shields on.
	 * @return <code>true</code> if the shields are on, <code>false</code> otherwise.
	 */
	public boolean getShieldsOn(){
		return(shieldsOn);
	}
	
	/**
	 * Tells caller whether this <code>Tank</code> has been resurrected.
	 * @return <code>true</code> if the <code>Tank</code> has been resurrected, <code>false</code> otherwise.
	 */
	public boolean getResurrected(){
		return(resurrected);
	}
	
	public String getColorName(){
		return (myColorName);
	}

	public Location getLocation(){
		return myLocation;
	}
	
	/**
	 * Sets this <code>Tank</code>'s <code>Location</code> to be that specified. Also notifies
	 * any <code>EnterableSquare</code>s being entered or left of the change brought about
	 * by calling this method.
	 * @param toSet The <code>Location</code> being set as the new <code>Location</code> for this
	 * <code>Tank</code>.
	 */
	public void setLocation(Location toSet){
		myTC.getLocationContents(myLocation);
		if(!myLocation.equals(toSet)){
			if(myTC.getLocationContents(myLocation) instanceof EnterableSquare){
				((EnterableSquare)myTC.getLocationContents(myLocation)).removeOccupant(this);
				myTC.fireLocationChangedNotification(myLocation);
			}
			try{
				myLocation = toSet;
				((EnterableSquare)myTC.getLocationContents(myLocation)).addOccupant(this);
			}catch(ClassCastException notEnterableLocation){
				System.out.println(notEnterableLocation + "\nNot an enterable location");
				return;
			}
			myTC.fireLocationChangedNotification(myLocation);
			fireLocationChangedNotification();
		}
	}
	
	public String getName(){
		return(myColorName);
	}
	
	/**
	 * Resets all the values of the <code>Tank</code> to be their initial values: score, move count,
	 * missile count, energy, the resurrected parameter, shields, radar setting, and health.
	 * Fires a notification that this <code>Tank</code> has been resurrected, because in being
	 * reset, enough has changed to justify this (and this is called by <code>resurrect()</code>).
	 * {@inheritDoc}
	 */
	public void reset(){
		myScore = 0;
		myMoveCount = 0;
		myMissiles = InitMissileCount;
		shieldsOn = false;
		myEnergy = MaxEnergy;
		myHealth = MaxHealth;
		radarSetting = InitRadarSet;
		resurrected = false;
		lastRadarCalculationCount = -1;
		lastTurnSensorsCalculated = -1;
		lastRadarCalculationLocation = new Location(-1, -1);
		fireResurrectedNotification();
	}
	
	/**
	 * Turns the <code>Tank</code>'s radar on, firing notifications if it is switched from being off.
	 */
	public void turnRadarOn(){
		if(!radarOn){
			radarOn = true;
			fireRadarSwitchNotification();
		}
	}

	/**
	 * Turns the <code>Tank</code>'s radar off, firing notifications if it is switched from being on.
	 */
	public void turnRadarOff(){
		if(radarOn){
			radarOn = false;
			fireRadarSwitchNotification();
		}
	}
	
	/**
	 * Tells caller whether this Tank has been hit by a missile in the past turn.
	 * @return <code>true</code> if the Tank has been hit by a missile, <code>false</code>
	 * otherwise.
	 */
	public boolean wasHitWithMissile(){
		return(hitWithMissile == myTC.getWorldCount());
	}
	
	/**
	 * Method invoked when this <code>Tank</code> has been hit by a <code>FlyingMissile</code>.
	 * Evaluates the consequences of being hit, including checking whether the shields were on,
	 * managing health and energy when hit by a missile, and notifying the shooter of the
	 * <code>FlyingMissile</code> that it has successfully shot (or killed) another <code>Tank</code>.
	 * Also fires a notification to the listener's that this <code>Tank</code> has been hit.
	 * @param hitter The <code>FlyingMissile</code> that has hit this <code>Tank</code>.
	 */
	public void hitWithMissile(FlyingMissile hitter){
		hitWithMissile = myTC.getWorldCount();
		lastEnemy = hitter.getShooter();
		if(myTC.getLocationContents(myLocation) instanceof EnergySquare
				|| myTC.getLocationContents(myLocation) instanceof HealthSquare){
			myHealth = 0;
			lastEnemy.killedOtherTank();
			myTC.tankKilled(this);
			return;
		}
		if(shieldsOn){
			myEnergy = ((myEnergy - FlyingMissile.EnergyTaken  < 0) ? 0:myEnergy - FlyingMissile.EnergyTaken);
			fireEnergyChangedNotification();
		} else {
			myHealth -= FlyingMissile.DamageDone;
			fireMissileHitNotification();
			if(myHealth <= 0){
				lastEnemy.killedOtherTank();
				myScore -= 2;
				myTC.tankKilled(this);
			} else {
				myScore -= 1;
				myScoreChange -= 1;
				lastEnemy.shotOtherTank();
				fireScoreChangedNotification();
				fireHealthChangedNotification();
			}
		}
	}

	/**
	 * Method called when this Tank has shot, but not killed, another tank.
	 */
	public void shotOtherTank(){
		myScore += 2;
		myScoreChange += 2;
		fireScoreChangedNotification();
	}
	
	/**
	 * Tells caller whether or not this <code>Tank</code> completed a successful movement
	 * during its last turn.
	 * @return <code>true</code> if the <code>Tank</code> successfully moved, <code>false</code> otherwise.
	 */
	public boolean movedLastTurn(){
		return movedLastTurn;
	}
    
  public boolean rotatedLastTurn(){
    return rotatedLastTurn;
  }
		
	/**
	 * Tells caller whether or not this <code>Tank</code> has completed its decision cycle for the turn.
	 * @return <code>true</code> if the <code>Tank</code> is done with this turn, <code>false</code> otherwise.
	 */
	public boolean finishedTurn(){
		return(finishedMoves);
	}
	
	/**
	 * Method called when a <code>Tank</code> has finished its turn.
	 * @param movedSuccessfully Tells whether the <code>Tank</code> actually moved this turn--if
	 * <code>true</code>, will set the <code>movedLastTurn()</code> method to return <code>true</code>.
	 */
	public void turnFinished(boolean movedSuccessfully){
		finishedMoves = true;
		movedLastTurn = movedSuccessfully;
		moveTaken();
		fireDecisionMadeNotification();
	}
	
	/**
	 * Method called when this <code>Tank</code> has killed another <code>Tank</code>.
	 */
	public void killedOtherTank(){
		myScore += 3;
		myScoreChange += 3;
		fireScoreChangedNotification();
	}

	/**
	 * Notification called on this <code>Tank</code> when it successfully fires a missile.
	 * Simulation control takes care of calling it.
	 */
	public void missileFired(){
		myMissiles -= 1;
		if(myMissiles < 0) myMissiles = 0;
		fireMissileCountChangeNotification();
	}
	
	/**
	 * Notification called on this <code>Tank</code> when it crashes into an obstacle (or other
	 * <code>Tank</code>). The simulation control takes care of calling it.
	 */
	public void crashed(){
		myHealth -= CrashHealthDamage;
		if(myHealth <= 0){
			myHealth = 0;
			myTC.tankKilled(this);
		}
		fireHealthChangedNotification();
	}
	
	/**
	 * Sets the <code>Tank</code>'s location to be its location on the previous turn. Also
	 * gives the caller the <code>Location</code> that this <code>Tank</code> was at the previous turn.
	 * Removes this <code>Tank</code> from its current square and places it back in the previous
	 * <code>Location</code> it occupied in the simulation.
	 * @return The <code>Tank</code>'s <code>Location</code> before this step in the simulation 
	 * (and now its current <code>Location</code>).
	 */
	public Location undoLastMove(){
		if(!movedLastTurn) return(myLocation);
		((EnterableSquare)myTC.getLocationContents(myLocation)).removeOccupant(this);
		myTC.fireLocationChangedNotification(myLocation);
		myLocation = lastLocation;
		((EnterableSquare)myTC.getLocationContents(myLocation)).addOccupant(this);
		myTC.fireLocationChangedNotification(myLocation);
		fireLocationChangedNotification();
		return(lastLocation);
	}
	
	/**
	 * Gives to caller the <code>Location</code> that this <code>Tank</code> was at the previous
	 * turn; useful for checking crashes and undoing moves.
	 * @return The <code>Location</code> occupied by this <code>Tank</code> previously.
	 */
	public Location getLocationPrevious(){
		return(lastLocation);
	}
  /**
   * <p>This method returns an instance of <code>TankInputInfo</code>.
   * It also determines whether the decision has been to move or rotate, and
   * if has been solely to rotate (with no movement), will rotate the <code>Tank</code>.
   * It also evaluates whether the decision has turned on radar, turned off radar,
   * turned on or off shields, or further set the radar.
   * <p><code>TankSoarJControl</code> takes care of movement and firing missiles, since
   * missiles need to be placed in the simulation to fly, and will
   * notify the <code>Tank</code> that it has fired a missile. Incidentally, the<code>
   * TankSoarJControl</code> will check with the <code>Tank</code> to make sure it has
   * missiles available. The <code>TankSoarJControl</code> will also tell the
   * <code>Tank</code> when it has moved to a new location.</p>
   * {@inheritDoc}
   */	
  final public Object makeDecision()
  {
    //TankSoarLogger.log("Tank::MakeDecision called....");    
    finishedMoves = false;
   
    //getDecision will parse the output link and place all of the information about the 
    //current decision into this TankOutputInfo
    TankOutputInfo decision = getDecision();
    int oldDirection = myDirection;
    lastLocation = myLocation;
    if(decision.move != null)//Definitely moved, possibly rotated
    {
      movedLastTurn = true;
      if(decision.rotate != null)
        rotatedLastTurn = true;
      else
        rotatedLastTurn = false;
    }
    else if(decision.move == null && decision.rotate != null)//didn't move, did rotate
    {
      if(decision.rotate.direction.startsWith("l")){
        myDirection = (myDirection+3)%4;
      } else if(decision.rotate.direction.startsWith("r")){
        myDirection = (myDirection + 1)%4;
      }
      rotatedLastTurn = true;
      movedLastTurn = false;
      fireRotationChangedNotification();
    } 
    else//didn't move, didn't rotate
    {
      rotatedLastTurn = false;
      movedLastTurn = false;
      myDirection = oldDirection;
    }
    if(decision.shields != null){
      if(decision.shields.Switch == TankOutputInfo.on && !shieldsOn){
        shieldsOn = true;
      } else if(decision.shields.Switch == TankOutputInfo.off && shieldsOn){
        shieldsOn = false;
      }
    }
    if(decision.radar != null){
      if(decision.radar.Switch == TankOutputInfo.on && !radarOn){
        radarOn = true;
        fireRadarSwitchNotification();
      } else if(decision.radar.Switch == TankOutputInfo.off && radarOn){
        radarOn = false;
        fireRadarSwitchNotification();
      }
    }
    if(decision.radar_power != null && decision.radar_power.setting != radarSetting && radarOn){
      radarSetting = decision.radar_power.setting;
      fireRadarChangedNotification();
    }
    lastDecision = decision;
    return(decision);
  }

	/**
	 * Method invoked when this <code>Tank</code> has finished its turn. It is called by
	 * <code>turnFinished(boolean movedSuccessfully)</code> when it happens.
	 * This method increments the move count and total decision counts of all the
	 * <code>Tank</code>s, calculates the score change for the turn not due to missile
	 * firing or being hit by missiles, and calculates the new score of the <code>Tank</code>,
	 * notifying <code>myTC</code> if this <code>Tank</code> has won. Also calculates
	 * increases in health or energy due to presence of a <code>HealthSquare</code> or
	 * <code>EnergySquare</code>, and incrementing missile stores if <code>Tank</code>
	 * is on a <code>MissileSquare</code>.
	 */
	private void moveTaken(){
		if(movedLastTurn){
			myMoveCount++;
		}
		TotalTankDecisionCount++;
		myScoreChange = 0;
		myHealth -= HealthLostPerTurn;
		fireHealthChangedNotification();
		boolean energyChanged = radarOn || shieldsOn;
		if(radarOn){
			myEnergy -= radarSetting*RadarEnergyPerTurn;
		}
		myEnergy -= (shieldsOn ? ShieldEnergyPerTurn:0);
		Object o = myTC.getLocationContents(myLocation);
		if(o instanceof HealthSquare){
			myHealth = ((myHealth += HealthSquare.HealthPerTurn) > MaxHealth ? MaxHealth:myHealth);
			fireHealthChangedNotification();
		} else if(o instanceof EnergySquare){
			energyChanged = true;
			myEnergy = ((myEnergy += EnergySquare.EnergyPerTurn) > MaxEnergy ? MaxEnergy:myEnergy);
		} else if(o instanceof TSEmpty && ((TSEmpty)o).containsMissiles()){
			((TSEmpty)o).getMissileContainer().pickUpMissiles(this);
		}
		if(myEnergy <= 0){
			myEnergy = 0;
			if(radarOn){
				radarOn = false;
				fireRadarSwitchNotification();
			}
			if(shieldsOn){
				shieldsOn = false;
			}
		}
		if(energyChanged) fireEnergyChangedNotification();
		if(myHealth <= 0){
			myScore -= 2;
			myScoreChange -= 2;
			myTC.tankKilled(this);
		}
		if(myScore >= TankSoarJControl.FinalTankScore){
			myTC.tankWon(this);
		}
	}
  
  /**
   * Ensure that the Tank's input link has the most current World state,
   * so that if the Kernel starts running the agent, everything will be there,
   * and also in the case of TankSoarJControl starting the run.
   */
  public void getWorldState()
  {
    if(lastTurnSensorsCalculated != myTC.getWorldCount())
    {
      fillSensors(tankSensors);
    }
	
    // DJP: Addition to send current state over to Redux
    if (TankXML.kBroadcastState)
    	TankXML.SendInputToRedux(agent, tankSensors) ;
    
    iLink.doAllUpdates(tankSensors);
  }
	/**
	 * The workhorse function that actually does the computation of the next decision the
	 * <code>Tank</code> will make. Any subclasses of <code>Tank</code> should implement this method.
	 * Called by <code>Tank.makeDecision()</code>, which initializes <code>sensors</code>.
	 * @param sensors An instance of <code>TankInputInfo</code> that has already had its fields initialized
	 * to the correct values as seen by the <code>Tank</code>.
	 * @return An <code>TankOutputInfo</code> associated with one decision made by the <code>Tank</code>.
	 * @see Tank#makeDecision()
	 */
	protected TankOutputInfo getDecision()
  {
    //TankSoarLogger.log("Entered Tank::getDecision for " + agent.GetAgentName() + ".  About to process output link...");
		TankOutputInfo result = new TankOutputInfo();
    //Now Soar has made decisions to take some number of actions.
    processOutputLink(result);
		return (result);
	}
  
  public void RunTilOutput()
  {
    TankSoarLogger.log(agent.GetAgentName() + " running self til output.");
    agent.RunSelfTilOutput();//This causes a Soar kernel event to be fired, which moves flow
    //of control elsewhere....
  }
    
  /**processOutputLink reads the number of commands that an agent has placed on the
   *  output link after running.  When an Id is read, the appropriate processXXXX function is called.
   * @param info
   */
  protected void processOutputLink(TankOutputInfo info)
  {
    //TankSoarLogger.log("Tank " + agent.GetAgentName() + " processing its output link...");
    Identifier targetId = null;
    boolean goodOutput = false;
    boolean commandMissing = false;

    Identifier oLink = agent.GetOutputLink();
    //TankSoarLogger.log("Printing OLink before processing it<><><>");
    ///debugPrintOutputLink();

    int numCommands = agent.GetNumberCommands();

    for(int commandNum = 0; commandNum < numCommands; ++commandNum)
    {
      Identifier commandId = agent.GetCommand(commandNum);
      String commandName = commandId.GetAttribute();
      //TankSoarLogger.log("\t\t\t\tCommand name is: " + commandName + " for entry at: " + commandNum);
      if(commandName.equals(TankOutputInfo.MOVE))
        processMove(commandId, info);
      else if(commandName.equals(TankOutputInfo.ROTATE))
        processRotate(commandId, info);
      else if(commandName.equals(TankOutputInfo.FIRE))
        processFire(commandId, info);
      else if(commandName.equals(TankOutputInfo.RADAR))
        processRadarToggle(commandId, info);
      else if(commandName.equals(TankOutputInfo.RADAR_POWER))
        processRadarPower(commandId, info);
      else if(commandName.equals(TankOutputInfo.SHIELDS))
        processShieldToggle(commandId, info);
      else
        TankSoarLogger.log("\t\t\t\tThis command wasn't recognized: " + commandName);
    }//for existing commands

    agent.ClearOutputLinkChanges();
  }
  //Reads off the direction to move, adds status complete. Fails silently
	protected void processMove(Identifier moveId, TankOutputInfo info)
  {
    String paramValue = moveId.GetParameterValue(TankOutputInfo.DIRECTION);
    if(paramValue != null)
    {
      info.move = new TankOutputInfo.MoveOutput(paramValue);
      moveId.AddStatusComplete();
    }
  }
  //Reads off the direction to rotate, adds status complete. Fails silently
  protected void processRotate(Identifier rotateId, TankOutputInfo info)
  {
    String paramValue = rotateId.GetParameterValue(TankOutputInfo.DIRECTION);
    if(paramValue != null)
    {
      info.rotate = new TankOutputInfo.RotateOutput(paramValue);
      rotateId.AddStatusComplete();    
    }
  }

  /**Reads off the weapon to fire, adds status complete. Fails silently
   * 
   * @param fireId
   * @param info
   */
  protected void processFire(Identifier fireId, TankOutputInfo info)
  { 
    String paramValue = fireId.GetParameterValue(TankOutputInfo.WEAPON);
    if(paramValue != null)
    {
      info.fire = new TankOutputInfo.FireOutput(paramValue);
      fireId.AddStatusComplete();    
    }
  }
  /**Reads off the toggle value for the radar, adds status complete. Fails silently
   * 
   * @param radarCommandId
   * @param info
   */
  protected void processRadarToggle(Identifier radarCommandId, TankOutputInfo info)
  {
    String paramValue = radarCommandId.GetParameterValue(TankOutputInfo.SWITCH);
    if(paramValue != null)
    {
      boolean radarOn = paramValue.equals(TankOutputInfo.ON) ? true : false;
      info.radar = new TankOutputInfo.RadarOutput(radarOn); 
      radarCommandId.AddStatusComplete();
    }
  }
  /**
   * Reads off the strength of the radar, adds status complete. Fails silently
   */
  protected void processRadarPower(Identifier radarPowerId, TankOutputInfo info)
  {
    String paramValue = radarPowerId.GetParameterValue(TankOutputInfo.SETTING);
    if(paramValue != null)
    {
      info.radar_power = new TankOutputInfo.Radar_PowerOutput(Integer.valueOf(paramValue).intValue());
      radarPowerId.AddStatusComplete();
    }
  }
  /**Reads off the toggle value for the shield, adds status complete. Fails silently
   * 
   * @param shieldId
   * @param info
   */
  protected void processShieldToggle(Identifier shieldId, TankOutputInfo info)
  {
    String paramValue = shieldId.GetParameterValue(TankOutputInfo.SWITCH); 
    if(paramValue != null)
    {
      boolean shieldOn = paramValue.equals(TankOutputInfo.ON) ? true : false;
      info.shields = new TankOutputInfo.ShieldsOutput(shieldOn);
      shieldId.AddStatusComplete();
    }
  }
  /** The <code>TankInputInfo</code> that was the last set of input-link information
	 * calculated by this <code>Tank</code>. */
	private TankInputInfo tankSensors = new TankInputInfo();
	/** The last turn on which <code>tankSensors</code> was calculated. */
	private int lastTurnSensorsCalculated = -1;
	
	/**
	 * Method called by <code>TankSoarJControl</code> when all the <code>Tank</code>s
	 * in a simulation have finished their decisions. Since all the <code>Tank</code>s
	 * are done, fills the <code>TankInputInfo</code> internal to this <code>Tank</code>
	 * with whatever data is available. This method should also be invoked any time
	 * an agent is created or destroyed.
	 */
	public void allTanksFinished(){
		lastTurnSensorsCalculated = myTC.getWorldCount();
		fillSensors(tankSensors);
		fireAllInformationAvailableNotification(tankSensors);
	}
	
	/**
	 * Fills the sensors that are contained in a <code>TankOutputInfo</code> that the
	 * <code>Tank</code> controls. The sensors filled are the blocked sensors,
	 * the clock, the direction of the <code>Tank</code>, health and energy,
	 * the radar, missiles, score, score change, x and y coordinates, shield status,
	 * and whether or not the <code>Tank</code> has been resurrected.
	 * @param sensors The <code>TankInputInfo</code> that is being filled with
	 * the resulting calculations.
	 * @return The <code>TankInputInfo</code> that has been filled with the information
	 * available to this <code>Tank</code>.
	 */
	public TankInputInfo fillSensors(TankInputInfo sensors)
  {
		if(sensors == null) sensors = new TankInputInfo();
		sensors.initializeNonradarSensors();
		myTC.fillSensors(sensors, this);//fills in RWaves and Incoming
		fillBlockedSensor(sensors);
		sensors.clock = TotalTankDecisionCount;
		sensors.random = getNextRandomDouble();
		sensors.direction = myDirection;
		fillHealthEnergy(sensors);
		fillRadar(sensors);
		sensors.missiles = myMissiles;
		sensors.myScore = myScore;
		sensors.scoreChange = myScoreChange;
		sensors.X = myLocation.getX();
		sensors.Y = myLocation.getY();
		sensors.shieldStatus = (shieldsOn ? TankInputInfo.yes:TankInputInfo.no);
		sensors.resurrected = (resurrected ? TankInputInfo.yes:TankInputInfo.no);
		return(sensors);
	}
	
	/**
	 * Calculates whether the <code>Tank</code> is blocked in any direction
	 * using <code>edu.umich.JavaBaseEnvironment.Location.getAdjacent(int)</code> in all
	 * four directions.
	 * @param sensors The <code>TankInputInfo</code> whose blocked sensor is 
	 * being filled with the resulting calculations.
	 */
	private void fillBlockedSensor(TankInputInfo sensors){
		Object o = myTC.getLocationContents(myLocation.getAdjacent(myDirection));
		if(!(o instanceof EnterableSquare) || ((EnterableSquare)o).containsAgent()){
			sensors.blocked.forward = TankInputInfo.yes;
		}
		o = myTC.getLocationContents(myLocation.getAdjacent((myDirection+1)%4));
		if(!(o instanceof EnterableSquare) || ((EnterableSquare)o).containsAgent()){
			sensors.blocked.right = TankInputInfo.yes;
		}
		o = myTC.getLocationContents(myLocation.getAdjacent((myDirection+2)%4));
		if(!(o instanceof EnterableSquare) || ((EnterableSquare)o).containsAgent()){
			sensors.blocked.backward = TankInputInfo.yes;
		}
		o = myTC.getLocationContents(myLocation.getAdjacent((myDirection+3)%4));
		if(!(o instanceof EnterableSquare) || ((EnterableSquare)o).containsAgent()){
			sensors.blocked.left = TankInputInfo.yes;
		}
	}
	
	/**
	 * Sets the health, energy, healthrecharger, and energyrecharger
	 * parameters of that <code>TankInputInfo</code> given to be the data
	 * available to the <code>Tank</code>. If the <code>Tank</code> is
	 * on a <code>HealthSquare</code> or <code>EnergySquare</code>, sets
	 * the healthrecharger or energyrecharger parameters accordingly.
	 * @param sensors The <code>TankInputInfo</code> whose health, energy,
	 * healthrecharger, and energyrecharger parameters are being set.
	 */
	private void fillHealthEnergy(TankInputInfo sensors){
		sensors.health = myHealth;
		if(myTC.getLocationContents(myLocation) instanceof HealthSquare){
			sensors.healthrecharger = TankInputInfo.yes;
		}
    else
      sensors.healthrecharger = TankInputInfo.no;

		sensors.energy = myEnergy;
		if(myTC.getLocationContents(myLocation) instanceof EnergySquare){
			sensors.energyrecharger = TankInputInfo.yes;
		}
    else
      sensors.energyrecharger = TankInputInfo.no;
	}
	
	/**
	 * Sets the radar-related parameters in the given <code>TankInputInfo</code>.
	 * These are the radarDistance, radarSetting, radarStatus, and the array
	 * of radarSights, that is, what is seen by the radar.
	 * @param sensors The <code>TankInputInfo</code> whose radar parameters
	 * are being set.
	 */
	private void fillRadar(TankInputInfo sensors)
  {
		sensors.radarSights = getRadarSights();
		sensors.radarDistance = radarDistance;
		sensors.radarSetting = radarSetting;
		sensors.radarStatus = radarOn;
	}
	
	/**
	 * Gives to the caller the <code>int</code> direction as specified in this class
	 * opposite that passed in.
	 * @param direction The direction whose opposite we wish to find.
	 * @return The opposite direction to that passed in, in reality,
	 * <code>(direction+2)%4</code>.
	 */
	public static int getOppositeDirection(int direction){
		return((direction+2)%4);
	}
	
	/**
	 * Gives to the caller the <code>int</code> direction as specified in this
	 * class in the direction of the turn specified.
	 * @param direction The direction from which we wish to calculate the turn.
	 * @param turn The direction being turned, either <code>Tank.TURNRIGHT</code>
	 * or <code>Tank.TURNLEFT</code>.
	 * @return The direction that the turn leads.
	 */
	public static int getTurnDirection(int direction, int turn){
		if(turn == TURNRIGHT){
			return((direction+1)%4);
		} else if(turn == TURNLEFT){
			return((direction+3)%4);
		}
		return(direction);
	}
	
	/**
	 * Gives caller the <code>Tank</code>-relative direction that the point specified
	 * by x and y lies from this <code>Tank</code>. Uses the largest distance that the
	 * location lies from the <code>Tank</code> along the x or y-axis. If the distances are
	 * the same, defaults to using the y-coordinates as defining the direction.<p>If the
	 * coordinates of this <code>Tank</code>'s location are given as arguments, will return
	 * a direction, but which it is is undefined.</p>
	 * <p>Same as <code>getDirectionFromTank(loc.getX(), loc.getY())</code>.
	 * @param loc The <code>Location</code> whose direction is being queried.
	 * @return The integer value of the direction the (x, y) coordinate is from
	 * this <code>Tank</code>, either <code>FORWARD</code>, <code>RIGHT</code>, 
	 * <code>LEFT</code>, or <code>BACKWARD</code>.
	 * 
	 */
	public int getDirectionFromTank(Location loc){
		return (getDirectionFromTank(loc.getX(), loc.getY()));
	}
	
	/**
	 * Gives caller the <code>Tank</code>-relative direction that the point specified
	 * by x and y lies from this <code>Tank</code>. Uses the largest distance that the
	 * location lies from the <code>Tank</code> along the x or y-axis. If the distances are
	 * the same, defaults to using the y-coordinates as defining the direction.<p>If the
	 * coordinates of this <code>Tank</code>'s location are given as arguments, will return
	 * a direction, but which it is is undefined.</p>
	 * @param x The x-coordinate of the location whose direction is being queried.
	 * @param y The y-coordinate of the location whose direction is being queried.
	 * @return The integer value of the direction the (x, y) coordinate is from
	 * this <code>Tank</code>, either <code>FORWARD</code>, <code>RIGHT</code>, 
	 * <code>LEFT</code>, or <code>BACKWARD</code>.
	 */
	public int getDirectionFromTank(int x, int y){
		int direction = 0, myX = myLocation.getX(), myY = myLocation.getY();
		if(x < myLocation.getX() && ((myX - x) > Math.abs(myY -y))){
			direction = LEFT;
		} else if(x > myX && ((x - myX) > Math.abs(myY - y))){
			direction = RIGHT;
		} else if(y < myY){
			direction = FORWARD;
		} else {
			direction = BACKWARD;
		}
		switch(myDirection){
		case(NORTH):{ break; }
		case(EAST):{ direction = (direction+3)%4; break; }
		case(SOUTH):{ direction = (direction+2)%4; break; }
		case(WEST):{ direction = (direction+1)%4; break; }
		}
		return(direction);
	}
	
	/**
	 * Seeds the random number generator so that deterministically random generation
	 * can be used.
	 */
	public static void seedRandomNumber(){
		if(tankRandom == null){
			tankRandom = new Random(123456);
		} else {
			tankRandom.setSeed(123456);
		}
	}
	
	/**
	 * Seeds the random number generator so that there is no deterministically
	 * random generation, using <code>Random.setSeed(long)</code> with 
	 * <code>System.currentTimeMillis()</code>.
	 */
	public static void unseedRandomNumber(){
		if(tankRandom == null){
			tankRandom = new Random();
		} else {
			tankRandom.setSeed(System.currentTimeMillis());
		}
	}
	/**
	 * Returns the next pseudorandom, uniformly distributed double from 0.0 to 1.0 in
	 * <code>Tank</code>'s random number generator. If the random number
	 * generated has not been seeded, uses a seed based on the current time.
	 * @return A random double greater than or equal to 0.0 but strictly less than 1.0.
	 * @see java.util.Random#nextDouble()
	 */
	public static double getNextRandomDouble(){
		if(tankRandom == null){
			tankRandom = new Random();
		}
		return(tankRandom.nextDouble());
	}
	/*------------- LISTENER STUFF -------------*/
	
	/**
	 * Checks the <code>addees</code> and <code>removees</code> to see if
	 * there are any <code>TankListener</code>s queued to be added
	 * or removed to this <code>Tank</code>'s listeners.
	 */
	private void checkAddRemove(){
		if(addees.size() != 0){
			Iterator iter = addees.iterator();
			while(iter.hasNext()){
				myListeners.add(iter.next());
			}
			addees.clear();
		}
		if(removees.size() != 0){
			Iterator iter = removees.iterator();
			while(iter.hasNext()){
				myListeners.remove(iter.next());
			}
			removees.clear();
		}
	}
	
	/** An <code>ArrayList</code> that serves as a queue to avoid
	 * <code>ConcurrentModificationExceptions</code> by queueing the add
	 * listener calls to this <code>Tank</code> */
	private ArrayList addees = new ArrayList();
	
	/** An <code>ArrayList</code> that serves as a queue to avoid
	 * <code>ConcurrentModificationExceptions</code> by queueing the remove
	 * listener calls to this <code>Tank</code> */
	private ArrayList removees = new ArrayList();

	/**
	 * Adds the specified <code>TankListener</code> to the listeners to this
	 * <code>Tank</code>.
	 * @param toAdd The <code>TankListener</code> to add to the set of
	 * listeners.
	 */
	public void addTankListener(TankListener toAdd){
		if(myListeners == null){
			myListeners = new ArrayList();
		}
		addees.add(toAdd);
	}

	/**
	 * Removes the specified <code>TankListener</code> from the listeners to this
	 * <code>Tank</code>.
	 * @param toRemove The <code>TankListener</code> to remove from the set of
	 * listeners.
	 */
	public void removeTankListener(TankListener toRemove){
		if(myListeners == null) return;
		removees.add(toRemove);
	}

	/**
	 * Notifies all listeners that this <code>Tank</code> has made a decision
	 * in the form of a <code>TankOutputInfo</code>.
	 */
	private void fireDecisionMadeNotification(){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		Iterator iter = myListeners.iterator();
		while(iter.hasNext()){
			((TankListener)iter.next()).decisionMade(this, lastDecision);
		}
	}
	
	/**
	 * Notifies all listeners that this <code>Tank</code> has changed its radar setting
	 * (not called to indicate that the radar has been switched).
	 */
	protected void fireRadarChangedNotification(){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		Iterator iter = myListeners.iterator();
		while(iter.hasNext()){
			TankListener tl = (TankListener)iter.next();
			tl.radarSettingChanged(this);
		}
	}
	
	/**
	 * Notifies all listeners that this <code>Tank</code> has switched its radar
	 * on or off.
	 */
	protected void fireRadarSwitchNotification(){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		Iterator iter = myListeners.iterator();
		while(iter.hasNext()){
			TankListener tl = (TankListener)iter.next();
			tl.radarSwitch(this);
		}
	}
	
	/**
	 * Notifies all listeners that this <code>Tank</code>'s energy has changed.
	 */
	private void fireEnergyChangedNotification(){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		Iterator iter = myListeners.iterator();
		while(iter.hasNext()){
			TankListener tl = (TankListener)iter.next();
			tl.energyChanged(this);
		}
	}
	
	/**
	 * Notifies all listeners that this <code>Tank</code> has been hit with a missile.
	 */
	private void fireMissileHitNotification(){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		for(Iterator iter = myListeners.iterator(); iter.hasNext();){
			TankListener tl = (TankListener)iter.next();
			tl.missileHit(this);
		}
	}
	
	/**
	 * Notifies all listeners that this <code>Tank</code>'s score has changed.
	 */
	private void fireScoreChangedNotification(){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		Iterator iter = myListeners.iterator();
		while(iter.hasNext()){
			TankListener tl = (TankListener)iter.next();
			tl.scoreChanged(this);
		}
	}
	
	/**
	 * Notifies all listeners that this <code>Tank</code>'s <code>Location</code>
	 * has changed (by its own action or otherwise).
	 */
	private void fireLocationChangedNotification(){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		Iterator iter = myListeners.iterator();
		while(iter.hasNext()){
			TankListener tl = (TankListener)iter.next();
			tl.locationChanged(this);
		}
	}
	
	/**
	 * Notifies all listeners that this <code>Tank</code>'s missile count
	 * has changed.
	 */
	private void fireMissileCountChangeNotification(){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		Iterator iter = myListeners.iterator();
		while(iter.hasNext()){
			TankListener tl = (TankListener)iter.next();
			tl.missileCountChanged(this);
		}
		
	}
	
	/**
	 * Notifies all listeners that the direction this <code>Tank</code>
	 * faces has changed.
	 */
	private void fireRotationChangedNotification(){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		Iterator iter = myListeners.iterator();
		while(iter.hasNext()){
			((TankListener)iter.next()).rotationChanged(this);
		}
	}
	
	/**
	 * Notifies all listeners that this <code>Tank</code>, as well as all
	 * the other <code>Tank</code>s in the simulation, has finished its turn.
	 * Allows listeners to update themselves for the beginning of the next turn.
	 * @param sensors A <code>TankInputInfo</code> containing all the information
	 * available to the <code>Tank</code> once the turn ended.
	 */
	protected void fireAllInformationAvailableNotification(TankInputInfo sensors){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		Iterator iter = myListeners.iterator();
		while(iter.hasNext()){
			((TankListener)iter.next()).allDecisionsMade(this, sensors);
		}
	}
	
	/**
	 * Notifies all listeners that this <code>Tank</code>'s health has changed.
	 */
	protected void fireHealthChangedNotification(){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		Iterator iter = myListeners.iterator();
		while(iter.hasNext()){
			((TankListener)iter.next()).healthChanged(this);
		}
	}
	
	/**
	 * Notifies all listeners that this <code>Tank</code> has been resurrected.
	 */
	protected void fireResurrectedNotification(){
		checkAddRemove();
		if(myListeners == null || myTC.hasQuit()) return;
		Iterator iter = myListeners.iterator();
		while(iter.hasNext()){
			((TankListener)iter.next()).resurrected(this);
		}
	}
	
}
