package soar2d.player;

import java.io.*;
import java.util.ArrayList;

/**
 * @author voigtjr
 *
 * Holds the information necessary to create a player
 */
public class PlayerConfig {
	/**
	 * Name of the player as reported in the log
	 */
	private String name;
	/**
	 * Productions, null indicates human player
	 */
	private File productions;
	/**
	 * Color of player, see Simulation.kColors for legal colors
	 */
	private String color;
	/**
	 * Initial location of player, use null for no initial location
	 * Player should return to this point on reset
	 */
	private java.awt.Point initialLocation;
	/**
	 * Direction integer denoting where player is facing, see Direction.java
	 */
	private int facing = 0;

	/**
	 * If true, set the players points to the points member on reset
	 * Boolean necessary since points can be zero or negative
	 */
	private boolean hasPoints = false;
	/**
	 * How many points to set the player to when reset
	 */
	private int points;
	
	/**
	 * Initial energy to set the player to on reset. use negative to indicate
	 * default.
	 */
	private int energy = -1;
	/**
	 * Initial health to set the player to on reset. use negative to indicate
	 * default.
	 */
	private int health = -1;
	/**
	 * Initial missiles to set the player to on reset. use negative to indicate
	 * default.
	 */
	private int missiles = -1;
	
	/**
	 * Commands to run during shutdown before the agent is destroyed.
	 * Great place to do things like save the rete, etc.
	 */
	private ArrayList<String> shutdownCommands;

	public PlayerConfig() {
		
	}

	public PlayerConfig(PlayerConfig config) {
		if (config.color != null) {
			this.color = new String(config.color);
		}
		this.energy = config.energy;
		this.facing = config.facing;
		this.hasPoints = config.hasPoints;
		this.health = config.health;
		if (config.initialLocation != null) {
			this.initialLocation = new java.awt.Point(config.initialLocation);
		}
		this.missiles = config.missiles;
		if (config.name != null) {
			this.name = new String(config.name);
		}
		this.points = config.points;
		if (config.productions != null) {
			this.productions = config.productions.getAbsoluteFile();
		}
		if (config.shutdownCommands != null) {
			this.shutdownCommands = new ArrayList<String>(config.shutdownCommands);
		}
	}

	public boolean hasName() {
		return name != null;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public boolean hasProductions() {
		return productions != null;
	}

	public File getProductions() {
		return productions;
	}

	public void setProductions(File productions) {
		this.productions = productions;
	}

	public boolean hasColor() {
		return color != null;
	}

	public String getColor() {
		return color;
	}

	public void setColor(String color) {
		this.color = color;
	}

	public boolean hasInitialLocation() {
		return initialLocation != null;
	}

	public java.awt.Point getInitialLocation() {
		return initialLocation;
	}

	public void setInitialLocation(java.awt.Point initialLocation) {
		this.initialLocation = initialLocation;
	}

	public boolean hasFacing() {
		return facing > 0;
	}

	public int getFacing() {
		return facing;
	}

	public void setFacing(int facing) {
		if ((facing < 0) || (facing > 4)) {
			facing = 0;
		}
		this.facing = facing;
	}

	public boolean hasPoints() {
		return hasPoints;
	}

	public int getPoints() {
		return points;
	}

	public void setPoints(int points) {
		this.hasPoints = true;
		this.points = points;
	}

	public void setPoints(boolean hasPoints) {
		this.hasPoints = hasPoints;
	}

	public boolean hasEnergy() {
		return energy >= 0;
	}

	public int getEnergy() {
		return energy;
	}

	public void setEnergy(int energy) {
		this.energy = energy;
	}

	public boolean hasHealth() {
		return health >= 0;
	}

	public int getHealth() {
		return health;
	}

	public void setHealth(int health) {
		this.health = health;
	}

	public boolean hasMissiles() {
		return missiles >= 0;
	}

	public int getMissiles() {
		return missiles;
	}

	public void setMissiles(int missiles) {
		this.missiles = missiles;
	}
	
	public void addShutdownCommand(String command) {
		if (shutdownCommands == null) {
			shutdownCommands = new ArrayList<String>();
		}
		shutdownCommands.add(command);
	}
	
	public ArrayList<String> getShutdownCommands() {
		return shutdownCommands;
	}
}

