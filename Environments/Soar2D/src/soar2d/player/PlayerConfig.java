package soar2d.player;

import java.io.*;

public class PlayerConfig {
	private String name;
	private File productions;
	private String color;
	private java.awt.Point initialLocation;
	private int facing = 0;

	private boolean hasPoints = false;
	private int points;
	
	private int energy = -1;
	private int health = -1;
	private int missiles = -1;

	public PlayerConfig() {
		
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
}

