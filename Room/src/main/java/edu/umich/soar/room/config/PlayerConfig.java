package edu.umich.soar.room.config;

import edu.umich.soar.room.core.PlayerColor;

public class PlayerConfig {
	public String name = null;
	public String productions = null;
	public String script = null;
	public PlayerColor color = null;
	public int [] pos = null;
	public String facing = null;
	public int points = -1;			// keep in synch with SimConfig.Keys.points
	public int energy = -1;
	public int health = -1;
	public int missiles = -1;
	public String [] shutdown_commands = null;
	
	boolean hasPoints = false; // has a special bool because initial points could be negative
	public boolean hasPoints() {
		return hasPoints;
	};
}
