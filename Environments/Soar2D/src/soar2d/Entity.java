package soar2d;

import java.util.logging.*;

public class Entity {
	Logger logger = Soar2D.logger;

	private String name;
	private int facingInt;
	private int points;
	private String color;

	public Entity(String name, int facingInt, int points, String color) {
		this.name = name;
		this.facingInt = facingInt;
		this.points = points;
		this.color = color;
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
		
	}
	
	public void reset() {
		
	}
	public void shutdown() {
		
	}
	
	public MoveInfo getMove() {
		return null;
	}
}
