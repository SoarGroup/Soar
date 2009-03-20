package org.msoar.gridmap2d.players;

import org.apache.log4j.Logger;
import org.msoar.gridmap2d.Direction;
import org.msoar.gridmap2d.Simulation;
import org.msoar.gridmap2d.Gridmap2D;
import org.msoar.gridmap2d.config.PlayerConfig;


public class Player {
	private static Logger logger = Logger.getLogger(Player.class);
	
	private String playerID;
	private PlayerConfig playerConfig;
	private String name;	// player name
	private int points;	// current point count
	private boolean pointsChanged;
	private int pointsDelta;
	private Direction facing;	// what direction I'm currently facing
	private String color;	// valid color string
	private int[] location;
	private boolean moved;
	private boolean fragged;

	public Player(String playerID) throws Exception {
		this.playerID = playerID;
		this.playerConfig = Gridmap2D.config.playerConfigs().get(playerID);
		
		assert playerConfig.name != null;
		this.name = playerConfig.name;
		
		assert playerConfig.color != null;
		this.color = playerConfig.color;

		this.reset();
	}
	
	public void reset() throws Exception {
		location = new int[] { -1, -1 };
		
		if (playerConfig.facing != null) {
			this.setFacing(Direction.parse(playerConfig.facing));
		} else {
			this.setFacing(Direction.values()[Simulation.random.nextInt(4) + 1]);
		}
		
		if (playerConfig.hasPoints()) {
			this.points = playerConfig.points;
		} else {
			this.points = Gridmap2D.config.generalConfig().default_points;
		}

		pointsChanged = true;
		pointsDelta = 0;
		fragged = false;
	}

	public String getName() {
		return this.name;
	}
	public void setName(String name) {
		String previous = this.name;
		this.name = name;
		logger.warn(previous + " name changed to: " + this.name);
	}
	
	public Direction getFacing() {
		return facing;
	}
	
	public void setFacing(Direction facing) {
		this.facing = facing;
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
		
	public int getPoints() {
		return points;
	}

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
	
	public String getColor() {
		return this.color;
	}

	public void setColor(String color) {
		String previous = this.color;
		this.color = color;
		logger.warn(this.name + " (" + previous + ") color changed to: " + this.name);
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

	public String toString() {
		return getName();
	}

	public String getID() {
		return playerID;
	}

	protected void update(int[] newLocation) {
		moved = (newLocation[0] != this.location[0]) || (newLocation[1] != this.location[1]);
		if (moved) {
			this.location = org.msoar.gridmap2d.Arrays.copyOf(newLocation, newLocation.length);
		}
	}
	
	public int[] getLocation() {
		return org.msoar.gridmap2d.Arrays.copyOf(location, location.length);
	}
	
	public boolean getMoved() {
		return moved;
	}
	
	public void setFragged(boolean fragged) {
		this.fragged = fragged;
		this.moved = true;
	}
	
	public boolean getFragged() {
		return fragged;
	}
}
