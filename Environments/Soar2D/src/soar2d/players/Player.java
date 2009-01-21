package soar2d.players;

import java.util.Arrays;

import org.apache.log4j.Logger;

import soar2d.Direction;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.config.PlayerConfig;
import soar2d.map.CellObject;

/**
 * @author voigtjr
 *
 * This class represents a generic player object in the world. This
 * can be a Tank, Eater, whatever.
 */
public class Player {
	private static Logger logger = Logger.getLogger(Player.class);

	private String name;	// player name
	private int facingInt;	// what direction I'm currently facing
	private int points;	// current point count
	private String color;	// valid color string
	protected int [] previousLocation = new int [] { -1, -1 };	// where i was last update
	protected boolean moved = false;	// if I moved since last update
	protected String playerID;
	protected PlayerConfig playerConfig;

	private boolean pointsChanged = false;
	private int pointsDelta = 0;

	private double headingRadians = 0;	// heading in radians
	public double getHeadingRadians() {
		return headingRadians;
	}
	public void setHeadingRadians(double heading) {
		this.headingRadians = heading;
	}
	
	/**
	 * @param playerConfig configuration params
	 */
	public Player(String playerID) {
		this.playerID = playerID;
		this.playerConfig = Soar2D.config.playerConfigs().get(playerID);
		
		assert playerConfig.name != null;
		this.name = playerConfig.name;
		
		this.reset();
		
		assert playerConfig.color != null;
		this.color = playerConfig.color;
	}
	
	public String getName() {
		return this.name;
	}
	public void setName(String name) {
		String previous = this.name;
		this.name = name;
		logger.warn(previous + " name changed to: " + this.name);
	}
	
	public int getPoints() {
		return points;
	}
	/**
	 * @param points the new point value
	 * @param comment why the change happened, keep it very brief
	 * 
	 * set the points to a specific value, the comment goes in the log
	 */
	public void setPoints(int points, String comment) {
		pointsChanged = true;
		pointsDelta = points - this.points;
		
		this.points = points;
		if (comment != null) {
			logger.info(this.name + " score set to: " + Integer.toString(this.points) + " (" + comment + ")");
		} else {
			logger.info(this.name + " score set to: " + Integer.toString(this.points));
		}
	}
	/**
	 * @param delta the change in points
	 * @param comment why the change happened, keep it very brief
	 * 
	 * this is a handy function for changing points. puts a message in the log and 
	 * why the change happened
	 */
	public void adjustPoints(int delta, String comment) {
		pointsChanged = (delta != 0);
		pointsDelta = delta;
		
		int previous = this.points;
		this.points += delta;
		if (comment != null) {
			logger.info(this.name + " score: " + Integer.toString(previous) + " -> " + Integer.toString(this.points) + " (" + comment + ")");
		} else {
			logger.info(this.name + " score: " + Integer.toString(previous) + " -> " + Integer.toString(this.points));
		}
	}
	
	public int getMissiles() {
		return 0;
	}
	/**
	 * see Tank
	 */
	public void setMissiles(int missiles, String comment) {
	}
	/**
	 * see Tank
	 */
	public void adjustMissiles(int delta, String comment) {
	}

	public int getEnergy() {
		return 0;
	}
	/**
	 * see Tank
	 */
	public void setEnergy(int energy, String comment) {
	}
	/**
	 * see Tank
	 */
	public void adjustEnergy(int delta, String comment) {
	}

	public int getHealth() {
		return 0;
	}
	/**
	 * see Tank
	 */
	public void setHealth(int health, String comment) {
	}
	/**
	 * see Tank
	 */
	public void adjustHealth(int delta, String comment) {
	}
	
	/**
	 * see Tank
	 */
	public boolean shieldsUp() {
		return false;
	}

	public int getFacingInt() {
		return facingInt;
	}
	/**
	 * @param facingInt make sure it is valid
	 */
	public void setFacingInt(int facingInt) {
		this.facingInt = facingInt;
		this.headingRadians = Direction.radiansOf[getFacingInt()];
	}

	public String getColor() {
		return this.color;
	}
	/**
	 * @param color make sure it is valid
	 */
	public void setColor(String color) {
		String previous = this.color;
		this.color = color;
		logger.warn(this.name + " (" + previous + ") color changed to: " + this.name);
	}
	
	/**
	 * @param world world object so stuff can be looked up
	 * @param location current location since I don't keep track of where I end up
	 * 
	 * called to update the player's sensors and what not, this is basically an input update
	 */
	public void update(int [] location) {
		moved = (location[0] != this.previousLocation[0]) || (location[1] != this.previousLocation[1]);
		if (moved) {
			this.previousLocation = Arrays.copyOf(location, location.length);
		}
	}
	
	/**
	 * called to write sensor data to the input link
	 */
	public void commit(int [] location) {
	}
	
