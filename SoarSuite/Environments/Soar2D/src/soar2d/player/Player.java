package soar2d.player;

import java.util.logging.*;

import soar2d.*;

public class Player {
	protected Logger logger = Soar2D.logger;

	private String name;
	private int facingInt;
	private int points;
	private String color;
	protected java.awt.Point previousLocation = new java.awt.Point(-1, -1);
	protected boolean moved = false;

	public Player(PlayerConfig playerConfig) {
		this.name = playerConfig.getName();
		if (playerConfig.hasFacing()) {
			this.facingInt = playerConfig.getFacing();
		} else {
			this.facingInt = Simulation.random.nextInt(4) + 1;
		}
		
		if (playerConfig.hasPoints()) {
			this.points = playerConfig.getPoints();
		} else {
			this.points = Soar2D.config.kDefaultPoints;
		}
		
		assert playerConfig.hasColor();
		this.color = playerConfig.getColor();
	}
	
	public String getName() {
		return this.name;
	}
	public void setName(String name) {
		String previous = this.name;
		this.name = name;
		logger.info(previous + " name changed to: " + this.name);
	}
	
	public int getPoints() {
		return points;
	}
	public void setPoints(int points, String comment) {
		this.points = points;
		if (comment != null) {
			logger.info(this.name + " score set to: " + Integer.toString(this.points) + " (" + comment + ")");
		} else {
			logger.info(this.name + " score set to: " + Integer.toString(this.points));
		}
	}
	public void adjustPoints(int delta, String comment) {
		int previous = this.points;
		this.points += delta;
		if (comment != null) {
			logger.info(this.name + " score: " + Integer.toString(previous) + " -> " + Integer.toString(this.points) + " (" + comment + ")");
		} else {
			logger.info(this.name + " score: " + Integer.toString(previous) + " -> " + Integer.toString(this.points));
		}
	}

	public int getFacingInt() {
		return facingInt;
	}
	public void setFacingInt(int facingInt) {
		this.facingInt = facingInt;
	}

	public String getColor() {
		return this.color;
	}
	public void setColor(String color) {
		String previous = this.color;
		this.color = color;
		logger.info(this.name + " (" + previous + ") color changed to: " + this.name);
	}
	
	public void update(World world, java.awt.Point location) {
		moved = (location.x != this.previousLocation.x) || (location.y != this.previousLocation.y);
	}
	
	public boolean moved() {
		return moved;
	}
	
	public void reset() {
		
	}
	public void shutdown() {
		
	}
	public MoveInfo getMove() {
		return new MoveInfo();
	}
}
