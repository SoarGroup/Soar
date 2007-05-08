package soar2d.player;

import java.awt.geom.Point2D;
import java.awt.geom.Point2D.Float;
import java.util.logging.*;

import soar2d.*;

/**
 * @author voigtjr
 *
 * This class represents a generic player object in the world. This
 * can be a Tank, Eater, whatever.
 */
public class Player {
	protected Logger logger = Soar2D.logger;	// handy java logger

	private String name;	// player name
	private int facingInt;	// what direction I'm currently facing
	private int points;	// current point count
	private String color;	// valid color string
	protected java.awt.Point previousLocation = new java.awt.Point(-1, -1);	// where i was last update
	protected boolean moved = false;	// if I moved since last update
	protected PlayerConfig playerConfig;

	private boolean pointsChanged = false;
	private int pointsDelta = 0;

	private float headingRadians = 0;	// heading in radians
	public float getHeadingRadians() {
		return headingRadians;
	}
	public void setHeadingRadians(float heading) {
		this.headingRadians = heading;
	}
	
	/**
	 * @param playerConfig configuration params
	 */
	public Player(PlayerConfig playerConfig) {
		this.playerConfig = playerConfig;
		
		this.name = playerConfig.getName();
		
		this.reset();
		
		assert playerConfig.hasColor();
		this.color = playerConfig.getColor();
	}
	
	public String getName() {
		return this.name;
	}
	public void setName(String name) {
		String previous = this.name;
		this.name = name;
		logger.warning(previous + " name changed to: " + this.name);
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
		pointsDelta = this.points - points;
		
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
		logger.warning(this.name + " (" + previous + ") color changed to: " + this.name);
	}
	
	/**
	 * @param world world object so stuff can be looked up
	 * @param location current location since I don't keep track of where I end up
	 * 
	 * called to update the player's sensors and what not, this is basically an input update
	 */
	public void update(World world, java.awt.Point location) {
		moved = (location.x != this.previousLocation.x) || (location.y != this.previousLocation.y);
		if (moved) {
			this.previousLocation = new java.awt.Point(location);
		}
	}
	
	/**
	 * called to write sensor data to the input link
	 */
	public void commit(java.awt.Point location) {
	}
	
	/**
	 * called to reset player state between runs
	 */
	public void reset() {
		previousLocation = new java.awt.Point(-1, -1);
		
		if (playerConfig.hasFacing()) {
			this.facingInt = playerConfig.getFacing();
		} else {
			this.facingInt = Simulation.random.nextInt(4) + 1;
		}
		
		if (playerConfig.hasPoints()) {
			this.points = playerConfig.getPoints();
		} else {
			this.points = Soar2D.config.getDefaultPoints();
		}
		pointsChanged = true;
		pointsDelta = 0;
		headingRadians = 0;
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
		// TODO Auto-generated method stub
		
	}

	public int getObservedDistance() {
		// TODO Auto-generated method stub
		return 0;
	}

	public RadarCell[][] getRadar() {
		// TODO Auto-generated method stub
		return null;
	}
	
	public int getRWaves() {
		return 0;
	}

	public int getBlocked() {
		// TODO Auto-generated method stub
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

		// TODO Auto-generated method stub
	public void setIncoming(int i) {
	}

	public int getIncoming() {
		return 0;	}

	public void resetSensors() {
		// TODO Auto-generated method stub
		
	}

	public boolean getHumanMove() {
		// TODO Auto-generated method stub
		return false;
	}

	public void setSmell(int distance, String color) {
		// TODO Auto-generated method stub
		
	}

	public int getSmellDistance() {
		// TODO Auto-generated method stub
		return 0;
	}

	public String getSmellColor() {
		// TODO Auto-generated method stub
		return null;
	}

	public void setSound(int soundNear) {
		// TODO Auto-generated method stub
		
	}
	
	public int getSound() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	public void fragged() {
		
	}

	public void setOnHealthCharger(boolean b) {
		// TODO Auto-generated method stub
		
	}

	public void setOnEnergyCharger(boolean b) {
		// TODO Auto-generated method stub
		
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
	
	private Point2D.Float velocity = new Point2D.Float(0,0);
	public void setVelocity(Point2D.Float velocity) {
		assert velocity != null;
		this.velocity = velocity;
	}
	public Point2D.Float getVelocity() {
		return this.velocity;
	}
}