	/**
	 * called to reset player state between runs
	 */
	public void reset() {
		previousLocation = new int [] { -1, -1 };

		if (playerConfig.facing != null) {
			this.setFacingInt(Direction.getInt(playerConfig.facing));
		} else {
			this.setFacingInt(Simulation.random.nextInt(4) + 1);
		}
		
		// Nick, for some reason, would like to keep the scores across resets
		// Because, he says, he is super-awesome.
		if (Soar2D.config.generalConfig().tosca == false) {
			if (playerConfig.hasPoints()) {
				this.points = playerConfig.points;
			} else {
				this.points = Soar2D.config.generalConfig().default_points;
			}
		}

		pointsChanged = true;
		pointsDelta = 0;
		
		collisionX = false;
		collisionY = false;
		rotationSpeed = 0;
		destinationHeading = null;
		velocity = new double [] { 0, 0 };
		speed = 0;
		carriedObject = null;
	}
	/**
	 * called when things are shutting down
	 */
	public void shutdown() {
		
	}
	/**
	 * @return the move
	 * 
	 * called to get the current move, this is basically the output read
	 */
	public MoveInfo getMove() {
		return new MoveInfo();
	}

	public void setShields(boolean setting) {
	}

	public boolean getRadarSwitch() {
		return false;
	}
	
	public void setRadarSwitch(boolean setting) {
	}
	
	public int getRadarPower() {
		return 0;
	}
	
	public void setRadarPower(int setting) {
	}

	public void radarTouch(int i) {
		
	}

	public int getObservedDistance() {
		return 0;
	}

	public RadarCell[][] getRadar() {
		return null;
	}
	
	public int getRWaves() {
		return 0;
	}

	public int getBlocked() {
		return 0;
	}
	
	public int hashCode() {
		return name.hashCode();
	}

	public boolean equals(Object other) {
		Player player = null;
		try {
			player = (Player)other;
		} catch (ClassCastException c) {
			return false;
		}
		return name.equals(player.name);
	}

	public void setIncoming(int i) {
	}

	public int getIncoming() {
		return 0;	}

	public void resetSensors() {
		
	}

	public boolean getHumanMove() {
		return false;
	}

	public void setSmell(int distance, String color) {
		
	}

	public int getSmellDistance() {
		return 0;
	}

	public String getSmellColor() {
		return null;
	}

	public void setSound(int soundNear) {
		
	}
	
	public int getSound() {
		return 0;
	}
	
	public void fragged() {
		
	}

	public void setOnHealthCharger(boolean b) {
		
	}

	public void setOnEnergyCharger(boolean b) {
		
	}

	public boolean pointsChanged() {
		return pointsChanged;
	}
	
	public int getPointsDelta() {
		return pointsDelta;
	}
	
	public void resetPointsChanged() {
		pointsChanged = false;
		pointsDelta = 0;
	}
	
	public void playersChanged() {
	}

	public boolean getOnHealthCharger() {
		return false;
	}

	public boolean getOnEnergyCharger() {
		return false;
	}
	
	public boolean getResurrect() {
		return false;
	}
	
	public void mapReset() {
		// this is for tosca's reward system
	}
	
	private double [] velocity = new double [] { 0, 0 };
	public void setVelocity(double [] velocity) {
		assert velocity != null;
		this.velocity = velocity;
	}
	public double [] getVelocity() {
		return this.velocity;
	}

	protected int locationId = -1;

	public int getLocationId() {
		return locationId;
	}
	
	public void setLocationId(int id) {
		locationId = id;
	}

	private double speed = 0;
	public void setSpeed(double speed) {
		this.speed = speed;
	}
	public double getSpeed() {
		return speed;
	}
	
	private Double destinationHeading = null;
	public boolean hasDestinationHeading() {
		return destinationHeading != null;
	}
	public double getDestinationHeading() {
		assert destinationHeading != null;
		return destinationHeading;
	}
	public void setDestinationHeading(double heading) {
		this.destinationHeading = heading;
	}
	public void resetDestinationHeading() {
		this.destinationHeading = null;
	}
	
	private double rotationSpeed = 0;
	public void setRotationSpeed(double rotationSpeed) {
		this.rotationSpeed = rotationSpeed;
	}
	public double getRotationSpeed() {
		return rotationSpeed;
	}
	
	protected boolean collisionX = false;
	protected boolean collisionY = false;
	public void setCollisionX(boolean x) {
		this.collisionX = x;
	}
	public void setCollisionY(boolean y) {
		this.collisionY = y;
	}
	public boolean getCollisionX() {
		return this.collisionX;
	}
	public boolean getCollisionY() {
		return this.collisionY;
	}
	
	public void rotateComplete() {
		
	}
	public void updateGetStatus(boolean success) {
		
	}
	public void updateDropStatus(boolean success) {
		
	}
	
	CellObject carriedObject = null;
	public void carry(CellObject object) {
		assert carriedObject == null;
		carriedObject = object;
	}
	public CellObject drop() {
		assert carriedObject != null;
		CellObject temp = carriedObject;
		carriedObject = null;
		return temp;
	}
	public boolean isCarrying() {
		return carriedObject != null;
	}
	public String getCarryType() {
		if (carriedObject == null) {
			return "none";
		}
		return carriedObject.getProperty("id");
	}
	public int getCarryId() {
		if (carriedObject == null) {
			return -1;
		}
		return carriedObject.getIntProperty("number");
	}
	public void receiveMessage(Player player, String message) {
		assert false;		
	}
	
	public String toString() {
		return getName();
	}
	public String getID() {
		return playerID;
	}
}
