package tanksoar;

import java.util.logging.*;

import utilities.*;

public class Missile {
	private static Logger logger = Logger.getLogger("simulation");
	
	private java.awt.Point location;
	private int direction;
	private Tank owner;

	private static int nextID = 0;
	private int id;
	private int flightPhase; // 0, 1 == affects current location, 2 == affects current location + 1
	
	public Missile(java.awt.Point location, int direction, Tank owner) {
		this.location = location;
		this.direction = direction;
		this.owner = owner;

		id = nextID++;
		flightPhase = 0;
	}
	
	public Tank getOwner() {
		return owner;
	}
	
	public int getID() {
		return id;
	}
	
	void move() {
		if (flightPhase == 2) {
			location = Direction.translate(location, direction);
		}
		location = Direction.translate(location, direction);
		
		++flightPhase;
		flightPhase %= 3;
	}
	
	public java.awt.Point getLocation() {
		return location;
	}
	
	java.awt.Point[] getThreatenedLocations() {
		// FIXME: this is slow and cumbersome
		java.awt.Point[] threats;
		if (flightPhase == 2) {
			threats = new java.awt.Point[2];
			threats[0] = new java.awt.Point(location);
			threats[1] = new java.awt.Point(location);
			threats[1] = Direction.translate(threats[1], direction);
		} else {
			threats = new java.awt.Point[1];
			threats[0] = new java.awt.Point(location);
		}
		return threats;
	}
	
	public int getDirection() {
		return direction;
	}
	
	public int getFlightPhase() {
		return flightPhase;
	}
}

